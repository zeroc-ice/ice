
// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_INCOMING_CONNECTION_FACTORY_H
#define ICEE_INCOMING_CONNECTION_FACTORY_H

#include <IceE/IncomingConnectionFactoryF.h>
#include <IceE/EndpointF.h>
#include <IceE/ConnectionF.h>
#include <IceE/ObjectAdapterF.h>
#include <IceE/InstanceF.h>
#include <IceE/AcceptorF.h>
#include <IceE/TransceiverF.h>

#include <IceE/Mutex.h>
#include <IceE/Monitor.h>
#include <IceE/Shared.h>
#include <IceE/Thread.h>
#include <list>

namespace IceInternal
{

class IncomingConnectionFactory : public Ice::Monitor<Ice::Mutex>, public Ice::Shared
{
public:

    void activate();
    void hold();
    void destroy();

    void waitUntilHolding() const;
    void waitUntilFinished();

    EndpointPtr endpoint() const;
    bool equivalent(const EndpointPtr&) const;
    std::list<Ice::ConnectionPtr> connections() const;
#ifndef ICEE_NO_BATCH
    void flushBatchRequests();
#endif

    virtual std::string toString() const;

private:

    IncomingConnectionFactory(const InstancePtr&, const EndpointPtr&, const ::Ice::ObjectAdapterPtr&);
    virtual ~IncomingConnectionFactory();
    friend class ::Ice::ObjectAdapter;

    enum State
    {
	StateActive,
	StateHolding,
	StateClosed
    };

    void setState(State);

    void run();

    class ThreadPerIncomingConnectionFactory : public Ice::Thread
    {
    public:
	
	ThreadPerIncomingConnectionFactory(const IncomingConnectionFactoryPtr&);
	virtual void run();

    private:
	
	IncomingConnectionFactoryPtr _factory;
    };
    friend class ThreadPerIncomingConnectionFactory;
    Ice::ThreadPtr _threadPerIncomingConnectionFactory;

    const InstancePtr _instance;
    AcceptorPtr _acceptor;
    const TransceiverPtr _transceiver;
    const EndpointPtr _endpoint;

    const ::Ice::ObjectAdapterPtr _adapter;

    const bool _warn;

    std::list<Ice::ConnectionPtr> _connections;

    State _state;
};

}

#endif
