// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Evictor;

public abstract class EvictorBase extends Ice.LocalObjectImpl implements Ice.ServantLocator
{

    public
    EvictorBase()
    {
	_size = 1000;
    }

    public
    EvictorBase(int size)
    {
	_size = size < 0 ? 1000 : size;
    }

    public abstract Ice.Object
    add(Ice.Current c, Ice.LocalObjectHolder cookie);

    public abstract void
    evict(Ice.Object servant, Ice.LocalObject cookie);

    synchronized public final Ice.Object
    locate(Ice.Current c, Ice.LocalObjectHolder cookie)
    {
	//
	// Create a cookie.
	//
	EvictorCookie ec = new EvictorCookie();
	cookie.value = ec;

	//
	// Check if we a servant in the map already.
	//
	ec.entry = (EvictorEntry)_map.get(idCopy);
	boolean newEntry = ec.entry == null;
	if(!newEntry)
	{
	    //
	    // Got an entry already, dequeue the entry from
	    // its current position.
	    //
	    ec.entry.pos.remove();
	}
	else
	{
	    //
	    // We do not have entry. Ask the derived class to
	    // instantiate a servant and add a new entry to the map.
	    //
	    ec.entry = new EvictorEntry();
	    Ice.LocalObjectHolder cookieHolder = new Ice.LocalObjectHolder();
	    ec.entry.servant = add(c, cookieHolder); // Down-call
	    if(ec.entry.servant == null)
	    {
		return null;
	    }
	    ec.entry.userCookie = cookieHolder.value;
	    ec.entry.useCount = 0;
	    _map.put(c.id, ec.entry);
	}

	//
	// Increment the use count of the servant and enqueue
	// the entry at the front, so we get LRU order.
	//
	++(ec.entry.useCount);
	_queue.addFirst(c.id);
	ec.entry.pos = _queue.iterator();
	ec.entry.pos.next(); // Position the iterator on the element.

	return ec.entry.servant;
    }

    synchronized public final void
    finished(Ice.Current c, Ice.Object o, Ice.LocalObject cookie)
    {
	EvictorCookie ec = (EvictorCookie)cookie;

	//
	// Decrement use count and check if
	// there is something to evict.
	//
	--(ec.entry.useCount);
	evictServants();
    }

    synchronized public final void
    deactivate(String category)
    {
	_size = 0;
	evictServants();
    }

    private class EvictorEntry
    {
	Ice.Object servant;
	Ice.LocalObject userCookie;
	java.util.Iterator pos;
        int useCount;
    }

    private class EvictorCookie extends Ice.LocalObjectImpl
    {
	public EvictorEntry entry;
    }

    private void evictServants()
    {
	//
	// If the evictor queue has grown larger than the limit,
	// look at the excess elements to see whether any of them
	// can be evicted.
	//
	for(int i = _map.size() - _size; i > 0; --i)
	{
	    java.util.Iterator p = _queue.riterator();
	    Ice.Identity id = (Ice.Identity)p.next();
	    EvictorEntry e = (EvictorEntry)_map.get(id);
	    if(e.useCount == 0)
	    {
		evict(e.servant, e.userCookie); // Down-call
		p.remove();
		_map.remove(id);
	    }
	}
    }

    private java.util.Map _map = new java.util.HashMap();
    private Evictor.LinkedList _queue = new Evictor.LinkedList();
    private int _size;
}
