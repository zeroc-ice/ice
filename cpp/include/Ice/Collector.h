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

}

namespace __Ice
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

    Collector(const Collector&);
    void operator=(const Collector&);

    Collector(const ::Ice::ObjectAdapter_ptr&, const Transceiver_ptr&,
	       const Endpoint_ptr&);
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

    ::Ice::ObjectAdapter_ptr adapter_;
    Transceiver_ptr transceiver_;
    Endpoint_ptr endpoint_;
    ThreadPool_ptr threadPool_;
    int responseCount_;
    State state_;
#ifndef ICE_NO_TRACE
    TraceLevels_ptr traceLevels_;
    ::Ice::Logger_ptr logger_;
#endif
};

class CollectorFactory : public EventHandler, public JTCMutex
{
public:

    void destroy();
    void hold();
    void activate();

    Endpoint_ptr endpoint() const;
    bool equivalent(const Endpoint_ptr&) const;

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

    CollectorFactory(const CollectorFactory&);
    void operator=(const CollectorFactory&);

    CollectorFactory(const ::Ice::ObjectAdapter_ptr&, const Endpoint_ptr&);
    virtual ~CollectorFactory();
    friend class ::Ice::ObjectAdapter;

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

    ::Ice::ObjectAdapter_ptr adapter_;
    Acceptor_ptr acceptor_;
    Transceiver_ptr transceiver_;
    Endpoint_ptr endpoint_;
    ThreadPool_ptr threadPool_;
    std::list<Collector_ptr> collectors_;
    State state_;
#ifndef ICE_NO_TRACE
    TraceLevels_ptr traceLevels_;
    ::Ice::Logger_ptr logger_;
#endif
};

}

#endif
