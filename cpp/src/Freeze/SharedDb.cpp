// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Freeze/SharedDb.h>
#include <IceUtil/StaticMutex.h>
#include <Freeze/Exception.h>
#include <Freeze/Util.h>

using namespace std;
using namespace IceUtil;
using namespace Ice;

namespace
{

StaticMutex _mapMutex = ICE_STATIC_MUTEX_INITIALIZER;
StaticMutex _refCountMutex = ICE_STATIC_MUTEX_INITIALIZER;  

}

Freeze::SharedDb::SharedDbMap* Freeze::SharedDb::sharedDbMap = 0;

Freeze::SharedDbPtr 
Freeze::SharedDb::get(const ConnectionIPtr& connection, 
		      const string& dbName,
		      const vector<MapIndexBasePtr>& indices,
		      bool createDb)
{
    StaticMutex::Lock lock(_mapMutex);

    if(sharedDbMap == 0)
    {
	sharedDbMap = new SharedDbMap;
    }

    MapKey key;
    key.envName = connection->envName();
    key.communicator = connection->communicator();
    key.dbName = dbName;

    {
	SharedDbMap::iterator p = sharedDbMap->find(key);
	if(p != sharedDbMap->end())
	{
	    p->second->connectIndices(indices);
	    return p->second;
	}
    }

    //
    // MapKey not found, let's create and open a new Db
    //
    auto_ptr<SharedDb> result(new SharedDb(key, connection, indices, createDb));
    
    //
    // Insert it into the map
    //
    pair<SharedDbMap::iterator, bool> insertResult;
    insertResult= sharedDbMap->insert(SharedDbMap::value_type(key, result.get()));
    assert(insertResult.second);
    
    return result.release();
}

Freeze::SharedDb::~SharedDb()
{
    if(_trace >= 1)
    {
	Trace out(_key.communicator->getLogger(), "Freeze.Map");
	out << "closing Db \"" << _key.dbName << "\"";
    }

    cleanup(false);
}

void Freeze::SharedDb::__incRef()
{
    if(_trace >= 2)
    {
	Trace out(_key.communicator->getLogger(), "Freeze.Map");
	out << "incremeting reference count for Db \"" << _key.dbName << "\"";
    }

    IceUtil::StaticMutex::Lock lock(_refCountMutex);
    _refCount++;
}

void Freeze::SharedDb::__decRef()
{
    if(_trace >= 2)
    {
	Trace out(_key.communicator->getLogger(), "Freeze.Map");
	out << "removing reference count for Db \"" << _key.dbName << "\"";
    }

    IceUtil::StaticMutex::Lock lock(_refCountMutex);
    if(--_refCount == 0)
    {
        IceUtil::StaticMutex::TryLock mapLock(_mapMutex);
	if(!mapLock.acquired())
	{
	    //
	    // Reacquire mutex in proper order and check again
	    //
	    lock.release();
	    mapLock.acquire();
	    lock.acquire();
	    if(_refCount > 0)
	    {
		return;
	    }
	}

	//
	// Remove from map
	//
	size_t one;
	one = sharedDbMap->erase(_key);
	assert(one == 1);

	if(sharedDbMap->size() == 0)
	{
	    delete sharedDbMap;
	    sharedDbMap = 0;
	}

	//
	// Keep lock to prevent somebody else to re-open this Db
	// before it's closed.
	//
	delete this;
    }
}

  
Freeze::SharedDb::SharedDb(const MapKey& key, 
			   const ConnectionIPtr& connection, 
			   const vector<MapIndexBasePtr>& indices,
			   bool createDb) :
    Db(connection->dbEnv(), 0),
    _key(key),
    _refCount(0),
    _trace(connection->trace())
{
    if(_trace >= 1)
    {
	Trace out(_key.communicator->getLogger(), "Freeze.Map");
	out << "opening Db \"" << _key.dbName << "\"";
    }

    DbTxn* txn = 0;
    DbEnv* dbEnv = connection->dbEnv();
    
    try
    {
	dbEnv->txn_begin(0, &txn, 0);

	u_int32_t flags = DB_THREAD;
	if(createDb)
	{
	    flags |= DB_CREATE;
	}
	open(txn, key.dbName.c_str(), 0, DB_BTREE, flags, FREEZE_DB_MODE);

	for(vector<MapIndexBasePtr>::const_iterator p = indices.begin();
	    p != indices.end(); ++p)
	{
	    const MapIndexBasePtr& indexBase = *p; 
		
	    if(indexBase->_impl != 0)
	    {
		DatabaseException ex(__FILE__, __LINE__);
		ex.message = "Index \"" + indexBase->name() + "\" already initialized!";
		throw ex;
	    }

	    auto_ptr<MapIndexI> indexI(new MapIndexI(connection, *this, txn, createDb, indexBase));
	    
	    bool inserted = _indices.insert(IndexMap::value_type(indexBase->name(), indexI.get())).second;
	    if(!inserted)
	    {
		DatabaseException ex(__FILE__, __LINE__);
		ex.message = "Index \"" + indexBase->name() + "\" listed twice!";
		throw ex;
	    }
	    
	    indexBase->_impl = indexI.release();
	}

	DbTxn* toCommit = txn;
	txn = 0;
	toCommit->commit(0);
    }
    catch(const ::DbException& dx)
    {
	if(txn != 0)
	{
	    try
	    {
		txn->abort();
	    }
	    catch(...)
	    {
	    }
	}

	cleanup(true);

	if(dx.get_errno() == ENOENT)
	{
	    NotFoundException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
	else
	{
	    DatabaseException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}

	DatabaseException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
    catch(...)
    {
	if(txn != 0)
	{
	    try
	    {
		txn->abort();
	    }
	    catch(...)
	    {
	    }
	}

	cleanup(true);
	throw;
    }
}

void
Freeze::SharedDb::connectIndices(const vector<MapIndexBasePtr>& indices) const
{
    for(vector<MapIndexBasePtr>::const_iterator p = indices.begin();
	p != indices.end(); ++p)
    {
	const MapIndexBasePtr& indexBase = *p; 

	if(indexBase->_impl != 0)
	{
	    DatabaseException ex(__FILE__, __LINE__);
	    ex.message = "Index \"" + indexBase->name() + "\" already initialized!";
	    throw ex;
	}

	IndexMap::const_iterator q = _indices.find(indexBase->name());
	
	if(q == _indices.end())
	{
	    DatabaseException ex(__FILE__, __LINE__);
	    ex.message = "\"" + _key.dbName + "\" already opened but without index \"" 
		+ indexBase->name() +"\"";
	    throw ex;
	}
	
	indexBase->_impl = q->second;
    }
}

void
Freeze::SharedDb::cleanup(bool noThrow)
{
    try
    {
	for(IndexMap::iterator p = _indices.begin(); p != _indices.end(); ++p)
	{
	    delete p->second;
	}
	_indices.clear();
	
	close(0);
    }
    catch(const ::DbException& dx)
    {
	if(!noThrow)
	{
	    DatabaseException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
    }
}
