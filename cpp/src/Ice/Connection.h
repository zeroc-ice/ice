// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
    void hold();
    void activate();
    void incProxyUsageCount();
    void decProxyUsageCount();
    void prepareRequest(Outgoing*);
    void sendRequest(Outgoing*, bool, bool);
    void removeRequest(Outgoing*);
    void prepareBatchRequest(Outgoing*);
    void finishBatchRequest(Outgoing*);
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
    void closeConnection();
    void registerWithPool();
    void unregisterWithPool();
    void compress(BasicStream&, BasicStream&);
    void uncompress(BasicStream&, BasicStream&);

    TransceiverPtr _transceiver;
    EndpointPtr _endpoint;
    ::Ice::ObjectAdapterPtr _adapter;
    ::Ice::LoggerPtr _logger;
    TraceLevelsPtr _traceLevels;
    ThreadPoolPtr _clientThreadPool;
    ThreadPoolPtr _serverThreadPool;
    ::Ice::Int _nextRequestId;
    std::map< ::Ice::Int, Outgoing*> _requests;
    std::map< ::Ice::Int, Outgoing*>::iterator _requestsHint;
    std::auto_ptr< ::Ice::LocalException> _exception;
    BasicStream _batchStream;
    int _responseCount;
    int _proxyUsageCount;
    State _state;
    bool _warn;
};

}

#endif
