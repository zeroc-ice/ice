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

class Emitter : public EventHandler, public JTCMutex
{
public:

    void destroy();
    void prepareRequest(Outgoing*);
    void sendRequest(Outgoing*, bool);
    int timeout() const;

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

    Emitter(const Emitter&);
    void operator=(const Emitter&);

    Emitter(const Instance_ptr&, const Transceiver_ptr&, const Endpoint_ptr&);
    virtual ~Emitter();
    friend class EmitterFactory;

    enum State
    {
	StateActive,
	StateHolding,
	StateClosing,
	StateClosed
    };

    void setState(State, const ::Ice::LocalException&);

    Transceiver_ptr transceiver_;
    Endpoint_ptr endpoint_;
    ThreadPool_ptr threadPool_;
    ::Ice::Int nextRequestId_;
    std::map< ::Ice::Int, Outgoing*> requests_;
    std::auto_ptr< ::Ice::LocalException> exception_;
    State state_;
#ifndef ICE_NO_TRACE
    TraceLevels_ptr traceLevels_;
    ::Ice::Logger_ptr logger_;
#endif
};

class EmitterFactory : public Shared, public JTCMutex
{
public:

    Emitter_ptr create(const Endpoint_ptr&);

private:

    EmitterFactory(const EmitterFactory&);
    void operator=(const EmitterFactory&);

    EmitterFactory(const Instance_ptr&);
    virtual ~EmitterFactory();
    void destroy();
    friend class Instance;

    Instance_ptr instance_;
    std::map<Endpoint_ptr, Emitter_ptr> emitters_;
};

}

#endif
