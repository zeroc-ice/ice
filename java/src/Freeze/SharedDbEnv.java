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

class SharedDbEnv extends com.sleepycat.db.DbEnv implements com.sleepycat.db.DbErrcall, Runnable
{
    public static SharedDbEnv
    get(Ice.Communicator communicator, String envName)
    {
	MapKey key = new MapKey(envName, communicator);

	synchronized(_map) 
	{
	    SharedDbEnv result = (SharedDbEnv) _map.get(key);
	    if(result == null)
	    {
		try
		{
		    result = new SharedDbEnv(key);
		}
		catch(com.sleepycat.db.DbException dx)
		{
		    DatabaseException ex = new DatabaseException();
		    ex.initCause(dx);
		    ex.message = errorPrefix(envName) + "creation: " + dx.getMessage();
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
    getEnvName()
    {
	return _key.envName;
    }

    public Ice.Communicator
    getCommunicator()
    {
	return _key.communicator;
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

		//
		// Join thread
		//
		synchronized(this)
		{
		    _done = true;
		    notify();
		}

		for(;;)
		{
		    try
		    {
			_thread.join();
			break;
		    }
		    catch(InterruptedException ex)
		    {
		    }
		}

		if(_trace >= 1)
		{
		    _key.communicator.getLogger().trace
			("Freeze.DbEnv", "closing database environment \"" + _key.envName + "\"");
		}

		//
		// Keep lock to prevent somebody else from re-opening this DbEnv
		// before it's closed.
		//
		try
		{
		    super.close(0);
		}
		catch(com.sleepycat.db.DbException dx)
		{
		    DatabaseException ex = new DatabaseException();
		    ex.initCause(dx);
		    ex.message = errorPrefix(_key.envName) + "close: " + dx.getMessage();
		    throw ex;
		}
	    }
	}
    }


    synchronized public void 
    deleteOldLogs()
    {
	try
	{
	    String[] list = log_archive(com.sleepycat.db.Db.DB_ARCH_ABS);
	    
	    for(int i = 0; i < list.length; i++)
	    {
		//
		// Remove each file
		//
		java.io.File file = new java.io.File(list[i]);
		boolean ok = file.delete();
		if(!ok)
		{
		    _key.communicator.getLogger().warning(
			"could not delete file \"" + list[i] + "\"");
		}
	    }
	}
	catch(com.sleepycat.db.DbException dx)
	{
	    DatabaseException ex = new DatabaseException();
	    ex.initCause(dx);
	    ex.message = errorPrefix(_key.envName) + "log_archive: " + dx.getMessage();
	    throw ex;
	}
    }

    public void
    run()
    {
	for(;;)
	{
	    synchronized(this)
	    {
		while(!_done)
		{
		    try
		    {
			wait(_checkpointPeriod);
		    }
		    catch(InterruptedException ex)
		    {
			continue;
		    }
		    //
		    // Loop
		    //
		}
		if(_done)
		{
		    return;
		}
	    }
	    
	    try
	    {
		txn_checkpoint(_kbyte, 0, 0);
	    }
	    catch(com.sleepycat.db.DbException dx)
	    {
		_key.communicator.getLogger().warning(
		    "checkpoint on DbEnv \"" + _key.envName + "\" raised DbException: " 
		    + dx.getMessage());
	    }

	    if(_autoDelete)
	    {
		try
		{
		    deleteOldLogs();
		}
		catch(DatabaseException ex)
		{
		    _key.communicator.getLogger().warning(
			"checkpoint on DbEnv \"" + _key.envName + "\" raised DatabaseException: " 
			+ ex.getMessage());
		}
		    
	    }
	}
    }
    
    public void 
    errcall(String errorPrefix, String message)
    {
	_key.communicator.getLogger().error
	    ("Freeze database error in DbEnv \"" + _key.envName + "\": " + message);
    }

    protected void 
    finalize()
    {
	assert(_refCount == 0);
    }

    private SharedDbEnv(MapKey key) throws com.sleepycat.db.DbException
    {	
	super(0);
	_key = key;

	Ice.Properties properties = key.communicator.getProperties();
	_trace = properties.getPropertyAsInt("Freeze.Trace.DbEnv");
	
	if(_trace >= 1)
	{
	    _key.communicator.getLogger().trace
		("Freeze.DbEnv", "opening database environment \"" + _key.envName + "\"");
	}

	String propertyPrefix = "Freeze.DbEnv." + _key.envName;
	
	set_errcall(this);
	
	//
	// Deadlock detection
	//
	set_lk_detect(com.sleepycat.db.Db.DB_LOCK_MINLOCKS);
	
	//
	// Async tx
	//
	set_flags(com.sleepycat.db.Db.DB_TXN_NOSYNC, true);

	int flags = com.sleepycat.db.Db.DB_INIT_LOCK |
	    com.sleepycat.db.Db.DB_INIT_LOG |
	    com.sleepycat.db.Db.DB_INIT_MPOOL |
	    com.sleepycat.db.Db.DB_INIT_TXN;

	if(properties.getPropertyAsInt(
	       propertyPrefix + ".DbRecoverFatal") != 0)
	{
	    flags |= com.sleepycat.db.Db.DB_RECOVER_FATAL | 
		com.sleepycat.db.Db.DB_CREATE;
	}
	else
	{
	    flags |= com.sleepycat.db.Db. DB_RECOVER |
		com.sleepycat.db.Db.DB_CREATE;
	}
	
	if(properties.getPropertyAsIntWithDefault(
	       propertyPrefix + ".DbPrivate", 1) != 0)
	{
	    flags |= com.sleepycat.db.Db.DB_PRIVATE;
	}
	
	String dbHome = properties.getPropertyWithDefault(
	    propertyPrefix + ".DbHome", _key.envName);
	
	//
	// TODO: FREEZE_DB_MODE
	//

	try
	{
	    open(dbHome, flags, 0);
	}
	catch(java.io.FileNotFoundException dx)
	{
	    NotFoundException ex = new NotFoundException();
	    ex.initCause(dx);
	    ex.message = errorPrefix(_key.envName) + "open: " + dx.getMessage();
	    throw ex;
	}

	//
	// Default checkpoint period is every 10 minutes
	//
	_checkpointPeriod = properties.getPropertyAsIntWithDefault(
	    propertyPrefix + ".CheckpointPeriod", 10) * 60 * 1000;
	
	_kbyte = properties.getPropertyAsInt(
	    propertyPrefix + ".PeriodicCheckpointMinSize");

	_autoDelete = (properties.getPropertyAsIntWithDefault(
			   propertyPrefix + ".OldLogsAutoDelete", 1) != 0);
	
	_thread = new Thread(this);
	_thread.start();

	_refCount = 1;
    }

    private static String
    errorPrefix(String envName)
    {
	return  "DbEnv(\"" + envName + "\"): ";
    }

    private static class MapKey
    {
	final String envName;
	final Ice.Communicator communicator;
	
	MapKey(String envName, Ice.Communicator communicator)
	{
	    this.envName = envName;
	    this.communicator = communicator;
	}

	public boolean
	equals(Object o)
	{   
	    try
	    {
		MapKey k = (MapKey)o;
		return (communicator == k.communicator) && envName.equals(k.envName);
	    }
	    catch(ClassCastException ex)
	    {
		communicator.getLogger().trace("Freeze.DbEnv", "equals cast failed");
		return false;
	    }
	}
	
	public int hashCode()
	{
	    return envName.hashCode() ^ communicator.hashCode();
	}
    }

    private MapKey _key;
    private int _refCount = 0;
    private boolean _done = false;
    private int _trace = 0;
    private long _checkpointPeriod = 0;
    private int _kbyte = 0;
    private boolean _autoDelete = false;
    private Thread _thread;

    //
    // Hash map of (MapKey, SharedDbEnv)
    //
    private static java.util.Map _map = new java.util.HashMap();
}

