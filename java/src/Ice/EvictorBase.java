// **********************************************************************
//
// Copyright (c) 2003
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

package Ice;

public abstract class EvictorBase extends LocalObjectImpl implements Ice.Evictor
{

    public 
    EvictorBase()
    {
	_size = 0;
	_initialized = false;
	_hits = 0;
	_misses = 0;
	_evictions = 0;
	_traceLevel = 0;
    }

    synchronized public final void
    setSize(int size)
    {
	if(size < 0)
	{
	    return; // Ignore stupid values.
	}

	_size = size;
	evictServants();
    }

    synchronized public final int
    getSize()
    {
	return _size;
    }

    public abstract Ice.Object instantiate(Current curr, Ice.LocalObjectHolder cookie);

    public abstract void evict(Identity id, Ice.Object servant, Ice.LocalObject cookie);

    synchronized public final Ice.Object
    locate(Ice.Current c, Ice.LocalObjectHolder cookie)
    {
	if(!_initialized) // Lazy initialization of evictor size and tracing.
	{
	    _initialized = true;
	    initialize(c);
	}

	//
	// Make a copy of the ID. We need to do this because
	// Ice.Current.id is the same reference on every call:
	// it's contents change, but it is the same object every time.
	// We need a separate object to insert into the queue and the map.
	//
	Ice.Identity idCopy = new Ice.Identity();
	idCopy.name = c.id.name;
	idCopy.category = c.id.category;

	//
	// Check if we have an entry for the servant already.
	//
	EvictorCookie ec = new EvictorCookie();
	cookie.value = ec;
	ec.entry = (EvictorEntry)_map.get(idCopy);
	boolean newEntry = ec.entry == null;
	if(!newEntry)
	{
	    //
	    // Got an entry already, dequeue the entry from its current position.
	    //
	    ec.entry.pos.remove();

	    ++_hits; // Update statistics.
	    if(_traceLevel >= 3)
	    {
	        String msg = c.id.category.length() == 0 ? "default category" : ("category `" + c.id.category + "'");
		msg += ": cache hit for name `" + c.id.name + "'";
		_communicator.getLogger().trace("Ice::Evictor", msg);
	    }
	}
	else
	{
	    //
	    // Don't have an entry yet. Ask the application to instantiate a servant
	    // and create a new entry in the map.
	    //
	    ec.entry = new EvictorEntry();
	    Ice.LocalObjectHolder cookieHolder = new Ice.LocalObjectHolder();
	    ec.entry.servant = instantiate(c, cookieHolder); // Down-call to application-supplied method
	    if(ec.entry.servant == null)
	    {
		throw new Ice.ObjectNotExistException();
	    }
	    ec.entry.userCookie = cookieHolder.value;
	    ec.entry.useCount = 0;
	    _map.put(idCopy, ec.entry);

	    ++_misses; // Update statistics.
	    if(_traceLevel >= 2)
	    {
	        String msg = c.id.category.length() == 0 ? "default category" : ("category `" + c.id.category + "'");
		msg += ": cache miss for name `" + c.id.name + "'";
		_communicator.getLogger().trace("Ice::Evictor", msg);
	    }
	}

	//
	// Increment the use count of the servant and enqueue the entry at the front,
	// so we get LRU order.
	//
	++(ec.entry.useCount);
	_queue.addFirst(idCopy);
	ec.entry.pos = _queue.iterator();
	ec.entry.pos.next(); // Position the iterator on the element.

	//
	// If we added an entry, that might make another entry eligible for eviction.
	//
	if(newEntry)
	{
	    evictServants();
	}

	return ec.entry.servant;
    }

    synchronized public final void
    finished(Ice.Current c, Ice.Object o, Ice.LocalObject cookie)
    {
	EvictorCookie ec = (EvictorCookie)cookie;

	//
	// Decrement use count and check if there is something to evict.
	//
	--(ec.entry.useCount);
	evictServants();
    }

    synchronized public final void
    deactivate(String category)
    {
        if(_traceLevel >= 1)
	{
	    String msg = "deactivating ";
	    msg += category.length() == 0 ? "default category" : ("category `" + category + "'");
	    msg += ", number of cached servants = " + _map.size();
	    _communicator.getLogger().trace("Ice::Evictor", msg);

	    msg = "#evictions = " + _evictions;
	    msg += ", #hits = " + _hits;
	    msg += ", #misses = " + _misses;
	    double total = (double)_hits + (double)_misses;
	    double ratio = total == 0 ? 100 : ((double)_hits * 100 / total);
	    java.text.DecimalFormat f = new java.text.DecimalFormat("###.00");
	    msg += ", %hits = " + f.format(ratio) + "%";
	    _communicator.getLogger().trace("Ice::Evictor", msg);
	}
	_size = 0;
	evictServants();
    }

    synchronized private void
    evictServants()
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
	    assert(e != null);
	    if(e.useCount == 0)
	    {
		if(_traceLevel >= 2)
		{
		    String msg = id.category.length() == 0 ? "default category" : ("category `" + id.category + "'");
		    msg += ": evicting `" + id.name + "'";
		    _communicator.getLogger().trace("Ice::Evictor", msg);
		}

		evict(id, e.servant, e.userCookie); // Down-call to application-supplied method.
		p.remove();
		_map.remove(id);
	        ++_evictions; // Update statistics.
	    }
	}
    }

    private void
    initialize(Ice.Current c)
    {
	_communicator = c.adapter.getCommunicator();

	Ice.Properties p = _communicator.getProperties();
	int num;

	//
	// Check evictor size properties.
	//
	if((num = p.getPropertyAsInt("Ice.Evictor.Size")) >= 0)
	{
	    _size = num;
	}
	if((num = p.getPropertyAsInt(c.adapter.getName() + ".Evictor.Size")) >= 0)
	{
	    _size = num;
	}
	if(c.id.category.length() != 0
	   && (num = p.getPropertyAsInt(c.adapter.getName() + "." + c.id.category + ".Evictor.Size")) >= 0)
	{
	    _size = num;
	}
	if(_size < 0)
	{
	    _size = defaultSize;
	}

	//
	// Check evictor trace properties.
	//
	if(p.getProperty("Ice.Evictor.Trace").length() != 0)
	{
	    _traceLevel = p.getPropertyAsInt("Ice.Evictor.Trace");
	}
	if(p.getProperty(c.adapter.getName() + ".Evictor.Trace").length() != 0)
	{
	    _traceLevel = p.getPropertyAsInt(c.adapter.getName() + ".Evictor.Trace");
	}
	if(c.id.category.length() != 0
	   && p.getProperty(c.adapter.getName() + "." + c.id.category + ".Evictor.Trace").length() != 0)
	{
	    _traceLevel = p.getPropertyAsInt(c.adapter.getName() + "." + c.id.category + ".Evictor.Trace");
	}
    }

    private final static int defaultSize = 1000;

    private class EvictorEntry
    {
        int useCount;
	java.util.Iterator pos;
	Ice.Object servant;
	Ice.LocalObject userCookie;
    }

    private class EvictorCookie extends Ice.LocalObjectImpl
    {
	public EvictorEntry entry;
    }

    private IceInternal.LinkedList _queue = new IceInternal.LinkedList();
    private java.util.Map _map = new java.util.HashMap();
    private int _size;
    private boolean _initialized;
    private int _hits;
    private int _misses;
    private int _evictions;
    private int _traceLevel;
    private Ice.Communicator _communicator;
}
