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

public abstract class Map extends java.util.AbstractMap
{
    public
    Map(Connection connection, String dbName, boolean createDb)
    {
	_connection = (ConnectionI) connection;
	_errorPrefix = "Freeze DB DbEnv(\"" + _connection.envName() + "\") Db(\"" + dbName + "\") :";
	_trace = _connection.trace();
	_db = SharedDb.get(_connection, dbName, createDb);
	
	_connection.registerMap(this);
    }

    public void
    close()
    {
	if(_db != null)
	{
	    closeAllIterators();
	    try
	    {
		_db.close();
	    }
	    finally
	    {
		_db = null;
	    }
	}
    }

    //
    // A concrete implementation of a Freeze.Map must provide
    // implementations of the following to encode & decode the
    // key/value pairs.
    //
    public abstract byte[] encodeKey(Object o, Ice.Communicator communicator);
    public abstract Object decodeKey(byte[] b, Ice.Communicator communicator);
    public abstract byte[] encodeValue(Object o, Ice.Communicator communicator);
    public abstract Object decodeValue(byte[] b, Ice.Communicator communicator);

    public int
    size()
    {
	if(_db == null)
	{
	    DBException ex = new DBException();
	    ex.message = _errorPrefix + "\"" + _db.dbName() + "\" has been closed";
	    throw ex;
	}

	//
	// The number of records cannot be cached and then adjusted by
	// the member functions since the map would no longer work in
	// the presence of transactions - if a record is added (and
	// the size adjusted) and the transaction aborted then the
	// cached map size() would be incorrect.
	//
	
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

    public boolean
    containsValue(Object value)
    {
	for(;;)
	{
	    EntryIterator p = null;
	    try
	    {
		p = (EntryIterator)entrySet().iterator();

		if(value == null)
		{
		    while(p.hasNext())
		    {
			Entry e = (Entry)p.next();
			if(e.getValue() == null)
			{
			    p.close();
			    return true;
			}
		    }
		} 
		else 
		{
		    while(p.hasNext()) 
		    {
			Entry e = (Entry)p.next();
			if(value.equals(e.getValue()))
			{
			    p.close();
			    return true;
			}
		    }
		}
		return false;
	    }
	    catch(DBDeadlockException e)
	    {
		if(_connection.dbTxn() != null)
		{
		    throw e;
		}
		else
		{
		    //
		    // Try again
		    //
		}
	    }
	    finally
	    {
		if(p != null)
		{
		    p.close();
		}
	    }
	}
    }

    public boolean
    containsKey(Object key)
    {
	if(_db == null)
	{
	    DBException ex = new DBException();
	    ex.message = _errorPrefix + "\"" + _db.dbName() + "\" has been closed";
	    throw ex;
	}

	byte[] k = encodeKey(key, _connection.communicator());

	com.sleepycat.db.Dbt dbKey = new com.sleepycat.db.Dbt(k);

	com.sleepycat.db.Dbt dbValue = new com.sleepycat.db.Dbt();
	dbValue.set_flags(com.sleepycat.db.Db.DB_DBT_PARTIAL);

	if(_trace >= 1)
	{
	    _connection.communicator().getLogger().trace
		("DB", "checking key in database \"" + _db.dbName() + "\"");
	}

	for(;;)
	{
	    try
	    {
		int rc = _db.get(_connection.dbTxn(), dbKey, dbValue, 0);
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
		if(_connection.dbTxn() != null)
		{
		    DBDeadlockException ex = new DBDeadlockException();
		    ex.initCause(e);
		    ex.message = _errorPrefix + "Db.get: " + e.getMessage();
		    throw ex;
		}
		else
		{
		    //
		    // Try again
		    //
		}
	    }
	    catch(com.sleepycat.db.DbException e)
	    {
		DBException ex = new DBException();
		ex.initCause(e);
		ex.message = _errorPrefix + "Db.get: " + e.getMessage();
		throw ex;
	    }
	}
    }

    public Object
    get(Object key)
    {
	byte[] k = encodeKey(key, _connection.communicator());
	com.sleepycat.db.Dbt dbKey = new com.sleepycat.db.Dbt(k);
	byte[] v = getImpl(dbKey);
	if(v == null)
	{
	    return null;
	}
	else
	{
	    return decodeValue(v, _connection.communicator());
	}
    }

    public Object
    put(Object key, Object value)
    {
	byte[] k = encodeKey(key, _connection.communicator());
	com.sleepycat.db.Dbt dbKey = new com.sleepycat.db.Dbt(k);
	byte[] v = getImpl(dbKey);
	Object o = null;
	if(v != null)
	{
	    o = decodeValue(v, _connection.communicator());
	}
	putImpl(dbKey, value);
	return o;
    }

    public Object
    remove(Object key)
    {
	byte[] k = encodeKey(key, _connection.communicator());
	com.sleepycat.db.Dbt dbKey = new com.sleepycat.db.Dbt(k);
	byte[] v = getImpl(dbKey);
	
	if(v != null && removeImpl(dbKey))
	{
	    return decodeValue(v, _connection.communicator());
	}
	else
	{
	    DBNotFoundException ex = new DBNotFoundException();
	    ex.message = _errorPrefix + "Db.del: DB_NOTFOUND";
	    throw ex;
	}
    }

    //
    // Proprietary API calls. These are much faster than the
    // corresponding Java collections API methods since the unwanted
    // reads are avoided.
    //
    public void
    fastPut(Object key, Object value)
    {
	byte[] k = encodeKey(key, _connection.communicator());
	com.sleepycat.db.Dbt dbKey = new com.sleepycat.db.Dbt(k);
	putImpl(dbKey, value);
    }

    //
    // Returns true if the record was removed, false otherwise.
    //
    public boolean
    fastRemove(Object key)
    {
	byte[] k = encodeKey(key, _connection.communicator());
	com.sleepycat.db.Dbt dbKey = new com.sleepycat.db.Dbt(k);
	return removeImpl(dbKey);
    }

    public void
    clear()
    {
	if(_db == null)
	{
	    DBException ex = new DBException();
	    ex.message = _errorPrefix + "\"" + _db.dbName() + "\" has been closed";
	    throw ex;
	}

	com.sleepycat.db.DbTxn txn = _connection.dbTxn();

	for(;;)
	{
	    try
	    {
		_db.truncate(txn, txn != null ? 0 : com.sleepycat.db.Db.DB_AUTO_COMMIT);
		break;
	    }
	    catch(com.sleepycat.db.DbDeadlockException e)
	    {
		if(txn != null)
		{
		    DBDeadlockException ex = new DBDeadlockException();
		    ex.initCause(e);
		    ex.message = _errorPrefix + "Db.truncate: " + e.getMessage();
		    throw ex;
		}
		else
		{
		    //
		    // Try again
		    //
		}
	    }
	    catch(com.sleepycat.db.DbException e)
	    {
		DBException ex = new DBException();
		ex.initCause(e);
		ex.message = _errorPrefix + "Db.truncate: " + e.getMessage();
		throw ex;
	    }
	}
    }
	
    public java.util.Set
    entrySet()
    {
        if(_entrySet == null)
	{
            _entrySet = new java.util.AbstractSet()
	    {
		public java.util.Iterator
		iterator()
		{
		    return new EntryIterator();
		}
		
		public boolean
		contains(Object o)
		{
		    if(!(o instanceof Map.Entry))
		    {
			return false;
		    }
		    Map.Entry entry = (Map.Entry)o;
		    Object value = entry.getValue();
		    
		    byte[] v = getImpl(entry.getDbKey());
		    return v != null && valEquals(decodeValue(v, _connection.communicator()), value);
		}
		
		public boolean
		remove(Object o)
		{
		    if(!(o instanceof Map.Entry))
		    {
			return false;
		    }
		    Map.Entry entry = (Map.Entry)o;
		    Object value = entry.getValue();

		    byte[] v = getImpl(entry.getDbKey());
		    if(v != null && valEquals(decodeValue(v, _connection.communicator()), value))
		    {
			return removeImpl(entry.getDbKey());
		    }
		    return false;
		}
		
		public int
		size()
		{
		    return Map.this.size();
		}
		
		public void
		clear()
		{
		    Map.this.clear();
		}
            };
        }

        return _entrySet;
    }

    public void
    closeAllIterators()
    {
	closeAllIteratorsExcept(null);
    }
    
    void
    closeAllIteratorsExcept(Object except)
    {
	java.util.Iterator p = _iteratorList.iterator();
	while(p.hasNext())
	{
	    Object o = ((java.lang.ref.WeakReference) p.next()).get();
	    if(o != except && o != null)
	    {
		((EntryIterator) o).close();
	    }
	    if(o == null || o != except)
	    {
		p.remove();
	    }
	}
    }

    protected void
    finalize()
    {
	close();
    }

    private static boolean
    valEquals(Object o1, Object o2)
    {
        return (o1 == null ? o2 == null : o1.equals(o2));
    }

    private byte[]
    getImpl(com.sleepycat.db.Dbt dbKey)
    {
	if(_db == null)
	{
	    DBException ex = new DBException();
	    ex.message = _errorPrefix + "\"" + _db.dbName() + "\" has been closed";
	    throw ex;
	}

	com.sleepycat.db.Dbt dbValue = new com.sleepycat.db.Dbt();

	if(_trace >= 1)
	{
	    _connection.communicator().getLogger().trace
		("DB", "reading value from database \"" + _db.dbName() + "\"");
	}

	for(;;)
	{
	    try
	    {
		int rc = _db.get(_connection.dbTxn(), dbKey, dbValue, 0);
		if(rc == com.sleepycat.db.Db.DB_NOTFOUND)
		{
		    return null;
		}
		else
		{
		    return dbValue.get_data();
		}
	    }
	    catch(com.sleepycat.db.DbDeadlockException e)
	    {
		if(_connection.dbTxn() != null)
		{
		    DBDeadlockException ex = new DBDeadlockException();
		    ex.initCause(e);
		    ex.message = _errorPrefix + "Db.get: " + e.getMessage();
		    throw ex;
		}
		else
		{
		    //
		    // Try again
		    //
		}
	    }
	    catch(com.sleepycat.db.DbException e)
	    {
		DBException ex = new DBException();
		ex.initCause(e);
		ex.message = _errorPrefix + "Db.get: " + e.getMessage();
		throw ex;
	    }
	}
    }

    private void
    putImpl(com.sleepycat.db.Dbt dbKey, Object value)
    {
	if(_db == null)
	{
	    DBException ex = new DBException();
	    ex.message = _errorPrefix + "\"" + _db.dbName() + "\" has been closed";
	    throw ex;
	}

	byte[] v = encodeValue(value, _connection.communicator());	
	com.sleepycat.db.Dbt dbValue = new com.sleepycat.db.Dbt(v);
	
	if(_trace >= 1)
	{
	    _connection.communicator().getLogger().trace
		("DB", "writing value in database \"" + _db.dbName() + "\"");
	}

	com.sleepycat.db.DbTxn txn = _connection.dbTxn();
	if(txn == null)
	{
	    closeAllIterators();
	}

	for(;;)
	{
	    try
	    {
		_db.put(txn, dbKey, dbValue, txn != null ? 0 : com.sleepycat.db.Db.DB_AUTO_COMMIT);
		break;
	    }
	    catch(com.sleepycat.db.DbDeadlockException e)
	    {
		if(txn != null)
		{
		    DBDeadlockException ex = new DBDeadlockException();
		    ex.initCause(e);
		    ex.message = _errorPrefix + "Db.put: " + e.getMessage();
		    throw ex;
		}
		else
		{
		    //
		    // Try again
		    //
		}
	    }
	    catch(com.sleepycat.db.DbException e)
	    {
		DBException ex = new DBException();
		ex.initCause(e);
		ex.message = _errorPrefix + "Db.put: " + e.getMessage();
		throw ex;
	    }
	}
    }

    private boolean
    removeImpl(com.sleepycat.db.Dbt dbKey)
    {
	if(_db == null)
	{
	    DBException ex = new DBException();
	    ex.message = _errorPrefix + "\"" + _db.dbName() + "\" has been closed";
	    throw ex;
	}

	if(_trace >= 1)
	{
	    _connection.communicator().getLogger().trace
		("DB", "deleting value from database \"" + _db.dbName() + "\"");
	}

	com.sleepycat.db.DbTxn txn = _connection.dbTxn();
	if(txn == null)
	{
	    closeAllIterators();
	}

	for(;;)
	{
	    try
	    {
		int rc = _db.del(txn, dbKey, txn != null ? 0 : com.sleepycat.db.Db.DB_AUTO_COMMIT);
		return (rc == 0);
	    }
	    catch(com.sleepycat.db.DbDeadlockException e)
	    {
		if(txn != null)
		{
		    DBDeadlockException ex = new DBDeadlockException();
		    ex.initCause(e);
		    ex.message = _errorPrefix + "Db.del: " + e.getMessage();
		    throw ex;
		}
		else
		{
		    //
		    // Try again
		    //
		}
	    }
	    catch(com.sleepycat.db.DbException e)
	    {
		DBException ex = new DBException();
		ex.initCause(e);
		ex.message = _errorPrefix + "Db.del: " + e.getMessage();
		throw ex;
	    }
	}
    }

    /**
     *
     * The entry iterator class needs to be public to allow clients to
     * explicitly close the iterator and free resources allocated for
     * the iterator as soon as possible.
     *
     **/
    public class EntryIterator implements java.util.Iterator
    {
        EntryIterator()
        {
	    if(_trace >= 3)
	    {
		_connection.communicator().getLogger().trace
		    ("DB", "starting transaction for cursor on database \"" + _db.dbName() + "\"");
	    }

	    try
	    {
		com.sleepycat.db.DbTxn txn = _connection.dbTxn();
		if(txn == null)
		{
		    //
		    // Start transaction
		    //
		    txn = _connection.dbEnv().txn_begin(null, 0);
		    _txn = txn;
		}
		
		//
		// Open cursor with this transaction
		//
		_cursor = _db.cursor(txn, 0);
	    }
	    catch(com.sleepycat.db.DbDeadlockException dx)
	    {
		dead();
		DBDeadlockException ex = new DBDeadlockException();
		ex.initCause(dx);
		ex.message = _errorPrefix + "EntryIterator constructor: " + dx.getMessage();
		throw ex;
	    }
	    catch(com.sleepycat.db.DbException dx)
	    {
		DBException ex = new DBException();
		ex.initCause(dx);
		ex.message = _errorPrefix + "EntryIterator constructor: " + dx.getMessage();
		throw ex;
	    }

	    _iteratorList.add(new java.lang.ref.WeakReference(this));
        }

        public boolean
        hasNext()
        {
	    if(_current == null || _current == _lastReturned)
	    {
		//
		// Move _cursor, set _current
		//
		
		com.sleepycat.db.Dbt dbKey = new com.sleepycat.db.Dbt();
		com.sleepycat.db.Dbt dbValue = new com.sleepycat.db.Dbt();
		
		try
		{
		    if(_cursor.get(dbKey, dbValue, com.sleepycat.db.Db.DB_NEXT) == 0)
		    {
			_current = new Entry(this, Map.this, _connection.communicator(), 
					     dbKey, dbValue.get_data());
			return true;
		    }
		    else
		    {
			return false;
		    }
		}
		catch(com.sleepycat.db.DbDeadlockException dx)
		{
		    dead();
		    DBDeadlockException ex = new DBDeadlockException();
		    ex.initCause(dx);
		    ex.message = _errorPrefix + "Dbc.get: " + dx.getMessage();
		    throw ex;
		}
		catch(com.sleepycat.db.DbException dx)
		{
		    DBException ex = new DBException();
		    ex.initCause(dx);
		    ex.message = _errorPrefix + "Dbc.get: " + dx.getMessage();
		    throw ex;
		}
	    }
	    else
	    {
		return true;
	    }
        }
	
        public Object
        next()
        {
	    if(hasNext())
	    {
		_lastReturned = _current;
		return _lastReturned;
	    }
	    else
	    {
		throw new java.util.NoSuchElementException();
	    }
        }

        public void
        remove()
        {
	    if(_txn != null)
	    {
		closeAllIteratorsExcept(this);
	    }

	    //
	    // Removes the last object returned by next()
	    //
	    if(_lastReturned == null)
	    {
		throw new IllegalStateException();
	    }
	    
	    if(_lastReturned == _current)
	    {
		try
		{
		    if(_cursor.del(0) == com.sleepycat.db.Db.DB_KEYEMPTY)
		    {
			throw new IllegalStateException();
		    }
		}
		catch(com.sleepycat.db.DbDeadlockException e)
		{
		    dead();
		    DBDeadlockException ex = new DBDeadlockException();
		    ex.initCause(e);
		    ex.message = _errorPrefix + "Dbc.del: " + e.getMessage();
		    throw ex;
		}
		catch(com.sleepycat.db.DbException e)
		{ 
		    DBException ex = new DBException();
		    ex.initCause(e);
		    ex.message = _errorPrefix + "Dbc.del: " + e.getMessage();
		    throw ex;
		}
	    }
	    else
	    {
		//
		// Duplicate the cursor and move the _lastReturned 
		// element to delete it (using the duplicate)
		//
		
		com.sleepycat.db.Dbc clone = null;

		try
		{
		    clone = _cursor.dup(com.sleepycat.db.Db.DB_POSITION);

		    //
		    // No interested in data
		    //
		    com.sleepycat.db.Dbt dbValue = new com.sleepycat.db.Dbt();
		    dbValue.set_flags(com.sleepycat.db.Db.DB_DBT_PARTIAL);
		    
		    int rc = clone.get(_lastReturned.getDbKey(), dbValue, com.sleepycat.db.Db.DB_SET);

		    if(rc == com.sleepycat.db.Db.DB_NOTFOUND)	
		    {
			throw new IllegalStateException();
		    }
		    if(clone.del(0) == com.sleepycat.db.Db.DB_KEYEMPTY)
		    {
			throw new IllegalStateException();
		    }
		}
		catch(com.sleepycat.db.DbDeadlockException e)
		{
		    dead();
		    DBDeadlockException ex = new DBDeadlockException();
		    ex.initCause(e);
		    ex.message = _errorPrefix + "EntryIterator.remove: " + e.getMessage();
		    throw ex;
		}
		catch(com.sleepycat.db.DbException e)
		{ 
		    DBException ex = new DBException();
		    ex.initCause(e);
		    ex.message = _errorPrefix + "EntryIterator.remove: " + e.getMessage();
		    throw ex;
		}
		finally
		{
		    if(clone != null)
		    {
			closeCursor(clone);
		    }
		}
	    }
        }

        //
        // Extra operations.
        //
        public void
        close()
        {
	    if(_cursor != null)	
	    {
		com.sleepycat.db.Dbc cursor = _cursor;
		_cursor = null;
		closeCursor(cursor);
	    }
	    
	    if(_txn != null)
	    {
		if(_trace >= 3)
		{
		    _connection.communicator().getLogger().trace
			("DB", "committing transaction for cursor on database \"" + _db.dbName() + "\"");
		}

		try
		{
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
		    _txn = null;
		}
	    }   
	}
	

	protected void
        finalize()
        {
            close();
        }

	void
	setValue(Map.Entry entry, Object value)
	{
	    if(_txn != null)
	    {
		closeAllIteratorsExcept(this);
	    }

	    //
	    // Are we trying to update the current value?
	    //
	    if(_current == entry)
	    {
		//
		// Yes, update it directly
		//
		byte[] v = encodeValue(value, _connection.communicator());
		com.sleepycat.db.Dbt dbValue = new com.sleepycat.db.Dbt(v);
		
		try
		{
		    _cursor.put(entry.getDbKey(), dbValue, com.sleepycat.db.Db.DB_CURRENT);
		}
		catch(com.sleepycat.db.DbDeadlockException e)
		{
		    dead();
		    DBDeadlockException ex = new DBDeadlockException();
		    ex.initCause(e);
		    ex.message = _errorPrefix + "Dbc.put: " + e.getMessage();
		    throw ex;
		}
		catch(com.sleepycat.db.DbException e)
		{ 
		    DBException ex = new DBException();
		    ex.initCause(e);
		    ex.message = _errorPrefix + "Dbc.put: " + e.getMessage();
		    throw ex;
		}
	    }
	    else
	    {
		//
		// Duplicate the cursor and move the entry
		// element to update it (using the duplicate cursor)
		//
		
		com.sleepycat.db.Dbc clone = null;

		try
		{
		    clone = _cursor.dup(com.sleepycat.db.Db.DB_POSITION);

		    //
		    // Not interested in data
		    //
		    com.sleepycat.db.Dbt dummy = new com.sleepycat.db.Dbt();
		    dummy.set_flags(com.sleepycat.db.Db.DB_DBT_PARTIAL);
		    
		    int rc = clone.get(entry.getDbKey(), dummy, com.sleepycat.db.Db.DB_SET);

		    if(rc == com.sleepycat.db.Db.DB_NOTFOUND)	
		    {
			DBNotFoundException ex = new DBNotFoundException();
			ex.message = _errorPrefix + "Dbc.get: DB_NOTFOUND";
			throw ex;
		    }
		   
		    byte[] v = encodeValue(value, _connection.communicator());
		    com.sleepycat.db.Dbt dbValue = new com.sleepycat.db.Dbt(v);
		    clone.put(entry.getDbKey(), dbValue, com.sleepycat.db.Db.DB_CURRENT);
		}
		catch(com.sleepycat.db.DbDeadlockException e)
		{
		    dead();
		    DBDeadlockException ex = new DBDeadlockException();
		    ex.initCause(e);
		    ex.message = _errorPrefix + "EntryIterator.setValue: " + e.getMessage();
		    throw ex;
		}
		catch(com.sleepycat.db.DbException e)
		{ 
		    DBException ex = new DBException();
		    ex.initCause(e);
		    ex.message = _errorPrefix + "EntryIterator.setValue: " + e.getMessage();
		    throw ex;
		}
		finally
		{
		    if(clone != null)
		    {
			closeCursor(clone);
		    }
		}
	    }
	}

	private void
	closeCursor(com.sleepycat.db.Dbc cursor)
	{
	    try
	    {
		cursor.close();
	    }
	    catch(com.sleepycat.db.DbDeadlockException e)
	    {
		dead();
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
	}

	private void
	dead()
	{
	    if(_cursor != null)	
	    {
		com.sleepycat.db.Dbc cursor = _cursor;
		_cursor = null;
		closeCursor(cursor);
	    }

	    if(_txn != null)
	    {
		if(_trace >= 3)
		{
		    _connection.communicator().getLogger().trace
			("DB", "rolling back transaction for cursor on database \"" 
			 + _db.dbName() + "\"");
		}

		try
		{
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
		    _txn = null;
		}
	    }
	}

	private com.sleepycat.db.DbTxn _txn;
        private com.sleepycat.db.Dbc _cursor;
        private Entry _current;
        private Entry _lastReturned;
    }

    static class Entry implements java.util.Map.Entry 
    {
        public
        Entry(Map.EntryIterator iterator, Map map, Ice.Communicator communicator, 
	      com.sleepycat.db.Dbt dbKey, byte[] valueBytes)
        {
            _iterator = iterator;
	    _map = map;
	    _communicator = communicator;
            _dbKey = dbKey;
	    _valueBytes = valueBytes;
        }

	public Object
	getKey()
	{
            if(!_haveKey)
            {
                assert(_dbKey != null);
                _key = _map.decodeKey(_dbKey.get_data(), _communicator);
                _haveKey = true;
            }
	    return _key;
	}

	public Object
	getValue()
	{
            if(!_haveValue)
            {
                assert(_valueBytes != null);
                _value = _map.decodeValue(_valueBytes, _communicator);
                _haveValue = true;
		//
		// Not needed anymore
		//
		_valueBytes = null;
            }
	    return _value;
	}

	public Object
	setValue(Object value)
	{
	    Object old = getValue();
	    _iterator.setValue(this, value);
	    _value = value;
            _haveValue = true;
	    return old;
	}

	public boolean
	equals(Object o)
	{
	    if(!(o instanceof Map.Entry))
	    {
		return false;
	    }
	    Map.Entry e = (Map.Entry)o;
	    return eq(getKey(), e.getKey()) && eq(getValue(), e.getValue());
	}

	public int
	hashCode()
	{
	    return ((getKey()   == null) ? 0 : getKey().hashCode()) ^
	           ((getValue() == null) ? 0 : getValue().hashCode());
	}

	public String
	toString()
	{
	    return getKey() + "=" + getValue();
	}

	com.sleepycat.db.Dbt
        getDbKey()
	{
	    return _dbKey;
	}

	private /*static*/ boolean
	eq(Object o1, Object o2)
	{
	    return (o1 == null ? o2 == null : o1.equals(o2));
	}

	private Map.EntryIterator _iterator;
	private Map _map;
	private Ice.Communicator _communicator;
	private com.sleepycat.db.Dbt _dbKey;
	private byte[] _valueBytes;
	private Object _key;
        private boolean _haveKey = false;
	private Object _value;
        private boolean _haveValue = false;
    }

    public static class Patcher implements IceInternal.Patcher
    {
        public
        Patcher(String type)
        {
            this.type = type;
        }

        public void
        patch(Ice.Object v)
        {
            value = v;
        }

        public String
        type()
        {
            return this.type;
        }

        public Ice.Object
        value()
        {
            return this.value;
        }

        public String type;
        public Ice.Object value;
    }
    
    private ConnectionI _connection;
    private java.util.Set _entrySet;
    private java.util.List _iteratorList = new java.util.LinkedList();
    private SharedDb _db;
    private String _errorPrefix;
    private int _trace;
}
