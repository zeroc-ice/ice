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

class ICE_API CollectorI : public EventHandlerI, public JTCRecursiveMutex
{
public:

    void destroy();
    bool destroyed() const;
    void hold();
    void activate();
    void prepareReply(Incoming*);
    void sendReply(Incoming*);

    //
    // Operations from EventHandlerI
    //
    virtual bool server() const;
    virtual bool readable() const;
    virtual void read(Stream&);
    virtual void message(Stream&);
    virtual void exception(const ::Ice::LocalException&);
    virtual void finished();

private:

    CollectorI(const CollectorI&);
    void operator=(const CollectorI&);

    CollectorI(const ::Ice::ObjectAdapter&, const Endpoint&,
	       const Transceiver&);
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

    ::Ice::ObjectAdapter adapter_;
    Endpoint endpoint_;
    Transceiver transceiver_;
    ThreadPool threadPool_;
    int responseCount_;
    State state_;
#ifndef ICE_NO_TRACE
    TraceLevels traceLevels_;
    ::Ice::Logger logger_;
#endif
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
    virtual bool server() const;
    virtual bool readable() const;
    virtual void read(Stream&);
    virtual void message(Stream&);
    virtual void exception(const ::Ice::LocalException&);
    virtual void finished();
    
private:

    CollectorFactoryI(const CollectorFactoryI&);
    void operator=(const CollectorFactoryI&);

    CollectorFactoryI(const ::Ice::ObjectAdapter&, const Endpoint&);
    virtual ~CollectorFactoryI();
    friend class ::Ice::ObjectAdapterI; // May create CollectorFactoryIs

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

    ::Ice::ObjectAdapter adapter_;
    Endpoint endpoint_;
    Acceptor acceptor_;
    ThreadPool threadPool_;
    std::list<Collector> collectors_;
    State state_;
#ifndef ICE_NO_TRACE
    TraceLevels traceLevels_;
    ::Ice::Logger logger_;
#endif
};

}

#endif
