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

class DBI extends Ice.LocalObjectImpl implements DB
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

    synchronized public long
    getNumberOfRecords()
    {
	if(_db == null)
	{
	    DBException ex = new DBException();
	    ex.message = _errorPrefix + "\"" + _name + "\" has been closed";
	    throw ex;
	}

	//
	// TODO: DB_FAST_STAT doesn't seem to do what the
	// documentation says...
	//
	try
	{
	    com.sleepycat.db.DbBtreeStat s = (com.sleepycat.db.DbBtreeStat)_db.stat(0);
	    return s.bt_ndata;
	}
	catch(com.sleepycat.db.DbException e)
	{
	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "Db.stat: " + e.getMessage();
	    throw ex;
	}
    }

    synchronized public DBCursor
    getCursor()
    {
	if(_db == null)
	{
	    DBException ex = new DBException();
	    ex.message = _errorPrefix + "\"" + _name + "\" has been closed";
	    throw ex;
	}

	com.sleepycat.db.Dbc cursor;

	try
	{
	    cursor = _db.cursor(null, 0);
	}
	catch(com.sleepycat.db.DbException e)
	{
	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "Db.cursor: " + e.getMessage();
	    throw ex;
	}

	//
	// Note that the read of the data is partial (that is the data
	// will not actually be read into memory since it isn't needed
	// yet).
	//
	com.sleepycat.db.Dbt dbData = new com.sleepycat.db.Dbt();
	dbData.set_flags(com.sleepycat.db.Db.DB_DBT_PARTIAL);
	com.sleepycat.db.Dbt dbKey = new com.sleepycat.db.Dbt();
	dbKey.set_flags(com.sleepycat.db.Db.DB_DBT_PARTIAL);

	try
	{
	    try
	    {
		int rc = cursor.get(dbKey, dbData, com.sleepycat.db.Db.DB_FIRST);
		if(rc == com.sleepycat.db.Db.DB_NOTFOUND)
		{
		    DBNotFoundException ex = new DBNotFoundException();
		    ex.message = _errorPrefix + "Dbc.get: DB_NOTFOUND";
		    throw ex;
		}
	    }
	    catch(com.sleepycat.db.DbException e)
	    {
		DBException ex = new DBException();
		ex.initCause(e);
		ex.message = _errorPrefix + "Dbc.get: " + e.getMessage();
		throw ex;
	    }
	}
	catch(DBException e)
	{
	    //
	    // Cleanup on failure.
	    //
	    try
	    {
		cursor.close();
	    }
	    catch(com.sleepycat.db.DbException ignore)
	    {
		// Ignore
	    }
	    throw e;
	}
	
	return new DBCursorI(_communicator, _name, cursor);
    }
    
    synchronized public DBCursor
    getCursorAtKey(byte[] key)
    {
	if(_db == null)
	{
	    DBException ex = new DBException();
	    ex.message = _errorPrefix + "\"" + _name + "\" has been closed";
	    throw ex;
	}

	com.sleepycat.db.Dbc cursor;

	try
	{
	    cursor = _db.cursor(null, 0);
	}
	catch(com.sleepycat.db.DbException e)
	{
	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "Db.cursor: " + e.getMessage();
	    throw ex;
	}

	//
	// Move to the requested record
	//
	com.sleepycat.db.Dbt dbKey = new com.sleepycat.db.Dbt(key);
	
	//
	// Note that the read of the data is partial (that is the data
	// will not actually be read into memory since it isn't needed
	// yet).
	//
	com.sleepycat.db.Dbt dbData = new com.sleepycat.db.Dbt();
	dbData.set_flags(com.sleepycat.db.Db.DB_DBT_PARTIAL);

	try
	{
	    try
	    {
		int rc = cursor.get(dbKey, dbData, com.sleepycat.db.Db.DB_SET);
		if(rc == com.sleepycat.db.Db.DB_NOTFOUND)
		{
		    DBNotFoundException ex = new DBNotFoundException();
		    ex.message = _errorPrefix + "Dbc.get: DB_NOTFOUND";
		    throw ex;
		}
	    }
	    catch(com.sleepycat.db.DbException e)
	    {
		DBException ex = new DBException();
		ex.initCause(e);
		ex.message = _errorPrefix + "Dbc.get: " + e.getMessage();
		throw ex;
	    }
	}
	catch(DBException e)
	{
	    //
	    // Cleanup on failure.
	    //
	    try
	    {
		cursor.close();
	    }
	    catch(com.sleepycat.db.DbException ignore)
	    {
		// Ignore
	    }
	    throw e;
	}

	return new DBCursorI(_communicator, _name, cursor);
    }

    synchronized public void
    put(byte[] key, byte[] value)
    {
	if(_db == null)
	{
	    DBException ex = new DBException();
	    ex.message = _errorPrefix + "\"" + _name + "\" has been closed";
	    throw ex;
	}

	//
	// TODO: This can be optimized so that these only need to be
	// allocated once.
        //
	com.sleepycat.db.Dbt dbKey = new com.sleepycat.db.Dbt(key);
	com.sleepycat.db.Dbt dbData = new com.sleepycat.db.Dbt(value);

	if(_trace >= 1)
	{
	    _communicator.getLogger().trace("DB", "writing value in database \"" + _name + "\"");
	}

	try
	{
	    _db.put(null, dbKey, dbData, 0);
	}
	catch(com.sleepycat.db.DbDeadlockException e)
	{
	    DBDeadlockException ex = new DBDeadlockException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "Db.put: " + e.getMessage();
	    throw ex;
	}
	catch(com.sleepycat.db.DbException e)
	{
	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "Db.put: " + e.getMessage();
	    throw ex;
	}
    }

    synchronized public boolean
    contains(byte[] key)
    {
	if(_db == null)
	{
	    DBException ex = new DBException();
	    ex.message = _errorPrefix + "\"" + _name + "\" has been closed";
	    throw ex;
	}

	com.sleepycat.db.Dbt dbKey = new com.sleepycat.db.Dbt(key);

	com.sleepycat.db.Dbt dbData = new com.sleepycat.db.Dbt();
	dbData.set_flags(com.sleepycat.db.Db.DB_DBT_PARTIAL);

	if(_trace >= 1)
	{
	    _communicator.getLogger().trace("DB", "checking key in database \"" + _name + "\"");
	}

	try
	{
	    int rc =_db.get(null, dbKey, dbData, 0);
	    if(rc == com.sleepycat.db.Db.DB_NOTFOUND)
	    {
		return false;
	    }
	    else
	    {
		assert(rc == 0);
		return true;
	    }
	}
	catch(com.sleepycat.db.DbDeadlockException e)
	{
	    DBDeadlockException ex = new DBDeadlockException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "Db.get: " + e.getMessage();
	    throw ex;
	}
	catch(com.sleepycat.db.DbException e)
	{
	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "Db.get: " + e.getMessage();
	    throw ex;
	}
    }

    synchronized public byte[]
    get(byte[] key)
    {
	if(_db == null)
	{
	    DBException ex = new DBException();
	    ex.message = _errorPrefix + "\"" + _name + "\" has been closed";
	    throw ex;
	}

	com.sleepycat.db.Dbt dbKey = new com.sleepycat.db.Dbt(key);
	com.sleepycat.db.Dbt dbData = new com.sleepycat.db.Dbt();

	if(_trace >= 1)
	{
	    _communicator.getLogger().trace("DB", "reading value from database \"" + _name + "\"");
	}

	try
	{
	    int rc =_db.get(null, dbKey, dbData, 0);
	    if(rc == com.sleepycat.db.Db.DB_NOTFOUND)
	    {
		DBNotFoundException ex = new DBNotFoundException();
		ex.message = _errorPrefix + "Db.get: DB_NOTFOUND";
		throw ex;
	    }
	}
	catch(com.sleepycat.db.DbDeadlockException e)
	{
	    DBDeadlockException ex = new DBDeadlockException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "Db.get: " + e.getMessage();
	    throw ex;
	}
	catch(com.sleepycat.db.DbException e)
	{
	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "Db.get: " + e.getMessage();
	    throw ex;
	}
    
	return dbData.get_data();
    }

    synchronized public void
    del(byte[] key)
    {
	if(_db == null)
	{
	    DBException ex = new DBException();
	    ex.message = _errorPrefix + "\"" + _name + "\" has been closed";
	    throw ex;
	}

	com.sleepycat.db.Dbt dbKey = new com.sleepycat.db.Dbt(key);

	if(_trace >= 1)
	{
	    _communicator.getLogger().trace("DB", "deleting value from database \"" + _name + "\"");
	}
	
	try
	{
	    int rc =_db.del(null, dbKey, 0);
	    if(rc == com.sleepycat.db.Db.DB_NOTFOUND)
	    {
		DBNotFoundException ex = new DBNotFoundException();
		ex.message = _errorPrefix + "Db.del: DB_NOTFOUND";
		throw ex;
	    }
	}
	catch(com.sleepycat.db.DbDeadlockException e)
	{
	    DBDeadlockException ex = new DBDeadlockException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "Db.del: " + e.getMessage();
	    throw ex;
	}
	catch(com.sleepycat.db.DbException e)
	{
	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "Db.del: " + e.getMessage();
	    throw ex;
	}
    }

    synchronized public void
    clear()
    {
	if(_db == null)
	{
	    DBException ex = new DBException();
	    ex.message = _errorPrefix + "\"" + _name + "\" has been closed";
	    throw ex;
	}

	int count; // ignored
	try
	{
	    _db.truncate(null, 0);
	}
	catch(com.sleepycat.db.DbException e)
	{
	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "Db.truncate: " + e.getMessage();
	    throw ex;
	}
    }

    synchronized public void
    close()
    {
	if(_db == null)
	{
	    return;
	}

	if(_trace >= 1)
	{
	    _communicator.getLogger().trace("DB", "closing database \"" + _name + "\"");
	}

	try
	{
	    _db.close(0);
	}
	catch(com.sleepycat.db.DbException e)
	{
	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "Db.truncate: " + e.getMessage();
	    throw ex;
	}

	_dbEnvObj.remove(_name);
	_dbEnvObj = null;
	_db = null;
    }

    synchronized public void
    remove()
    {
	if(_db == null)
	{
	    return;
	}
	
	if(_trace >= 1)
	{
	    _communicator.getLogger().trace("DB", "removing database \"" + _name + "\"");
	}

	//
	// Remove first needs to close the database object. It's not
	// possible to remove an open database.
	//
	try
	{
	    _db.close(0);
	}
	catch(com.sleepycat.db.DbException e)
	{
	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "Db.truncate: " + e.getMessage();
	    throw ex;
	}
	
	//
	// Take a copy of the DBEnvironment to make cleanup easier.
	//
	DBEnvironmentI dbEnvCopy = _dbEnvObj;
	
	_dbEnvObj.remove(_name);
	_dbEnvObj = null;
	_db = null;
	
	//
	// Ask the DBEnvironment to erase the database.
	//
	dbEnvCopy.eraseDB(_name);
    }

    synchronized public Evictor
    createEvictor(EvictorPersistenceMode persistenceMode)
    {
	if(_db == null)
	{
	    DBException ex = new DBException();
	    ex.message = _errorPrefix + "\"" + _name + "\" has been closed";
	    throw ex;
	}

	return new EvictorI(this, persistenceMode);
    }

    DBI(Ice.Communicator communicator, DBEnvironmentI dbEnvObj, com.sleepycat.db.Db db, String name, boolean create)
    {
	_communicator = communicator;
	_dbEnvObj = dbEnvObj;
	_db = db;
	_name = name;
	_errorPrefix = "Freeze::DB(\"" + _name + "\"): ";
	_trace = _communicator.getProperties().getPropertyAsInt("Freeze.Trace.DB");

	if(_trace >= 1)
	{
	    _communicator.getLogger().trace("DB","opening database \"" + _name + "\" in environment \"" +
					    _dbEnvObj.getName() + "\"");
	}
	
	int flags = (create) ? com.sleepycat.db.Db.DB_CREATE : 0;
	try
	{
	    _db.open(_name, null, com.sleepycat.db.Db.DB_BTREE, flags, 0);
	    //TODO: FREEZE_DB_MODE)
	}
	catch(java.io.FileNotFoundException e)
	{
	    DBNotFoundException ex = new DBNotFoundException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "Db.open: " + e.getMessage();
	    throw ex;
	}
	catch(com.sleepycat.db.DbException e)
	{
	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "Db.open: " + e.getMessage();
	    throw ex;
	}
	
	_dbEnvObj.add(_name, this);
    }

    private Ice.Communicator _communicator;
    private int _trace = 0;

    private DBEnvironmentI _dbEnvObj;
    private com.sleepycat.db.Db _db;

    private String _name;
    private String _errorPrefix;
}
