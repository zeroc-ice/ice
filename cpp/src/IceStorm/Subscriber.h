// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include <IceStorm/IceStormInternal.h>
#include <IceStorm/SubscriberRecord.h>
#include <IceStorm/Instrumentation.h>
#include <Ice/ObserverHelper.h>
#include <IceUtil/RecMutex.h>

namespace IceStorm
{

class Instance;
typedef IceUtil::Handle<Instance> InstancePtr;

class Subscriber;
typedef IceUtil::Handle<Subscriber> SubscriberPtr;

class Subscriber : public IceUtil::Shared
{
public:

    static SubscriberPtr create(const InstancePtr&, const IceStorm::SubscriberRecord&);

    ~Subscriber();

    Ice::ObjectPrx proxy() const; // Get the per subscriber object.
    Ice::Identity id() const; // Return the id of the subscriber.
    IceStorm::SubscriberRecord record() const; // Get the subscriber record.

    // Returns false if the subscriber should be reaped.
    bool queue(bool, const EventDataSeq&);
    bool reap();
    void resetIfReaped();
    bool errored() const;

    void destroy();

    // To be called by the AMI callbacks only.
    void completed(const Ice::AsyncResultPtr&);
    void error(bool, const Ice::Exception&);

    void shutdown();

    void updateObserver();

    enum SubscriberState
    {
        SubscriberStateOnline, // Online waiting to send events.
        SubscriberStateOffline, // Offline, retrying.
        SubscriberStateError, // Error state, awaiting reaping.
        SubscriberStateReaped // Reaped.
    };

    virtual void flush() = 0;

protected:

    void setState(SubscriberState);

    Subscriber(const InstancePtr&, const IceStorm::SubscriberRecord&, const Ice::ObjectPrx&, int, int);

    // Immutable
    const InstancePtr _instance;
    const IceStorm::SubscriberRecord _rec; // The subscriber record.
    const int _retryCount; // The retryCount.
    const int _maxOutstanding; // The maximum number of oustanding events.
    const Ice::ObjectPrx _proxy; // The per subscriber object proxy, if any.
    const Ice::ObjectPrx _proxyReplica; // The replicated per subscriber object proxy, if any.

    IceUtil::Monitor<IceUtil::RecMutex> _lock;

    bool _shutdown;

    SubscriberState _state; // The subscriber state.

    int _outstanding; // The current number of outstanding responses.
    int _outstandingCount; // The current number of outstanding events when batching events (only used for metrics).
    EventDataSeq _events; // The queue of events to send.

    // The next time to try sending a new event if we're offline.
    IceUtil::Time _next;
    int _currentRetry;

    IceInternal::ObserverHelperT<IceStorm::Instrumentation::SubscriberObserver> _observer;
};

bool operator==(const IceStorm::SubscriberPtr&, const Ice::Identity&);
bool operator==(const IceStorm::Subscriber&, const IceStorm::Subscriber&);
bool operator!=(const IceStorm::Subscriber&, const IceStorm::Subscriber&);
bool operator<(const IceStorm::Subscriber&, const IceStorm::Subscriber&);

}

#endif // SUBSCRIBER_H
