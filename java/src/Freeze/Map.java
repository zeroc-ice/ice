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

public abstract class Map extends java.util.AbstractMap
{
    public
    Map(DB db)
    {
	_db = db;
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
	//
	// It's not necessary to close outstanding iterators.
	//
	// If it was it would be a problem - it doesn't change the Map
	// - therefore open iterators should not be invalidated
	// (according the Java spec).
	//
	// closeIterators();

	//
	// The number of records cannot be cached and then adjusted by
	// the member functions since the map would no longer work in
	// the presence of transactions - if a record is added (and
	// the size adjusted) and the transaction aborted then the
	// cached map size() would be incorrect.
	//

	return (int)_db.getNumberOfRecords();
    }

    public boolean
    containsValue(Object value)
    {
	boolean rc = super.containsValue(value);
	closeIterators();
	return rc;
    }

    public boolean
    containsKey(Object key)
    {
	//
	// It's not necessary to close outstanding iterators.
	//
	// If it was it would be a problem - it doesn't change the Map
	// - therefore open iterators should not be invalidated
	// (according the Java spec).
	//
	//closeIterators();

	byte[] k = encodeKey(key, _db.getCommunicator());
	return _db.contains(k);
    }

    public Object
    get(Object key)
    {
	//
	// It's not necessary to close outstanding iterators.
	//
	// If it was it would be a problem - it doesn't change the Map
	// - therefore open iterators should not be invalidated
	// (according the Java spec).
	//
	//closeIterators();

	byte[] k = encodeKey(key, _db.getCommunicator());
	try
	{
	    byte[] v = _db.get(k);
	    return decodeValue(v, _db.getCommunicator());
	}
	catch(DBNotFoundException e)
	{
	    return null;
	}
    }

    public Object
    put(Object key, Object value)
    {
	closeIterators();

	byte[] k = encodeKey(key, _db.getCommunicator());
	Object o;

	try
	{
	    byte[] v = _db.get(k);
	    o = decodeValue(v, _db.getCommunicator());
	}
	catch(DBNotFoundException e)
	{
	    o = null;
	}
	byte[] v = encodeValue(value, _db.getCommunicator());
	_db.put(k, v);

	return o;
    }

    public Object
    remove(Object key)
    {
	closeIterators();

	byte[] k = encodeKey(key, _db.getCommunicator());
	Object o;
	try
	{
	    byte[] v = _db.get(k);
	    o = decodeValue(v, _db.getCommunicator());
	}
	catch(DBNotFoundException e)
	{
	    o = null;
	}
	_db.del(k);
	return o;
    }

    //
    // Proprietary API calls. These are much faster than the
    // corresponding Java collections API methods since the unwanted
    // reads are avoided.
    //
    public void
    fastPut(Object key, Object value)
    {
	closeIterators();

	byte[] k = encodeKey(key, _db.getCommunicator());
	byte[] v = encodeValue(value, _db.getCommunicator());
	_db.put(k, v);
    }

    //
    // Returns true if the record was removed, false otherwise.
    //
    public boolean
    fastRemove(Object key)
    {
	closeIterators();

	byte[] k = encodeKey(key, _db.getCommunicator());
	try
	{
	    _db.del(k);
	}
	catch(Freeze.DBNotFoundException e)
	{
	    return false;
	}
	return true;
    }

