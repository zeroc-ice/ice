// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze;

//
// Stripped down LinkedList implementation for use in the Evictor. The
// API is similar to java.util.LinkedList.
//
// Major differences:
//  * listIterator() is not implemented.
//  * Operation riterator() returns a reverse iterator.
//  * This implementation also has the property that an Iterator can be
//    retained over structural changes to the list itself (similar to an
//    STL list).
//
public class LinkedList<T>
{
    public
    LinkedList()
    {
        _header.next = _header.previous = _header;
    }

    public T
    getFirst()
    {
        if(_size == 0)
        {
            throw new java.util.NoSuchElementException();
        }

        return _header.next.element;
    }

    public T
    getLast()
    {
        if(_size == 0)
        {
            throw new java.util.NoSuchElementException();
        }

        return _header.previous.element;
    }

    public void
    addFirst(T o)
    {
        addBefore(o, _header.next);
    }

    public boolean
    isEmpty()
    {
        return _size == 0;
    }

    public int
    size()
    {
        return _size;
    }

    public java.util.Iterator<T>
    iterator()
    {
        return new ForwardIterator();
    }

    public java.util.Iterator<T>
    riterator()
    {
        return new ReverseIterator();
    }

    private class ForwardIterator implements java.util.Iterator<T>
    {
        @Override
        public boolean
        hasNext()
        {
            return _next != null;
        }

        @Override
        public T
        next()
        {
            if(_next == null)
            {
                throw new java.util.NoSuchElementException();
            }

            _current = _next;

            if(_next.next != _header)
            {
                _next = _next.next;
            }
            else
            {
                _next = null;
            }
            return _current.element;
        }

        @Override
        public void
        remove()
        {
            if(_current == null)
            {
                throw new IllegalStateException();
            }
            LinkedList.this.remove(_current);
            _current = null;
        }

        ForwardIterator()
        {
            if(_header.next == _header)
            {
                _next = null;
            }
            else
            {
                _next = _header.next;
            }
            _current = null;
        }

        private Entry<T> _current;
        private Entry<T> _next;
    }

    private class ReverseIterator implements java.util.Iterator<T>
    {
        @Override
        public boolean
        hasNext()
        {
            return _next != null;
        }

        @Override
        public T
        next()
        {
            if(_next == null)
            {
                throw new java.util.NoSuchElementException();
            }

            _current = _next;

            if(_next.previous != _header)
            {
                _next = _next.previous;
            }
            else
            {
                _next = null;
            }
            return _current.element;
        }

        @Override
        public void
        remove()
        {
            if(_current == null)
            {
                throw new IllegalStateException();
            }
            LinkedList.this.remove(_current);
            _current = null;
        }

        ReverseIterator()
        {
            if(_header.next == _header)
            {
                _next = null;
            }
            else
            {
                _next = _header.previous;
            }
            _current = null;
        }

        private Entry<T> _current;
        private Entry<T> _next;
    }

    private static class Entry<T>
    {
        T element;
        Entry<T> next;
        Entry<T> previous;

        Entry(T element, Entry<T> next, Entry<T> previous)
        {
            this.element = element;
            this.next = next;
            this.previous = previous;
        }
    }

    private Entry<T>
    addBefore(T o, Entry<T> e)
    {
        Entry<T> newEntry = new Entry<T>(o, e, e.previous);
        newEntry.previous.next = newEntry;
        newEntry.next.previous = newEntry;
        _size++;
        return newEntry;
    }

    private void
    remove(Entry<T> e)
    {
        if(e == _header)
        {
            throw new java.util.NoSuchElementException();
        }

        e.previous.next = e.next;
        e.next.previous = e.previous;
        _size--;
    }

    private Entry<T> _header = new Entry<T>(null, null, null);
    private int _size = 0;
}
