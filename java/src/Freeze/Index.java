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

package Freeze;

public abstract class Index implements com.sleepycat.db.DbSecondaryKeyCreate
{

    //
    // Implementation details
    //
    
    public int
    secondaryKeyCreate(com.sleepycat.db.Db secondary,
			 com.sleepycat.db.Dbt key,
			 com.sleepycat.db.Dbt value,
			 com.sleepycat.db.Dbt result)
    {
	Ice.Communicator communicator = _store.communicator();

	Ice.Identity ident = ObjectStore.unmarshalKey(key.get_data(), communicator);
	ObjectRecord rec = ObjectStore.unmarshalValue(value.get_data(), communicator);

	byte[] secondaryKey = marshalKey(rec.servant);
	if(secondaryKey != null)
	{
	    result.set_data(secondaryKey);
	    result.set_size(secondaryKey.length);
	    return 0;
	}
	else
	{
	    //
	    // Don't want to index this one
	    //
	    return com.sleepycat.db.Db.DB_DONOTINDEX;
	}
    }
    
    //
    // Alias for Berkeley DB 4.1.25
    //
    public int
    secondary_key_create(com.sleepycat.db.Db secondary,
			 com.sleepycat.db.Dbt key,
			 com.sleepycat.db.Dbt value,
			 com.sleepycat.db.Dbt result)
    {
	return secondaryKeyCreate(secondary, key, value, result);
    }

    public String
    name()
    {
	return _name;
    }

    public String
    facet()
    {
	return _facet;
    }

    protected Index(String name, String facet)
    {
	_name = name;
	_facet = facet;
    }

    protected abstract byte[]
    marshalKey(Ice.Object servant);
    
    protected Ice.Identity[]
    untypedFindFirst(byte[] k, int firstN)
    {
	EvictorI.DeactivateController deactivateController = _store.evictor().deactivateController();
	deactivateController.lock();
	
	try
	{
	    com.sleepycat.db.Dbt key = new com.sleepycat.db.Dbt(k);
	    
	    com.sleepycat.db.Dbt pkey = new com.sleepycat.db.Dbt();
	    pkey.set_flags(com.sleepycat.db.Db.DB_DBT_MALLOC);
	    
	    com.sleepycat.db.Dbt value = new com.sleepycat.db.Dbt();
	    //
	    // dlen is 0, so we should not retrieve any value 
	    // 
	    value.set_flags(com.sleepycat.db.Db.DB_DBT_PARTIAL);
	    
	    Ice.Communicator communicator = _store.communicator();
	    _store.evictor().saveNow();
	    
	    java.util.List identities;
	    
	    try
	    {
		for(;;)
		{
		    com.sleepycat.db.Dbc dbc = null;
		    identities = new java.util.ArrayList(); 
		    
		    try
		    {
			//
			// Move to the first record
			// 
			dbc = _db.cursor(null, 0);
			int flags = com.sleepycat.db.Db.DB_SET;
			
			boolean found;
			
			do
			{
			    found = (dbc.pget(key, pkey, value, flags) == 0);
			    
			    if(found)
			    {
				Ice.Identity ident = ObjectStore.unmarshalKey(pkey.get_data(), communicator);
				identities.add(ident);
				flags = com.sleepycat.db.Db.DB_NEXT_DUP;
			    }
			}
			while((firstN <= 0 || identities.size() < firstN) && found);
			
			break; // for(;;)
		    }
		    catch(com.sleepycat.db.DbDeadlockException dx)
		    {
			if(_store.evictor().deadlockWarning())
			{
			    communicator.getLogger().warning
				("Deadlock in Freeze.Index.untypedFindFirst while iterating over Db \"" 
				 + _store.evictor().filename() + "/" + _dbName
				 + "\"; retrying ...");
			}
			
			//
			// Retry
			//
		    }
		    finally
		    {
			if(dbc != null)
			{
			    try
			    {
				dbc.close();
			    }
			    catch(com.sleepycat.db.DbDeadlockException dx)
			    {
				//
				// Ignored
				//
			    }
			}
		    }
		}
	    }
	    catch(com.sleepycat.db.DbException dx)
	    {
		DatabaseException ex = new DatabaseException();
		ex.initCause(dx);
		ex.message = _store.evictor().errorPrefix() + "Db.cursor: " + dx.getMessage();
		throw ex;
	    }
	    
	    if(identities.size() != 0)
	    {
		Ice.Identity[] result = new Ice.Identity[identities.size()];
		return (Ice.Identity[]) identities.toArray(result);
	    }
	    else
	    {
		return new Ice.Identity[0];
	    }
	}
	finally
	{
	    deactivateController.unlock();
	}
    }
	
