// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_COLLECTOR_H
#define ICE_COLLECTOR_H

#include <Ice/CollectorF.h>
#include <Ice/InstanceF.h>
#include <Ice/TransceiverF.h>
#include <Ice/AcceptorF.h>
#include <Ice/ThreadPoolF.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/EndpointF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/EventHandler.h>
#include <list>

namespace Ice
{

class LocalException;
class ObjectAdapterI;

}

namespace IceInternal
{

class Incoming;

class Collector : public EventHandler, public JTCRecursiveMutex
{
public:

    void destroy();
    bool destroyed() const;
    void hold();
    void activate();
    void prepareReply(Incoming*);
    void sendReply(Incoming*);

    //
    // Operations from EventHandler
    //
    virtual bool server() const;
    virtual bool readable() const;
    virtual void read(Stream&);
    virtual void message(Stream&);
    virtual void exception(const ::Ice::LocalException&);
    virtual void finished();

private:

    Collector(const InstancePtr&, const ::Ice::ObjectAdapterPtr&,
	      const TransceiverPtr&, const EndpointPtr&);
    virtual ~Collector();
    friend class CollectorFactory;

    enum State
    {
	StateActive,
	StateHolding,
	StateClosing,
	StateClosed
    };

    void setState(State);
    void closeConnection();
    void warning(const ::Ice::LocalException&) const;

    ::Ice::ObjectAdapterPtr _adapter;
    TransceiverPtr _transceiver;
    EndpointPtr _endpoint;
    ThreadPoolPtr _threadPool;
    int _responseCount;
    State _state;
#ifndef ICE_NO_TRACE
    TraceLevelsPtr _traceLevels;
    ::Ice::LoggerPtr _logger;
#endif
};

class CollectorFactory : public EventHandler, public JTCMutex
{
public:

    void destroy();
    void hold();
    void activate();

    EndpointPtr endpoint() const;
    bool equivalent(const EndpointPtr&) const;

    //
    // Operations from EventHandler
    //
    virtual bool server() const;
    virtual bool readable() const;
    virtual void read(Stream&);
    virtual void message(Stream&);
    virtual void exception(const ::Ice::LocalException&);
    virtual void finished();
    
private:

    CollectorFactory(const InstancePtr&, const ::Ice::ObjectAdapterPtr&,
		     const EndpointPtr&);
    virtual ~CollectorFactory();
    friend class ::Ice::ObjectAdapterI;

    enum State
    {
	StateActive,
	StateHolding,
	StateClosing,
	StateClosed
    };

    void setState(State);
    void clearBacklog();
    void warning(const ::Ice::LocalException&) const;

    ::Ice::ObjectAdapterPtr _adapter;
    AcceptorPtr _acceptor;
    TransceiverPtr _transceiver;
    EndpointPtr _endpoint;
    ThreadPoolPtr _threadPool;
    std::list<CollectorPtr> _collectors;
    State _state;
#ifndef ICE_NO_TRACE
    TraceLevelsPtr _traceLevels;
    ::Ice::LoggerPtr _logger;
#endif
};

}

#endif
