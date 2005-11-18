// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

    //
    // gcObjects contains the set of class instances that have at least one member of class type,
    // that is, gcObjects contains all those instances that can point at other instances.
    // Call this the the candiate set.
    // Build a multiset of instances that are immediately (not recursively) reachable from instances
    // in the candidate set. This adds leaf nodes (class instances that are pointed at, but cannot
    // point at anything themselves) to the multiset.
    //
    GCObjectMultiSet reachable;
    {
	for(GCObjectSet::const_iterator i = gcObjects.begin(); i != gcObjects.end(); ++i)
	{
	    (*i)->__gcReachable(reachable);
	}
    }

    //
    // Create a map of reference counts.
    //
    typedef map<GCShared*, int> ObjectCounts;
    ObjectCounts counts;
    {
	ObjectCounts::iterator pos;
	for(GCObjectMultiSet::const_iterator i = reachable.begin(); i != reachable.end(); ++i)
	{
	    pos = counts.find(*i);

	    //
	    // If this instance is not in the counts set yet, insert it with its reference count - 1;
	    // otherwise, decrement its reference count.
	    //
	    if(pos == counts.end())
	    {
		counts.insert(pos, ObjectCounts::value_type(*i, (*i)->__getRefUnsafe() - 1));
	    }
	    else
	    {
		--(pos->second);
	    }
	}
    }

    //
    // Any instances with a ref count > 0 are referenced from outside the set of class instances (and therefore
    // reachable from the program, for example, via Ptr variable on the stack). Remove these instances
    // (and all instances reachable from them) from the overall set of objects.
    //
    {
	GCObjectSet liveObjects;
	for(ObjectCounts::const_iterator i = counts.begin(); i != counts.end(); ++i)
	{
	    if(i->second > 0)
	    {
		recursivelyReachable(i->first, liveObjects);
	    }
	}

	for(GCObjectSet::const_iterator j = liveObjects.begin(); j != liveObjects.end(); ++j)
	{
	    counts.erase(*j);
	}
    }

    //
    // What is left in the counts set can be garbage collected.
    //
    {
	ObjectCounts::const_iterator i;
	for(i = counts.begin(); i != counts.end(); ++i)
	{
	    i->first->__gcClear(); // Decrement ref count of objects pointed at by this object.
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
