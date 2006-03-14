// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Time.h>
#include <Ice/GC.h>
#include <Ice/GCRecMutex.h>
#include <Ice/GCShared.h>
#include <map>


namespace IceInternal
{

void
recursivelyReachable(GCShared* p, GCObjectSet& o)
{
    if(o.find(p) == o.end())
    {
	assert(p);
	o.insert(p);
	GCObjectMultiSet tmp;
	p->__gcReachable(tmp);
	for(GCObjectMultiSet::const_iterator i = tmp.begin(); i != tmp.end(); ++i)
	{
	    recursivelyReachable(*i, o);
	}
    }
}

}

using namespace std;
using namespace IceUtil;

int IceInternal::GC::_numCollectors = 0;

IceInternal::GC::GC(int interval, StatsCallback cb)
{
    Monitor<Mutex>::Lock sync(*this);

    if(_numCollectors++ > 0)
    {
	abort(); // Enforce singleton.
    }
    _state = NotStarted;
    _collecting = false;
    _interval = interval;
    _statsCallback = cb;
}

IceInternal::GC::~GC()
{
    Monitor<Mutex>::Lock sync(*this);

    --_numCollectors;
}

void
IceInternal::GC::run()
{
    assert(_interval > 0);

    {
	Monitor<Mutex>::Lock sync(*this);

	_state = Started;
	notify();
    }

    Time waitTime = Time::seconds(_interval);
    while(true)
    {
	bool collect = false;
	{
	    Monitor<Mutex>::Lock sync(*this);

	    if(_state == Stopping)
	    {
		_state = Stopped;
		return;
	    }
	    if(!timedWait(waitTime))
	    {
		collect = true;
	    }
	}
	if(collect)
	{
	    collectGarbage();
	}
    }
}

void
IceInternal::GC::stop()
{
    {
	Monitor<Mutex>::Lock sync(*this);

	if(_state >= Stopping)
	{
	    return; // Don't attempt to stop the thread twice.
	}

	//
	// Wait until the thread is actually started. (If we don't do this, we
	// can get a problem if a call to stop() immediately follows a call to start():
	// the call to stop() may happen before pthread_create() has scheduled the thread's run()
	// function, and then the notify() that is used to tell the thread to stop can be lost.
	//
	while(_state < Started)
	{
	    wait();
	}
    }

    //
    // Tell the thread to stop.
    //
    {
	Monitor<Mutex>::Lock sync(*this);
	_state = Stopping;
	notify();
    }

    getThreadControl().join();
    assert(_state == Stopped);
}

void
IceInternal::GC::collectGarbage()
{
    //
    // Do nothing if the collector is running already.
    //
    {
	Monitor<Mutex>::Lock sync(*this);

	if(_collecting)
	{
	    return;
	}
	_collecting = true;
    }

    RecMutex::Lock sync(*gcRecMutex._m); // Prevent any further class reference count activity.

    Time t;
    GCStats stats;

    if(_statsCallback)
    {
	t = Time::now();
	stats.examined = static_cast<int>(gcObjects.size());
    }

    typedef map<GCShared*, int> CountMap;
    CountMap counts;

    {
	//
	// gcObjects contains the set of class instances that have at least one member of class type,
	// that is, gcObjects contains all those instances that can point at other instances.
	//
	// Create a map that, for each object in gcObjects, contains an <object, refcount> pair.
	// In addition, for each object in gcObjects, add the objects that are immediately (not
	// recursively) reachable from that object to a multiset. (We use a multiset because a
	// single object can be pointed at several times, either by itself or by other objects.)
	//
	GCObjectMultiSet reachable;
	{
	    for(GCObjectSet::const_iterator i = gcObjects.begin(); i != gcObjects.end(); ++i)
	    {
		counts.insert(CountMap::value_type(*i, (*i)->__getRefUnsafe()));
		(*i)->__gcReachable(reachable);
	    }
	}

	//
	// Decrement the reference count in the counts map once once for each entry in the multiset.
	// This drops the reference count of each map entry once for each time the corrsponding object
	// is pointed at.
        //
	{
	    for(GCObjectMultiSet::const_iterator i = reachable.begin(); i != reachable.end(); ++i)
	    {
		CountMap::iterator pos = counts.find(*i);
		assert(pos != counts.end());
		--(pos->second);
	    }
	}
    }

    {
	//
	// Any instances with a ref count > 0 are referenced from outside the objects in gcObjects (and
	// are therefore reachable from the program, for example, via Ptr variable on the stack). The set
	// of live objects therefore are all the objects with a reference count > 0, as well as all
	// objects that are (recursively) reachable from these objects.
	//
	GCObjectSet liveObjects;
	{
	    for(CountMap::const_iterator i = counts.begin(); i != counts.end(); ++i)
	    {
		if(i->second > 0)
		{
		    recursivelyReachable(i->first, liveObjects);
		}
	    }
	}

	//
	// Remove all live objects from the map.
	//
	{
	    for(GCObjectSet::const_iterator i = liveObjects.begin(); i != liveObjects.end(); ++i)
	    {
#ifndef NDEBUG
		bool erased =
#endif
		    counts.erase(*i);
		assert(erased);
	    }
	}
    }

    //
    // What is left in the counts map can be garbage collected.
    //
    {
	CountMap::const_iterator i;
	for(i = counts.begin(); i != counts.end(); ++i)
	{
	    //
	    // For classes with members that point at potentially-cyclic instances, __gcClear()
	    // decrements the reference count of those instances and clears the
	    // corrsponding Ptr members.
	    // For classes that cannot be part of a cycle (because they do not contain class members)
	    // and are therefore true leaves, __gcClear() assigns 0 to the corresponding class member,
	    // which either decrements the ref count or, if it reaches zero, deletes the instance as usual.
	    //
	    i->first->__gcClear();
	}
	for(i = counts.begin(); i != counts.end(); ++i)
	{
	    gcObjects.erase(i->first); // Remove this object from candidate set.
	    delete i->first; // Delete this object.
	}
    }

    if(_statsCallback)
    {
	stats.time = Time::now() - t;
	stats.collected = static_cast<int>(counts.size());
	_statsCallback(stats);
    }

    //
    // We clear explicitly under protection of the lock, instead of waiting for the
    // counts destructor. This avoids lots of lock contention later because, otherwise,
    // the destructor of each object in the counts set would acquire and release
    // gcRecMutex._m.
    //
    counts.clear();

    {
	Monitor<Mutex>::Lock sync(*this);

	_collecting = false;
    }
}
