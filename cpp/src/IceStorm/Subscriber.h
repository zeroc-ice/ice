// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
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
    // Subscriber state.
    //
    enum State
    {
	//
	// The Subscriber is active
	//
	StateActive,
	//
	// The Subscriber encountered an error during event
	// transmission
	//
	StateError,
	//
	// The Subscriber has been unsubscribed
	//
	StateUnsubscribed
    };

    //
    // This method is for ease of use with STL algorithms.
    // Return _state != StateActive
    //
    bool inactive() const;

    //
    // Retrieve the state of the Subscriber.
    //
    State state() const;

    //
    // Retrieve the identity of the Subscriber.
    //
    Ice::Identity id() const;

    //
    // Unsubscribe. Mark the state as Unsubscribed.
    //
    virtual void unsubscribe() = 0;

    //
    // Publish the given event. Mark the state as Error in the event
    // of a problem.
    //
    virtual void publish(const Event&) = 0;

protected:

    // Immutable
    TraceLevelsPtr _traceLevels; 

    IceUtil::Mutex _stateMutex;
    State _state;

private:

    //
    // This id is the full id of the subscriber for a particular topic
    // (that is <prefix>#<topicname>
    //
    // Immutable
    Ice::Identity _id;
};

typedef IceUtil::Handle<Subscriber> SubscriberPtr;

} // End namespace IceStorm

#endif
