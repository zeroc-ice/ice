// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
	
	_token = _connection.registerMap(this);
    }

    public void
    close()
    {
	close(false);
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
	    DatabaseException ex = new DatabaseException();
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
	    DatabaseException ex = new DatabaseException();
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
	    catch(DeadlockException e)
	    {
		if(_connection.dbTxn() != null)
		{
		    throw e;
		}
		else
		{
		    if(_connection.deadlockWarning())
		    {
			_connection.communicator().getLogger().warning
			    ("Deadlock in Freeze.Map.containsValue while iterating over Db \"" + _db.dbName() 
			     + "\"; retrying ...");
		    }

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
	    DatabaseException ex = new DatabaseException();
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
		("Freeze.Map", "checking key in Db \"" + _db.dbName() + "\"");
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
		    DeadlockException ex = new DeadlockException();
		    ex.initCause(e);
		    ex.message = _errorPrefix + "Db.get: " + e.getMessage();
		    throw ex;
		}
		else
		{
		    if(_connection.deadlockWarning())
		    {
			_connection.communicator().getLogger().warning
			    ("Deadlock in Freeze.Map.containsKey while reading Db \"" + _db.dbName() 
			     + "\"; retrying ...");
		    }

		    //
		    // Try again
		    //
		}
	    }
	    catch(com.sleepycat.db.DbException e)
	    {
		DatabaseException ex = new DatabaseException();
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
	    NotFoundException ex = new NotFoundException();
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
	    DatabaseException ex = new DatabaseException();
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
		    DeadlockException ex = new DeadlockException();
		    ex.initCause(e);
		    ex.message = _errorPrefix + "Db.truncate: " + e.getMessage();
		    throw ex;
		}
		else
		{
		    if(_connection.deadlockWarning())
		    {
			_connection.communicator().getLogger().warning
			    ("Deadlock in Freeze.Map.clear on Db \"" + _db.dbName() 
			     + "\"; retrying ...");
		    }
		    
		    //
		    // Try again
		    //
		}
	    }
	    catch(com.sleepycat.db.DbException e)
	    {
		DatabaseException ex = new DatabaseException();
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
	closeAllIteratorsExcept(null, false);
    }
    
    void
    closeAllIteratorsExcept(Object except, boolean finalizing)
    {
	synchronized(_iteratorList)
	{
	    java.util.Iterator p = _iteratorList.iterator();

	    while(p.hasNext())
	    {
		Object obj = p.next();
		if(obj != except)
		{
		    ((EntryIterator) obj).close(finalizing);
		}
	    }
	}
    }

    protected void
    finalize()
    {
	close(true);
    }

    //
    // The synchronization is only needed when finalizing is true
    //
    void 
    close(boolean finalizing)
    {
	synchronized(_connection)
	{
	    if(_db != null)
	    {
		closeAllIteratorsExcept(null, finalizing);
		try
		{
		    _db.close();
		}
		finally
		{
		    _db = null;
		    _connection.unregisterMap(_token);
		    _token = null;
		}
	    }
	}
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
	    DatabaseException ex = new DatabaseException();
	    ex.message = _errorPrefix + "\"" + _db.dbName() + "\" has been closed";
	    throw ex;
	}

	com.sleepycat.db.Dbt dbValue = new com.sleepycat.db.Dbt();

	if(_trace >= 1)
	{
	    _connection.communicator().getLogger().trace
		("Freeze.Map", "reading value from Db \"" + _db.dbName() + "\"");
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
		    DeadlockException ex = new DeadlockException();
		    ex.initCause(e);
		    ex.message = _errorPrefix + "Db.get: " + e.getMessage();
		    throw ex;
		}
		else
		{
		    if(_connection.deadlockWarning())
		    {
			_connection.communicator().getLogger().warning
			    ("Deadlock in Freeze.Map.getImpl while reading Db \"" + _db.dbName() 
			     + "\"; retrying ...");
		    }

		    //
		    // Try again
		    //
		}
	    }
	    catch(com.sleepycat.db.DbException e)
	    {
		DatabaseException ex = new DatabaseException();
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
	    DatabaseException ex = new DatabaseException();
	    ex.message = _errorPrefix + "\"" + _db.dbName() + "\" has been closed";
	    throw ex;
	}

	byte[] v = encodeValue(value, _connection.communicator());	
	com.sleepycat.db.Dbt dbValue = new com.sleepycat.db.Dbt(v);
	
	if(_trace >= 1)
	{
	    _connection.communicator().getLogger().trace
		("Freeze.Map", "writing value in Db \"" + _db.dbName() + "\"");
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
		    DeadlockException ex = new DeadlockException();
		    ex.initCause(e);
		    ex.message = _errorPrefix + "Db.put: " + e.getMessage();
		    throw ex;
		}
		else
		{
		    if(_connection.deadlockWarning())
		    {
			_connection.communicator().getLogger().warning
			    ("Deadlock in Freeze.Map.putImpl while writing into Db \"" + _db.dbName() 
			     + "\"; retrying ...");
		    }

		    //
		    // Try again
		    //
		}
	    }
	    catch(com.sleepycat.db.DbException e)
	    {
		DatabaseException ex = new DatabaseException();
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
	    DatabaseException ex = new DatabaseException();
	    ex.message = _errorPrefix + "\"" + _db.dbName() + "\" has been closed";
	    throw ex;
	}

	if(_trace >= 1)
	{
	    _connection.communicator().getLogger().trace
		("Freeze.Map", "deleting value from Db \"" + _db.dbName() + "\"");
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
		    DeadlockException ex = new DeadlockException();
		    ex.initCause(e);
		    ex.message = _errorPrefix + "Db.del: " + e.getMessage();
		    throw ex;
		}
		else
		{
		    if(_connection.deadlockWarning())
		    {
			_connection.communicator().getLogger().warning
			    ("Deadlock in Freeze.Map.removeImpl while writing into Db \"" + _db.dbName() 
			     + "\"; retrying ...");
		    }

		    //
		    // Try again
		    //
		}
	    }
	    catch(com.sleepycat.db.DbException e)
	    {
		DatabaseException ex = new DatabaseException();
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
		    ("Freeze.Map", "starting transaction for cursor on Db \"" + _db.dbName() + "\"");
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
		DeadlockException ex = new DeadlockException();
		ex.initCause(dx);
		ex.message = _errorPrefix + "EntryIterator constructor: " + dx.getMessage();
		throw ex;
	    }
	    catch(com.sleepycat.db.DbException dx)
	    {
		DatabaseException ex = new DatabaseException();
		ex.initCause(dx);
		ex.message = _errorPrefix + "EntryIterator constructor: " + dx.getMessage();
		throw ex;
	    }

	    synchronized(_iteratorList)
	    {
		_iteratorList.addFirst(this);
		java.util.Iterator p = _iteratorList.iterator();
		p.next();
		_iteratorListToken = p;

	    }
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
		    DeadlockException ex = new DeadlockException();
		    ex.initCause(dx);
		    ex.message = _errorPrefix + "Dbc.get: " + dx.getMessage();
		    throw ex;
		}
		catch(com.sleepycat.db.DbException dx)
		{
		    DatabaseException ex = new DatabaseException();
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
		closeAllIteratorsExcept(this, false);
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
		    DeadlockException ex = new DeadlockException();
		    ex.initCause(e);
		    ex.message = _errorPrefix + "Dbc.del: " + e.getMessage();
		    throw ex;
		}
		catch(com.sleepycat.db.DbException e)
		{ 
		    DatabaseException ex = new DatabaseException();
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
		    DeadlockException ex = new DeadlockException();
		    ex.initCause(e);
		    ex.message = _errorPrefix + "EntryIterator.remove: " + e.getMessage();
		    throw ex;
		}
		catch(com.sleepycat.db.DbException e)
		{ 
		    DatabaseException ex = new DatabaseException();
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
	    close(false);
	}

	//
	// The synchronized is needed because this method can be called 
	// concurrently by Connection, Map and Map.EntryIterator finalizers.
	//
	synchronized void
	close(boolean finalizing)
	{
	    if(finalizing && (_cursor != null || _txn != null) && _connection.closeInFinalizeWarning())
	    {
		_connection.communicator().getLogger().warning
		    ("finalize() closing a live iterator on Map \"" + _db.dbName() + "\"; the application "
		     + "should have closed it earlier by calling Map.EntryIterator.close(), "
		     + "Map.closeAllIterators(), Map.close(), Connection.close(), or (if also "
		     + "leaking a transaction) Transaction.commit() or Transaction.rollback()");
	    }
	 
	    if(_iteratorListToken != null)
	    {
		synchronized(_iteratorList)
		{
		    _iteratorListToken.remove();
		    _iteratorListToken = null;
		}
	    }
   
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
			("Freeze.Map", "committing transaction for cursor on Db \"" + _db.dbName() + "\"");
		}

		try
		{
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
		    _txn = null;
		}
	    }   
	}
	
	//
	// An alias for close()
	//
	public void
	destroy()
	{
	    close();
	}

	protected void
        finalize()
        {
            close(true);
        }

	void
	setValue(Map.Entry entry, Object value)
	{
	    if(_txn != null)
	    {
		closeAllIteratorsExcept(this, false);
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
		    DeadlockException ex = new DeadlockException();
		    ex.initCause(e);
		    ex.message = _errorPrefix + "Dbc.put: " + e.getMessage();
		    throw ex;
		}
		catch(com.sleepycat.db.DbException e)
		{ 
		    DatabaseException ex = new DatabaseException();
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
			NotFoundException ex = new NotFoundException();
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
		    DeadlockException ex = new DeadlockException();
		    ex.initCause(e);
		    ex.message = _errorPrefix + "EntryIterator.setValue: " + e.getMessage();
		    throw ex;
		}
		catch(com.sleepycat.db.DbException e)
		{ 
		    DatabaseException ex = new DatabaseException();
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
		DeadlockException ex = new DeadlockException();
		ex.initCause(e);
		ex.message = _errorPrefix + "Dbc.close: " + e.getMessage();
		throw ex;
	    }
	    catch(com.sleepycat.db.DbException e)
	    { 
		DatabaseException ex = new DatabaseException();
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
			("Freeze.Map", "rolling back transaction for cursor on Db \"" 
			 + _db.dbName() + "\"");
		}

		try
		{
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
		    _txn = null;
		}
	    }
	}

	private com.sleepycat.db.DbTxn _txn;
        private com.sleepycat.db.Dbc _cursor;
        private Entry _current;
        private Entry _lastReturned;
	private java.util.Iterator _iteratorListToken;
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
    private java.util.Iterator _token;
    private java.util.Set _entrySet;
    private LinkedList _iteratorList = new LinkedList();
    private SharedDb _db;
    private String _errorPrefix;
    private int _trace;
}
