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

class DBCursorI implements DBCursor
{
    public Ice.Communicator
    getCommunicator()
    {
	// immutable
	return _communicator;
    }

    synchronized public void
    curr(KeyHolder k, ValueHolder v)
    {
	if (_cursor == null)
	{
	    DBException ex = new DBException();
	    ex.message = _errorPrefix + "\"" + _name + "\" has been closed";
	    throw ex;
	}
	
	com.sleepycat.db.Dbt dbKey = new com.sleepycat.db.Dbt();
	com.sleepycat.db.Dbt dbData = new com.sleepycat.db.Dbt();
	
	if (_trace >= 1)
	{
	    _communicator.getLogger().trace("DB", "reading current value from database \"" + _name + "\"");
	}

	try
	{
	    int rc = _cursor.get(dbKey, dbData, com.sleepycat.db.Db.DB_CURRENT);
	    if (rc == com.sleepycat.db.Db.DB_NOTFOUND)
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

	//
	// Copy the data from the read key & data
	//
	k.value = dbKey.get_data();
	v.value = dbData.get_data();
    }

    synchronized public void
    set(byte[] v)
    {
	if (_cursor == null)
	{
	    DBException ex = new DBException();
	    ex.message = _errorPrefix + "\"" + _name + "\" has been closed";
	    throw ex;
	}

	com.sleepycat.db.Dbt dbKey = null;
	com.sleepycat.db.Dbt dbData = new com.sleepycat.db.Dbt(v);
	
	if (_trace >= 1)
	{
	    _communicator.getLogger().trace("DB", "setting current value in database \"" + _name + "\"");
	}

	try
	{
	    int rc = _cursor.put(dbKey, dbData, com.sleepycat.db.Db.DB_CURRENT);
	    /* Since the underlying data is btree this cannot occur.
	     *
	    if (rc == com.sleepycat.db.Db.DB_NOTFOUND)
	    {
		DBNotFoundException ex = new DBNotFoundException();
		ex.message = _errorPrefix + "Dbc.put: DB_NOTFOUND";
		throw ex;
	    }
	    */
	}
	catch(com.sleepycat.db.DbException e)
	{
	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "Dbc.put: " + e.getMessage();
	    throw ex;
	}
    }

    synchronized public boolean
    next()
    {
	if (_cursor == null)
	{
	    DBException ex = new DBException();
	    ex.message = _errorPrefix + "\"" + _name + "\" has been closed";
	    throw ex;
	}

	com.sleepycat.db.Dbt dbKey = new com.sleepycat.db.Dbt();
	dbKey.set_flags(com.sleepycat.db.Db.DB_DBT_PARTIAL);
	com.sleepycat.db.Dbt dbData = new com.sleepycat.db.Dbt();
	dbData.set_flags(com.sleepycat.db.Db.DB_DBT_PARTIAL);
	
	if (_trace >= 1)
	{
	    _communicator.getLogger().trace("DB", "moving to next value in database \"" + _name + "\"");
	}

	try
	{
	    int rc = _cursor.get(dbKey, dbData, com.sleepycat.db.Db.DB_NEXT);
	    if (rc == com.sleepycat.db.Db.DB_NOTFOUND)
	    {
		return false;
	    }
	}
	catch(com.sleepycat.db.DbException e)
	{
	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "Dbc.get: " + e.getMessage();
	    throw ex;
	}

	return true;
    }

    synchronized public boolean
    prev()
    {
	if (_cursor == null)
	{
	    DBException ex = new DBException();
	    ex.message = _errorPrefix + "\"" + _name + "\" has been closed";
	    throw ex;
	}

	com.sleepycat.db.Dbt dbKey = new com.sleepycat.db.Dbt();
	dbKey.set_flags(com.sleepycat.db.Db.DB_DBT_PARTIAL);
	com.sleepycat.db.Dbt dbData = new com.sleepycat.db.Dbt();
	dbData.set_flags(com.sleepycat.db.Db.DB_DBT_PARTIAL);
	
	if (_trace >= 1)
	{
	    _communicator.getLogger().trace("DB", "moving to previous value in database \"" + _name + "\"");
	}

	try
	{
	    int rc = _cursor.get(dbKey, dbData, com.sleepycat.db.Db.DB_PREV);
	    if (rc == com.sleepycat.db.Db.DB_NOTFOUND)
	    {
		return false;
	    }
	}
	catch(com.sleepycat.db.DbException e)
	{
	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "Dbc.get: " + e.getMessage();
	    throw ex;
	}

	return true;
    }

    synchronized public void
    del()
    {
	if (_cursor == null)
	{
	    DBException ex = new DBException();
	    ex.message = _errorPrefix + "\"" + _name + "\" has been closed";
	    throw ex;
	}

	if (_trace >= 1)
	{
	    _communicator.getLogger().trace("DB", "removing the current element in database \"" + _name + "\"");
	}

	try
	{
	    int rc = _cursor.del(0);
	    if (rc == com.sleepycat.db.Db.DB_KEYEMPTY)
	    {
		DBException ex = new DBException();
		ex.message = _errorPrefix + "Dbc.del: DB_KEYEMPTY";
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

    synchronized public DBCursor
    _clone()
    {
	if (_cursor == null)
	{
	    DBException ex = new DBException();
	    ex.message = _errorPrefix + "\"" + _name + "\" has been closed";
	    throw ex;
	}

	com.sleepycat.db.Dbc cursor;
	try
	{
	    cursor = _cursor.dup(com.sleepycat.db.Db.DB_POSITION);
	}
	catch(com.sleepycat.db.DbException e)
	{
	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "Dbc.close: " + e.getMessage();
	    throw ex;
	}
	return new DBCursorI(_communicator, _name, cursor);
    }

    synchronized public void
    close()
    {
	if (_cursor == null)
	{
	    return;
	}

	if (_trace >= 1)
	{
	    _communicator.getLogger().trace("DB", "closing cursor \"" + _name + "\"");
	}

	try
	{
	    _cursor.close();
	}
	catch(com.sleepycat.db.DbDeadlockException e)
	{
	    DBDeadlockException ex = new DBDeadlockException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "Dbc.close: " + e.getMessage();
	    throw ex;
	}
	catch(com.sleepycat.db.DbException e)
	{
	    DBException ex = new DBException();
	    ex.initCause(e);
	    ex.message = _errorPrefix + "Dbc.close: " + e.getMessage();
	    throw ex;
	}

	_cursor = null;
    }

    DBCursorI(Ice.Communicator communicator, String name, com.sleepycat.db.Dbc cursor)
    {
	_communicator = communicator;
	_name = name;
	_cursor = cursor;

	Ice.Properties properties = _communicator.getProperties();
	String value;

	value = properties.getProperty("Freeze.Trace.DB");
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

	_errorPrefix = "Freeze::DBCursor(\"" + _name + "\"): ";

	if (_trace >= 1)
	{
	    _communicator.getLogger().trace("DB", "creating cursor for \"" + _name + "\"");
	}
    }

    private Ice.Communicator _communicator;
    private int _trace = 0;

    private String _name;
    private String _errorPrefix;

    com.sleepycat.db.Dbc _cursor;
}
