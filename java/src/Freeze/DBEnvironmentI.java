// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Freeze;

class DBEnvironmentI extends Ice.LocalObjectImpl implements DBEnvironment, com.sleepycat.db.DbErrcall
{
    public String
    getName()
    {
	// No mutex lock necessary, _name is immutable
	return _name;
    }

    public Ice.Communicator
    getCommunicator()
    {
	// No mutex lock necessary, _communicator is immutable
	return _communicator;
    }
    
    synchronized public DB
    openDB(String name, boolean create)
    {
	if(_dbEnv == null)
	{
	    DBException ex = new DBException();
	    ex.message = _errorPrefix + "\"" + _name + "\" has been closed";
	    throw ex;
	}

	DB p = (DB)_dbMap.get(name);
	if(p != null)
	{
	    return p;
	}

	com.sleepycat.db.Db db;
	
	try
	{
	    db = new com.sleepycat.db.Db(_dbEnv, 0);
	}
	catch(com.sleepycat.db.DbException e)
	{
	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "Db.Db: " + e.getMessage();
	    throw ex;
	}
	
	try
	{
	    return new DBI(_communicator, this, db, name, create);
	}
	catch(DBException e)
	{
	    try
	    {
		db.close(0);
	    }
	    catch(com.sleepycat.db.DbException ignore)
	    {
	    }
	    throw e;
	}
    }

    public DBTransaction
    startTransaction()
    {
	return new DBTransactionI(_communicator, _dbEnv, _name);
    }

    synchronized public void
    close()
    {
	if(_dbEnv == null)
	{
	    return;
	}

	//
	// Build a list of values from the iterator. The iterator
	// cannot be used directly since closing each database
	// modifies the content of the map (hence invalidating the
	// iterator).
	//
	java.util.List dbs = new java.util.ArrayList();
        java.util.Iterator p = _dbMap.values().iterator();
        while(p.hasNext())
        {
	    dbs.add(p.next());
        }

	p = dbs.iterator();
	while(p.hasNext())
	{
            DB db = (DB)p.next();
	    db.close();
	}
	
	if(_trace >= 1)
	{
	    _communicator.getLogger().trace("DB", "closing database environment \"" + _name + "\"");
	}
	
	try
	{
	    _dbEnv.close(0);
	}
	catch(com.sleepycat.db.DbException e)
	{
	    _dbEnv = null;

	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "DbEnv.close: " + e.getMessage();
	    throw ex;
	}
	
	_dbEnv = null;
    }

    synchronized protected void
    add(String name, DB db)
    {
	_dbMap.put(name, db);
    }
    
    synchronized protected void
    remove(String name)
    {
	_dbMap.remove(name);
    }
    
    synchronized protected void
    eraseDB(String name)
    {
	//
	// The database should not be open.
	//
	assert(_dbMap.get(name) == null);

	com.sleepycat.db.Db db;
	try
	{
	    db = new com.sleepycat.db.Db(_dbEnv, 0);
	}
	catch(com.sleepycat.db.DbException e)
	{
	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "Db.Db: " + e.getMessage();
	    throw ex;
	}

	try
	{
	    //
	    // Any failure in remove will cause the database to be
	    // closed.
	    //
	    db.remove(name, null, 0);
	}
	catch(java.io.FileNotFoundException e)
	{
	    DBNotFoundException ex = new DBNotFoundException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "Db.remove: " + e.getMessage();
	    throw ex;
	}
	catch(com.sleepycat.db.DbException e)
	{
	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "Db.remove: " + e.getMessage();
	    throw ex;
	}
    }

    DBEnvironmentI(Ice.Communicator communicator, String name)
    {
	_communicator = communicator;
	_name = name;
	_errorPrefix = "Freeze::DBEnvironment(\"" + _name + "\"): ";
	_trace = getCommunicator().getProperties().getPropertyAsInt("Freeze.Trace.DB");
	_dbEnv = new com.sleepycat.db.DbEnv(0);
	
	if(_trace >= 1)
	{
	    _communicator.getLogger().trace("DB", "opening database environment \"" + _name + "\"");
	}
	
	try
	{
	    _dbEnv.open(_name,
			com.sleepycat.db.Db.DB_CREATE |
			com.sleepycat.db.Db.DB_INIT_LOCK |
			com.sleepycat.db.Db.DB_INIT_LOG |
			com.sleepycat.db.Db.DB_INIT_MPOOL |
			com.sleepycat.db.Db.DB_INIT_TXN |
			com.sleepycat.db.Db.DB_RECOVER,
			0); //TODO: FREEZE_DB_MODE)
	}
	catch(java.io.FileNotFoundException e)
	{
	    DBNotFoundException ex = new DBNotFoundException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "DbEnv.open: " + e.getMessage();
	    throw ex;
	}
	catch(com.sleepycat.db.DbException e)
	{
	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "DbEnv.open: " + e.getMessage();
	    throw ex;
	}
	
	_dbEnv.set_errcall(this);
    }

    //
    // com.sleepycat.db.DbErrcall interface implementation.
    //
    public void 
    errcall(String errorPrefix, String message)
    {
	_communicator.getLogger().error("Freeze database error: " + _name + ": " + message);
    }
    
    private Ice.Communicator _communicator;
    private int _trace = 0;
    private com.sleepycat.db.DbEnv _dbEnv;
    private String _name;
    private String _errorPrefix;

    private java.util.HashMap _dbMap = new java.util.HashMap();
}
