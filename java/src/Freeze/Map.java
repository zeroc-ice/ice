// **********************************************************************
//
// Copyright (c) 2002
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
    Map(DB db)
    {
	_db = db;
        _communicator = db.getCommunicator();
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
	//
	// It's not necessary to close outstanding iterators.
	//
	// If it was it would be a problem - it doesn't change the Map
	// - therefore open iterators should not be invalidated
	// (according the Java spec).
	//
	// closeIterators();

	EntryIterator p = (EntryIterator)entrySet().iterator();
	if(value == null)
	{
	    while(p.hasNext())
	    {
		Entry e = (Entry)p.next();
		if(e.getValue() == null)
		{
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
		    return true;
		}
	    }
	}
	p.close();

	return false;
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

	byte[] k = encodeKey(key, _communicator);
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

	byte[] k = encodeKey(key, _communicator);
	try
	{
	    byte[] v = _db.get(k);
	    return decodeValue(v, _communicator);
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

	byte[] k = encodeKey(key, _communicator);
	Object o;

	try
	{
	    byte[] v = _db.get(k);
	    o = decodeValue(v, _communicator);
	}
	catch(DBNotFoundException e)
	{
	    o = null;
	}
	byte[] v = encodeValue(value, _communicator);
	_db.put(k, v);

	return o;
    }

    public Object
    remove(Object key)
    {
	closeIterators();

	byte[] k = encodeKey(key, _communicator);
	Object o;
	try
	{
	    byte[] v = _db.get(k);
	    o = decodeValue(v, _communicator);
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

	byte[] k = encodeKey(key, _communicator);
	byte[] v = encodeValue(value, _communicator);
	_db.put(k, v);
    }

    //
    // Returns true if the record was removed, false otherwise.
    //
    public boolean
    fastRemove(Object key)
    {
	closeIterators();

	byte[] k = encodeKey(key, _communicator);
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
		    _iterators.add(new java.lang.ref.WeakReference(p));
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
			byte[] k = encodeKey(p.getKey(), _communicator);
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
	closeIteratorsExcept(null);
    }

    private void
    closeIteratorsExcept(java.util.Iterator i)
    {
	java.util.Iterator p = _iterators.iterator();
	while(p.hasNext())
	{
	    java.lang.ref.WeakReference ref = (java.lang.ref.WeakReference)p.next();
	    EntryIterator q = (EntryIterator)ref.get();
	    if(q != null && q != i)
	    {
		q.close();
	    }
	}

	//
	// This is more efficient than removing the list items element
	// by element in the iteration loop.
	//
	_iterators.clear();
	if(i != null)
	{
	    _iterators.add(new java.lang.ref.WeakReference(i));
	}
    }

    private Entry
    getEntry(Object key)
    {
	//
	// It's not necessary to close outstanding iterators.
	//
	// If it was it would be a problem - it doesn't change the Map
	// - therefore open iterators should not be invalidated
	// (according the Java spec).
	//
	// closeIterators();

	byte[] k = encodeKey(key, _communicator);
	byte[] v = _db.get(k);

	return new Entry(this, _communicator, key, v);
    }

    private static boolean
    valEquals(Object o1, Object o2)
    {
        return (o1 == null ? o2 == null : o1.equals(o2));
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
            try
            {
                _cursor = _db.getCursor();
                _next = getEntry();
            }
            catch(DBNotFoundException e)
            {
                // Database is empty.
            }
        }

        public boolean
        hasNext()
        {
            return getNext();
        }

        public Object
        next()
        {
            if(!getNext())
            {
                throw new java.util.NoSuchElementException();
            }
            assert(_next != null);

            if(_prev != null)
            {
                _prev.invalidateCursor();
            }

            _prev = _next;
            _next = null;
            return _prev;
        }

        public void
        remove()
        {
            if(_prev == null)
            {
                throw new IllegalStateException();
            }
            
            closeIteratorsExcept(this);
            
            //
            // Clone the cursor so that error handling is simpler.
            //
            DBCursor clone = _cursor._clone();
            
            try
            {
                clone.del();
                _prev.invalidateCursor();
                _prev = null;
                _next = null;
            }
            finally
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
            _prev = null;
            
            if(copy != null)
            {
                copy.close();
            }
        }

        protected void
        finalize()
        {
            close();
        }

        private Entry
        getEntry()
        {
            _cursor.curr(_keyHolder, _valueHolder);
            return new Entry(Map.this, _cursor, _keyHolder.value, _valueHolder.value);
        }

        private boolean
        getNext()
        {
            if(_next == null)
            {
                if(_cursor.next())
                {
                    try
                    {
                        _next = getEntry();
                    }
                    catch(DBNotFoundException ex)
                    {
                        // No element found.
                    }
                }
            }
            return _next != null;
        }

        private DBCursor _cursor;
        private Entry _next;
        private Entry _prev;
        private Freeze.KeyHolder _keyHolder = new Freeze.KeyHolder();
        private Freeze.ValueHolder _valueHolder = new Freeze.ValueHolder();
    }

    static class Entry implements java.util.Map.Entry 
    {
        public
        Entry(Map map, DBCursor cursor, byte[] keyData, byte[] valueData)
        {
            _map = map;
            _cursor = cursor;
            _communicator = cursor.getCommunicator();
            _keyData = keyData;
            _haveKey = false;
            _valueData = valueData;
            _haveValue = false;
        }

        public
        Entry(Map map, Ice.Communicator communicator, Object key, byte[] valueData)
        {
            _map = map;
            _cursor = null;
            _communicator = communicator;
            _key = key;
            _haveKey = true;
            _valueData = valueData;
            _haveValue = false;
        }

	public Object
	getKey()
	{
            if(!_haveKey)
            {
                assert(_keyData != null);
                _key = _map.decodeKey(_keyData, _communicator);
                _haveKey = true;
            }
	    return _key;
	}

	public Object
	getValue()
	{
            if(!_haveValue)
            {
                assert(_valueData != null);
                _value = _map.decodeValue(_valueData, _communicator);
                _haveValue = true;
            }
	    return _value;
	}

	public Object
	setValue(Object value)
	{
            Object old = getValue();
            if(_cursor != null)
            {
                byte[] v = _map.encodeValue(value, _communicator);
                _cursor.set(v);
            }
            else
            {
                _map.put(getKey(), value); // Invalidates iterators.
            }
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

        void
        invalidateCursor()
        {
            _cursor = null;
        }

	private /*static*/ boolean
	eq(Object o1, Object o2)
	{
	    return (o1 == null ? o2 == null : o1.equals(o2));
	}

	private Map _map;
	private DBCursor _cursor;
        private Ice.Communicator _communicator;
	private Object _key;
        private byte[] _keyData;
        private boolean _haveKey;
	private Object _value;
        private byte[] _valueData;
        private boolean _haveValue;
    }
    
    private java.util.Set _entrySet;
    private DB _db;
    private Ice.Communicator _communicator;
    private java.util.List _iterators = new java.util.LinkedList();
}
