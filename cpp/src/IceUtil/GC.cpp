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
    if(_numCollectors++ > 0)
    {
	abort(); // Enforce singleton.
    }

    Monitor<Mutex>::Lock sync(*this);
    _running = false;
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
    {
	Monitor<Mutex>::Lock sync(*this);
	if(_interval == 0)
	{
	    return;
	}
	_running = true;
    }

    while(true)
    {
	bool collect = false;
	{
	    Time waitTime = Time::seconds(_interval);
	    Monitor<Mutex>::Lock sync(*this);
	    if(!_running)
	    {
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
	if(!_running)
	{
	    return;
	}
    }
    {
	Monitor<Mutex>::Lock sync(*this);
	_running = false;
	notify();
    }
    getThreadControl().join();
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

    RecMutex::Lock sync(*gcRecMutex._m);

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
	stats.collected = 0;
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

	for(GCObjectSet::const_iterator k = reachable.begin(); k != reachable.end(); ++k)
	{
	    counts.erase(*k);
	}
    }

    //
    // What is left in the counts set can be garbage collected.
    //
    {
	ObjectCounts::const_iterator i;
	for(i = counts.begin(); i != counts.end(); ++i)
	{
	    i->first->__gcClear();
	}
	for(i = counts.begin(); i != counts.end(); ++i)
	{
	    delete i->first;
	    if(_statsCallback)
	    {
		++stats.collected;
	    }
	}
	counts.clear();
    }

    if(_statsCallback)
    {
	stats.msec = (Time::now() - t) * 1000.0L;
	_statsCallback(stats);
    }

    {
	Monitor<Mutex>::Lock sync(*this);

	_collecting = false;
    }
}
