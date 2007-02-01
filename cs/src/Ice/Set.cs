// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;
using System.Diagnostics;

namespace IceUtil
{
    public class Set : ICollection, ICloneable
    {
        public Set()
            : this(DEFAULT_CAPACITY, DEFAULT_LOAD_FACTOR)
        {
        }

        public Set(int capacity)
            : this(capacity, DEFAULT_LOAD_FACTOR)
        {
        }

        public Set(int capacity, double loadFactor)
        {
            if(loadFactor <= 0)
            {
                throw new ArgumentOutOfRangeException("loadFactor", loadFactor, "load factor must be greater than 0.0");
            }
            if(loadFactor > 1)
            {
                throw new ArgumentOutOfRangeException("loadfactor", loadFactor,
                                                      "load factor must not be greater than 1.0");
            }
            if(capacity > MAXIMUM_CAPACITY)
            {
                _capacity = MAXIMUM_CAPACITY;
            }
            else if(capacity < 1)
            {
                _capacity = 2;
            }
            else
            {
                int c = 1;
                while(c < capacity)
                {
                    c *= 2;
                }
                _capacity = c;
            }
            _loadFactor = loadFactor;
            _count = 0;
            _table = new Entry[_capacity];
        }

        public int Count
        {
            get
            {
                return _count;
            }
        }

        public bool IsSynchronized
        {
            get
            {
                return false;
            }
        }

        public object SyncRoot
        {
            get
            {
                return this;
            }
        }

        public void CopyTo(Array array, int index)
        {
            //
            // Check preconditions.
            //
            if(array == null)
            {
                throw new ArgumentNullException("array", "array parameter must not be null");
            }
            if(index < 0)
            {
                throw new ArgumentOutOfRangeException("index", _count, "index must not be less than zero");
            }
            if(index >= array.Length)
            {
                throw new ArgumentException("index out of bounds for array", "index");
            }
            if(array.Length - index > _count)
            {
                throw new ArgumentException("insufficient room in array", "array");
            }
            if(array.Rank != 1)
            {
                throw new ArgumentException("array must be one-dimensional", "array");
            }

            //
            // Copy the elements.
            //
            foreach(Entry e in _table)
            {
                Entry cursor = e;
                while(cursor != null)
                {
                    array.SetValue(cursor.value, index++);
                    cursor = cursor.next;
                }
            }
        }

        public IEnumerator GetEnumerator()
        {
            return new SetEnumerator(this);
        }

        public object Clone()
        {
            Set newSet = new Set(_capacity, _loadFactor);
            foreach(object o in this)
            {
                newSet.Add(o);
            }
            return newSet;
        }

        public bool Add(object value)
        {
            if(value == null)
            {
                throw new ArgumentNullException("value", "value parameter must not be null");
            }
            int hash = System.Math.Abs(value.GetHashCode());
            Entry e = FindEntry(hash % _capacity, value);
            if(e != null)
            {
                return false;
            }
            if(_count >= _capacity * _loadFactor)
            {
                if(_capacity == MAXIMUM_CAPACITY)
                {
                    throw new OutOfMemoryException("set exceeded maximum capacity of " + MAXIMUM_CAPACITY);
                }
                _capacity *= 2;
                Resize();
            }
            AddEntry(hash % _capacity, value);
            return true;
        }

        public bool Remove(object value)
        {
            if(value == null)
            {
                throw new ArgumentNullException("value", "value parameter must not be null");
            }
            int hash = System.Math.Abs(value.GetHashCode()) % _capacity;
            return RemoveEntry(hash, value);
        }

        public void Clear()
        {
            _table = new Entry[_capacity];
            _count = 0;
        }

        public bool Contains(object value)
        {
            if(value == null)
            {
                throw new ArgumentNullException("value", "value parameter must not be null");
            }
            int hash = System.Math.Abs(value.GetHashCode()) % _capacity;
            return FindEntry(hash, value) != null;
        }

