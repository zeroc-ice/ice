// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze;

class SharedDb extends com.sleepycat.db.Db
{
    public static SharedDb
    get(ConnectionI connection, String dbName, Map.Index[] indices, boolean createDb)
    {
	MapKey key = new MapKey(connection.envName(), connection.communicator(), dbName);

	synchronized(_map) 
	{
	    SharedDb result = (SharedDb) _map.get(key);
	    if(result == null)
	    {
		try
		{
		    result = new SharedDb(key, connection, indices, createDb);
		}
		catch(com.sleepycat.db.DbException dx)
		{
		    DatabaseException ex = new DatabaseException();
		    ex.initCause(dx);
		    ex.message = errorPrefix(key) + "creation: " + dx.getMessage();
		    throw ex;
		}

		Object previousValue = _map.put(key, result);
		assert(previousValue == null);
	    }
	    else
	    {
		result.connectIndices(indices);
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
		    _key.communicator.getLogger().trace("Freeze.Map", "closing Db \"" + _key.dbName + "\"");
		}

		//
		// Keep lock to prevent somebody else from re-opening this Db
		// before it's closed.
		//
		try
		{
		    cleanupIndices();
		   
		    super.close(0);
		}
		catch(com.sleepycat.db.DbException dx)
		{
		    DatabaseException ex = new DatabaseException();
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

    private SharedDb(MapKey key, ConnectionI connection, Map.Index[] indices,
		     boolean createDb) throws com.sleepycat.db.DbException
    {	
	super(connection.dbEnv(), 0);
	_key = key;
	_indices = indices;
	_trace = connection.trace();

	com.sleepycat.db.DbTxn txn = null;

	String txnId = null;

	try
	{
	    txn = connection.dbEnv().txnBegin(null, 0);

	    if(connection.txTrace() >= 1)
	    {
		txnId = Long.toHexString((txn.id() & 0x7FFFFFFF) + 0x80000000L); 

		_key.communicator.getLogger().trace
		    ("Freeze.Map", errorPrefix(_key) + "successfully started transaction " +
		      txnId + " to open Db \"" + _key.dbName + "\"");
	    }

	    int flags = 0;
	    
	    if(createDb)
	    {
		flags |= com.sleepycat.db.Db.DB_CREATE;
	    }

	    if(_trace >= 1)
	    {
		_key.communicator.getLogger().trace("Freeze.Map", "opening Db \"" + _key.dbName + "\"");
	    }
   
	    open(txn, key.dbName, null, com.sleepycat.db.Db.DB_BTREE, flags, 0);

	    if(_indices != null)
	    {
		for(int i = 0; i < _indices.length; ++i)
		{
		    _indices[i].associate(key.dbName, this, txn, createDb);
		}
	    }
	    
	    com.sleepycat.db.DbTxn toCommit = txn;
	    txn = null;
	    toCommit.commit(0);

	    if(connection.txTrace() >= 1)
	    {
		_key.communicator.getLogger().trace
		    ("Freeze.Map", errorPrefix(_key) + "successfully committed transaction " +
		     txnId);
	    }

	    //
	    // TODO: FREEZE_DB_MODE
	    //
	}
	catch(java.io.FileNotFoundException dx)
	{
	    cleanupIndices();
	    NotFoundException ex = new NotFoundException();
	    ex.initCause(dx);
	    ex.message = errorPrefix(_key) + "Db.open: " + dx.getMessage();
	    throw ex;
	}
	catch(com.sleepycat.db.DbException dx)
	{
	    cleanupIndices();
	    DatabaseException ex = new DatabaseException();
	    ex.initCause(dx);
	    ex.message = errorPrefix(_key) + "Db.open: " + dx.getMessage();
	    throw ex;
	}
	finally
	{
	    if(txn != null)
	    {
		try
		{
		    txn.abort();

		    if(connection.txTrace() >= 1)
		    {
			_key.communicator.getLogger().trace
			    ("Freeze.Map", errorPrefix(_key) + "successfully rolled back transaction " + txnId);
		    }
		}
		catch(com.sleepycat.db.DbException dx)
		{
		    if(connection.txTrace() >= 1)
		    {
			_key.communicator.getLogger().trace
			    ("Freeze.Map", errorPrefix(_key) + "failed to roll back transaction " +
			     txnId + ": " + dx.getMessage());
		    }
		}
	    }
	}

	_refCount = 1;
    }

    private void
    connectIndices(Map.Index[] indices)
    {
	if(indices != null)
	{
	    assert(_indices != null && indices.length == _indices.length);

	    for(int i = 0; i < indices.length; ++i)
	    {
		indices[i].init(_indices[i]);
	    }
	}
    }

    private void
    cleanupIndices()
    {
	if(_indices != null)
	{
	    for(int i = 0; i < _indices.length; ++i)
	    {
		_indices[i].close();
	    }
	    _indices = null;
	}
    }
	

    private static String
    errorPrefix(MapKey k)
    {
	return "Freeze DB DbEnv(\"" + k.envName + "\") Db(\"" + k.dbName + "\"): ";
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
		communicator.getLogger().trace("Freeze.Map", "equals cast failed");
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
    private Map.Index[] _indices;

    //
    // Hash map of (MapKey, SharedDb)
    //
    private static java.util.Map _map = new java.util.HashMap();
}

