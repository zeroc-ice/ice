// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_CONNECTION_H
#define ICEE_CONNECTION_H

#include <IceE/ConnectionF.h>
#include <IceE/OutgoingConnectionFactoryF.h>
#include <IceE/InstanceF.h>
#include <IceE/TransceiverF.h>
#include <IceE/EndpointF.h>
#include <IceE/LoggerF.h>
#include <IceE/TraceLevelsF.h>

#ifndef ICEE_PURE_CLIENT
#   include <IceE/ObjectAdapterF.h>
#   include <IceE/ServantManagerF.h>
#   include <IceE/IncomingConnectionFactoryF.h>
#   include <IceE/Incoming.h>
#endif

#include <IceE/Mutex.h>
#include <IceE/Monitor.h>
#include <IceE/Time.h>
#include <IceE/Thread.h> // For ThreadPerConnection.
#include <IceE/Identity.h>
#include <IceE/BasicStream.h>

namespace IceInternal
{

class Outgoing;

}

namespace Ice
{

class LocalException;

class ICE_API Connection : public IceUtil::Monitor<IceUtil::Mutex>, public IceUtil::Shared
{
public:

    void waitForValidation();
    enum DestructionReason
    {
#ifndef ICEE_PURE_CLIENT
	ObjectAdapterDeactivated,
#endif
	CommunicatorDestroyed
    };

    void activate();
#ifndef ICEE_PURE_CLIENT
    void hold();
#endif
    void destroy(DestructionReason);
    void close(bool); // From Connection.

    bool isDestroyed() const;
    bool isFinished() const;

    void throwException() const; // Throws the connection exception if destroyed.

#ifndef ICEE_PURE_CLIENT
    void waitUntilHolding() const;
#endif
    void waitUntilFinished(); // Not const, as this might close the connection upon timeout.

    void sendRequest(IceInternal::BasicStream*, IceInternal::Outgoing*);

#ifdef ICEE_HAS_BATCH
    void prepareBatchRequest(IceInternal::BasicStream*);
    void finishBatchRequest(IceInternal::BasicStream*);
    void abortBatchRequest();
    void flushBatchRequests(); // From Connection.
#endif

#ifndef ICEE_PURE_CLIENT
    void sendResponse(IceInternal::BasicStream*);
    void sendNoResponse();
#endif

    IceInternal::EndpointPtr endpoint() const;

#ifndef ICEE_PURE_CLIENT
    void setAdapter(const ObjectAdapterPtr&); // From Connection.
    ObjectAdapterPtr getAdapter() const; // From Connection.
    ObjectPrx createProxy(const Identity&) const; // From Connection.
#endif

    std::string type() const; // From Connection.
    Ice::Int timeout() const; // From Connection.
    std::string toString() const;  // From Connection

private:

#ifndef ICEE_PURE_CLIENT
    Connection(const IceInternal::InstancePtr&, const IceInternal::TransceiverPtr&, 
	       const IceInternal::EndpointPtr&, const ObjectAdapterPtr&);
#else
    Connection(const IceInternal::InstancePtr&, const IceInternal::TransceiverPtr&, 
	       const IceInternal::EndpointPtr&);
#endif
    ~Connection();

#ifndef ICEE_PURE_CLIENT
    friend class IceInternal::IncomingConnectionFactory;
#endif
    friend class IceInternal::OutgoingConnectionFactory;

    enum State
    {
	StateNotValidated,
	StateActive,
#ifndef ICEE_PURE_CLIENT
	StateHolding,
#endif
	StateClosing,
	StateClosed
    };

    void validate();
    void setState(State, const LocalException&);
    void setState(State);

    void initiateShutdown() const;

#ifndef ICEE_PURE_CLIENT
    void readStreamAndParseMessage(IceInternal::BasicStream&, Int&, Int&);
#else
    void readStreamAndParseMessage(IceInternal::BasicStream&, Int&);
#endif

#ifdef ICEE_HAS_BATCH
    void flushBatchRequestsInternal(bool);
    void resetBatch(bool);
#endif

#ifndef ICEE_PURE_BLOCKING_CLIENT

    void run();

    class ThreadPerConnection : public IceUtil::Thread
    {
    public:
	
	ThreadPerConnection(const ConnectionPtr&);
	virtual void run();

    private:
	
	ConnectionPtr _connection;
    };
    friend class ThreadPerConnection;
    // Defined as mutable because "isFinished() const" sets this to 0.
    mutable IceUtil::ThreadPtr _threadPerConnection;
#endif

    const IceInternal::InstancePtr _instance;
    IceInternal::TransceiverPtr _transceiver;
    const std::string _desc;
    const std::string _type;
    const IceInternal::EndpointPtr _endpoint;

    const LoggerPtr _logger;
    const IceInternal::TraceLevelsPtr _traceLevels;

    const bool _warn;

#ifndef ICEE_PURE_CLIENT
    IceInternal::Incoming _in;
#endif
#ifndef ICEE_PURE_BLOCKING_CLIENT
    IceInternal::BasicStream _stream;
#endif
#ifdef ICEE_HAS_BATCH
    const bool _batchAutoFlush;
    IceInternal::BasicStream _batchStream;
    bool _batchStreamInUse;
    int _batchRequestNum;
    size_t _batchMarker;
#endif

#if !defined(ICEE_PURE_BLOCKING_CLIENT)
    bool _blocking;
#endif

    std::auto_ptr<LocalException> _exception;

    //
    // Technically this isn't necessary for PURE_CLIENT, but its a
    // pain to get rid of.
    //
    int _dispatchCount;

    State _state; // The current state.
    IceUtil::Time _stateTime; // The last time when the state was changed.

    //
    // We have a separate monitor for sending, so that we don't block
    // the whole connection when we do a blocking send. The monitor is
    // also used by outgoing calls to wait for replies when thread per
    // connection is used. The _nextRequestId, _requests and
    // _requestsHint attributes are also protected by this monitor.
    // Calls on the (non thread-safe) Outgoing objects should also
    // only be made with this monitor locked.
    //
    // Finally, it's safe to lock the _sendMonitor with the connection
    // already locked. The contrary isn't permitted.
    //
    IceUtil::Monitor<IceUtil::Mutex> _sendMonitor;
    Int _nextRequestId;
#ifndef ICEE_PURE_BLOCKING_CLIENT
    std::map<Int, IceInternal::Outgoing*> _requests;
    std::map<Int, IceInternal::Outgoing*>::iterator _requestsHint;
#endif
};

}

#endif
