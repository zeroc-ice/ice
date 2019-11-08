//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include <IceStorm/IceStormInternal.h>
#include <IceStorm/SubscriberRecord.h>
#include <IceStorm/Instrumentation.h>
#include <Ice/ObserverHelper.h>
#include <IceUtil/RecMutex.h>

namespace IceStorm
{

#ifdef ICE_CPP11_MAPPING
class SendQueueSizeMaxReached : public ::Ice::LocalExceptionHelper<SendQueueSizeMaxReached, ::Ice::LocalException>
{
public:

    virtual ~SendQueueSizeMaxReached();

    SendQueueSizeMaxReached(const SendQueueSizeMaxReached&) = default;

    /**
        * The file and line number are required for all local exceptions.
        * @param file The file name in which the exception was raised, typically __FILE__.
        * @param line The line number at which the exception was raised, typically __LINE__.
        */
    SendQueueSizeMaxReached(const char* file, int line) : ::Ice::LocalExceptionHelper<SendQueueSizeMaxReached, ::Ice::LocalException>(file, line)
    {
    }

    /**
        * Obtains a tuple containing all of the exception's data members.
        * @return The data members in a tuple.
        */
    std::tuple<> ice_tuple() const
    {
        return std::tie();
    }

    /**
        * Obtains the Slice type ID of this exception.
        * @return The fully-scoped type ID.
        */
    static const ::std::string& ice_staticId();
};
#else
class SendQueueSizeMaxReached : public ::Ice::LocalException
{
public:

    /**
        * The file and line number are required for all local exceptions.
        * @param file The file name in which the exception was raised, typically __FILE__.
        * @param line The line number at which the exception was raised, typically __LINE__.
        */
    SendQueueSizeMaxReached(const char* file, int line);
    virtual ~SendQueueSizeMaxReached() throw();

    /**
        * Obtains the Slice type ID of this exception.
        * @return The fully-scoped type ID.
        */
    virtual ::std::string ice_id() const;
    /**
        * Polymporphically clones this exception.
        * @return A shallow copy of this exception.
        */
    virtual SendQueueSizeMaxReached* ice_clone() const;
    /**
        * Throws this exception.
        */
    virtual void ice_throw() const;
};
#endif // ICE_CPP11_MAPPING

class Instance;
typedef IceUtil::Handle<Instance> InstancePtr;

class Subscriber;
typedef IceUtil::Handle<Subscriber> SubscriberPtr;

class Subscriber : public IceUtil::Shared
{
public:

    static SubscriberPtr create(const InstancePtr&, const IceStorm::SubscriberRecord&);

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
