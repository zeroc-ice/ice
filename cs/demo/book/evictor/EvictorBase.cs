// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Evictor
{
    public abstract class EvictorBase : Ice.LocalObjectImpl, Ice.ServantLocator
    {

	public EvictorBase()
	{
	    _size = 1000;
	}

	public EvictorBase(int size)
	{
	    _size = size < 0 ? 1000 : size;
	}

	public abstract Ice.Object add(Ice.Current c, out Ice.LocalObject cookie);

	public abstract void evict(Ice.Object servant, Ice.LocalObject cookie);

	public Ice.Object locate(Ice.Current c, out Ice.LocalObject cookie)
	{
	    lock(this)
	    {
		//
		// Create a cookie.
		//
		EvictorCookie ec = new EvictorCookie();

		//
		// Check if we a servant in the map already.
		//
		ec.entry = (EvictorEntry)_map[c.id];
		bool newEntry = ec.entry == null;
		if(!newEntry)
		{
		    //
		    // Got an entry already, dequeue the entry from
		    // its current position.
		    //
		    ec.entry.pos.Remove();
		}
		else
		{
		    //
		    // We do not have an entry. Ask the derived class to
		    // instantiate a servant and add a new entry to the map.
		    //
		    ec.entry = new EvictorEntry();
		    Ice.LocalObject theCookie;
		    ec.entry.servant = add(c, out theCookie); // Down-call
		    if(ec.entry.servant == null)
		    {
			return null;
		    }
		    ec.entry.userCookie = theCookie;
		    ec.entry.useCount = 0;
		    _map[c.id] = ec.entry;
		}

		//
		// Increment the use count of the servant and enqueue
		// the entry at the front, so we get LRU order.
		//
		++(ec.entry.useCount);
		_queue.AddFirst(c.id);
		ec.entry.pos = (LinkedList.Enumerator)_queue.GetEnumerator();
                ec.entry.pos.MovePrev();
                cookie = ec;

		return ec.entry.servant;
	    }
	}

	public void finished(Ice.Current c, Ice.Object o, Ice.LocalObject cookie)
	{
	    lock(this)
	    {
		EvictorCookie ec = (EvictorCookie)cookie;

		//
		// Decrement use count and check if
		// there is something to evict.
		//
		--(ec.entry).useCount;
		evictServants();
	    }
	}

	public void deactivate(string category)
	{
	    lock(this)
	    {
		_size = 0;
		evictServants();
	    }
	}

	private class EvictorEntry
	{
	    internal Ice.Object servant;
	    internal Ice.LocalObject userCookie;
	    internal LinkedList.Enumerator pos;
	    internal int useCount;
	}

	private class EvictorCookie : Ice.LocalObjectImpl
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
	    LinkedList.Enumerator p = (LinkedList.Enumerator)_queue.GetEnumerator();
	    for(int i = _map.Count - _size; i > 0; --i)
	    {
                p.MovePrev();
		Ice.Identity id = (Ice.Identity)p.Current;
		EvictorEntry e = (EvictorEntry)_map[id];
		if(e.useCount == 0)
		{
		    evict(e.servant, e.userCookie); // Down-call
		    p.Remove();
		    _map.Remove(id);
		}
	    }
	}

	private System.Collections.Hashtable _map = new System.Collections.Hashtable();
	private LinkedList _queue = new LinkedList();
	private int _size;
    }
}
