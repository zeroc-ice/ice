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

#ifndef ICE_UTIL_CACHE_H
#define ICE_UTIL_CACHE_H

#include <IceUtil/Handle.h>
#include <IceUtil/Mutex.h>
#include <map>

namespace IceUtil
{

//
// An abstraction to efficiently populate a Cache, without holding
// a lock while loading from a database.
//
// TODO: implement efficiently!
//

template<typename Key, typename Value>
class Cache
{
public:

    typedef typename std::map<Key, Handle<Value> >::iterator Position;

    Handle<Value> getIfPinned(const Key& key) const
    {
	Mutex::Lock sync(_mutex);
	typename CacheMap::const_iterator p = _map.find(key);
	if(p != _map.end())
	{
	    return (*p).second;
	}
	else
	{
	    return 0;
	}
    }
    
    Handle<Value> pin(const Key& key)
    {
	Mutex::Lock sync(_mutex);
	typename CacheMap::iterator p = _map.find(key);
	if(p != _map.end())
	{
	    return (*p).second;
	}
	else
	{
	    Handle<Value> val = load(key);
	 
	    if(val != 0)
	    {
		std::pair<CacheMap::iterator, bool> ir = 
		    _map.insert(CacheMap::value_type(key, val));
		pinned(val, ir.first);
	    }
	    return val;
	}
    }
    
    void unpin(Position p)
    {
	Mutex::Lock sync(_mutex);
	_map.erase(p);
    }
    
    bool unpin(const Key& key)
    {
	Mutex::Lock sync(_mutex);
	return _map.erase(key) > 0;
    }

    void clear()
    {
	Mutex::Lock sync(_mutex);
	_map.clear();
    }

    size_t size()
    {
	Mutex::Lock sync(_mutex);
	return _map.size();
    }


    std::pair<Handle<Value>, bool> insert(const Key& key, const Handle<Value>& val)
    {
	assert(val != 0);

	Mutex::Lock sync(_mutex);
	std::pair<CacheMap::iterator, bool> ir = 
	    _map.insert(CacheMap::value_type(key, val));

	if(ir.second)
	{
	    //
	    // Let's check if it's in the store
	    //
	    Handle<Value> existingVal = load(key);
	    if(existingVal != 0)
	    {
		ir.first->second = existingVal;
		pinned(existingVal, ir.first);
		return std::pair<Handle<Value>, bool>(existingVal, false);
	    }
	    else
	    {
		pinned(val, ir.first);
		return std::pair<Handle<Value>, bool>(val, true);
	    }
	}
	else
	{
	    return std::pair<Handle<Value>, bool>(ir.first->second, false);
	}
    }

    bool pin(const Key& key, const Handle<Value>& val)
    {
	Mutex::Lock sync(_mutex);
	std::pair<CacheMap::iterator, bool> ir = 
	    _map.insert(CacheMap::value_type(key, val));
	if(ir.second)
	{
	    pinned(val, ir.first);
	}
	return ir.second;
    }

protected:

    virtual Handle<Value> load(const Key&) = 0;

    virtual void pinned(const Handle<Value>&, Position p)
    {
    }
    
    virtual ~Cache()
    {
    }

private:

    typedef std::map<Key, Handle<Value> > CacheMap;
    
    Mutex _mutex;
    CacheMap  _map;
};

}

#endif
