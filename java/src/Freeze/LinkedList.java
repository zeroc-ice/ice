// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
public class LinkedList
{
    public
    LinkedList()
    {
        _header.next = _header.previous = _header;
    }

    public java.lang.Object
    getFirst()
    {
        if(_size == 0)
        {
            throw new java.util.NoSuchElementException();
        }

        return _header.next.element;
    }

    public java.lang.Object
    getLast()
    {
        if(_size == 0)
        {
            throw new java.util.NoSuchElementException();
        }

        return _header.previous.element;
    }

    public void
    addFirst(java.lang.Object o)
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

    public java.util.Iterator
    iterator()
    {
        return new ForwardIterator();
    }

    public java.util.Iterator
    riterator()
    {
        return new ReverseIterator();
    }

    private class ForwardIterator implements java.util.Iterator
    {
        public boolean
        hasNext()
        {
            return _next != null;
        }

        public java.lang.Object
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

        private Entry _current;
        private Entry _next;
    }

    private class ReverseIterator implements java.util.Iterator
    {
        public boolean
        hasNext()
        {
            return _next != null;
        }

        public java.lang.Object
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

        private Entry _current;
        private Entry _next;
    }

    private static class Entry 
    {
        java.lang.Object element;
        Entry next;
        Entry previous;

        Entry(java.lang.Object element, Entry next, Entry previous)
        {
            this.element = element;
            this.next = next;
            this.previous = previous;
        }
    }

    private Entry
    addBefore(java.lang.Object o, Entry e)
    {
        Entry newEntry = new Entry(o, e, e.previous);
        newEntry.previous.next = newEntry;
        newEntry.next.previous = newEntry;
        _size++;
        return newEntry;
    }

    private void
    remove(Entry e)
    {
        if(e == _header)
        {
            throw new java.util.NoSuchElementException();
        }

        e.previous.next = e.next;
        e.next.previous = e.previous;
        _size--;
    }

    private Entry _header = new Entry(null, null, null);
    private int _size = 0;
}
