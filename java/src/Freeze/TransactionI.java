// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
	    DeadlockException ex = new DeadlockException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "DbTxn.commit: " + e.getMessage();
	    throw ex;
	}
	catch(com.sleepycat.db.DbException e)
	{
	    DatabaseException ex = new DatabaseException();
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
	    DeadlockException ex = new DeadlockException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "DbTxn.abort: " + e.getMessage();
	    throw ex;
	}
	catch(com.sleepycat.db.DbException e)
	{
	    DatabaseException ex = new DatabaseException();
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
	    _txn = _connection.dbEnv().txnBegin(null, 0);
	}
	catch(com.sleepycat.db.DbException e)
	{
	    DatabaseException ex = new DatabaseException();
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
