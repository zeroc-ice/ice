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

class ConnectionI extends Ice.LocalObjectImpl implements Connection
{

    public Transaction
    beginTransaction()
    {
	if(_transaction != null)
	{
	    throw new Freeze.TransactionAlreadyInProgressException();
	}
	closeAllIterators();
	_transaction = new TransactionI(this);
	return _transaction;
    }

    public Transaction
    currentTransaction()
    {
	return _transaction;
    }

    public void
    close()
    {
	close(false);
    }

    public Ice.Communicator
    getCommunicator()
    {
	return _communicator;
    }

    public String
    getName()
    {
	return _envName;
    }

    protected void
    finalize()
    {
	close(true);
    }

    void
    close(boolean finalizing)
    {
	if(_transaction != null)
	{
	    if(finalizing)
	    {
		_communicator.getLogger().warning
		    ("Finalizing Connection on DbEnv \"" +  _envName + "\" with active transaction");
	    }
	    
	    try
	    {
		_transaction.rollback();
	    }
	    catch(Freeze.DatabaseException dx)
	    {
		//
		// Ignored
		//
	    }
	}

	
	synchronized(this)
	{
	    while(!_mapList.isEmpty())
	    {
		((Map) _mapList.get(0)).close(finalizing);
	    }
	}
	
	_dbEnv = null;

	if(_dbEnvHolder != null)
	{
	    try
	    {
		_dbEnvHolder.close();
	    }
	    finally
	    {
		_dbEnvHolder = null;
	    }
	}
    }

    ConnectionI(Ice.Communicator communicator, String envName)
    {
	_communicator = communicator;
	_dbEnvHolder = SharedDbEnv.get(communicator, envName);
	_dbEnv = _dbEnvHolder;
	_envName = envName;
	_trace = _communicator.getProperties().getPropertyAsInt("Freeze.Trace.Map");
	Ice.Properties properties = _communicator.getProperties();

	_deadlockWarning = properties.getPropertyAsInt("Freeze.Warn.Deadlocks") != 0;
	_keepIterators = properties.getPropertyAsIntWithDefault("Freeze.Map.KeepIterators", 1) != 0;
    }

    ConnectionI(Ice.Communicator communicator, String envName, com.sleepycat.db.DbEnv dbEnv)
    {
	_communicator = communicator;
	_dbEnv = dbEnv;
	_envName = envName;
	_trace = _communicator.getProperties().getPropertyAsInt("Freeze.Trace.Map");
    }

    //
    // The synchronization is only needed only during finalization
    //

    synchronized void
    closeAllIterators()
    {
	java.util.Iterator p = _mapList.iterator();
	while(p.hasNext())
	{
	    ((Map) p.next()).closeAllIterators();
	}
    }

    synchronized void
    registerMap(Map map)
    {
	_mapList.add(map);
    }

    synchronized void
    unregisterMap(Map map)
    {
	_mapList.remove(map);
    }

    void
    clearTransaction()
    {
	_transaction = null;
    }

    com.sleepycat.db.DbTxn
    dbTxn()
    {
	if(_transaction == null)
	{
	    return null;
	}
	else
	{
	    return _transaction.dbTxn();
	}
    }

    com.sleepycat.db.DbEnv
    dbEnv()
    {
	return _dbEnv;
    }

    String
    envName()
    {
	return _envName;
    }

    Ice.Communicator
    communicator()
    {
	return _communicator;
    }

    final int
    trace()
    {
	return _trace;
    }

    final boolean
    deadlockWarning()
    {
	return _deadlockWarning;
    }

    final boolean
    keepIterators()
    {
	return _keepIterators;
    }

    private Ice.Communicator _communicator;
    private SharedDbEnv _dbEnvHolder;
    private com.sleepycat.db.DbEnv _dbEnv;
    private String _envName;
    private TransactionI _transaction;
    private java.util.List _mapList = new java.util.LinkedList();
    private int _trace;
    private boolean _deadlockWarning;
    private boolean _keepIterators;
}
