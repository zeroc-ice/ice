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

#include <IceUtil/GC.h>
#include <IceUtil/Time.h>
#include <IceUtil/GCRecMutex.h>
#include <IceUtil/GCShared.h>
#include <map>


namespace IceUtil
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

int IceUtil::GC::_numCollectors = 0;

IceUtil::GC::GC(int interval, StatsCallback cb)
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

IceUtil::GC::~GC()
{
    Monitor<Mutex>::Lock sync(*this);

    --_numCollectors;
}

void
IceUtil::GC::run()
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
IceUtil::GC::stop()
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
IceUtil::GC::collectGarbage()
{
    {
	Monitor<Mutex>::Lock sync(*this);

	if(_collecting)
	{
	    return;
	}
	_collecting = true;
    }

    typedef map<GCShared*, int> ObjectCounts;
    ObjectCounts counts;

    Time t;
    GCStats stats;

    RecMutex::Lock sync(*gcRecMutex._m); // Prevent any further class reference count activity.

    if(_statsCallback)
    {
	t = Time::now();
    }

    //
    // Initialize a set of pairs <GCShared*, ref count> for all class instances.
    //
    {
	for(GCObjectSet::const_iterator i = gcObjects.begin(); i != gcObjects.end(); ++i)
	{
	    assert(*i);
	    counts[*i] = (*i)->_ref;
	}
    }

    if(_statsCallback)
    {
	stats.examined = counts.size();
    }

    //
    // For each class instance in the set, find which class instances can be reached from that instance.
    // For each reachable instance, decrement the reachable instance's ref count.
    //
    {
	for(GCObjectSet::const_iterator i = gcObjects.begin(); i != gcObjects.end(); ++i)
	{
	    GCObjectMultiSet reachable;
	    (*i)->__gcReachable(reachable);
	    for(GCObjectMultiSet::const_iterator j = reachable.begin(); j != reachable.end(); ++j)
	    {
		--(counts.find(*j)->second);
	    }
	}
    }

    //
    // Any instances with a ref count > 0 are referenced from outside the set of class instances (and therefore
    // reachable from the program, for example, via Ptr variable on the stack). Remove these reachable instances
    // (and all instances reachable from them) from the overall set of objects.
    //
    {
	GCObjectSet reachable;
	for(ObjectCounts::const_iterator i = counts.begin(); i != counts.end(); ++i)
	{
	    if(i->second > 0)
	    {
		recursivelyReachable(i->first, reachable);
	    }
	}

	for(GCObjectSet::const_iterator j = reachable.begin(); j != reachable.end(); ++j)
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
	stats.msec = (Time::now() - t) * 1000.0L;
	stats.collected = counts.size();
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
