// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SUBSCRIBER_POOL_H
#define SUBSCRIBER_POOL_H

#include <IceStorm/Subscriber.h>

#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/Time.h>
#include <IceUtil/Thread.h>
#include <Ice/Identity.h>
#include <list>
#include <set>

namespace IceStorm
{

//
// Forward declarations.
//
class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class Instance;
typedef IceUtil::Handle<Instance> InstancePtr;

class SubscriberPool;
typedef IceUtil::Handle<SubscriberPool> SubscriberPoolPtr;

class SubscriberPoolMonitor : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    SubscriberPoolMonitor(const SubscriberPoolPtr&, const IceUtil::Time&);
    ~SubscriberPoolMonitor();

    virtual void run();

    void startMonitor();
    void stopMonitor();
    void destroy();

private:

    const SubscriberPoolPtr _manager;
    const IceUtil::Time _timeout;
    bool _needCheck;
    bool _destroyed;
};

typedef IceUtil::Handle<SubscriberPoolMonitor> SubscriberPoolMonitorPtr;

class SubscriberPool : public IceUtil::Shared, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    SubscriberPool(const InstancePtr&);
    ~SubscriberPool();

    void flush(std::list<SubscriberPtr>&);
    void flush(const SubscriberPtr&);
    void add(const SubscriberPtr&);
    void remove(const SubscriberPtr&);
    void destroy();

    //
    // For use by the subscriber worker.
    //
    void dequeue(SubscriberPtr&, bool, const IceUtil::Time&, bool&);
    //
    // For use by the monitor.
    //
    void check();

private:

    bool invariants();
    
    const TraceLevelsPtr _traceLevels;
    const unsigned int _sizeMax;
    const unsigned int _sizeWarn;
    const unsigned int _size;
    const IceUtil::Time _timeout;
    const IceUtil::Time _stallCheck;
    SubscriberPoolMonitorPtr _subscriberPoolMonitor;

    std::list<SubscriberPtr> _pending;
    std::list<SubscriberPtr> _subscribers;
    bool _destroyed;
    std::list<IceUtil::ThreadPtr> _workers;

    int _reap;
    unsigned int _inUse;

    IceUtil::Time _lastStallCheck;
    IceUtil::Time _lastDequeue;
};

} // End namespace IceStorm

#endif
