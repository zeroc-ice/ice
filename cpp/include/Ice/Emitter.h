// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_EMITTER_H
#define ICE_EMITTER_H

#include <Ice/EmitterF.h>
#include <Ice/InstanceF.h>
#include <Ice/TransceiverF.h>
#include <Ice/ConnectorF.h>
#include <Ice/ThreadPoolF.h>
#include <Ice/EndpointF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/EventHandler.h>
#include <map>

namespace Ice
{

class LocalException;

}

namespace __Ice
{

class Outgoing;

class ICE_API EmitterI : public EventHandlerI, public JTCMutex
{
public:

    void destroy();
    const Endpoint& endpoint(); // const& for performance reasons
    void prepareRequest(Outgoing*);
    void sendRequest(Outgoing*, bool);

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

    EmitterI(const EmitterI&);
    void operator=(const EmitterI&);

    EmitterI(const Instance&, const Endpoint&, const Transceiver&);
    virtual ~EmitterI();
    friend class EmitterFactoryI; // May create EmitterIs

    enum State
    {
	StateActive,
	StateHolding,
	StateClosing,
	StateClosed
    };

    void setState(State, const ::Ice::LocalException&);

    Endpoint endpoint_;
    Transceiver transceiver_;
    ThreadPool threadPool_;
    ::Ice::Int nextRequestId_;
    std::map< ::Ice::Int, Outgoing*> requests_;
    std::auto_ptr< ::Ice::LocalException> exception_;
    State state_;
#ifndef ICE_NO_TRACE
    TraceLevels traceLevels_;
    ::Ice::Logger logger_;
#endif
};

class ICE_API EmitterFactoryI : public Shared, public JTCMutex
{
public:

    Emitter create(const Endpoint&);

private:

    EmitterFactoryI(const EmitterFactoryI&);
    void operator=(const EmitterFactoryI&);

    EmitterFactoryI(const Instance&);
    virtual ~EmitterFactoryI();
    void destroy();
    friend class InstanceI; // May create and destroy EmitterFactoryIs

    Instance instance_;
    std::map<Endpoint, Emitter> emitters_;
};

}

#endif
