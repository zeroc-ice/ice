// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Freeze/ObjectStore.h>
#include <Freeze/EvictorI.h>
#include <Freeze/Util.h>
#include <Freeze/IndexI.h>
#include <Freeze/Catalog.h>
#include <Freeze/TransactionI.h>

using namespace std;
using namespace Ice;
using namespace Freeze;


Freeze::ObjectStore::ObjectStore(const string& facet, 
				 bool createDb,  EvictorI* evictor,  
				 const vector<IndexPtr>& indices,
				 bool populateEmptyIndices) :
    _facet(facet),
    _evictor(evictor),
    _indices(indices),
    _communicator(evictor->communicator())
{
    if(facet == "")
    {
	_dbName = EvictorI::defaultDb;
    }
    else
    {
	_dbName = facet;
    }

    ConnectionPtr catalogConnection = createConnection(_communicator, evictor->dbEnv()->getEnvName());
    Catalog catalog(catalogConnection, catalogName());
    
    Catalog::iterator p = catalog.find(evictor->filename());
    if(p != catalog.end())
    {
	if(p->second.evictor == false)
	{
	    DatabaseException ex(__FILE__, __LINE__);
	    ex.message = evictor->filename() + " is an evictor database";
	    throw ex;
	}
    }

    DbEnv* dbEnv = evictor->dbEnv()->getEnv();

    try
    {
	_db.reset(new Db(dbEnv, 0));

	TransactionPtr tx = catalogConnection->beginTransaction();
	DbTxn* txn = getTxn(tx);

	u_int32_t flags = DB_THREAD;
	if(createDb)
	{
	    flags |= DB_CREATE;
	}
	_db->open(txn, evictor->filename().c_str(), _dbName.c_str(), DB_BTREE, flags, FREEZE_DB_MODE);

	for(size_t i = 0; i < _indices.size(); ++i)
	{
	    _indices[i]->_impl->associate(this, txn, createDb, populateEmptyIndices);
	}
	
	if(p == catalog.end())
	{
	    CatalogData catalogData;
	    catalogData.evictor = true;
	    catalog.put(Catalog::value_type(evictor->filename(), catalogData));
	}

	tx->commit();
    }
    catch(const DbException& dx)
    {
	TransactionPtr tx = catalogConnection->currentTransaction();
	if(tx != 0)
	{
	    try
	    {
		tx->rollback();
	    }
	    catch(...)
	    {
	    }
	}

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
    }
    catch(...)
    {
	TransactionPtr tx = catalogConnection->currentTransaction();
	if(tx != 0)
	{
	    try
	    {
		tx->rollback();
	    }
	    catch(...)
	    {
	    }
	}
	throw;
    }
}

Freeze::ObjectStore::~ObjectStore()
{
    if(_db.get() != 0)
    {
	close();
    }
}

