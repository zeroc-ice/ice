// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_CONNECTION_FACTORY_H
#define ICE_CONNECTION_FACTORY_H

#include <IceUtil/RecMutex.h>
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

namespace Ice
{

class LocalException;
class ObjectAdapterI;

}

namespace IceInternal
{

class OutgoingConnectionFactory : public ::IceUtil::Shared, public ::IceUtil::Mutex
{
public:

    ConnectionPtr create(const std::vector<EndpointPtr>&);
    void setRouter(const ::Ice::RouterPrx&);
    void removeAdapter(const ::Ice::ObjectAdapterPtr&);

private:

    OutgoingConnectionFactory(const InstancePtr&);
    virtual ~OutgoingConnectionFactory();
    void destroy();
    friend class Instance;

    InstancePtr _instance;
    std::map<EndpointPtr, ConnectionPtr> _connections;
};

class IncomingConnectionFactory : public EventHandler, public ::IceUtil::Monitor< ::IceUtil::Mutex>
{
public:

    void hold();
    void activate();

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
    
private:

    IncomingConnectionFactory(const InstancePtr&, const EndpointPtr&, const ::Ice::ObjectAdapterPtr&);
    virtual ~IncomingConnectionFactory();
    void destroy();
    void waitUntilFinished();
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

    EndpointPtr _endpoint;
    ::Ice::ObjectAdapterPtr _adapter;
    AcceptorPtr _acceptor;
    TransceiverPtr _transceiver;
    ThreadPoolPtr _serverThreadPool;
    std::list<ConnectionPtr> _connections;
    State _state;
    bool _warn;
    bool _registeredWithPool;
};

}

#endif
