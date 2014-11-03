// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Evictor
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Diagnostics;

    public class LinkedList<T> : ICollection<T>, ICollection, ICloneable
    {
        public LinkedList()
        {
            Clear();
        }

        public int Count
        {
            get
            {
                return _count;
            }
        }

        public bool IsReadOnly
        {
            get
            {
                return false;
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

        public void CopyTo(T[] array, int index)
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
            Node n = _head;
            while(n != null)
            {
                array[index++] = n.val;
                n = (Node)n.next;
            }
        }

        void ICollection.CopyTo(Array array, int index)
        {
            T[] arr = array as T[];
            if(arr == null)
            {
                throw new ArgumentException("array");
            }
            CopyTo(arr, index);
        }

        public IEnumerator GetEnumerator()
        {
            return new Enumerator(this);
        }

        IEnumerator<T> IEnumerable<T>.GetEnumerator()
        {
            return new Enumerator(this);
        }

        public object Clone()
        {
            LinkedList<T> l = new LinkedList<T>();
            Node cursor = _head;
            while(cursor != null)
            {
                l.Add(cursor.val);
                cursor = cursor.next;
            }
            return l;
        }

        public void Add(T value)
        {
            Node n = new Node();
            n.val = value;
            if(_tail == null)
            {
                n.prev = null;
                _head = n;
            }
            else
            {
                n.prev = _tail;
                _tail.next = n;
            }
            n.next = null;
            _tail = n;
            _count++;
        }

        public void AddFirst(T value)
        {
            Node n = new Node();
            n.val = value;
            if(_head == null)
            {
                n.next = null;
                _tail = n;
            }
            else
            {
                n.next = _head;
                _head.prev = n;
            }
            n.prev = null;
            _head = n;
            _count++;
        }

        public void Clear()
        {
            _head = null;
            _tail = null;
            _count = 0;
        }

        public bool Contains(T value)
        {
            return Find(value) != null;
        }

        public bool Remove(T value)
        {
            Node n = Find(value);
            if(n != null)
            {
                Remove(n);
                return true;
            }
            return false;
        }

        private Node Find(T value)
        {
            Node n = _head;
            while(n != null)
            {
                if(Object.Equals(value, n.val))
                {
                    return n;
                }
                n = n.next;
            }
            return null;
        }

        private void Remove(Node n)
        {
            Debug.Assert(n != null);
            Debug.Assert(_count != 0);
            
            if(n.prev != null)
            {
                n.prev.next = n.next;
            }
            else
            {
                _head = n.next;
            }
            if(n.next != null)
            {
                n.next.prev = n.prev;
            }
            else
            {
                _tail = n.prev;
            }
            _count--;
        }

        internal class Node
        {
            internal Node next;
            internal Node prev;
            internal T val;
        }

        private Node _head;
        private Node _tail;
        private int _count;

        public class Enumerator : IEnumerator<T>, IEnumerator, IDisposable
        {
            internal Enumerator(LinkedList<T> list)
            {
                _list = list;
                _current = null;
                _movePrev = null;
                _moveNext = null;
                _removed = false;
            }

            public void Reset()
            {
                _current = null;
                _movePrev = null;
                _moveNext = null;
                _removed = false;
            }

            public T Current
            {
                get
                {
                    if(_current == null)
                    {
                        throw new InvalidOperationException("iterator not positioned on an element");
                    }
                    return _current.val;
                }
            }

            object IEnumerator.Current
            {
                get
                {
                    return Current;
                }
            }

            public bool MoveNext()
            {
                if(_removed)
                {
                    _current = _moveNext;
                    _moveNext = null;
                    _movePrev = null;
                    _removed = false;
                }
                else
                {
                    if(_current == _list._tail) // Make sure the iterator "sticks" if on last element.
                    {
                        return false;
                    }
                    _current = _current == null ? _list._head : _current.next;
                }
                return _current != null;
            }

            public bool MovePrev()
            {

                if(_removed)
                {
                    _current = _movePrev;
                    _movePrev = null;
                    _moveNext = null;
                    _removed = false;
                }
                else
                {
                    if(_current == _list._head) // Make sure the iterator "sticks" if on first element.
                    {
                        return false;
                    }
                    _current = _current == null ? _list._tail : _current.prev;
                }
                return _current != null;
            }

            public void Remove()
            {
                if(_current == null)
                {
                    throw new InvalidOperationException("iterator is not positioned on an element");
                }
                _removed = true;
                _moveNext = _current.next; // Remember where to move next for call to MoveNext().
                _movePrev = _current.prev; // Remember where to move next for call to MovePrev().
                _list.Remove(_current);
                _current = null;
            }

            public void Dispose()
            {
                if(_list == null)
                {
                    throw new ObjectDisposedException(null);
                }
                Reset();
                _list = null;
            }

            private LinkedList<T> _list; // The list we are iterating over.
            private Node _current;    // Current iterator position.
            private Node _moveNext;   // Remembers node that followed a removed element.
            private Node _movePrev;   // Remembers node that preceded a removed element.
            private bool _removed;    // True after a call to Remove(), false otherwise.
        }
    }
}
