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

#include <Ice/Ice.h>
#include <IcePack/WaitQueue.h>

using namespace std;

IcePack::WaitItem::WaitItem(const Ice::ObjectPtr& object) :
    _object(object)
{
}

IcePack::WaitItem::~WaitItem()
{
}

const IceUtil::Time&
IcePack::WaitItem:: getExpirationTime()
{
    return _expiration;
}

void 
IcePack::WaitItem::setExpirationTime(const IceUtil::Time& time)
{
    _expiration = time;
}

IcePack::WaitQueue::WaitQueue() : _destroyed(false)
{
}

void
IcePack::WaitQueue::run()
{
    IceUtil::Monitor< IceUtil::Mutex>::Lock sync(*this);

    while(true)
    {
	if(_waitQueue.empty() && _workQueue.empty() && !_destroyed)
	{
	    wait();
	}
	
	if(!_workQueue.empty())
	{
	    //
	    // Execute all the work queue items.
	    //
	    for(list<WaitItemPtr>::iterator p = _workQueue.begin(); p != _workQueue.end(); ++p)
	    {
		try
		{
		    if(_destroyed)
		    {
			(*p)->expired(true);
		    }
		    else
		    {
			(*p)->execute();
		    }
		}
		catch(...)
		{
		}
	    }
	    _workQueue.clear();
	}

	if(_destroyed)
	{
	    break;
	}

	if(!_waitQueue.empty())
	{
	    //
	    // Notify expired items.
	    //
	    while(!_waitQueue.empty())
	    {
		WaitItemPtr item = _waitQueue.front();

		if(item->getExpirationTime() <= IceUtil::Time::now())
		{
		    try
		    {
			item->expired(false);
		    }
		    catch(...)
		    {
		    }
		    _waitQueue.pop_front();		
		}
		else
		{
		    //
		    // Wait until the next item expire or a notification. Note: in any case we 
		    // get out of this loop to get a chance to execute the work queue.
		    //
		    timedWait(item->getExpirationTime() - IceUtil::Time::now());
		    break;
		}
	    }
	}
    }

    assert(_workQueue.empty());

    if(!_waitQueue.empty())
    {
	for(list<WaitItemPtr>::iterator p = _waitQueue.begin(); p != _waitQueue.end(); ++p)
	{
	    (*p)->expired(true);
	}
    }
}

void
IcePack::WaitQueue::destroy()
{
    {
	IceUtil::Monitor< IceUtil::Mutex>::Lock sync(*this);
	_destroyed = true;
	notify();
    }

    getThreadControl().join();
}

void
IcePack::WaitQueue::add(const WaitItemPtr& item, const IceUtil::Time& wait)
{
    IceUtil::Monitor< IceUtil::Mutex>::Lock sync(*this);

    //
    // We'll have to notify the thread if it's sleeping for good.
    //
    bool notifyThread = _workQueue.empty() && _waitQueue.empty();

    if(wait == IceUtil::Time::seconds(0))
    {
	item->setExpirationTime(IceUtil::Time::now());
	_workQueue.push_back(item);
    }
    else
    {
	IceUtil::Time expire = IceUtil::Time::now() + wait;
	item->setExpirationTime(expire);
	
	list<WaitItemPtr>::iterator p = _waitQueue.begin();
	while(p != _waitQueue.end())
	{
	    if((*p)->getExpirationTime() >= expire)
	    {
		break;
	    }
	    ++p;
	}
	_waitQueue.insert(p, item);
    }

    if(notifyThread)
    {
	notify();
    }
}

void
IcePack::WaitQueue::notifyAllWaitingOn(const Ice::ObjectPtr& object)
{
    IceUtil::Monitor< IceUtil::Mutex>::Lock sync(*this);

    //
    // TODO: OPTIMIZATION: Use a map with the object as a key.
    //

    list<WaitItemPtr>::iterator p = _waitQueue.begin();
    while(p != _waitQueue.end())
    {
	if((*p)->isWaitingOn(object))
	{
	    _workQueue.push_back(*p);
	    p = _waitQueue.erase(p);
	}
	else
	{
	    ++p;
	}
    }

    if(!_workQueue.empty())
    {
	notify();
    }
}
