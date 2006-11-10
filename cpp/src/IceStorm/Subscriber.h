// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SUBSCRIBERS_H
#define SUBSCRIBERS_H

#include <IceStorm/IceStormInternal.h> // F
#include <IceStorm/Event.h>

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

    void reachable();
    Ice::ObjectPrx proxy() const;
    Ice::Identity id() const;
    bool persistent() const;

    enum QueueState
    {
	QueueStateError,
	QueueStateFlush,
	QueueStateNoFlush
    };
    virtual QueueState queue(bool, const std::vector<EventPtr>&);
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

    void setError(const Ice::Exception&);
    void setUnreachable(const Ice::Exception&);

protected:

    Subscriber(const InstancePtr&, const Ice::ObjectPrx&, bool, const Ice::Identity&);

    // Immutable
    const InstancePtr _instance;
    const Ice::Identity _id;
    const bool _persistent;
    const Ice::ObjectPrx _proxy;

    IceUtil::Mutex _mutex;

    enum State
    {
	//
	// The Subscriber is active.
	//
	StateActive,
	//
	// The Subscriber encountered an error during event
	// transmission.
	//
	StateError,
	//
	// The Subscriber is no longer reachable.
	//
	StateUnreachable
    };
    State _state;

    bool _busy;
    EventSeq _events;

    //
    // Not protected by _mutex. These members are protected by the
    // SubscriberPool mutex.
    //
    bool _resetMax;
    IceUtil::Time _maxSend;
};

bool operator==(const IceStorm::SubscriberPtr&, const Ice::Identity&);

}

#endif // SUBSCRIBERS_H
