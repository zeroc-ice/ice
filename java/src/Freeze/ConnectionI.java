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
	if(_transaction != null)
	{
	    try
	    {
		_transaction.rollback();
	    }
	    catch(Freeze.DBException dx)
	    {
		//
		// Ignored
		//
	    }
	}

	java.util.Iterator p = _mapList.iterator();
	while(p.hasNext())
	{
	    Object o = ((java.lang.ref.WeakReference) p.next()).get();
	    if(o != null)
	    {
		((Map) o).close();
	    }
	}
	_mapList.clear();
	
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
	close();
    }

    ConnectionI(Ice.Communicator communicator, String envName)
    {
	_communicator = communicator;
	_dbEnvHolder = SharedDbEnv.get(communicator, envName);
	_dbEnv = _dbEnvHolder;
	_envName = envName;
	_trace = _communicator.getProperties().getPropertyAsInt("Freeze.Trace.DB");
    }

    ConnectionI(Ice.Communicator communicator, com.sleepycat.db.DbEnv dbEnv)
    {
	_communicator = communicator;
	_dbEnv = dbEnv;
	_envName = "External";
	_trace = _communicator.getProperties().getPropertyAsInt("Freeze.Trace.DB");
    }

    void
    closeAllIterators()
    {
	java.util.Iterator p = _mapList.iterator();
	while(p.hasNext())
	{
	    Object o = ((java.lang.ref.WeakReference) p.next()).get();
	    if(o != null)
	    {
		((Map) o).closeAllIterators();
	    }
	    else
	    {
		p.remove();
	    }
	}
    }

    void
    registerMap(Map map)
    {
	_mapList.add(new java.lang.ref.WeakReference(map));
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

    int
    trace()
    {
	return _trace;
    }

    private Ice.Communicator _communicator;
    private SharedDbEnv _dbEnvHolder;
    private com.sleepycat.db.DbEnv _dbEnv;
    private String _envName;
    private TransactionI _transaction;
    private java.util.List _mapList = new java.util.LinkedList();
    private int _trace;
}
