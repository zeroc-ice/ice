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

class SharedDb extends com.sleepycat.db.Db
{
    public static SharedDb
    get(ConnectionI connection, String dbName, boolean createDb)
    {
	MapKey key = new MapKey(connection.envName(), connection.communicator(), dbName);

	synchronized(_map) 
	{
	    SharedDb result = (SharedDb) _map.get(key);
	    if(result == null)
	    {
		try
		{
		    result = new SharedDb(key, connection, createDb);
		}
		catch(com.sleepycat.db.DbException dx)
		{
		    DBException ex = new DBException();
		    ex.initCause(dx);
		    ex.message = errorPrefix(key) + "creation: " + dx.getMessage();
		    throw ex;
		}

		Object previousValue = _map.put(key, result);
		assert(previousValue == null);
	    }
	    else
	    {
		result._refCount++;
	    }
	    return result;
	}
    }
    
    public String 
    dbName()
    {
	return _key.dbName;
    }

    public void
    close()
    {
	synchronized(_map) 
	{
	    if(--_refCount == 0)
	    {	    
		//
		// Remove from map
		//
		Object value = _map.remove(_key);
		assert(value == this);

		if(_trace >= 1)
		{
		    _key.communicator.getLogger().trace("DB", "closing DB \"" + _key.dbName + "\"");
		}

		//
		// Keep lock to prevent somebody else from re-opening this Db
		// before it's closed.
		//
		try
		{
		    super.close(0);
		}
		catch(com.sleepycat.db.DbException dx)
		{
		    DBException ex = new DBException();
		    ex.initCause(dx);
		    ex.message = errorPrefix(_key) + "close: " + dx.getMessage();
		    throw ex;
		}
	    }
	}
    }

    protected void 
    finalize()
    {
	assert(_refCount == 0);
    }

    private SharedDb(MapKey key, ConnectionI connection, boolean createDb) throws com.sleepycat.db.DbException
    {	
	super(connection.dbEnv(), 0);
	_key = key;
	_trace = connection.trace();

	try
	{
	    int flags = com.sleepycat.db.Db.DB_AUTO_COMMIT;
	    
	    if(createDb)
	    {
		flags |= com.sleepycat.db.Db.DB_CREATE;
	    }
	    
	    open(null, key.dbName, null, com.sleepycat.db.Db.DB_BTREE, flags, 0);

	    //
	    // TODO: FREEZE_DB_MODE
	    //
	}
	catch(java.io.FileNotFoundException dx)
	{
	    DBNotFoundException ex = new DBNotFoundException();
	    ex.initCause(dx);
	    ex.message = errorPrefix(_key) + "Db.open: " + dx.getMessage();
	    throw ex;
	}
	catch(com.sleepycat.db.DbException dx)
	{
	    DBException ex = new DBException();
	    ex.initCause(dx);
	    ex.message = errorPrefix(_key) + "Db.open: " + dx.getMessage();
	    throw ex;
	}
	_refCount = 1;
    }

    private static String
    errorPrefix(MapKey k)
    {
	return "Freeze DB DbEnv(\"" + k.envName + "\") Db(\"" + k.dbName + "\") :";
    }

    private static class MapKey
    {
	final String envName;
	final Ice.Communicator communicator;
	final String dbName;
	
	MapKey(String envName, Ice.Communicator communicator, String dbName)
	{
	    this.envName = envName;
	    this.communicator = communicator;
	    this.dbName = dbName;
	}

	public boolean
	equals(Object o)
	{   
	    try
	    {
		MapKey k = (MapKey)o;
		return (dbName.equals(k.dbName)) && (communicator == k.communicator) && envName.equals(k.envName);
	    }
	    catch(ClassCastException ex)
	    {
		communicator.getLogger().trace("DB", "equals cast failed");
		return false;
	    }
	}
	
	public int hashCode()
	{
	    return dbName.hashCode() ^ envName.hashCode() ^ communicator.hashCode();
	}
    }

    private MapKey _key;
    private int _refCount = 0;
    private int _trace;

    //
    // Hash map of (MapKey, SharedDb)
    //
    private static java.util.Map _map = new java.util.HashMap();
}

