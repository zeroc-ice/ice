// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include <IceUtil/Mutex.h>
#include <Ice/Current.h> // For Ice::Context
#include <Ice/Identity.h>

#include <vector>

namespace IceStorm
{

//
// Forward declarations.
//
class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

//
// Note that at present this requires to copy the event which isn't
// nice. If this indeed becomes a bottleneck then either the event can
// carry a reference to the blob, context & op (while event itself
// isn't copied), or the op, blob & context can be passed along as
// arguments (or do copy on write, or some such trick).
//
struct Event
{
    bool forwarded;
    int cost;
    std::string op;
    bool nonmutating;
    std::vector< Ice::Byte> data;
    Ice::Context context;
};

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
    virtual void publish(const Event&) = 0;

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
