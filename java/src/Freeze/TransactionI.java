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

class TransactionI extends Ice.LocalObjectImpl implements Transaction
{
    public void
    commit()
    {
	try
	{
	    _connection.closeAllIterators();
	    _txn.commit(0);
	}
	catch(com.sleepycat.db.DbDeadlockException e)
	{
	    DBDeadlockException ex = new DBDeadlockException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "DbTxn.commit: " + e.getMessage();
	    throw ex;
	}
	catch(com.sleepycat.db.DbException e)
	{
	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "DbTxn.commit: " + e.getMessage();
	    throw ex;
	}
	finally
	{
	    _connection.clearTransaction();
	    _connection = null;
	    _txn = null;
	}
    }

    public void
    rollback()
    {
	try
	{
	    _connection.closeAllIterators();
	    _txn.abort();
	}
	catch(com.sleepycat.db.DbDeadlockException e)
	{
	    DBDeadlockException ex = new DBDeadlockException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "DbTxn.abort: " + e.getMessage();
	    throw ex;
	}
	catch(com.sleepycat.db.DbException e)
	{
	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "DbTxn.abort: " + e.getMessage();
	    throw ex;
	}
	finally
	{
	    _connection.clearTransaction();
	    _connection = null;
	    _txn = null;
	}
    }
 
    TransactionI(ConnectionI connection)
    {
	_connection = connection;
	_errorPrefix = "Freeze DB DbEnv(\"" + _connection.envName() + "\") :";

	try
	{
	    _txn = _connection.dbEnv().txn_begin(null, 0);
	}
	catch(com.sleepycat.db.DbException e)
	{
	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "txn_begin: " + e.getMessage();
	    throw ex;
	}
    }

    com.sleepycat.db.DbTxn
    dbTxn()
    {
	return _txn;
    }

    private ConnectionI _connection;
    private com.sleepycat.db.DbTxn _txn;

    private String _errorPrefix;
}
