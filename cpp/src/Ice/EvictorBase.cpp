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

#include <Ice/EvictorBase.h>
#include <Ice/LocalException.h>
#include <Ice/Properties.h>
#include <Ice/Communicator.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/LoggerUtil.h>
#include <iomanip>

using namespace std;
using namespace Ice;

Ice::EvictorBase::EvictorBase()
    : _size(0),
      _initialized(false),
      _hits(0),
      _misses(0),
      _evictions(0),
      _traceLevel(0),
      _communicator(0)
{
}

void
Ice::EvictorBase::setSize(Int size)
{
    if(size < 1)
    {
	return; // Ignore stupid values.
    }

    IceUtil::Mutex::Lock lock(_mutex);
    _size = size;
    evictServants();
}

Int
Ice::EvictorBase::getSize() const
{
    IceUtil::Mutex::Lock lock(_mutex);
    return _size;
}

ObjectPtr
Ice::EvictorBase::locate(const Current& c, LocalObjectPtr& cookie)
{
    IceUtil::Mutex::Lock lock(_mutex);

    if(!_initialized) // Lazy initialization of evictor size and tracing.
    {
	_initialized = true;
	initialize(c);
    }

    //
    // Check if we have an entry for the servant already.
    //
    EvictorCookiePtr ec = new EvictorCookie;
    cookie = ec;
    EvictorMap::iterator i = _map.find(c.id);
    if(i != _map.end())
    {
	//
	// Got an entry already, dequeue the entry from its current position.
	//
	ec->entry = i->second;
	_queue.erase(ec->entry->pos);

	++_hits; // Update statistics.
	if(_traceLevel >= 3)
	{
	    Trace out(_communicator->getLogger(), "Ice::Evictor");
	    if(c.id.category.empty())
	    {
		out << "default category";
	    }
	    else
	    {
		out << "category`" << c.id.category << "'";
	    }
	    out << ": cache hit for name `" << c.id.name << "'";
	    out.flush();
	}
    }
    else
    {
	//
	// Don't have an entry yet. Ask the application to instantiate a servant
	// and create a new entry in the map.
	//
	ec->entry = new EvictorEntry;
	ec->entry->servant = instantiate(c, ec->entry->userCookie); // Down-call to application-supplied method.
	if(!ec->entry->servant)
	{
	    throw ObjectNotExistException(__FILE__, __LINE__);
	}
	ec->entry->useCount = 0;
	_map[c.id] = ec->entry;

	++_misses; // Update statistics.
	if(_traceLevel >= 2)
	{
	    Trace out(_communicator->getLogger(), "Ice::Evictor");
	    if(c.id.category.empty())
	    {
		out << "default category";
	    }
	    else
	    {
		out << "category`" << c.id.category << "'";
	    }
	    out << ": cache miss for name `" << c.id.name << "'";
	    out.flush();
	}
    }

    //
    // Increment the use count of the servant and enqueue the entry at the front,
    // so we get LRU order.
    //
    ++(ec->entry->useCount);
    _queue.push_front(c.id);
    ec->entry->pos = _queue.begin();

    //
    // If we added an entry, that might make another entry eligible for eviction.
    //
    if(i == _map.end())
    {
	evictServants();
    }

    return ec->entry->servant;
}

void
Ice::EvictorBase::finished(const Current&, const ObjectPtr&, const LocalObjectPtr& cookie)
{
    IceUtil::Mutex::Lock lock(_mutex);

    EvictorCookiePtr ec = EvictorCookiePtr::dynamicCast(cookie);

    //
    // Decrement use count and check if there is something to evict.
    //
    --(ec->entry->useCount);
    evictServants();
}

void
Ice::EvictorBase::deactivate(const string& category)
{
    IceUtil::Mutex::Lock lock(_mutex);

    if(_traceLevel >= 1)
    {
	Trace out(_communicator->getLogger(), "Ice::Evictor");
	out << "deactivating ";
	if(category.empty())
	{
	    out << "default category";
	}
	else
	{
	    out << "category `" << category << "'";
	}
	out << ", number of cached servants = " << _map.size();
	out.flush();

	out << "#evictions = " << _evictions;
	out << ", #hits = " << _hits;
	out << ", #misses = " << _misses;
	double total = (double)_hits + (double)_misses;
	double ratio = total == 0 ? 100 : ((double)_hits * 100 / total);
	out << ", %hits = " << fixed << setprecision(2) << ratio << "%";
	out.flush();
    }

    _size = 0;
    evictServants();
}

void
Ice::EvictorBase::evictServants()
{
    //
    // If the evictor queue has grown larger than the limit,
    // look at the excess elements to see whether any of them
    // can be evicted.
    //
    for(int i = static_cast<int>(_map.size() - _size); i > 0; --i)
    {
	EvictorQueue::reverse_iterator p = _queue.rbegin();
	assert(p != _queue.rend());
	EvictorMap::iterator mPos = _map.find(*p);
	assert(mPos != _map.end());
	if(mPos->second->useCount == 0)
	{
	    if(_traceLevel >= 2)
	    {
		Trace out(_communicator->getLogger(), "Ice::Evictor");
		if((*p).category.empty())
		{
		    out << "default category";
		}
		else
		{
		    out << "category `" << (*p).category << "'";
		}
		out << ": evicting `" << (*p).name << "'";
		out.flush();
	    }

	    evict(*p, mPos->second->servant, mPos->second->userCookie); // Down-call to application-supplied method.
	    _queue.erase(mPos->second->pos);
	    _map.erase(mPos);
	    ++_evictions; // Update statistics.
	}
    }
}

void
Ice::EvictorBase::initialize(const Current& c)
{
    _communicator = c.adapter->getCommunicator();

    PropertiesPtr p = _communicator->getProperties();
    Int num;

    //
    // Check evictor size properties.
    //
    if((num = p->getPropertyAsInt("Ice.Evictor.Size")) > 0)
    {
	_size = num;
    }
    if((num = p->getPropertyAsInt(c.adapter->getName() + ".Evictor.Size")) > 0)
    {
	_size = num;
    }
    if(!c.id.category.empty()
	&& (num = p->getPropertyAsInt(c.adapter->getName() + "." + c.id.category + ".Evictor.Size")) > 0)
    {
	_size = num;
    }
    if(_size < 1)
    {
	_size = defaultSize;
    }

    //
    // Check evictor trace properties.
    //
    if(!p->getProperty("Ice.Evictor.Trace").empty())
    {
	_traceLevel = p->getPropertyAsInt("Ice.Evictor.Trace");
    }
    if(!p->getProperty(c.adapter->getName() + ".Evictor.Trace").empty())
    {
	_traceLevel = p->getPropertyAsInt(c.adapter->getName() + ".Evictor.Trace");
    }
    if(!c.id.category.empty()
       && !p->getProperty(c.adapter->getName() + "." + c.id.category + ".Evictor.Trace").empty())
    {
	_traceLevel = p->getPropertyAsInt(c.adapter->getName() + "." + c.id.category + ".Evictor.Trace");
    }
}
