// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/SubscriberPool.h>
#include <IceStorm/Instance.h>
#include <IceStorm/TraceLevels.h>
#include <IceStorm/Subscriber.h>

#include <Ice/Communicator.h>
#include <Ice/Properties.h>
#include <Ice/LoggerUtil.h>

using namespace IceStorm;
using namespace std;

namespace
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
        IceUtil::Time interval = IceUtil::Time::seconds(24 * 60); // A long time.
        SubscriberPtr sub;
        bool requeue = false;
        bool computeInterval = false;
        while(true)
        {
            _manager->dequeue(sub, requeue, interval, computeInterval);
            if(!sub)
            {
                return;
            }

            //
            // If flush returns true then the subscriber needs to be
            // flushed again, so therefore we will re-enqueue the
            // subscriber in the call to dequeue.
            //
            if(computeInterval)
            {
                IceUtil::Time start = IceUtil::Time::now();
                requeue = sub->flush();
                interval = IceUtil::Time::now() - start;
            }
            else
            {
                requeue = sub->flush();
                interval = IceUtil::Time::seconds(24 * 60); // A long time.
            }
        }
    }

private:

    const SubscriberPoolPtr _manager;
};

}

SubscriberPoolMonitor::SubscriberPoolMonitor(const SubscriberPoolPtr& manager, const IceUtil::Time& timeout) :
    _manager(manager),
    _timeout(timeout),
    _needCheck(false),
    _destroyed(false)
{
    start();
}

SubscriberPoolMonitor::~SubscriberPoolMonitor()
{
}

