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
	return getCursorImpl(null);
    }
    
    synchronized public DBCursor
    getCursorAtKey(byte[] key)
    {
	return getCursorAtKeyImpl(null, key);
    }

    synchronized public void
    put(byte[] key, byte[] value)
    {
	putImpl(null, key, value);
    }

    synchronized public boolean
    contains(byte[] key)
    {
	return containsImpl(null, key);
    }

    synchronized public byte[]
    get(byte[] key)
    {
	return getImpl(null, key);
    }

    synchronized public void
    del(byte[] key)
    {
	delImpl(null, key);
    }

    synchronized public DBCursor
    getCursorWithTxn(DBTransaction txn)
    {
	assert txn != null;
	return getCursorImpl(((DBTransactionI)txn).getTxnId());
    }
    
    synchronized public DBCursor
    getCursorAtKeyWithTxn(DBTransaction txn, byte[] key)
    {
	assert txn != null;
	return getCursorAtKeyImpl(((DBTransactionI)txn).getTxnId(), key);
    }

    synchronized public void
    putWithTxn(DBTransaction txn, byte[] key, byte[] value)
    {
	assert txn != null;
	putImpl(((DBTransactionI)txn).getTxnId(), key, value);
    }

    synchronized public boolean
    containsWithTxn(DBTransaction txn, byte[] key)
    {
	assert txn != null;
	return containsImpl(((DBTransactionI)txn).getTxnId(), key);
    }

    synchronized public byte[]
    getWithTxn(DBTransaction txn, byte[] key)
    {
	assert txn != null;
	return getImpl(((DBTransactionI)txn).getTxnId(), key);
    }

    synchronized public void
    delWithTxn(DBTransaction txn, byte[] key)
    {
	assert txn != null;
	delImpl(((DBTransactionI)txn).getTxnId(), key);
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
	    ex.message = _errorPrefix + "Db.remove: " + e.getMessage();
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

    synchronized public void
    sync()
    {
	if(_db == null)
	{
	    return;
	}
	
	if(_trace >= 1)
	{
	    _communicator.getLogger().trace("DB", "synchronizing database \"" + _name + "\"");
	}

	try
	{
	    _db.sync(0);
	}
	catch(com.sleepycat.db.DbException e)
	{
	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "Db.sync: " + e.getMessage();
	    throw ex;
	}	
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

    DBI(Ice.Communicator communicator, DBEnvironmentI dbEnvObj, com.sleepycat.db.Db db, com.sleepycat.db.DbTxn txn,
	String name, boolean create)
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
            //
            // The signature for the open() method changed in version 4.1.
            // We use reflection to invoke it with the proper arguments.
            //
            java.lang.reflect.Method m;
            Class[] types;
            Object[] args;
            if(com.sleepycat.db.Db.DB_VERSION_MAJOR > 4 ||
               (com.sleepycat.db.Db.DB_VERSION_MAJOR == 4 && com.sleepycat.db.Db.DB_VERSION_MINOR >= 1))
            {
                types = new Class[6];
                types[0] = com.sleepycat.db.DbTxn.class;
                types[1] = String.class;
                types[2] = String.class;
                types[3] = Integer.TYPE;
                types[4] = Integer.TYPE;
                types[5] = Integer.TYPE;
                args = new Object[6];
                args[0] = txn;
                args[1] = _name;
                args[2] = null;
                args[3] = new Integer(com.sleepycat.db.Db.DB_BTREE);
                args[4] = new Integer(flags);
                args[5] = new Integer(0);
                //
                // Equivalent to:
                //
                //_db.open(null, _name, null, com.sleepycat.db.Db.DB_BTREE, flags, 0);
            }
            else
            {
                types = new Class[5];
                types[0] = String.class;
                types[1] = String.class;
                types[2] = Integer.TYPE;
                types[3] = Integer.TYPE;
                types[4] = Integer.TYPE;
                args = new Object[5];
                args[0] = _name;
                args[1] = null;
                args[2] = new Integer(com.sleepycat.db.Db.DB_BTREE);
                args[3] = new Integer(flags);
                args[4] = new Integer(0);
                //
                // Equivalent to:
                //
                //_db.open(_name, null, com.sleepycat.db.Db.DB_BTREE, flags, 0);
            }

            m = com.sleepycat.db.Db.class.getDeclaredMethod("open", types);
            m.invoke(_db, args);
	    //TODO: FREEZE_DB_MODE
	}
        catch(NoSuchMethodException e)
        {
	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = "reflection error";
	    throw ex;
        }
        catch(IllegalAccessException e)
        {
	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = "reflection error";
	    throw ex;
        }
        catch(IllegalArgumentException e)
        {
	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = "reflection error";
	    throw ex;
        }
        catch(java.lang.reflect.InvocationTargetException e)
        {
            Throwable t = e.getCause();
            if(t instanceof java.io.FileNotFoundException)
            {
                DBNotFoundException ex = new DBNotFoundException();
                ex.initCause(t);
                ex.message = _errorPrefix + "Db.open: " + t.getMessage();
                throw ex;
            }
            else if(t instanceof com.sleepycat.db.DbException)
            {
                DBException ex = new DBException();
                ex.initCause(t);
                ex.message = _errorPrefix + "Db.open: " + t.getMessage();
                throw ex;
            }
            else
            {
                DBException ex = new DBException();
                ex.initCause(t);
                ex.message = "Db.open: unexpected exception";
                throw ex;
            }
        }
	
	_dbEnvObj.add(_name, this);
    }

    private DBCursor
    getCursorImpl(com.sleepycat.db.DbTxn txn)
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
	    cursor = _db.cursor(txn, 0);
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
    
    private DBCursor
    getCursorAtKeyImpl(com.sleepycat.db.DbTxn txn, byte[] key)
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
	    cursor = _db.cursor(txn, 0);
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

    private void
    putImpl(com.sleepycat.db.DbTxn txn, byte[] key, byte[] value)
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
	    _db.put(txn, dbKey, dbData, 0);
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

    private boolean
    containsImpl(com.sleepycat.db.DbTxn txn, byte[] key)
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
	    int rc =_db.get(txn, dbKey, dbData, 0);
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

    private byte[]
    getImpl(com.sleepycat.db.DbTxn txn, byte[] key)
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
	    int rc =_db.get(txn, dbKey, dbData, 0);
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

    private void
    delImpl(com.sleepycat.db.DbTxn txn, byte[] key)
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
	    int rc =_db.del(txn, dbKey, 0);
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

    private Ice.Communicator _communicator;
    private int _trace = 0;

    private DBEnvironmentI _dbEnvObj;
    private com.sleepycat.db.Db _db;

    private String _name;
    private String _errorPrefix;
}
