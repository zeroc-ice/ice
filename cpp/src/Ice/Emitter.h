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
#include <Ice/Connection.h>

namespace Ice
{

class LocalException;

}

namespace IceInternal
{

class Outgoing;

class Emitter : virtual public EventHandler, virtual public ::Ice::OutgoingConnection, public JTCMutex
{
public:

    Emitter(const InstancePtr&, const TransceiverPtr&, const EndpointPtr&);
    virtual ~Emitter();

    void destroy();
    bool destroyed() const;
    void prepareRequest(Outgoing*);
    void sendRequest(Outgoing*, bool);
    void removeRequest(Outgoing*);
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
    virtual void read(BasicStream&);
    virtual void message(BasicStream&);
    virtual void exception(const ::Ice::LocalException&);
    virtual void finished();
    virtual bool tryDestroy();

    //
    // Operations from OutgoingConnection
    //
    virtual ::Ice::InternetAddress getLocalAddress();
    virtual ::Ice::InternetAddress getRemoteAddress();
    virtual ::Ice::ProtocolInfoPtr getProtocolInfo();
    virtual void flush();

private:

    enum State
    {
	StateActive,
	StateClosed
    };

    void setState(State, const ::Ice::LocalException&);

    TransceiverPtr _transceiver;
    EndpointPtr _endpoint;
    ::Ice::LoggerPtr _logger;
    TraceLevelsPtr _traceLevels;
    ThreadPoolPtr _threadPool;
    ::Ice::Int _nextRequestId;
    std::map< ::Ice::Int, Outgoing*> _requests;
    std::map< ::Ice::Int, Outgoing*>::iterator _requestsHint;
    std::auto_ptr< ::Ice::LocalException> _exception;
    BasicStream _batchStream;
    State _state;
};

class EmitterFactory : public ::IceUtil::Shared, public JTCMutex
{
public:

    EmitterFactory(const InstancePtr&);
    virtual ~EmitterFactory();

    void destroy();
    EmitterPtr create(const std::vector<EndpointPtr>&);

private:

    InstancePtr _instance;
    std::map<EndpointPtr, EmitterPtr> _emitters;
};

}

#endif
