// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Thread.h>
#include <IceUtil/Monitor.h>
#include <Ice/Ice.h>
#include <IceStorm/Flushable.h>
#include <IceStorm/TraceLevels.h>
#include <IceStorm/Flusher.h>
#include <algorithm>

using namespace IceStorm;
using namespace std;

FlusherThread::FlusherThread(const Ice::CommunicatorPtr& communicator, const TraceLevelsPtr& traceLevels) :
    _communicator(communicator),
    _traceLevels(traceLevels),
    _destroy(false)
{
    _flushTime = communicator->getProperties()->getPropertyAsIntWithDefault("IceStorm.Flush.Timeout", 1000);
    if(_flushTime < 100)
    {
        _flushTime = 100; // Minimum of 100 ms
    }
}
    
FlusherThread::~FlusherThread()
{
}

void
FlusherThread::run()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    while(!_destroy)
    {
        long tout = calcTimeout();
        if(tout == 0)
        {
    	    wait();
        }
        else
        {
    	    timedWait(IceUtil::Time::milliSeconds(tout));
        }
        if(_destroy)
        {
    	    continue;
        }
        flushAll();
    }

    //
    // We break a cycle by clearing the subscriber list.
    //
    _subscribers.clear();
}

void
FlusherThread::destroy()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    _destroy = true;
    notify();
}

//
// It would be possible to write add/remove in such a way as to
// avoid blocking while flushing by having a queue of actions
// which are only performed before flushing. For now, however,
// this issue is ignored.
//
void
FlusherThread::add(const FlushablePtr& subscriber)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    bool isEmpty = _subscribers.empty();
    _subscribers.push_back(subscriber);

    //
    // If the set of subscribers was previously empty then wake up
    // the flushing thread since it will be waiting indefinitely
    //
    if(isEmpty)
    {
        notify();
    }
}

void
FlusherThread::remove(const FlushablePtr& subscriber)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    _subscribers.remove(subscriber);
}

void
FlusherThread::flushAll()
{
    // This is always called with the monitor locked
    //IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);


    // remove_if doesn't work with handle types. remove_if also
    // isn't present in the STLport implementation
    //
    // _subscribers.remove_if(IceUtil::constMemFun(&Flushable::inactive));
    //
    _subscribers.erase(remove_if(_subscribers.begin(), _subscribers.end(),
    			     IceUtil::constMemFun(&Flushable::inactive)), _subscribers.end());
    
    _communicator->flushBatchRequests();
    
    //
    // Trace after the flush so that the correct number of objects
    // are displayed
    //
    if(_traceLevels->flush > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->flushCat);
        out << _subscribers.size() << " object(s)";
    }
}

long
FlusherThread::calcTimeout()
{
    return (_subscribers.empty()) ? 0 : _flushTime;
}

Flusher::Flusher(const Ice::CommunicatorPtr& communicator, const TraceLevelsPtr& traceLevels)
{
    _thread = new FlusherThread(communicator, traceLevels);
    _thread->start();
}

Flusher::~Flusher()
{
}

void
Flusher::add(const FlushablePtr& subscriber)
{
    _thread->add(subscriber);
}

void
Flusher::remove(const FlushablePtr& subscriber)
{
    _thread->remove(subscriber);
}

void
Flusher::stopFlushing()
{
    _thread->destroy();
    _thread->getThreadControl().join();
}
