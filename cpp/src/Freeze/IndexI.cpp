// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Freeze/IndexI.h>
#include <Freeze/Util.h>

using namespace Freeze;
using namespace Ice;
using namespace std;


static int 
callback(Db* secondary, const Dbt* key, const Dbt* value, Dbt* result)
{
    void* indexObj = secondary->get_app_private();
    IndexI* index = static_cast<IndexI*>(indexObj);
    assert(index != 0);
    return index->secondaryKeyCreate(secondary, key, value, result);
}


Freeze::IndexI::IndexI(Index& index, const string& name) :
    _index(index),
    _name(name),
    _evictor(0)
{
}
 
vector<Identity>
Freeze::IndexI::untypedFindFirst(const Key& bytes, Int firstN) const
{
    Dbt dbKey;
    initializeInDbt(bytes, dbKey);

    //
    // Berkeley DB 4.1.25 bug: it should not write into dbKey
    //
    dbKey.set_ulen(static_cast<u_int32_t>(bytes.size()));
    
    Key pkey(1024);
    Dbt pdbKey;
    initializeOutDbt(pkey, pdbKey);
    
    Dbt dbValue;
    dbValue.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);

   
    Ice::CommunicatorPtr communicator = _evictor->communicator();
    _evictor->saveNow();

    vector<Identity> identities;

    try
    {
	for(;;)
	{
	    Dbc* dbc = 0;
	    identities.clear();
	    
	    try
	    {
		//
		// Move to the first record
		// 
		_db->cursor(0, &dbc, 0);
		bool more;
		
		for(;;)
		{
		    try
		    {
			more = (dbc->pget(&dbKey, &pdbKey, &dbValue, DB_SET) == 0);
			if(more)
			{
			    pkey.resize(pdbKey.get_size());
			}
			break; // for(;;)
		    }
		    catch(const DbMemoryException& dx)
		    {
			handleMemoryException(dx, pkey, pdbKey);
		    }
		}
		    
		while((firstN <= 0 || identities.size() < static_cast<size_t>(firstN)) && more)
		{
		    EvictorStorageKey esk;
		    EvictorI::unmarshal(esk, pkey, communicator);
		    
		    if(esk.facet.size() == 0)
		    {
			identities.push_back(esk.identity);
		    }
		    //
		    // Else skip "orphan" facet (could be just a temporary inconsistency
		    // on disk)
		    //
		    for(;;)
		    {
			try
			{
			    more = (dbc->pget(&dbKey, &pdbKey, &dbValue, DB_NEXT_DUP) == 0);
			    if(more)
			    {
				pkey.resize(pdbKey.get_size());
			    }
			    break; // for(;;)
			}
			catch(const DbMemoryException& dx)
			{
			    handleMemoryException(dx, pkey, pdbKey);
			}
		    }
		}
		Dbc* toClose = dbc;
		dbc = 0;
		toClose->close();
		break; // for (;;)
	    }
	    catch(const DbDeadlockException&)
	    {
		if(dbc != 0)
		{
		    try
		    {
			dbc->close();
		    }
		    catch(const DbDeadlockException&)
		    {
			//
			// Ignored
			//
		    }
		}

		if(_evictor->deadlockWarning())
		{
		    Warning out(_evictor->communicator()->getLogger());
		    out << "Deadlock in Freeze::IndexI::untypedFindFirst while searching \"" 
			<< _evictor->dbName() << "\"; retrying ...";
		}

		//
		// Retry
		//
	    }
	    catch(...)
	    {
		if(dbc != 0)
		{
		    try
		    {
			dbc->close();
		    }
		    catch(const DbDeadlockException&)
		    {
			//
			// Ignored
			//
		    }
		}
		throw;
	    }
	}
    }
    catch(const DbException& dx)
    {
	DatabaseException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
    
    return identities;
}
    
vector<Identity>
Freeze::IndexI::untypedFind(const Key& bytes) const
{
    return untypedFindFirst(bytes, 0);
}