    public void
    clear()
    {
	closeIterators();

	_db.clear();
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
		    EntryIterator p = new EntryIterator();
		    _iterators.add(p);
		    return p;
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
		    
		    Entry p = getEntry(entry.getKey());
		    return p != null && valEquals(p.getValue(), value);
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

		    Entry p = getEntry(entry.getKey());
		    if(p != null && valEquals(p.getValue(), value))
		    {
			closeIterators();
			byte[] k = encodeKey(p.getKey(), _db.getCommunicator());
			_db.del(k);
			return true;
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

    //
    // Because of the way that Berkeley DB cursors implement their
    // locking it's necessary to ensure that all cursors are closed
    // prior to doing a database operation otherwise self-deadlocks
    // will occur. See "Berkeley DB Transactional Data Store locking
    // conventions" section in the Berkeley DB reference guide for
    // details.
    //
    private void
    closeIterators()
    {
	java.util.Iterator p = _iterators.iterator();
	while(p.hasNext())
	{
	    EntryIterator q = (EntryIterator)p.next();
	    q.close();
	}

	//
	// This is more efficient than removing the list items element
	// by element in the iteration loop.
	//
	_iterators.clear();
    }

    private Entry
    getEntry(Object key)
    {
	closeIterators();

	byte[] k = encodeKey(key, _db.getCommunicator());
	byte[] v = _db.get(k);

	Object o = decodeValue(v, _db.getCommunicator());

	return new Entry(key, o);
    }

    private static boolean
    valEquals(Object o1, Object o2)
    {
        return (o1 == null ? o2 == null : o1.equals(o2));
    }

    class EntryIterator implements java.util.Iterator
    {
	EntryIterator()
	{
	    try
	    {
		_cursor = _db.getCursor();
		_next = getCurr();
	    }
	    catch(DBNotFoundException e)
	    {
		_next = null;
	    }
	    catch(DBException e)
	    {
		if(_cursor != null)
		{
		    try
		    {
			_cursor.close();
		    }
		    catch(DBException ignore)
		    {
			// Ignore
		    }
		}
		throw e;
	    }
	}
	
	public boolean
	hasNext()
	{
	    return _next != null;
	}
	
	public Object
	next()
	{
	    return nextEntry();
	}
	
	public void
	remove()
	{
	    if(_current == null)
	    {
		throw new IllegalStateException();
	    }
	    
	    //
	    // Clone the cursor so that error handling is simpler.
	    //
	    DBCursor clone = _cursor._clone();
	    
	    try
	    {
		//
		// If _next is null then the iterator is currently at
		// the end.
		//
		if(_next != null)
		{
		    clone.prev();
		}
		
		clone.del();
	    }
	    finally
	    {
		if(clone != null)
		{
		    try
		    {
			clone.close();
		    }
		    catch(DBException ignore)
		    {
			// Ignore
		    }
		}
	    }
	    
	    _current = null;
	}
	
	//
	// Extra operation.
	//
	public void
	close()
	{
	    DBCursor copy = _cursor;
	    
	    //
	    // Clear the internal iterator state.
	    //
	    _cursor = null;
	    _next = null;
	    _current = null;
	    
	    if(copy != null)
	    {
		copy.close();
	    }
	}
	
	private Entry
	nextEntry()
	{
	    if(_next == null)
	    {
		throw new java.util.NoSuchElementException();
	    }
	    
	    _current = _next;
	    
	    if(_cursor.next())
	    {
		_next = getCurr();
	    }
	    else
	    {
		_next = null;
	    }
	    
	    return _current;
	}
	
	private Entry
	getCurr()
	{
	    Freeze.KeyHolder k = new Freeze.KeyHolder();
	    Freeze.ValueHolder v = new Freeze.ValueHolder();
	    _cursor.curr(k, v);
	    
	    Object key = decodeKey(k.value, _db.getCommunicator());
	    Object value = decodeValue(v.value, _db.getCommunicator());
	    return new Entry(key, value);
	}
	
	private DBCursor _cursor = null;
	private Entry _next;
	private Entry _current;
    }
    
    static class Entry implements java.util.Map.Entry 
    {
	public
	Entry(Object key, Object value)
	{
	    _key = key;
	    _value = value;
	}
	
	public
	Entry(Map.Entry e)
	{
	    _key = e.getKey();
	    _value = e.getValue();
	}
	
	public Object
	getKey()
	{
	    return _key;
	}
	
	public Object
	getValue()
	{
	    return _value;
	}
	
	public Object
	setValue(Object value)
	{
	    throw new UnsupportedOperationException();
	}
	
	public boolean
	equals(Object o)
	{
	    if(!(o instanceof Map.Entry))
	    {
		return false;
	    }
	    Map.Entry e = (Map.Entry)o;
	    return eq(_key, e.getKey()) &&  eq(_value, e.getValue());
	}
	
	public int
	hashCode()
	{
	    Object v;
	    return ((_key   == null) ? 0 : _key.hashCode()) ^
	           ((_value == null) ? 0 : _value.hashCode());
	}
	
	public String
	toString()
	{
	    return _key + "=" + _value;
	}
	
	private /*static*/ boolean
	eq(Object o1, Object o2)
	{
	    return (o1 == null ? o2 == null : o1.equals(o2));
	}
	
	private Object _key;
	private Object _value;
    }
    
    private java.util.Set _entrySet;
    private DB _db;
    private java.util.List _iterators = new java.util.LinkedList();
}
