// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Freeze;

class DBTransactionI implements DBTransaction
{
    public synchronized void
    commit()
    {
	if (_tid == null)
	{
	    String s = _errorPrefix + "transaction has already been committed or aborted";
	    DBException ex = new DBException();
	    ex.message = s;
	    throw ex;
	}
	
	if (_trace >= 2)
	{
	    _communicator.getLogger().trace("DB", "committing transaction for environment \"" + _name + "\"");
	}
	
	try
	{
	    _tid.commit(0);
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
	
	_tid = null;
    }
    
    public synchronized void
    abort()
    {
	if (_tid == null)
	{
	    String s = _errorPrefix + "transaction has already been committed or aborted";
	    DBException ex = new DBException();
	    ex.message = s;
	    throw ex;
	}
	
	if (_trace >= 2)
	{
	    _communicator.getLogger().trace("DB", "aborting transaction for environment \"" + _name +
					     "\" due to deadlock");
	}
	
	try
	{
	    _tid.abort();
	}
	catch(com.sleepycat.db.DbException e)
	{
	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "DbTxn.abort: " + e.getMessage();
	    throw ex;
	}
	
	_tid = null;
    }

    protected DBTransactionI(Ice.Communicator communicator, com.sleepycat.db.DbEnv dbEnv, String name)
    {
	_communicator = communicator;
	_name = name;

	_errorPrefix = "Freeze::DBTransaction(\"" + _name + "\"): ";
	
	String value = _communicator.getProperties().getProperty("Freeze.Trace.DB");
	if (value != null)
	{
	    try
	    {
		_trace = Integer.parseInt(value);
	    }
	    catch (NumberFormatException ex)
	    {
		// TODO: Do anything?
	    }
	}
	
	if (_trace >= 2)
	{
	    _communicator.getLogger().trace("DB", "starting transaction for environment \"" + _name + "\"");
	}

	try
	{
	    _tid = dbEnv.txn_begin(null, 0);
	}
	catch(com.sleepycat.db.DbException e)
	{
	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "DbEnv.txn_begin: " + e.getMessage();
	    throw ex;
	}
    }

    private Ice.Communicator _communicator;
    private int _trace = 0;

    private com.sleepycat.db.DbTxn _tid;

    private String _name;
    private String _errorPrefix;
}
