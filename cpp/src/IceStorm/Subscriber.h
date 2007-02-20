// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SUBSCRIBERS_H
#define SUBSCRIBERS_H

#include <IceStorm/IceStormInternal.h> // F

namespace IceStorm
{

class Instance;
typedef IceUtil::Handle<Instance> InstancePtr;

class Subscriber;
typedef IceUtil::Handle<Subscriber> SubscriberPtr;

class Subscriber : public IceUtil::Shared
{
public:

    static SubscriberPtr create(const InstancePtr&, const Ice::ObjectPrx&, const IceStorm::QoS&);
    static SubscriberPtr create(const InstancePtr&, const TopicLinkPrx&, int);

    ~Subscriber();

    Ice::ObjectPrx proxy() const;
    Ice::Identity id() const;
    bool persistent() const;

    enum QueueState
    {
        QueueStateError,
        QueueStateFlush,
        QueueStateNoFlush
    };
    virtual QueueState queue(bool, const std::vector<EventDataPtr>&);
    //
    // Return true if flush() must be called again, false otherwise.
    //
    virtual bool flush() = 0;
    virtual void destroy();

    //
    // These methods must only be called by the SubscriberPool they
    // are not internally mutex protected.
    //
    void flushTime(const IceUtil::Time&);
    IceUtil::Time pollMaxFlushTime(const IceUtil::Time&);

    void error(const Ice::Exception&);

protected:

    Subscriber(const InstancePtr&, const Ice::ObjectPrx&, bool, const Ice::Identity&);

    // Immutable
    const InstancePtr _instance;
    const Ice::Identity _id;
    const bool _persistent;
    const Ice::ObjectPrx _proxy;

    IceUtil::Mutex _mutex;

    enum SubscriberState
    {
        SubscriberStateOnline,
        SubscriberStateFlushPending,
        SubscriberStateSending,
        SubscriberStateOffline,
        SubscriberStateError
    };
    SubscriberState _state; // The subscriber state.
    EventDataSeq _events; // The queue of events to send.

    //
    // Not protected by _mutex. These members are protected by the
    // SubscriberPool mutex.
    //
    bool _resetMax;
    IceUtil::Time _maxSend;
};

bool operator==(const IceStorm::SubscriberPtr&, const Ice::Identity&);
bool operator==(const IceStorm::Subscriber&, const IceStorm::Subscriber&);
bool operator!=(const IceStorm::Subscriber&, const IceStorm::Subscriber&);
bool operator<(const IceStorm::Subscriber&, const IceStorm::Subscriber&);

}

#endif // SUBSCRIBERS_H
