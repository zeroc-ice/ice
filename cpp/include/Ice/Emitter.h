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

    Instance instance() const;
    void destroy();
    void prepareRequest(Outgoing*);
    void sendRequest(Outgoing*);

    //
    // Operations from EventHandlerI
    //
    virtual bool server();
    virtual void receive();
    virtual void finished();

private:

    EmitterI(const EmitterI&);
    void operator=(const EmitterI&);

    EmitterI(const Instance&, const Transceiver&);
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

    Instance instance_;
    ThreadPool threadPool_;
    Transceiver transceiver_;
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

    void destroy();
    Emitter create();

private:

    EmitterFactoryI(const EmitterFactoryI&);
    void operator=(const EmitterFactoryI&);

    EmitterFactoryI(const Instance&, const Endpoint&);
    virtual ~EmitterFactoryI();
    friend class EmitterFactoryFactoryI; // May create EmitterFactoryIs

    Instance instance_;
    Connector connector_;
    Emitter emitter_;
};

class ICE_API EmitterFactoryFactoryI : public Shared, public JTCMutex
{
public:

    EmitterFactory create(const Endpoint&);

private:

    EmitterFactoryFactoryI(const EmitterFactoryFactoryI&);
    void operator=(const EmitterFactoryFactoryI&);

    EmitterFactoryFactoryI(const Instance&);
    virtual ~EmitterFactoryFactoryI();
    void destroy();
    friend class InstanceI; // May create and destroy EmitterFactoryFactoryIs

    Instance instance_;
    std::map<Endpoint, EmitterFactory> factories_;
};

}

#endif
