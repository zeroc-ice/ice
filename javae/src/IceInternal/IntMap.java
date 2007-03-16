// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class IntMap
{
    public
    IntMap(int initialCapacity, float loadFactor)
    {
        if(initialCapacity > MAXIMUM_CAPACITY)
        {
            initialCapacity = MAXIMUM_CAPACITY;
        }

        // Find a power of 2 >= initialCapacity
        int capacity = 1;
        while(capacity < initialCapacity)
        {
            capacity <<= 1;
        }

        _loadFactor = loadFactor;
        _threshold = (int)(capacity * loadFactor);
        _table = new Entry[capacity];
    }

    public
    IntMap(int initialCapacity)
    {
        this(initialCapacity, DEFAULT_LOAD_FACTOR);
    }

    public
    IntMap()
    {
        _loadFactor = DEFAULT_LOAD_FACTOR;
        _threshold = (int)(DEFAULT_INITIAL_CAPACITY);
        _table = new Entry[DEFAULT_INITIAL_CAPACITY];
    }

    public int
    size()
    {
        return _size;
    }

    public boolean
    isEmpty()
    {
        return _size == 0;
    }

    public Object
    get(int key)
    {
        int i = indexFor(key, _table.length);
        Entry e = _table[i];
        while(true)
        {
            if(e == null)
            {
                return e;
            }
            if(key == e.key)
            {
                return e.value;
            }
            e = e.next;
        }
    }

    public boolean
    containsKey(int key)
    {
        int i = indexFor(key, _table.length);
        Entry e = _table[i];
        while(e != null)
        {
            if(key == e.key)
            {
                return true;
            }
            e = e.next;
        }
        return false;
    }

    public Object
    put(int key, Object value)
    {
        int i = indexFor(key, _table.length);

        for(Entry e = _table[i]; e != null; e = e.next)
        {
            if(key == e.key)
            {
                Object oldValue = e.value;
                e.value = value;
                return oldValue;
            }
        }

        _modCount++;
        addEntry(key, value, i);
        return null;
    }

    public Object
    remove(int key)
    {
        int i = indexFor(key, _table.length);
        Entry prev = _table[i];
        Entry e = prev;

        while(e != null)
        {
            Entry next = e.next;
            if(key == e.key)
            {
                _modCount++;
                _size--;
                if(prev == e)
                {
                    _table[i] = next;
                }
                else
                {
                    prev.next = next;
                }
                e.next = _entryCache;
                _entryCache = e;
                return e.value;
            }
            prev = e;
            e = next;
        }

	if(e != null)
	{
	    return e.value;
	}
	return e;
    }

    public void
    clear()
    {
        _modCount++;
        Entry tab[] = _table;
        for(int i = 0; i < tab.length; i++)
        {
            tab[i] = null;
        }
        _size = 0;
    }

    public java.util.Enumeration
    elements()
    {
        return new EntryIterator();
    }

    public static final class Entry
    {
        int key;
        Object value;
        Entry next;

        Entry(int k, Object v, Entry n)
        {
            key = k;
            value = v;
            next = n;
        }

        public int
        getKey()
        {
            return key;
        }

        public Object
        getValue()
        {
            return value;
        }

        public Object
        setValue(Object newValue)
        {
            Object oldValue = value;
            value = newValue;
            return oldValue;
        }
    }

    private static int
    indexFor(int key, int length)
    {
        return key & (length - 1);
    }

    private void
    addEntry(int key, Object value, int bucketIndex)
    {
        Entry e;
        if(_entryCache != null)
        {
            e = _entryCache;
            _entryCache = _entryCache.next;
            e.key = key;
            e.value = value;
            e.next = _table[bucketIndex];
        }
        else
        {
            e = new Entry(key, value, _table[bucketIndex]);
        }
        _table[bucketIndex] = e;
        if(_size++ >= _threshold)
        {
            resize(2 * _table.length);
        }
    }

    private void
    resize(int newCapacity)
    {
        // assert (newCapacity & -newCapacity) == newCapacity; // power of 2
        Entry[] oldTable = _table;
        int oldCapacity = oldTable.length;

        // check if needed
        if(_size < _threshold || oldCapacity > newCapacity)
        {
            return;
        }

        Entry[] newTable = new Entry[newCapacity];
        transfer(newTable);
        _table = newTable;
        _threshold = (int)(newCapacity * _loadFactor);
    }

    private void
    transfer(Entry[] newTable)
    {
        Entry[] src = _table;
        int newCapacity = newTable.length;
        for(int j = 0; j < src.length; j++)
        {
            Entry e = src[j];
            if(e != null)
            {
                src[j] = null;
                do
                {
                    Entry next = e.next;
                    int i = indexFor(e.key, newCapacity);
                    e.next = newTable[i];
                    newTable[i] = e;
                    e = next;
                }
                while(e != null);
            }
        }
    }

    private class EntryIterator implements java.util.Enumeration
    {
        EntryIterator()
        {
            _expectedModCount = _modCount;
            Entry[] t = _table;
            int i = t.length;
            Entry n = null;
            if(_size != 0) // advance to first entry
            {
                while(i > 0 && (n = t[--i]) == null)
                    ;
            }
            _next = n;
            _index = i;
        }

        public boolean
        hasMoreElements()
        {
            return _next != null;
        }

        public Object
        nextElement()
        {
	    if(IceUtil.Debug.ASSERT)
	    {
		IceUtil.Debug.Assert(_modCount == _expectedModCount);
	    }
            Entry e = _next;
            if(e == null)
            {
                throw new java.util.NoSuchElementException();
            }

            Entry n = e.next;
            Entry[] t = _table;
            int i = _index;
            while(n == null && i > 0)
            {
                n = t[--i];
            }
            _index = i;
            _next = n;
            return _current = e;
        }

        public void
        remove()
        {
            if(_current == null)
            {
                throw new IllegalStateException();
            }
	    if(IceUtil.Debug.ASSERT)
	    {
		IceUtil.Debug.Assert(_modCount == _expectedModCount);
	    }
            int k = _current.key;
            _current = null;
            IntMap.this.remove(k);
            _expectedModCount = _modCount;
        }

        private Entry _next;
        private int _expectedModCount;
        private int _index;
        private Entry _current;
    }

    //
    // The default initial capacity - MUST be a power of two.
    //
    private static final int DEFAULT_INITIAL_CAPACITY = 16;

    //
    // The maximum capacity, used if a higher value is implicitly specified
    // by either of the constructors with arguments.
    // MUST be a power of two <= 1<<30.
    //
    private static final int MAXIMUM_CAPACITY = 1 << 30;

    //
    // The default load factor.
    //
    private static final float DEFAULT_LOAD_FACTOR = 0.75f;

    //
    // The table, resized as necessary. Length MUST Always be a power of two.
    //
    private Entry[] _table;

    //
    // The number of key-value mappings contained in this map.
    //
    private int _size;

    //
    // The next size value at which to resize (capacity * load factor).
    //
    private int _threshold;

    //
    // The load factor for the hash table.
    //
    private /*final*/ float _loadFactor;

    //
    // The number of times this map has been structurally modified
    // Structural modifications are those that change the number of
    // mappings in the map or otherwise modify its internal structure
    // (e.g., rehash). This field is used to make iterators fail-fast.
    //
    private volatile int _modCount;

    private Entry _entryCache;
}
