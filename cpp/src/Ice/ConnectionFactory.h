// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_CONNECTION_FACTORY_H
#define ICE_CONNECTION_FACTORY_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <Ice/ConnectionFactoryF.h>
#include <Ice/ConnectionF.h>
#include <Ice/InstanceF.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/EndpointF.h>
#include <Ice/AcceptorF.h>
#include <Ice/TransceiverF.h>
#include <Ice/RouterF.h>
#include <Ice/EventHandler.h>
#include <list>
#include <set>

namespace Ice
{

class LocalException;
class ObjectAdapterI;

}

namespace IceInternal
{

class OutgoingConnectionFactory : public ::IceUtil::Shared, public ::IceUtil::Monitor< ::IceUtil::Mutex>
{
public:

    void destroy();

    void waitUntilFinished();

    ConnectionPtr create(const std::vector<EndpointPtr>&);
    void setRouter(const ::Ice::RouterPrx&);
    void removeAdapter(const ::Ice::ObjectAdapterPtr&);

private:

    OutgoingConnectionFactory(const InstancePtr&);
    virtual ~OutgoingConnectionFactory();
    friend class Instance;

    const InstancePtr _instance;
    bool _destroyed;
    std::map<EndpointPtr, ConnectionPtr> _connections;
    std::set<EndpointPtr> _pending;
};

class IncomingConnectionFactory : public EventHandler, public ::IceUtil::Monitor< ::IceUtil::Mutex>
{
public:

    void activate();
    void hold();
    void destroy();

    void waitUntilHolding() const;
    void waitUntilFinished();

    EndpointPtr endpoint() const;
    bool equivalent(const EndpointPtr&) const;
    std::list<ConnectionPtr> connections() const;

    //
    // Operations from EventHandler
    //
    virtual bool readable() const;
    virtual void read(BasicStream&);
    virtual void message(BasicStream&, const ThreadPoolPtr&);
    virtual void finished(const ThreadPoolPtr&);
    virtual void exception(const ::Ice::LocalException&);
    virtual std::string toString() const;
    
private:

    IncomingConnectionFactory(const InstancePtr&, const EndpointPtr&, const ::Ice::ObjectAdapterPtr&);
    virtual ~IncomingConnectionFactory();
    friend class ::Ice::ObjectAdapterI;

    enum State
    {
	StateActive,
	StateHolding,
	StateClosed
    };

    void setState(State);
    void registerWithPool();
    void unregisterWithPool();

    AcceptorPtr _acceptor;
    const TransceiverPtr _transceiver;
    const EndpointPtr _endpoint;

    const ::Ice::ObjectAdapterPtr _adapter;

    bool _registeredWithPool;

    const bool _warn;

    std::list<ConnectionPtr> _connections;

    State _state;
};

}

#endif
