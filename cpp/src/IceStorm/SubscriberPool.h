// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SUBSCRIBER_POOL_H
#define SUBSCRIBER_POOL_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/Time.h>
#include <IceUtil/Thread.h>
#include <list>

namespace IceStorm
{

//
// Forward declarations.
//
class Instance;
typedef IceUtil::Handle<Instance> InstancePtr;

class Subscriber;
typedef IceUtil::Handle<Subscriber> SubscriberPtr;

class SubscriberPoolMonitor;
typedef IceUtil::Handle<SubscriberPoolMonitor> SubscriberPoolMonitorPtr;

class SubscriberPool : public IceUtil::Shared, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    SubscriberPool(const InstancePtr&);
    ~SubscriberPool();

    void add(std::list<SubscriberPtr>&);
    void destroy();

private:
    
    friend class SubscriberPoolWorker;
    SubscriberPtr dequeue(const SubscriberPtr&);
    friend class SubscriberPoolMonitor;
    void check();

    const InstancePtr _instance;
    const int _sizeMax;
    const int _sizeWarn;
    const int _size;
    const IceUtil::Time _timeout;
    SubscriberPoolMonitorPtr _subscriberPoolMonitor;

    std::list<SubscriberPtr> _pending;
    bool _destroy;
    std::list<IceUtil::ThreadPtr> _workers;

    int _inUse;
    int _running;
    double _load;

    IceUtil::Time _lastNext;
};

typedef IceUtil::Handle<SubscriberPool> SubscriberPoolPtr;

} // End namespace IceStorm

#endif