void
SubscriberPoolMonitor::run()
{
    for(;;)
    {
        {
            Lock sync(*this);
            if(_destroyed)
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
                if(_destroyed)
                {
                    return;
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
SubscriberPoolMonitor::startMonitor()
{
    Lock sync(*this);
    if(!_needCheck)
    {
        _needCheck = true;
        notify();
    }
}

void
SubscriberPoolMonitor::stopMonitor()
{
    Lock sync(*this);
    _needCheck = false;
}

void
SubscriberPoolMonitor::destroy()
{
    Lock sync(*this);
    _destroyed = true;
    notify();
}

SubscriberPool::SubscriberPool(const InstancePtr& instance) :
    _traceLevels(instance->traceLevels()),
    _sizeMax(instance->properties()->getPropertyAsIntWithDefault("IceStorm.SubscriberPool.SizeMax", 0)),
    _sizeWarn(instance->properties()->getPropertyAsIntWithDefault("IceStorm.SubscriberPool.SizeWarn", 0)),
    _size(instance->properties()->getPropertyAsIntWithDefault("IceStorm.SubscriberPool.Size", 1)),
    // minimum 50ms, default 1s.
    _timeout(IceUtil::Time::milliSeconds(max(instance->properties()->getPropertyAsIntWithDefault(
                                                 "IceStorm.SubscriberPool.Timeout", 1000), 50))),
    // 10 * the stall timeout.
    _stallCheck(_timeout * 10),
    _destroyed(false),
    _reap(0),
    _inUse(0)
{
    try
    {
        __setNoDelete(true);
        _subscriberPoolMonitor = new SubscriberPoolMonitor(this, _timeout);
        for(unsigned int i = 0; i < _size; ++i)
        {
            ++_inUse;
            _workers.push_back(new SubscriberPoolWorker(this));
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        {
            Ice::Error out(_traceLevels->logger);
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
SubscriberPool::flush(list<SubscriberPtr>& subscribers)
{
    Lock sync(*this);
    if(_destroyed)
    {
        return;
    }
    //
    // Splice on the new set of subscribers to SubscriberPool.
    //
    _pending.splice(_pending.end(), subscribers);
    assert(invariants());
    notifyAll();
}

void
SubscriberPool::flush(const SubscriberPtr& subscriber)
{
    Lock sync(*this);
    if(_destroyed)
    {
        return;
    }
    _pending.push_back(subscriber);
    assert(invariants());
    notify();
}

void
SubscriberPool::add(const SubscriberPtr& subscriber)
{
    Lock sync(*this);
    if(_destroyed)
    {
        return;
    }
    _subscribers.push_back(subscriber);
    assert(invariants());
}

void
SubscriberPool::remove(const SubscriberPtr& subscriber)
{
    Lock sync(*this);
    if(_destroyed)
    {
        return;
    }
    //
    // Note that this cannot remove based on the subscriber id because
    // the pool is TopicManager scoped and not topic scoped therefore
    // its quite possible to have two subscribers with the same id in
    // the list.
    //
    list<SubscriberPtr>::iterator p = find(_subscribers.begin(), _subscribers.end(), subscriber);
    assert(p != _subscribers.end());
    _subscribers.erase(p);
}

//
// The passed subscriber need to be enqueued again.
//
void
SubscriberPool::dequeue(SubscriberPtr& subscriber, bool requeue, const IceUtil::Time& interval, bool& computeInterval)
{
    Lock sync(*this);

    if(_destroyed)
    {
        subscriber = 0;
        return;
    }

    if(subscriber)
    {
        if(requeue)
        {
            _pending.push_back(subscriber);
            //
            // Its necessary to notify here since this thread might go
            // on and kill itself in which case if another worker is
            // in wait() it will not wake up and process the subscriber.
            //
            notify();
            assert(invariants());
        }
        subscriber->flushTime(interval);
    }
    //
    // Clear the reference.
    //
    subscriber = 0;

    //
    // The worker is no longer in use.
    //
    --_inUse;

    //
    // If _sizeMax is 1 we never spawn up new threads if a stall is
    // detected.
    //
    if(_sizeMax != 1)
    {
        //
        // Reap dead workers, if necessary.
        //
        if(_reap > 0)
        {
            if(_traceLevels->subscriberPool > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->subscriberPoolCat);
                out << "reaping: " << _reap << " workers";
            }
            list<IceUtil::ThreadPtr>::iterator p = _workers.begin();
            while(p != _workers.end() && _reap > 0)
            {
                if(!(*p)->isAlive())
                {
                    (*p)->getThreadControl().join();
                    p = _workers.erase(p);
                    --_reap;
                }
                else
                {
                    ++p;
                }
            }
        }

        //
        // If we have extra workers every _stallCheck period we run
        // through the complete set of subscribers and determine how
        // many have stalled since the last check. If this number is
        // less than the number of extra threads then we terminate the
        // calling worker.
        //
        // - The flush time is protected by the subscriber pool mutex.
        // - The flush time is only computed if we have extra threads,
        // otherwise it is set to some large value.
        // - The max flush time is reset to the next sending interval
        // after after _stallCheck period.
        // - Every subscriber is considered to be stalled iff it has
        // never sent an event or we have just created the first
        // additional worker. The first handles the case where a
        // subscriber stalls for a long time on the first message
        // send. The second means that we can disable computation of
        // the flush latency if there are no additional threads.
        //
        if(_workers.size() > _size)
        {
            IceUtil::Time now = IceUtil::Time::now();
            if(now - _lastStallCheck > _stallCheck)
            {
                _lastStallCheck = now;
                unsigned int stalls = 0;
                for(list<SubscriberPtr>::const_iterator p = _subscribers.begin(); p != _subscribers.end(); ++p)
                {
                    if((*p)->pollMaxFlushTime(now) > _timeout)
                    {
                        ++stalls;
                    }
                }

                if(_traceLevels->subscriberPool > 0)
                {
                    Ice::Trace out(_traceLevels->logger, _traceLevels->subscriberPoolCat);
                    out << "checking stalls. extra workers: " << _workers.size() - _size
                        << " subscribers: " << _subscribers.size() << " stalls: " << stalls;
                }

                if((_workers.size() - _size) > stalls)
                {
                    if(_traceLevels->subscriberPool > 0)
                    {
                        Ice::Trace out(_traceLevels->logger, _traceLevels->subscriberPoolCat);
                        out << "destroying workers";
                    }
                    ++_reap;
                    return;
                }
            }
        }
    }
        
    while(_pending.empty() && !_destroyed)
    {
        //
        // If we wait then there is no need to monitor anymore.
        //
        _subscriberPoolMonitor->stopMonitor();
        wait();
    }

    if(_destroyed)
    {
        return;
    }

    _lastDequeue = IceUtil::Time::now();

    subscriber = _pending.front();
    _pending.pop_front();

    ++_inUse;

    //
    // If all threads are now in use then we need to start the
    // monitoring, otherwise we don't need to monitor.
    //
    if(_inUse == _workers.size() && (_workers.size() < _sizeMax || _sizeMax != 1))
    {
        _subscriberPoolMonitor->startMonitor();
    }
    else
    {
        _subscriberPoolMonitor->stopMonitor();
    }
    //
    // We only need to compute the push interval if we've created
    // stall threads.
    //
    computeInterval = (_workers.size() - _size) > 0;
}

void
SubscriberPool::destroy()
{
    //
    // First mark the pool as destroyed. This causes all of the worker
    // threads to unblock and terminate. We also clear the set of
    // subscribers here since there is a cycle (instance -> pool ->
    // subscribers -> instance).  No new subscribers can be added once
    // _destroyed is set.
    //
    {
        Lock sync(*this);
        _destroyed = true;
        notifyAll();
        if(_subscriberPoolMonitor)
        {
            _subscriberPoolMonitor->destroy();
        }
        _subscribers.clear();
        _pending.clear();
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
    if(_destroyed)
    {
        return;
    }

    IceUtil::Time now = IceUtil::Time::now();
    IceUtil::Time interval = now - _lastDequeue;
/*
    if(_traceLevels->subscriberPool > 1)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->subscriberPoolCat);
        out << "check called: interval: "  << interval << " timeout: " << _timeout
            << " pending: " << _pending.size() << " running: " << _workers.size()
            << " sizeMax: " << _sizeMax;
    }
*/

    if(interval > _timeout && _pending.size() > 0 && (_workers.size() < _sizeMax || _sizeMax == 0))
    {
        if(_traceLevels->subscriberPool > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->subscriberPoolCat);
            out << "detected stall: creating thread: threads: " << _workers.size();
        }
        
        //
        // We'll now start stall checking at regular intervals if this
        // is the first newly created worker. Here we need to
        // initially set the stall check and the number of requests at
        // this point.
        //
        if(_workers.size() == _size)
        {
            _lastStallCheck = now;
        }

        ++_inUse;
        _workers.push_back(new SubscriberPoolWorker(this));
    }
}

bool
SubscriberPool::invariants()
{
    set<SubscriberPtr> subs;
    list<SubscriberPtr>::const_iterator p;
    for(p = _subscribers.begin(); p != _subscribers.end(); ++p)
    {
        assert(subs.find(*p) == subs.end());
        subs.insert(*p);
    }
    subs.clear();
    for(p = _pending.begin(); p != _pending.end(); ++p)
    {
        assert(subs.find(*p) == subs.end());
        subs.insert(*p);
    }
    return true;
}
