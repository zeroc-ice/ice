// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_CONNECTION_H
#define ICE_CONNECTION_H

#include <IceUtil/RecMutex.h>

#include <Ice/ConnectionF.h>
#include <Ice/ConnectionFactoryF.h>
#include <Ice/InstanceF.h>
#include <Ice/TransceiverF.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/EndpointF.h>
#include <Ice/ThreadPoolF.h>
#include <Ice/LoggerF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/OutgoingAsyncF.h>
#include <Ice/DefaultsAndOverridesF.h>
#include <Ice/EventHandler.h>

namespace Ice
{

class LocalException;

}

namespace IceInternal
{

class Outgoing;

class Connection : public EventHandler, public ::IceUtil::RecMutex
{
public:

    bool destroyed() const;
    void validate();
    void hold();
    void activate();
    void incUsageCount();
    void decUsageCount();
    void prepareRequest(BasicStream*);
    void sendRequest(Outgoing*, bool, bool);
    void sendAsyncRequest(const OutgoingAsyncPtr&, bool);
    void prepareBatchRequest(BasicStream*);
    void finishBatchRequest(BasicStream*);
    void abortBatchRequest();
    void flushBatchRequest(bool);
    int timeout() const;
    EndpointPtr endpoint() const;
    void setAdapter(const ::Ice::ObjectAdapterPtr&);
    ::Ice::ObjectAdapterPtr getAdapter() const;

    //
    // Operations from EventHandler
    //
    virtual bool readable() const;
    virtual void read(BasicStream&);
    virtual void message(BasicStream&, const ThreadPoolPtr&);
    virtual void finished(const ThreadPoolPtr&);
    virtual void exception(const ::Ice::LocalException&);
    virtual std::string toString() const;

private:

    Connection(const InstancePtr&, const TransceiverPtr&, const EndpointPtr&, const ::Ice::ObjectAdapterPtr&);
    virtual ~Connection();
    enum DestructionReason
    {
	ObjectAdapterDeactivated,
	CommunicatorDestroyed
    };
    void destroy(DestructionReason);
    friend class IncomingConnectionFactory;
    friend class OutgoingConnectionFactory;

    enum State
    {
	StateActive,
	StateHolding,
	StateClosing,
	StateClosed
    };

    void setState(State, const ::Ice::LocalException&);
    void setState(State);
    void closeConnection() const;
    void registerWithPool();
    void unregisterWithPool();
    static void compress(BasicStream&, BasicStream&);
    static void uncompress(BasicStream&, BasicStream&);

    const TransceiverPtr _transceiver;
    const EndpointPtr _endpoint;
    ::Ice::ObjectAdapterPtr _adapter;
    const ::Ice::LoggerPtr _logger;
    const TraceLevelsPtr _traceLevels;
    const DefaultsAndOverridesPtr _defaultsAndOverrides;
    const ThreadPoolPtr _clientThreadPool;
    const ThreadPoolPtr _serverThreadPool;
    const bool _warn;
    const std::vector< ::Ice::Byte> _requestHdr;
    const std::vector< ::Ice::Byte> _requestBatchHdr;
    ::Ice::Int _nextRequestId;
    std::map< ::Ice::Int, Outgoing*> _requests;
    std::map< ::Ice::Int, Outgoing*>::iterator _requestsHint;
    std::map< ::Ice::Int, OutgoingAsyncPtr> _asyncRequests;
    std::map< ::Ice::Int, OutgoingAsyncPtr>::iterator _asyncRequestsHint;
    std::auto_ptr< ::Ice::LocalException> _exception;
    BasicStream _batchStream;
    int _responseCount;
    int _usageCount;
    State _state;
    bool _registeredWithPool;
};

}

#endif
