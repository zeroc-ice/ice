// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_CONNECTION_FACTORY_H
#define ICE_CONNECTION_FACTORY_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/Thread.h> // For ThreadPerIncomingConnectionFactory.
#include <Ice/ConnectionFactoryF.h>
#include <Ice/ConnectionIF.h>
#include <Ice/InstanceF.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/EndpointIF.h>
#include <Ice/Endpoint.h>
#include <Ice/ConnectorF.h>
#include <Ice/AcceptorF.h>
#include <Ice/TransceiverF.h>
#include <Ice/RouterInfoF.h>
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

class OutgoingConnectionFactory : public IceUtil::Shared, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    void destroy();

    void waitUntilFinished();

    Ice::ConnectionIPtr create(const std::vector<EndpointIPtr>&, bool, bool, Ice::EndpointSelectionType, bool&);
    void setRouterInfo(const RouterInfoPtr&);
    void removeAdapter(const Ice::ObjectAdapterPtr&);
    void flushBatchRequests();

private:

    OutgoingConnectionFactory(const InstancePtr&);
    virtual ~OutgoingConnectionFactory();
    friend class Instance;

    const InstancePtr _instance;
    bool _destroyed;
    std::multimap<ConnectorPtr, Ice::ConnectionIPtr> _connections;
    std::set<ConnectorPtr> _pending; // Connectors for which connection establishment is pending.
};

class IncomingConnectionFactory : public EventHandler, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    void activate();
    void hold();
    void destroy();

    void waitUntilHolding() const;
    void waitUntilFinished();

    EndpointIPtr endpoint() const;
    bool equivalent(const EndpointIPtr&) const;
    std::list<Ice::ConnectionIPtr> connections() const;
    void flushBatchRequests();

    //
    // Operations from EventHandler
    //
    virtual bool datagram() const;
    virtual bool readable() const;
    virtual void read(BasicStream&);
    virtual void message(BasicStream&, const ThreadPoolPtr&);
    virtual void finished(const ThreadPoolPtr&);
    virtual void exception(const Ice::LocalException&);
    virtual std::string toString() const;
    
private:

    IncomingConnectionFactory(const InstancePtr&, const EndpointIPtr&, const Ice::ObjectAdapterPtr&,
                              const std::string&);
    virtual ~IncomingConnectionFactory();
    friend class Ice::ObjectAdapterI;

    enum State
    {
        StateActive,
        StateHolding,
        StateClosed
    };

    void setState(State);
    void registerWithPool();
    void unregisterWithPool();

    void run();

    class ThreadPerIncomingConnectionFactory : public IceUtil::Thread
    {
    public:
        
        ThreadPerIncomingConnectionFactory(const IncomingConnectionFactoryPtr&);
        virtual void run();

    private:
        
        IncomingConnectionFactoryPtr _factory;
    };
    friend class ThreadPerIncomingConnectionFactory;
    IceUtil::ThreadPtr _threadPerIncomingConnectionFactory;

    AcceptorPtr _acceptor;
    const TransceiverPtr _transceiver;
    const EndpointIPtr _endpoint;

    Ice::ObjectAdapterPtr _adapter;

    bool _registeredWithPool;
    int _finishedCount;

    const bool _warn;

    std::list<Ice::ConnectionIPtr> _connections;

    State _state;

    bool _threadPerConnection;
    size_t _threadPerConnectionStackSize;
};

}

#endif
