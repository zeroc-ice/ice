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
    secondary_key_create(com.sleepycat.db.Db secondary,
			 com.sleepycat.db.Dbt key,
			 com.sleepycat.db.Dbt value,
			 com.sleepycat.db.Dbt result)
    {
	Ice.Communicator communicator = _evictor.communicator();

	EvictorStorageKey esk = EvictorI.unmarshalKey(key.get_data(), communicator);
	if(esk.facet.length == 0)
	{
	    ObjectRecord rec = EvictorI.unmarshalValue(value.get_data(), communicator);

	    byte[] secondaryKey = marshalKey(rec.servant);
	    if(secondaryKey != null)
	    {
		result.set_data(secondaryKey);
		result.set_size(secondaryKey.length);
		return 0;
	    }
	}

	//
	// Don't want to index this one
	//
	return com.sleepycat.db.Db.DB_DONOTINDEX;
    }

    protected Index(String name)
    {
	_name = name;
    }

    protected abstract byte[]
    marshalKey(Ice.Object servant);
    
    protected Ice.Identity[]
    untypedFindFirst(byte[] k, int firstN)
    {
	com.sleepycat.db.Dbt key = new com.sleepycat.db.Dbt(k);
	
	com.sleepycat.db.Dbt pkey = new com.sleepycat.db.Dbt();
	pkey.set_flags(com.sleepycat.db.Db.DB_DBT_MALLOC);
	
	com.sleepycat.db.Dbt value = new com.sleepycat.db.Dbt();
	//
	// dlen is 0, so we should not retrieve any value 
	// 
	value.set_flags(com.sleepycat.db.Db.DB_DBT_PARTIAL);

	Ice.Communicator communicator = _evictor.communicator();
	_evictor.saveNow();

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
		    boolean more = (dbc.pget(key, pkey, value, com.sleepycat.db.Db.DB_SET) == 0);
		    
		    while((firstN <= 0 || identities.size() < firstN) && more)
		    {
			EvictorStorageKey esk = EvictorI.unmarshalKey(pkey.get_data(), communicator);
			assert(esk.facet.length == 0);
			identities.add(esk.identity);

			more = (dbc.pget(key, pkey, value, com.sleepycat.db.Db.DB_NEXT_DUP) == 0);
		    }
		    break; // for (;;)
		}
		catch(com.sleepycat.db.DbDeadlockException dx)
		{
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
	    ex.message = _evictor.errorPrefix() + "Db.cursor: " + dx.getMessage();
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

    protected Ice.Identity[]
    untypedFind(byte[] key)
    {
	return untypedFindFirst(key, 0);
    }

    protected int
    untypedCount(byte[] k)
    {
	com.sleepycat.db.Dbt key = new com.sleepycat.db.Dbt(k);
	com.sleepycat.db.Dbt value = new com.sleepycat.db.Dbt();
	//
	// dlen is 0, so we should not retrieve any value 
	// 
	value.set_flags(com.sleepycat.db.Db.DB_DBT_PARTIAL);
	_evictor.saveNow();

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
	    ex.message = _evictor.errorPrefix() + "Db.cursor: " + dx.getMessage();
	    throw ex;
	}
    }

    
    void
    associate(EvictorI evictor, com.sleepycat.db.DbTxn txn, boolean createDb, boolean populateIndex)
	throws com.sleepycat.db.DbException, java.io.FileNotFoundException
    {
	assert(txn != null);
	_evictor = evictor;
	
	_db= new com.sleepycat.db.Db(evictor.dbEnv(), 0);
	_db.set_flags(com.sleepycat.db.Db.DB_DUP | com.sleepycat.db.Db.DB_DUPSORT);

	int flags = 0;
	if(createDb)
	{
	    flags = com.sleepycat.db.Db.DB_CREATE;
	}
	_db.open(txn, evictor.dbName() + "." + _name, null, com.sleepycat.db.Db.DB_BTREE, flags, 0);

	flags = 0;
	if(populateIndex)
	{
	    flags = com.sleepycat.db.Db.DB_CREATE;
	}
	evictor.db().associate(txn, _db, this, flags);
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
		ex.message = _evictor.errorPrefix() + "Db.close: " + dx.getMessage();
		throw ex;
	    }
	    _db = null;   
	}
    }

    
    final com.sleepycat.db.Db
    db()
    {
	return _db;
    }

    final EvictorI
    evictor()
    {
	return _evictor;
    }
    
    final protected Ice.Communicator
    communicator()
    {
	return _evictor.communicator();
    }

    private String _name;
    private com.sleepycat.db.Db _db;
    private EvictorI _evictor;   
}
