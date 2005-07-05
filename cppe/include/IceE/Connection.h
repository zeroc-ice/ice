// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
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
#  include <IceE/ObjectAdapterF.h>
#  include <IceE/ServantManagerF.h>
#  include <IceE/IncomingConnectionFactoryF.h>
#endif

#include <IceE/Mutex.h>
#include <IceE/Monitor.h>
#include <IceE/Time.h>
#include <IceE/Thread.h> // For ThreadPerConnection.
#include <IceE/Identity.h>
#include <IceE/BasicStream.h>

namespace IceEInternal
{

class Outgoing;

}

namespace IceE
{

class LocalException;

class ICEE_API Connection : public IceE::Monitor<IceE::Mutex>, public IceE::Shared
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
#ifndef ICEE_PURE_CLIENT
    void activate();
    void hold();
#endif
    void destroy(DestructionReason);
    void close(bool); // From Connection.

    bool isDestroyed() const;
    bool isFinished() const;

#ifndef ICEE_PURE_CLIENT
    void waitUntilHolding() const;
#endif
    void waitUntilFinished(); // Not const, as this might close the connection upon timeout.

    void prepareRequest(IceEInternal::BasicStream*);
    void sendRequest(IceEInternal::BasicStream*, IceEInternal::Outgoing*);

#ifndef ICEE_NO_BATCH
    void prepareBatchRequest(IceEInternal::BasicStream*);
    void finishBatchRequest(IceEInternal::BasicStream*);
    void abortBatchRequest();
    void flushBatchRequests(); // From Connection.
#endif

#ifndef ICEE_PURE_CLIENT
    void sendResponse(IceEInternal::BasicStream*);
    void sendNoResponse();
#endif

    IceEInternal::EndpointPtr endpoint() const;

//#ifndef ICEE_PURE_CLIENT
    void setAdapter(const ObjectAdapterPtr&); // From Connection.
    ObjectAdapterPtr getAdapter() const; // From Connection.
//#endif
    ObjectPrx createProxy(const Identity&) const; // From Connection.

    void exception(const LocalException&);
    std::string type() const; // From Connection.
    IceE::Int timeout() const; // From Connection.
    std::string toString() const;  // From Connection

private:

#ifndef ICEE_PURE_CLIENT
    Connection(const IceEInternal::InstancePtr&, const IceEInternal::TransceiverPtr&, 
	       const IceEInternal::EndpointPtr&, const ObjectAdapterPtr&);
#else
    Connection(const IceEInternal::InstancePtr&, const IceEInternal::TransceiverPtr&, 
	       const IceEInternal::EndpointPtr&);
#endif
    ~Connection();
#ifndef ICEE_PURE_CLIENT
    friend class IceEInternal::IncomingConnectionFactory;
#endif
    friend class IceEInternal::OutgoingConnectionFactory;

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

    void setState(State, const LocalException&);
    void setState(State);
    void validate();

    void initiateShutdown() const;

#ifndef ICEE_PURE_CLIENT
    void parseMessage(IceEInternal::BasicStream&, Int&, Int&, 
		      IceEInternal::ServantManagerPtr&, ObjectAdapterPtr&);
    void invokeAll(IceEInternal::BasicStream&, Int, Int,
		   const IceEInternal::ServantManagerPtr&, const ObjectAdapterPtr&);
#else
    void parseMessage(IceEInternal::BasicStream&, Int&);
#endif

    void run();

    class ThreadPerConnection : public IceE::Thread
    {
    public:
	
	ThreadPerConnection(const ConnectionPtr&);
	virtual void run();

    private:
	
	ConnectionPtr _connection;
    };
    friend class ThreadPerConnection;
    // Defined as mutable because "isFinished() const" sets this to 0.
    mutable IceE::ThreadPtr _threadPerConnection;

    const IceEInternal::InstancePtr _instance;
    IceEInternal::TransceiverPtr _transceiver;
    const std::string _desc;
    const std::string _type;
    const IceEInternal::EndpointPtr _endpoint;

#ifndef ICEE_PURE_CLIENT
    ObjectAdapterPtr _adapter;
    IceEInternal::ServantManagerPtr _servantManager;
#endif

    const LoggerPtr _logger;
    const IceEInternal::TraceLevelsPtr _traceLevels;

    const bool _warn;

    const std::vector<Byte> _requestHdr;
#ifndef ICEE_NO_BATCH
    const std::vector<Byte> _requestBatchHdr;
#endif
#ifndef ICEE_PURE_CLIENT
    const std::vector<Byte> _replyHdr;
#endif

    Int _nextRequestId;

    std::map<Int, IceEInternal::Outgoing*> _requests;
    std::map<Int, IceEInternal::Outgoing*>::iterator _requestsHint;

    std::auto_ptr<LocalException> _exception;

#ifndef ICEE_NO_BATCH
    IceEInternal::BasicStream _batchStream;
    bool _batchStreamInUse;
    int _batchRequestNum;
#endif

    //
    // Technically this isn't necessary for PURE_CLIENT, but its a
    // pain to get rid of.
    //
    int _dispatchCount;

    State _state; // The current state.
    IceE::Time _stateTime; // The last time when the state was changed.

    //
    // We have a separate mutex for sending, so that we don't block
    // the whole connection when we do a blocking send.
    //
    IceE::Mutex _sendMutex;
};

}

#endif
