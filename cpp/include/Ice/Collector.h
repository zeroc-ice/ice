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
#include <Ice/EventHandler.h>
#include <list>
#include <map>

namespace Ice
{

class LocalException;

}

namespace __Ice
{

class Incoming;

class ICE_API CollectorI : public EventHandlerI, public JTCRecursiveMutex
{
public:

    Instance instance() const;
    void destroy();
    bool destroyed() const;
    void hold();
    void activate();
    void prepareReply(Incoming*);
    void sendReply(Incoming*);

    //
    // Operations from EventHandlerI
    //
    virtual bool server();
    virtual void receive();
    virtual void finished();

private:

    CollectorI(const CollectorI&);
    void operator=(const CollectorI&);

    CollectorI(const ::Ice::ObjectAdapter&, const Transceiver&);
    virtual ~CollectorI();
    friend class CollectorFactoryI; // May create CollectorIs

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

    Instance instance_;
    ::Ice::ObjectAdapter adapter_;
    ThreadPool threadPool_;
    Transceiver transceiver_;
    int responseCount_;
    State state_;
};

class ICE_API CollectorFactoryI : public EventHandlerI, public JTCMutex
{
public:

    void destroy();
    void hold();
    void activate();

    //
    // Operations from EventHandlerI
    //
    virtual bool server();
    virtual void receive();
    virtual void finished();
    
private:

    CollectorFactoryI(const CollectorFactoryI&);
    void operator=(const CollectorFactoryI&);

    CollectorFactoryI(const ::Ice::ObjectAdapter&, const Endpoint&);
    virtual ~CollectorFactoryI();
    friend class CollectorFactoryFactoryI; // May create CollectorFactoryIs

    enum State
    {
	StateActive,
	StateHolding,
	StateClosing,
	StateClosed
    };

    void setState(State);
    void warning(const ::Ice::LocalException&) const;

    Instance instance_;
    ::Ice::ObjectAdapter adapter_;
    ThreadPool threadPool_;
    Acceptor acceptor_;
    std::list<Collector> collectors_;
    State state_;
};

class ICE_API CollectorFactoryFactoryI : public Shared, public JTCMutex
{
public:

    CollectorFactory create(const ::Ice::ObjectAdapter&,
			    const Endpoint&);

private:

    CollectorFactoryFactoryI(const CollectorFactoryFactoryI&);
    void operator=(const CollectorFactoryFactoryI&);

    CollectorFactoryFactoryI(const Instance&);
    virtual ~CollectorFactoryFactoryI();
    void destroy();
    friend class InstanceI; // May create and destroy CollectorFactoryFactoryIs

    Instance instance_;
};

}

#endif
