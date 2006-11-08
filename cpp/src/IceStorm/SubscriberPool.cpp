// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/SubscriberPool.h>
#include <IceStorm/Instance.h>
#include <IceStorm/TraceLevels.h>
#include <IceStorm/Subscribers.h>

#include <Ice/Communicator.h>
#include <Ice/Properties.h>
#include <Ice/LoggerUtil.h>

using namespace IceStorm;
using namespace std;

namespace IceStorm
{

class SubscriberPoolWorker : public IceUtil::Thread
{
public:

    SubscriberPoolWorker(const SubscriberPoolPtr& manager) :
	_manager(manager)
    {
	start();
    }

    ~SubscriberPoolWorker()
    {
    }

    virtual void
    run()
    {
	SubscriberPtr sub;
	while(true)
	{
	    sub = _manager->dequeue(sub);
	    if(!sub)
	    {
		return;
	    }
	    //
	    // If SubscriberPool returns true then the subscriber needs to be
	    // SubscriberPooled again, so therefore we will re-enqueue the
	    // subscriber in the call to dequeue.
	    //
	    if(!sub->flush())
	    {
		sub = 0;
	    }
	}
    }

private:

    const SubscriberPoolPtr _manager;
};

class SubscriberPoolMonitor : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    SubscriberPoolMonitor(const SubscriberPoolPtr& manager, const IceUtil::Time& timeout) :
	_manager(manager),
	_timeout(timeout),
	_needCheck(false),
	_destroy(false)
    {
	start();
    }

    ~SubscriberPoolMonitor()
    {
    }

    virtual void
    run()
    {
	for(;;)
	{
	    {
		Lock sync(*this);
		if(_destroy)
		{
		    return;
		}

		if(_needCheck)
		{
		    timedWait(_timeout);
		    //
		    // Monitoring was stopped.
		    //
		    if(!_needCheck)
		    {
			continue;
		    }
		}
		else
		{
		    wait();
		    continue;
		}
	    }
	    //
	    // Call outside of the lock to prevent any deadlocks.
	    //
	    _manager->check();
	}
    }

    void
    startMonitor()
    {
	Lock sync(*this);
	if(!_needCheck)
	{
	    _needCheck = true;
	    notify();
	}
    }

    void
    stopMonitor()
    {
	Lock sync(*this);
	_needCheck = false;
    }

    void
    destroy()
    {
	Lock sync(*this);
	_destroy = true;
	notify();
    }

private:

    const SubscriberPoolPtr _manager;
    const IceUtil::Time _timeout;
    bool _needCheck;
    bool _destroy;
};

}

SubscriberPool::SubscriberPool(const InstancePtr& instance) :
    _instance(instance),
    _sizeMax(instance->properties()->getPropertyAsIntWithDefault(
		 "IceStorm.SubscriberPool.SizeMax", -1)),
    _sizeWarn(instance->properties()->getPropertyAsIntWithDefault(
		  "IceStorm.SubscriberPool.SizeWarn", 0)),
    _size(instance->properties()->getPropertyAsIntWithDefault(
	      "IceStorm.SubscriberPool.Size", 1)),
    _timeout(IceUtil::Time::milliSeconds(max(instance->properties()->getPropertyAsIntWithDefault(
						 "IceStorm.SubscriberPool.Timeout", 250), 50))), // minimum 50ms.
    _destroy(false),
    _inUse(0),
    _running(0),
    _load(1.0)
{
    try
    {
	__setNoDelete(true);
	_subscriberPoolMonitor = new SubscriberPoolMonitor(this, _timeout);
	for(int i = 0; i < _size; ++i)
	{
	    ++_running;
	    ++_inUse;
	    _workers.push_back(new SubscriberPoolWorker(this));
	}
    }
    catch(const IceUtil::Exception& ex)
    {
	{
	    Ice::Error out(_instance->traceLevels()->logger);
	    out << "SubscriberPool: " << ex;
	}
	destroy();
	__setNoDelete(false);
	throw;
    }

    __setNoDelete(false);
}

SubscriberPool::~SubscriberPool()
{
}
    
void
SubscriberPool::add(list<SubscriberPtr>& subscribers)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    //
    // Splice on the new set of subscribers to SubscriberPool.
    //
    _pending.splice(_pending.end(), subscribers);
    notifyAll();
}

