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
#include <Ice/ConnectionF.h>
#include <Ice/InstanceF.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/EndpointF.h>
#include <Ice/AcceptorF.h>
#include <Ice/TransceiverF.h>
#include <Ice/EventHandler.h>
#include <list>

namespace Ice
{

class LocalException;

}

namespace IceInternal
{

class CollectorFactory : public EventHandler, public JTCMutex
{
public:

    CollectorFactory(const InstancePtr&, const EndpointPtr&, const ::Ice::ObjectAdapterPtr&);
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
    void warning(const ::Ice::LocalException&) const;

    EndpointPtr _endpoint;
    ::Ice::ObjectAdapterPtr _adapter;
    ThreadPoolPtr _threadPool;
    AcceptorPtr _acceptor;
    TransceiverPtr _transceiver;
    std::list<ConnectionPtr> _connections;
    State _state;
    bool _warn;
};

}

#endif
