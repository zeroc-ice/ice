// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_CONNECTION_I_H
#define ICE_CONNECTION_I_H

#include <Ice/InstanceF.h>
#include <Ice/TransceiverF.h>
#include <Ice/ThreadPoolF.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/EndpointF.h>
#include <Ice/LoggerF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/EventHandler.h>
#include <Ice/Connection.h>
#include <list>

namespace IceInternal
{

class Outgoing;

}

namespace Ice
{

class LocalException;

class ConnectionI : virtual public ::IceInternal::EventHandler, virtual public Connection, public JTCRecursiveMutex
{
public:

    ConnectionI(const ::IceInternal::InstancePtr&, const ::IceInternal::TransceiverPtr&,
		const ::IceInternal::EndpointPtr&, const ObjectAdapterPtr&);
    virtual ~ConnectionI();

    bool destroyed() const;
    void hold();
    void activate();
    void prepareRequest(::IceInternal::Outgoing*);
    void sendRequest(::IceInternal::Outgoing*, bool);
    void removeRequest(::IceInternal::Outgoing*);
    void prepareBatchRequest(::IceInternal::Outgoing*);
    void finishBatchRequest(::IceInternal::Outgoing*);
    void abortBatchRequest();
    void flushBatchRequest();
    int timeout() const;

    //
    // Operations from EventHandler
    //
    virtual bool server() const;
    virtual bool readable() const;
    virtual void read(::IceInternal::BasicStream&);
    virtual void message(::IceInternal::BasicStream&);
    virtual void exception(const LocalException&);
    virtual void finished();

    //
    // Operations from Connection
    //
    virtual InternetAddress getLocalAddress();
    virtual InternetAddress getRemoteAddress();
    virtual ProtocolInfoPtr getProtocolInfo();

private:

    enum State
    {
	StateActive,
	StateHolding,
	StateClosing,
	StateClosed
    };

    void setState(State, const LocalException&);
    void setState(State);
    void closeConnection();
    void warning(const Exception&) const;

    ::IceInternal::TransceiverPtr _transceiver;
    ::IceInternal::EndpointPtr _endpoint;
    ObjectAdapterPtr _adapter;
    LoggerPtr _logger;
    ::IceInternal::TraceLevelsPtr _traceLevels;
    ::IceInternal::ThreadPoolPtr _threadPool;
    Int _nextRequestId;
    std::map<Int, ::IceInternal::Outgoing*> _requests;
    std::map<Int, ::IceInternal::Outgoing*>::iterator _requestsHint;
    std::auto_ptr<LocalException> _exception;
    ::IceInternal::BasicStream _batchStream;
    int _responseCount;
    State _state;
    bool _warn;
};

}

#endif