void
Freeze::ObjectStore::close()
{
    try
    {
	_db->close(0);
	
	for(size_t i = 0; i < _indices.size(); ++i)
	{
	    _indices[i]->_impl->close();
	}
	_indices.clear();
    }
    catch(const DbException& dx)
    {
	DatabaseException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
    _db.reset();
}
    
bool
Freeze::ObjectStore::dbHasObject(const Identity& ident) const
{
    Key key;    
    marshal(ident, key, _communicator);
    Dbt dbKey;
    initializeInDbt(key, dbKey);
    
    //
    // Keep 0 length since we're not interested in the data
    //
    Dbt dbValue;
    dbValue.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);

    for(;;)
    {
	try
	{
	    int err = _db->get(0, &dbKey, &dbValue, 0);
	    
	    if(err == 0)
	    {
		return true;
	    }
	    else if(err == DB_NOTFOUND)
	    {
		return false;
	    }
	    else
	    {
		assert(0);
		throw DatabaseException(__FILE__, __LINE__);
	    }
	}
	catch(const DbDeadlockException&)
	{
	    if(_evictor->deadlockWarning())
	    {
		Warning out(_communicator->getLogger());
		out << "Deadlock in Freeze::ObjectStore::dbHasObject while searching \"" 
		    << _evictor->filename() + "/" + _dbName << "\"; retrying ...";
	    }

	    //
	    // Ignored, try again
	    //
	}
	catch(const DbException& dx)
	{
	    DatabaseException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
    }

}

void
Freeze::ObjectStore::save(Key& key, Value& value, Byte status, DbTxn* tx)
{
    switch(status)
    {
	case EvictorElement::created:
	case EvictorElement::modified:
	{
	    Dbt dbKey;
	    Dbt dbValue;
	    initializeInDbt(key, dbKey);
	    initializeInDbt(value, dbValue);
	    u_int32_t flags = (status == EvictorElement::created) ? DB_NOOVERWRITE : 0;
	    int err = _db->put(tx, &dbKey, &dbValue, flags);
	    if(err != 0)
	    {
		throw DatabaseException(__FILE__, __LINE__);
	    }
	    break;
	}

	case EvictorElement::destroyed:
	{
	    Dbt dbKey;
	    initializeInDbt(key, dbKey);
	    int err = _db->del(tx, &dbKey, 0);
	    if(err != 0)
	    {
		throw DatabaseException(__FILE__, __LINE__);
	    }
	    break;
	}   
	default:
	{
	    assert(0);
	}
    }
}

void 
Freeze::ObjectStore::marshal(const Identity& ident, Key& bytes, const CommunicatorPtr& communicator)
{
    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
    IceInternal::BasicStream stream(instance.get());
    ident.__write(&stream);
    vector<Byte>(stream.b.begin(), stream.b.end()).swap(bytes);
}
    
void 
Freeze::ObjectStore::unmarshal(Identity& ident, const Key& bytes, const CommunicatorPtr& communicator)
{
    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
    IceInternal::BasicStream stream(instance.get());
    stream.b.resize(bytes.size());
    memcpy(&stream.b[0], &bytes[0], bytes.size());
    stream.i = stream.b.begin();
    ident.__read(&stream);
}

void
Freeze::ObjectStore::marshal(const ObjectRecord& v, Value& bytes, const CommunicatorPtr& communicator)
{
    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
    IceInternal::BasicStream stream(instance.get());
    stream.startWriteEncaps();
    v.__write(&stream);
    stream.writePendingObjects();
    stream.endWriteEncaps();
    vector<Byte>(stream.b.begin(), stream.b.end()).swap(bytes);
}

void
Freeze::ObjectStore::unmarshal(ObjectRecord& v, const Value& bytes, const CommunicatorPtr& communicator)
{
    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
    IceInternal::BasicStream stream(instance.get());
    stream.sliceObjects(false);
    stream.b.resize(bytes.size());
    memcpy(&stream.b[0], &bytes[0], bytes.size());
    stream.i = stream.b.begin();
    stream.startReadEncaps();
    v.__read(&stream);
    stream.readPendingObjects();
    stream.endReadEncaps();
}

const string&
Freeze::ObjectStore::dbName() const
{
    return _dbName;
}


Freeze::EvictorElementPtr
Freeze::ObjectStore::load(const Identity& ident)
{
    Key key;
    marshal(ident, key, _communicator);

    Dbt dbKey;
    initializeInDbt(key, dbKey);

    const size_t defaultValueSize = 4096;
    Value value(defaultValueSize);

    Dbt dbValue;
    initializeOutDbt(value, dbValue);

    for(;;)
    {
	try
	{
	    int rs = _db->get(0, &dbKey, &dbValue, 0);
	    if(rs == DB_NOTFOUND)
	    {
		return 0;
	    }
	    else if(rs != 0)
	    {
		assert(0);
		throw DatabaseException(__FILE__, __LINE__);
	    }
	    break; // for(;;)
	}
	catch(const DbDeadlockException&)
	{
	    if(_evictor->deadlockWarning())
	    {
		Warning out(_communicator->getLogger());
		out << "Deadlock in Freeze::ObjectStore::load while searching \"" 
		    << _evictor->filename() + "/" + _dbName << "\"; retrying ...";
	    }
	    //
	    // Ignored, try again
	    //
	}
	catch(const DbException& dx)
	{
	    handleDbException(dx, value, dbValue, __FILE__, __LINE__);
	}
    }
    
    EvictorElementPtr result = new EvictorElement(*this);
    unmarshal(result->rec, value, _communicator);

    _evictor->initialize(ident, _facet, result->rec.servant);
    return result;
}

void
Freeze::ObjectStore::pinned(const EvictorElementPtr& element, Cache::Position p)
{
    element->cachePosition = p;
    element->stale = false;
}



Freeze::EvictorElement::EvictorElement(ObjectStore& s) :
    store(s),
    usageCount(-1),
    keepCount(0),
    stale(true),
    status(clean)
{
}

Freeze::EvictorElement::~EvictorElement()
{
}