    protected Ice.Identity[]
    untypedFind(byte[] key)
    {
	return untypedFindFirst(key, 0);
    }

    protected int
    untypedCount(byte[] k)
    {
	EvictorI.DeactivateController deactivateController = _store.evictor().deactivateController();
	deactivateController.lock();
	
	try
	{
	    com.sleepycat.db.Dbt key = new com.sleepycat.db.Dbt(k);
	    com.sleepycat.db.Dbt value = new com.sleepycat.db.Dbt();
	    //
	    // dlen is 0, so we should not retrieve any value 
	    // 
	    value.set_flags(com.sleepycat.db.Db.DB_DBT_PARTIAL);
	    _store.evictor().saveNow();
	    
	    try
	    {
		for(;;)
		{
		    com.sleepycat.db.Dbc dbc = null;
		    try
		    {
			dbc = _db.cursor(null, 0);   
			boolean found = (dbc.get(key, value, com.sleepycat.db.Db.DB_SET) == 0);
			
			if(found)
			{
			    return dbc.count(0);
			}
			else
			{
			    return 0;
			}
		    }
		    catch(com.sleepycat.db.DbDeadlockException dx)
		    {
			if(_store.evictor().deadlockWarning())
			{
			    _store.communicator().getLogger().warning
				("Deadlock in Freeze.Index.untypedCount while iterating over Db \"" 
				 + _store.evictor().filename() + "/" + _dbName
				 + "\"; retrying ...");
			}
			
			//
			// Retry
			//
		    }
		    finally
		    {
			if(dbc != null)
			{
			    try
			    {
				dbc.close();
			    }
			    catch(com.sleepycat.db.DbDeadlockException dx)
			    {
				//
				// Ignored
				//
			    }
			}
		    }
		}
	    }
	    catch(com.sleepycat.db.DbException dx)
	    {
		DatabaseException ex = new DatabaseException();
		ex.initCause(dx);
		ex.message = _store.evictor().errorPrefix() + "Db.cursor: " + dx.getMessage();
		throw ex;
	    }
	}
	finally
	{
	    deactivateController.unlock();
	}
    }

    protected final Ice.Communicator
    communicator()
    {
	return _store.communicator();
    }
    
    void
    associate(ObjectStore store, com.sleepycat.db.DbTxn txn, boolean createDb, boolean populateIndex)
	throws com.sleepycat.db.DbException, java.io.FileNotFoundException
    {
	assert(txn != null);
	_store = store;
	
	_db= new com.sleepycat.db.Db(_store.evictor().dbEnv(), 0);
	_db.set_flags(com.sleepycat.db.Db.DB_DUP | com.sleepycat.db.Db.DB_DUPSORT);

	int flags = 0;
	if(createDb)
	{
	    flags = com.sleepycat.db.Db.DB_CREATE;
	}
	
	_dbName = EvictorI.indexPrefix + store.dbName() + "." + _name;

	_db.open(txn, _store.evictor().filename(), _dbName, com.sleepycat.db.Db.DB_BTREE, flags, 0);

	flags = 0;
	if(populateIndex)
	{
	    flags = com.sleepycat.db.Db.DB_CREATE;
	}
	_store.db().associate(txn, _db, this, flags);
    }

    void
    close()
    {
	if(_db != null)
	{
	    try
	    {
		_db.close(0);
	    }
	    catch(com.sleepycat.db.DbException dx)
	    {
		DatabaseException ex = new DatabaseException();
		ex.initCause(dx);
		ex.message = _store.evictor().errorPrefix() + "Db.close: " + dx.getMessage();
		throw ex;
	    }
	    _db = null;   
	}
    }
  
    private final String _name;
    private final String _facet;
    private String _dbName;

    private com.sleepycat.db.Db _db = null;
    private ObjectStore _store = null;
}