//
// The passed subscriber need to be enqueued again.
//
SubscriberPtr
SubscriberPool::dequeue(const SubscriberPtr& sub)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(sub)
    {
	_pending.push_back(sub);
    }

    //
    // Now we check if this thread can be destroyed, based on a
    // load factor.
    //
    // The load factor jumps immediately to the number of threads
    // that are currently in use, but decays exponentially if the
    // number of threads in use is smaller than the load
    // factor. This reflects that we create threads immediately
    // when they are needed, but want the number of threads to
    // slowly decline to the configured minimum.
    //
    double inUse = static_cast<double>(_inUse);
    if(_load < inUse)
    {
	_load = inUse;
    }
    else
    {
	const double loadFactor = 0.05; // TODO: Configurable?
	const double oneMinusLoadFactor = 1 - loadFactor;
	_load = _load * oneMinusLoadFactor + inUse * loadFactor;
    }

    if(_running > _size)
    {
	TraceLevelsPtr traceLevels = _instance->traceLevels();
	if(traceLevels->subscriberPool > 1)
	{
	    IceUtil::Time interval = IceUtil::Time::now() - _lastNext;
	    Ice::Trace out(traceLevels->logger, traceLevels->subscriberPoolCat);
	    out << "load check: " << _load;
	}

	int load = static_cast<int>(_load + 0.5);

	if(load < _running)
	{
	    assert(_inUse > 0);
	    --_inUse;

	    assert(_running > 0);
	    --_running;

	    if(traceLevels->subscriberPool > 0)
	    {
		Ice::Trace out(traceLevels->logger, traceLevels->subscriberPoolCat);
		out << "reducing SubscriberPool threads: load: " << _load << " threads: " << _running;
	    }

	    return 0;
	}
	
    }

    assert(_inUse > 0);
    --_inUse;

    while(_pending.empty() && !_destroy)
    {
	//
	// If we wait then there is no need to monitor anymore.
	//
	_subscriberPoolMonitor->stopMonitor();
	wait();
    }

    if(_destroy)
    {
	--_running;
	return 0;
    }

    _lastNext = IceUtil::Time::now();

    SubscriberPtr subscriber = _pending.front();
    _pending.pop_front();

    ++_inUse;

    //
    // If all threads are now in use then we need to start the
    // monitoring, otherwise we don't need to monitor.
    //
    if(_inUse == _running && (_running < _sizeMax || _sizeMax == -1))
    {
	_subscriberPoolMonitor->startMonitor();
    }
    else
    {
	_subscriberPoolMonitor->stopMonitor();
    }
    return subscriber;
}

void
SubscriberPool::destroy()
{
    //
    // First mark the pool as destroyed. This causes all of the worker
    // threads to unblock and terminate.
    //
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	_destroy = true;
	notifyAll();
	if(_subscriberPoolMonitor)
	{
	    _subscriberPoolMonitor->destroy();
	}
    }

    //
    // Next join with each worker.
    //
    for(list<IceUtil::ThreadPtr>::const_iterator p = _workers.begin(); p != _workers.end(); ++p)
    {
	(*p)->getThreadControl().join();
    }
    _workers.clear();

    //
    // Once all of the workers have gone then we'll no longer have
    // concurrent access to the pool monitor, so we can join with it
    // and then clear to remove the circular reference count.
    //
    if(_subscriberPoolMonitor)
    {
	_subscriberPoolMonitor->getThreadControl().join();
	_subscriberPoolMonitor = 0;
    }
}

void
SubscriberPool::check()
{
    Lock sync(*this);

    TraceLevelsPtr traceLevels = _instance->traceLevels();
    IceUtil::Time interval = IceUtil::Time::now() - _lastNext;
    if(traceLevels->subscriberPool > 1)
    {
	Ice::Trace out(traceLevels->logger, traceLevels->subscriberPoolCat);
	out << "check called: interval: "  << interval << " timeout: " << _timeout
	    << " pending: " << _pending.size() << " running: " << _running
	    << " sizeMax: " << _sizeMax;
    }
    
    if(interval > _timeout && _pending.size() > 0 && (_running < _sizeMax || _sizeMax == -1))
    {
	if(traceLevels->subscriberPool > 0)
	{
	    Ice::Trace out(traceLevels->logger, traceLevels->subscriberPoolCat);
	    out << "detected stall: creating thread: load: " << _load << " threads: " << _running;
	}
	
	++_running;
	++_inUse;
	_workers.push_back(new SubscriberPoolWorker(this));
    }
}