Int
Freeze::IndexI::untypedCount(const Key& bytes) const
{
 
    Dbt dbKey;
    initializeInDbt(bytes, dbKey);
    
    Dbt dbValue;
    dbValue.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);

    _evictor->saveNow();
    Int result = 0;
    
    try
    {
	for(;;)
	{
	    Dbc* dbc = 0;
	    
	    try
	    {
		//
		// Move to the first record
		// 
		_db->cursor(0, &dbc, 0);
		bool found = (dbc->get(&dbKey, &dbValue, DB_SET) == 0);
		
		if(found)
		{
		    db_recno_t count = 0;
		    dbc->count(&count, 0);
		    result = static_cast<Int>(count);
		}
		
		Dbc* toClose = dbc;
		dbc = 0;
		toClose->close();
		break; // for (;;)
	    }
	    catch(const DbDeadlockException&)
	    {
		if(dbc != 0)
		{
		    try
		    {
			dbc->close();
		    }
		    catch(const DbDeadlockException&)
		    {
			//
			// Ignored
			//
		    }
		}

		if(_evictor->deadlockWarning())
		{
		    Warning out(_evictor->communicator()->getLogger());
		    out << "Deadlock in Freeze::IndexI::untypedCount while searching \"" 
			<< _evictor->dbName() << "\"; retrying ...";
		}

		//
		// Retry
		//
	    }
	    catch(...)
	    {
		if(dbc != 0)
		{
		    try
		    {
			dbc->close();
		    }
		    catch(const DbDeadlockException&)
		    {
			//
			// Ignored
			//
		    }
		}
		throw;
	    }
	}
    }
    catch(const DbException& dx)
    {
	DatabaseException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
    
    return result;
}

void
Freeze::IndexI::associate(EvictorI* evictor, DbTxn* txn, 
			  bool createDb, bool populateIndex)
{
    assert(txn != 0);
    _evictor = evictor;
    _index._communicator = evictor->communicator();
    
    _db.reset(new Db(evictor->dbEnv(), 0));
    _db->set_flags(DB_DUP | DB_DUPSORT);
    _db->set_app_private(this);

    u_int32_t flags = 0;
    if(createDb)
    {
	flags = DB_CREATE;
    }
    _db->open(txn, (evictor->dbName() + "." + _name).c_str(), 0, DB_BTREE, flags, FREEZE_DB_MODE);

    flags = 0;
    if(populateIndex)
    {
	flags = DB_CREATE;
    }
    evictor->db()->associate(txn, _db.get(), callback, flags);
}

int
Freeze::IndexI::secondaryKeyCreate(Db* secondary, const Dbt* dbKey, 
				   const Dbt* dbValue, Dbt* result)
{
    Ice::CommunicatorPtr communicator = _evictor->communicator();

    EvictorStorageKey esk;
    Byte* first = static_cast<Byte*>(dbKey->get_data());
    Key key(first, first + dbKey->get_size());
    EvictorI::unmarshal(esk, key, communicator);

    if(esk.facet.size() == 0)
    {
	ObjectRecord rec;
	first = static_cast<Byte*>(dbValue->get_data());
	Value value(first, first + dbValue->get_size());
	EvictorI::unmarshal(rec, value, communicator);

	Key bytes;
	if(_index.marshalKey(rec.servant, bytes))
	{
	    result->set_flags(DB_DBT_APPMALLOC);
	    void* data = malloc(bytes.size());
	    memcpy(data, &bytes[0], bytes.size());
	    result->set_data(data);
	    result->set_size(static_cast<u_int32_t>(bytes.size()));
	    return 0;
	}
    }
    
    //
    // Don't want to index this one
    //
    return DB_DONOTINDEX;
}

void
Freeze::IndexI::close()
{
    if(_db.get() != 0)
    {
	try
	{
	    _db->close(0);
	}
	catch(const DbException& dx)
	{
	    DatabaseException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
	_db.reset(0);   
    }
}
