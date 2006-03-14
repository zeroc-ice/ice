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

#include <IceUtil/Mutex.h>
#include <Ice/Identity.h>
#include <IceStorm/Event.h>

#include <vector>

namespace IceStorm
{

//
// Forward declarations.
//
class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

//
// Subscriber interface.
//
class Subscriber : public virtual IceUtil::Shared
{
public:

    Subscriber(const TraceLevelsPtr& traceLevels, const Ice::Identity&);
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
    virtual void activate() = 0;

    //
    // Unsubscribe. Mark the state as Unsubscribed.
    //
    virtual void unsubscribe() = 0;

    //
    // Unsubscribe. Mark the state as Replaced.
    //
    virtual void replace() = 0;

    //
    // Publish the given event. Mark the state as Error in the event of
    // a problem.
    //
    virtual void publish(const EventPtr&) = 0;

protected:

    // Immutable
    TraceLevelsPtr _traceLevels; 

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

private:

    //
    // This id is the full id of the subscriber for a particular topic.
    //
    // Immutable.
    //
    Ice::Identity _id;
};

typedef IceUtil::Handle<Subscriber> SubscriberPtr;

} // End namespace IceStorm

#endif
