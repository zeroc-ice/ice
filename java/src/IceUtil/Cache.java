// **********************************************************************
//
// Copyright (c) 2004
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

package IceUtil;

//
// An abstraction to efficiently populate a Cache, without holding
// a lock while loading from a database.
//
// TODO: implement efficiently!
//

public class Cache
{
    public Cache(Store store)
    {
	_store = store;
    }
    
    public Object 
    getIfPinned(Object key)
    {
	synchronized(_map)
	{
	    return _map.get(key);
	}
    }
    
    public Object
    pin(Object key)
    {
	synchronized(_map)
	{
	    Object o = _map.get(key);
	    if(o == null)
	    {
		o = _store.load(key);
		if(o != null)
		{
		    _map.put(key, o);
		}
	    }
	    return o;
	}
    }

    public Object
    unpin(Object key)
    {
	synchronized(_map)
	{
	    return _map.remove(key);
	}
    }

    public void
    clear()
    {
	synchronized(_map)
	{
	    _map.clear();
	}
    }
    
    public int
    size()
    {
	synchronized(_map)
	{
	    return _map.size();
	}
    }

    public Object
    add(Object key, Object value)
    {
	assert value != null;

	synchronized(_map)
	{
	    Object existingVal = _map.put(key, value);
	    if(existingVal != null)
	    {
		_map.put(key, existingVal);
		return existingVal;
	    }
	    
	    //
	    // Let's check if it's in the store
	    //
	    existingVal = _store.load(key);
	    if(existingVal != null)
	    {
		_map.put(key, existingVal);
		return existingVal;
	    }
	    else
	    {
		return null;
	    }
	}
    }

    public Object
    pin(Object key, Object value)
    {
	synchronized(_map)
	{
	    Object existingVal = _map.put(key, value);
	    if(existingVal != null)
	    {
		_map.put(key, existingVal);
	    }
	    return existingVal;
	}
    }

    private final java.util.Map _map = new java.util.HashMap();
    private final Store _store;

}