        private Entry FindEntry(int hash, object value)
        {
            Entry cursor = _table[hash];
            while(cursor != null)
            {
                if(object.ReferenceEquals(value, cursor.value) || value.Equals(cursor.value))
                {
                    return cursor;
                }
                cursor = cursor.next;
            }
            return null;
        }

        private void Resize()
        {
            Entry[] newTable = new Entry[_capacity];
            foreach(Entry e in _table)
            {
                Entry cursor = e;
                while(cursor != null)
                {
                    int hash = System.Math.Abs(cursor.value.GetHashCode()) % _capacity;
                    newTable[hash] = new Entry(cursor.value, newTable[hash]);
                    cursor = cursor.next;
                }
            }
            _table = newTable;
        }

        private void AddEntry(int hash, object value)
        {
            _table[hash] = new Entry(value, _table[hash]);
            _count++;
        }

        private bool RemoveEntry(int hash, object value)
        {
            Entry prev = null;
            Entry cursor = _table[hash];
            while(cursor != null)
            {
                if(object.ReferenceEquals(value, cursor.value) || value.Equals(cursor.value))
                {
                    if(prev == null)
                    {
                        _table[hash] = cursor.next;
                    }
                    else
                    {
                        prev.next = cursor.next;
                    }
                    _count--;
                    return true;
                }
                prev = cursor;
                cursor = cursor.next;
            }
            return false;
        }

        internal class Entry
        {
            internal
            Entry() : this(null, null)
            {
            }

            internal
            Entry(object value, Entry nextEntry)
            {
                this.value = value;
                next = nextEntry;
            }
            internal object value;
            internal Entry next;
        }

        private const int DEFAULT_CAPACITY = 64; // Must be power of 2
        private const int MAXIMUM_CAPACITY = 2 << 29; // Must be power of 2
        private const double DEFAULT_LOAD_FACTOR = 0.8; // Must be >= 0
        private readonly double _loadFactor; // Selected load factor
        private int _capacity; // Current capacity
        private int _count; // Number of entries in the table
        private Entry[] _table; // Resized as necessary; number of elements is a power of 2

        public class SetEnumerator : IEnumerator
        {
            internal SetEnumerator(Set theSet)
            {
                _set = theSet;
                _index = 0;
                _current = null;
            }

            public void Reset()
            {
                _index = 0;
                _current = null;
            }

            public object Current
            {
                get
                {
                    if(_current == null)
                    {
                        throw new InvalidOperationException("iterator not positioned on an element");
                    }
                    return _current.value;
                }
            }

            public bool MoveNext()
            {
                Debug.Assert(_index <= _set._table.Length);

                if(_index == _set._table.Length) // Make sure the iterator "sticks" if on last element.
                {
                    return false;
                }

                if(_current == null || _current.next == null) // If at start of iteration, or after Remove(),
                {                                             // or at end of an overflow chain...
                    if(_current != null) // _current is at the end of an overflow chain.
                    {
                        _index++; // Start search for non-empty bucket in next array slot.
                    }
                    while(_index < _set._table.Length && _set._table[_index] == null) // Find non-empty bucket.
                    {
                        _index++;
                    }
                    if(_index < _set._table.Length)
                    {
                        _prev = null;
                        _current = _set._table[_index]; // Point _current at first entry in non-empty bucket.
                    }
                }
                else // _current points at an entry with a successor.
                {
                    _prev = _current;
                    _current = _current.next;
                }

                return _index < _set._table.Length;
            }

            public void Remove()
            {
                if(_current == null)
                {
                    throw new InvalidOperationException("iterator is not positioned on an element");
                }
                if(_prev == null)
                {
                    _set._table[_index] = _current.next;
                }
                else
                {
                    _prev.next = _current.next;
                }
                _set._count--;
            }

            private Set _set; // The set we are iterating over.
            private int _index; // Current index into table.
            private Entry _current; // Current element.
            private Entry _prev; // Element preceding current element (if any).
        }
    }
}

