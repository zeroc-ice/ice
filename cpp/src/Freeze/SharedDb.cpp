// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
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

Freeze::SharedDb::Map* Freeze::SharedDb::sharedDbMap = 0;

Freeze::SharedDbPtr 
Freeze::SharedDb::get(const ConnectionIPtr& connection, 
		      const string& dbName, bool createDb)
{
    StaticMutex::Lock lock(_mapMutex);

    if(sharedDbMap == 0)
    {
	sharedDbMap = new Map;
    }

    MapKey key;
    key.envName = connection->envName();
    key.communicator = connection->communicator();
    key.dbName = dbName;

    {
	Map::iterator p = sharedDbMap->find(key);
	if(p != sharedDbMap->end())
	{
	    return p->second;
	}
    }

    //
    // MapKey not found, let's create and open a new Db
    //
    auto_ptr<SharedDb> result(new SharedDb(key, connection, createDb));
    
    //
    // Insert it into the map
    //
    pair<Map::iterator, bool> insertResult;
    insertResult= sharedDbMap->insert(Map::value_type(key, result.get()));
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

    try
    {
	close(0);
    }
    catch(const ::DbException& dx)
    {
	DatabaseException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }

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

    try
    {
	u_int32_t flags = DB_AUTO_COMMIT | DB_THREAD;
	if(createDb)
	{
	    flags |= DB_CREATE;
	}
	open(0, key.dbName.c_str(), 0, DB_BTREE, flags, FREEZE_DB_MODE);
    }
    catch(const ::DbException& dx)
    {
	DatabaseException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
}
