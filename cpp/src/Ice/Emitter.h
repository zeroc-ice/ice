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

namespace Ice
{

class LocalException;

}

namespace IceInternal
{

class Outgoing;

class Emitter : public EventHandler, public JTCMutex
{
public:

    void destroy();
    bool destroyed() const;
    void prepareRequest(Outgoing*);
    void sendRequest(Outgoing*, bool);
    void prepareBatchRequest(Outgoing*);
    void finishBatchRequest(Outgoing*);
    void abortBatchRequest();
    void flushBatchRequest();
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

    Emitter(const InstancePtr&, const TransceiverPtr&, const EndpointPtr&);
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

    TransceiverPtr _transceiver;
    EndpointPtr _endpoint;
    ThreadPoolPtr _threadPool;
    ::Ice::Int _nextRequestId;
    std::map< ::Ice::Int, Outgoing*> _requests;
    std::auto_ptr< ::Ice::LocalException> _exception;
    Stream _batchStream;
    State _state;
#ifndef ICE_NO_TRACE
    TraceLevelsPtr _traceLevels;
    ::Ice::LoggerPtr _logger;
#endif
};

class EmitterFactory : public ::IceUtil::Shared, public JTCMutex
{
public:

    EmitterPtr create(const std::vector<EndpointPtr>&);

private:

    EmitterFactory(const InstancePtr&);
    virtual ~EmitterFactory();
    void destroy();
    friend class Instance;

    InstancePtr _instance;
    std::map<EndpointPtr, EmitterPtr> _emitters;
};

}

#endif
