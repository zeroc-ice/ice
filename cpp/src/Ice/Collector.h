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
#include <Ice/Connection.h>
#include <list>

namespace Ice
{

class LocalException;

}

namespace IceInternal
{

class Incoming;

class Collector : virtual public EventHandler, virtual public ::Ice::IncomingConnection, public JTCRecursiveMutex
{
public:

    Collector(const InstancePtr&, const ::Ice::ObjectAdapterPtr&, const TransceiverPtr&, const EndpointPtr&);
    virtual ~Collector();

    void destroy();
    bool destroyed() const;
    void hold();
    void activate();

    //
    // Operations from EventHandler
    //
    virtual bool server() const;
    virtual bool readable() const;
    virtual void read(BasicStream&);
    virtual void message(BasicStream&);
    virtual void exception(const ::Ice::LocalException&);
    virtual void finished();
    virtual bool tryDestroy();

    //
    // Operations from IncomingConnection
    //
    virtual ::Ice::InternetAddress getLocalAddress();
    virtual ::Ice::InternetAddress getRemoteAddress();
    virtual ::Ice::ProtocolInfoPtr getProtocolInfo();

private:

    enum State
    {
	StateActive,
	StateHolding,
	StateClosing,
	StateClosed
    };

    void setState(State);
    void closeConnection();
    void warning(const ::Ice::Exception&) const;

    ::Ice::ObjectAdapterPtr _adapter;
    TransceiverPtr _transceiver;
    EndpointPtr _endpoint;
    TraceLevelsPtr _traceLevels;
    ::Ice::LoggerPtr _logger;
    ThreadPoolPtr _threadPool;
    int _responseCount;
    State _state;
    bool _warnAboutExceptions;
};

class CollectorFactory : public EventHandler, public JTCMutex
{
public:

    CollectorFactory(const InstancePtr&, const ::Ice::ObjectAdapterPtr&, const EndpointPtr&);
    virtual ~CollectorFactory();

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
    virtual void read(BasicStream&);
    virtual void message(BasicStream&);
    virtual void exception(const ::Ice::LocalException&);
    virtual void finished();
    virtual bool tryDestroy();
    
private:

    enum State
    {
	StateActive,
	StateHolding,
	StateClosed
    };

    void setState(State);
    void clearBacklog();
    void warning(const ::Ice::Exception&) const;

    ::Ice::ObjectAdapterPtr _adapter;
    AcceptorPtr _acceptor;
    TransceiverPtr _transceiver;
    EndpointPtr _endpoint;
    TraceLevelsPtr _traceLevels;
    ::Ice::LoggerPtr _logger;
    ThreadPoolPtr _threadPool;
    std::list<CollectorPtr> _collectors;
    State _state;
    bool _warnAboutExceptions;
};

}

#endif
