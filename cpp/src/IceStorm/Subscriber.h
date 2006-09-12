// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include <IceUtil/Handle.h>
#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <Ice/CommunicatorF.h>
#include <Ice/Identity.h>

namespace IceStorm
{

//
// Forward declarations.
//
class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class SubscriberFactory;
typedef IceUtil::Handle<SubscriberFactory> SubscriberFactoryPtr;

class Event;
typedef IceUtil::Handle<Event> EventPtr;

class QueuedProxy;
typedef IceUtil::Handle<QueuedProxy> QueuedProxyPtr;

//
// Subscriber interface.
//
class Subscriber : public virtual IceUtil::Shared
{
public:

    Subscriber(const SubscriberFactoryPtr&, const Ice::CommunicatorPtr&,
	       const TraceLevelsPtr&, const QueuedProxyPtr&);
    ~Subscriber();

    virtual bool persistent() const = 0;

    //
    // Return true if the Subscriber is not active, false otherwise.
    //
    bool inactive() const;

    //
    // Retrieve true if the Subscriber is in the error state, false
    // otherwise.
    //
    bool error() const;

    //
    // Retrieve the identity of the Subscriber.
    //
    Ice::Identity id() const;

    //
    // Activate. Called after any other subscribers with the same
    // identity have been deactivated.
    //
    virtual void activate();

    //
    // Unsubscribe. Mark the state as Unsubscribed.
    //
    virtual void unsubscribe();

    //
    // Unsubscribe. Mark the state as Replaced.
    //
    virtual void replace();

    //
    // Publish the given event. Mark the state as Error in the event of
    // a problem.
    //
    virtual void publish(const EventPtr&) = 0;

protected:

    // Immutable
    const SubscriberFactoryPtr _factory;
    const std::string _desc;
    const TraceLevelsPtr _traceLevels; 
    const QueuedProxyPtr _obj;

    //
    // Subscriber state.
    //
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
	// The Subscriber has been unsubscribed.
	//
	StateUnsubscribed,
	//
	// The Subscriber has been replaced.
	//
	StateReplaced
    };

    IceUtil::Mutex _stateMutex;
    State _state;
};

typedef IceUtil::Handle<Subscriber> SubscriberPtr;

} // End namespace IceStorm

#endif
