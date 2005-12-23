// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Connection.h>
#include <IceE/Instance.h>
#include <IceE/LoggerUtil.h>
#include <IceE/Properties.h>
#include <IceE/TraceUtil.h>
#include <IceE/DefaultsAndOverrides.h>
#include <IceE/Transceiver.h>
#include <IceE/Endpoint.h>
#include <IceE/Outgoing.h>
#include <IceE/LocalException.h>
#include <IceE/Protocol.h>
#include <IceE/ReferenceFactory.h> // For createProxy().
#include <IceE/ProxyFactory.h> // For createProxy().
#include <IceE/BasicStream.h>

#ifndef ICEE_PURE_CLIENT
#   include <IceE/ObjectAdapter.h>
#   include <IceE/Incoming.h>
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(Connection* p) { p->__incRef(); }
void IceInternal::decRef(Connection* p) { p->__decRef(); }

bool
Ice::operator==(const Connection& l, const Connection& r)
{
    return &l == &r;
}

bool
Ice::operator!=(const Connection& l, const Connection& r)
{
    return &l != &r;
}

bool
Ice::operator<(const Connection& l, const Connection& r)
{
    return &l < &r;
}

void
Ice::Connection::waitForValidation()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    
    while(_state == StateNotValidated)
    {
	wait();
    }
    
    if(_state >= StateClosing)
    {
	assert(_exception.get());
	_exception->ice_throw();
    }
}

#ifndef ICEE_PURE_CLIENT
void
Ice::Connection::activate()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    setState(StateActive);
}

void
Ice::Connection::hold()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    setState(StateHolding);
}
#endif

void
Ice::Connection::destroy(DestructionReason reason)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    switch(reason)
    {
#ifndef ICEE_PURE_CLIENT
	case ObjectAdapterDeactivated:
	{
	    setState(StateClosing, ObjectAdapterDeactivatedException(__FILE__, __LINE__));
	    break;
	}
#endif

	case CommunicatorDestroyed:
	{
	    setState(StateClosing, CommunicatorDestroyedException(__FILE__, __LINE__));
	    break;
	}
    }
}

void
Ice::Connection::close(bool force)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(force)
    {
	setState(StateClosed, ForcedCloseConnectionException(__FILE__, __LINE__));
    }
    else
    {
#ifndef ICEE_PURE_BLOCKING_CLIENT
	//
	// If we do a graceful shutdown, then we wait until all
	// outstanding requests have been completed. Otherwise, the
	// CloseConnectionException will cause all outstanding
	// requests to be retried, regardless of whether the server
	// has processed them or not.
	//
	while(!_requests.empty())
	{
	    wait();
	}
#endif

	setState(StateClosing, CloseConnectionException(__FILE__, __LINE__));
    }
}

bool
Ice::Connection::isDestroyed() const
{
    //
    // We can not use trylock here, otherwise the outgoing connection
    // factory might return destroyed (closing or closed) connections,
    // resulting in connection retry exhaustion.
    //
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    return _state >= StateClosing;
}

bool
Ice::Connection::isFinished() const
{
#ifndef ICEE_PURE_BLOCKING_CLIENT
    IceUtil::ThreadPtr threadPerConnection;
#endif

    {
	//
	// We can use trylock here, because as long as there are still
	// threads operating in this connection object, connection
	// destruction is considered as not yet finished.
	//
	IceUtil::Monitor<IceUtil::Mutex>::TryLock sync(*this);
	
	if(!sync.acquired())
	{
	    return false;
	}

	if(_transceiver != 0
#ifndef ICEE_PURE_BLOCKING_CLIENT
	   || _dispatchCount != 0 || (_threadPerConnection && _threadPerConnection->getThreadControl().isAlive())
#endif
	  )
	{
	    return false;
	}

	assert(_state == StateClosed);

#ifndef ICEE_PURE_BLOCKING_CLIENT
	threadPerConnection = _threadPerConnection;
	_threadPerConnection = 0;
#endif
    }

#ifndef ICEE_PURE_BLOCKING_CLIENT
    if(threadPerConnection)
    {
        threadPerConnection->getThreadControl().join();
    }
#endif

    return true;
}

#ifndef ICEE_PURE_CLIENT

void
Ice::Connection::waitUntilHolding() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    while(_state < StateHolding || _dispatchCount > 0)
    {
	wait();
    }
}

#endif

void
Ice::Connection::waitUntilFinished()
{
#ifndef ICEE_PURE_BLOCKING_CLIENT
    IceUtil::ThreadPtr threadPerConnection;
#endif

    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	
	//
	// We wait indefinitely until connection closing has been
	// initiated. We also wait indefinitely until all outstanding
	// requests are completed. Otherwise we couldn't guarantee
	// that there are no outstanding calls when deactivate() is
	// called on the servant locators.
	//
	while(_state < StateClosing || _dispatchCount > 0)
	{
	    wait();
	}
	
	//
	// Now we must wait until close() has been called on the
	// transceiver.
	//
	while(_transceiver)
	{
	    if(_state != StateClosed && _endpoint->timeout() >= 0)
	    {
		IceUtil::Time timeout = IceUtil::Time::milliSeconds(_endpoint->timeout());
		IceUtil::Time waitTime = _stateTime + timeout - IceUtil::Time::now();
		
		if(waitTime > IceUtil::Time())
		{
		    //
		    // We must wait a bit longer until we close this
		    // connection.
		    //
		    if(!timedWait(waitTime))
		    {
			setState(StateClosed, CloseTimeoutException(__FILE__, __LINE__));
		    }
		}
		else
		{
		    //
		    // We already waited long enough, so let's close this
		    // connection!
		    //
		    setState(StateClosed, CloseTimeoutException(__FILE__, __LINE__));
		}
		
		//
		// No return here, we must still wait until close() is
		// called on the _transceiver.
		//
	    }
	    else
	    {
		wait();
	    }
	}
	
	assert(_state == StateClosed);

#ifndef ICEE_PURE_BLOCKING_CLIENT
	threadPerConnection = _threadPerConnection;
	_threadPerConnection = 0;
#endif
    }

#ifndef ICEE_PURE_BLOCKING_CLIENT
    if(threadPerConnection)
    {
	threadPerConnection->getThreadControl().join();
    }
#endif
}

//
// TODO: Should not be a member function of Connection.
//
void
Ice::Connection::prepareRequest(BasicStream* os)
{
    os->writeBlob(_requestHdr);
}


Int 
Ice::Connection::fillRequestId(BasicStream* os)
{
    //
    // Create a new unique request ID.
    //
    Int requestId = _nextRequestId++;
    if(requestId <= 0)
    {
	_nextRequestId = 1;
	requestId = _nextRequestId++;
    }

    //
    // Fill in the request ID.
    //
    const Byte* p = reinterpret_cast<const Byte*>(&requestId);
#ifdef ICE_BIG_ENDIAN
    reverse_copy(p, p + sizeof(Int), os->b.begin() + headerSize);
#else
    copy(p, p + sizeof(Int), os->b.begin() + headerSize);
#endif

    return requestId;
}

void
Ice::Connection::sendRequest(BasicStream* os)
{
    if(!_transceiver) // Has the transceiver already been closed?
    {
        assert(_exception.get()); 
        _exception->ice_throw(); // The exception is immutable at this point.
    }
	
    Int sz = static_cast<Int>(os->b.size());
    const Byte* p = reinterpret_cast<const Byte*>(&sz);
#ifdef ICE_BIG_ENDIAN
    reverse_copy(p, p + sizeof(Int), os->b.begin() + 10);
#else
    copy(p, p + sizeof(Int), os->b.begin() + 10);
#endif
	    
    //
    // Send the request.
    //
    os->i = os->b.begin();
    traceRequest("sending request", *os, _logger, _traceLevels);
    _transceiver->write(*os, _endpoint->timeout());
}

#ifdef ICEE_BLOCKING_CLIENT

void
Ice::Connection::sendBlockingRequest(BasicStream* os, BasicStream* is, Outgoing* out)
{
    Int requestId;

    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

	if(_exception.get())
	{
	    _exception->ice_throw();
	}

	assert(_state > StateNotValidated);
	assert(_state < StateClosing);

	//
	// Fill in request id if it is a twoway call.
	//
	if(out)
	{
	    requestId = fillRequestId(os);
	}
    }

    try
    {
        {
	    IceUtil::Mutex::Lock sendSync(_sendMutex);
	    sendRequest(os);

	    if(out)
	    {
	        readStream(*is);
 	    }
	}

	if(out)
	{
	    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

	    if(_state != StateClosed)
	    {
#ifndef ICEE_PURE_CLIENT
	        Int invokeNum = 0;
	        ServantManagerPtr servantManager;
	        ObjectAdapterPtr adapter;

	        parseMessage(*is, requestId, invokeNum, servantManager, adapter);
#else
	        parseMessage(*is, requestId);
#endif
	    }

	    //
            // parseMessage() can close the connection, so we must
            // check for closed state again.
	    //
	    if(_state == StateClosed)
	    {
	        try
	        {
	            _transceiver->close();
	        }
	        catch(const LocalException&)
	        {
	        }
	
	        _transceiver = 0;
		_exception->ice_throw();
	    }
	}
    }
    catch(const LocalException& ex)
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	setState(StateClosed, ex);
	assert(_exception.get());
	_exception->ice_throw();
    }
}

#endif

#ifndef ICEE_PURE_BLOCKING_CLIENT

void
Ice::Connection::sendRequest(BasicStream* os, Outgoing* out)
{
    Int requestId;

    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

	if(_exception.get())
	{
	    _exception->ice_throw();
	}

	assert(_state > StateNotValidated);
	assert(_state < StateClosing);

	//
	// Only add to the request map if this is a twoway call.
	//
	if(out)
	{
	    requestId = fillRequestId(os);

	    //
	    // Add to the requests map.
	    //
	    _requestsHint = _requests.insert(_requests.end(), pair<const Int, Outgoing*>(requestId, out));
	}
    }

    try
    {
	IceUtil::Mutex::Lock sendSync(_sendMutex);
	sendRequest(os);
    }
    catch(const LocalException& ex)
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	setState(StateClosed, ex);
	assert(_exception.get());

	if(out)
	{
	    //
	    // If the request has already been removed from the
	    // request map, we are out of luck. It would mean that
	    // finished() has been called already, and therefore the
	    // exception has been set using the Outgoing::finished()
	    // callback. In this case, we cannot throw the exception
	    // here, because we must not both raise an exception and
	    // have Outgoing::finished() called with an
	    // exception. This means that in some rare cases, a
	    // request will not be retried even though it could. But I
	    // honestly don't know how I could avoid this, without a
	    // very elaborate and complex design, which would be bad
	    // for performance.
	    //
	    map<Int, Outgoing*>::iterator p = _requests.find(requestId);
	    if(p != _requests.end())
	    {
		if(p == _requestsHint)
		{
		    _requests.erase(p++);
		    _requestsHint = p;
		}
		else
		{
		    _requests.erase(p);
		}

		_exception->ice_throw();
	    }
	}
	else
	{
	    _exception->ice_throw();
	}
    }
}

#endif

#ifdef ICEE_HAS_BATCH

void
Ice::Connection::prepareBatchRequest(BasicStream* os)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    //
    // Wait if flushing is currently in progress.
    //
    while(_batchStreamInUse && !_exception.get())
    {
	wait();
    }

    if(_exception.get())
    {
	_exception->ice_throw();
    }

    assert(_state > StateNotValidated);
    assert(_state < StateClosing);

    if(_batchStream.b.empty())
    {
	try
	{
	    _batchStream.writeBlob(_requestBatchHdr);
	}
	catch(const LocalException& ex)
	{
	    setState(StateClosed, ex);
	    ex.ice_throw();
	}
    }

    _batchStreamInUse = true;
    _batchStream.swap(*os);

    //
    // The batch stream now belongs to the caller, until
    // finishBatchRequest() or abortBatchRequest() is called.
    //
}

void
Ice::Connection::finishBatchRequest(BasicStream* os)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    //
    // Get the batch stream back and increment the number of requests
    // in the batch.
    //
    _batchStream.swap(*os);
    ++_batchRequestNum;

    //
    // Notify about the batch stream not being in use anymore.
    //
    assert(_batchStreamInUse);
    _batchStreamInUse = false;
    notifyAll();
}

void
Ice::Connection::abortBatchRequest()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    
    //
    // Destroy and reset the batch stream and batch count. We cannot
    // safe old requests in the batch stream, as they might be
    // corrupted due to incomplete marshaling.
    //
    BasicStream dummy(_instance.get());
    _batchStream.swap(dummy);
    _batchRequestNum = 0;

    //
    // Notify about the batch stream not being in use
    // anymore.
    //
    assert(_batchStreamInUse);
    _batchStreamInUse = false;
    notifyAll();
}

void
Ice::Connection::flushBatchRequests()
{
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

	while(_batchStreamInUse && !_exception.get())
	{
	    wait();
	}
	
	if(_exception.get())
	{
	    _exception->ice_throw();
	}

	if(_batchStream.b.empty())
	{
	    return; // Nothing to do.
	}

	assert(_state > StateNotValidated);
	assert(_state < StateClosing);

	_batchStream.i = _batchStream.b.begin();

	//
	// Prevent that new batch requests are added while we are
	// flushing.
	//
	_batchStreamInUse = true;
    }
    
    try
    {
	IceUtil::Mutex::Lock sendSync(_sendMutex);

	if(!_transceiver) // Has the transceiver already been closed?
	{
	    assert(_exception.get());
	    _exception->ice_throw(); // The exception is immutable at this point.
	}

	//
	// Fill in the number of requests in the batch.
	//
	const Byte* p = reinterpret_cast<const Byte*>(&_batchRequestNum);
#ifdef ICE_BIG_ENDIAN
	reverse_copy(p, p + sizeof(Int), _batchStream.b.begin() + headerSize);
#else
	copy(p, p + sizeof(Int), _batchStream.b.begin() + headerSize);
#endif
	
	Int sz = static_cast<Int>(_batchStream.b.size());
	p = reinterpret_cast<const Byte*>(&sz);
#ifdef ICE_BIG_ENDIAN
	reverse_copy(p, p + sizeof(Int), _batchStream.b.begin() + 10);
#else
	copy(p, p + sizeof(Int), _batchStream.b.begin() + 10);
#endif
	    
	//
	// Send the batch request.
	//
	_batchStream.i = _batchStream.b.begin();
	traceBatchRequest("sending batch request", _batchStream, _logger, _traceLevels);
	_transceiver->write(_batchStream, _endpoint->timeout());
    }
    catch(const LocalException& ex)
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	setState(StateClosed, ex);
	assert(_exception.get());

	//
	// Since batch requests are all oneways, we
	// must report the exception to the caller.
	//
	_exception->ice_throw();
    }

    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

	//
	// Reset the batch stream, and notify that flushing is over.
	//
	BasicStream dummy(_instance.get());
	_batchStream.swap(dummy);
	_batchRequestNum = 0;
	_batchStreamInUse = false;
	notifyAll();
    }
}

#endif

#ifndef ICEE_PURE_CLIENT

void
Ice::Connection::sendResponse(BasicStream* os)
{
    try
    {
	IceUtil::Mutex::Lock sendSync(_sendMutex);

	if(!_transceiver) // Has the transceiver already been closed?
	{
	    assert(_exception.get());
	    _exception->ice_throw(); // The exception is immutable at this point.
	}

	Int sz = static_cast<Int>(os->b.size());
	const Byte* p = reinterpret_cast<const Byte*>(&sz);
#ifdef ICE_BIG_ENDIAN
	reverse_copy(p, p + sizeof(Int), os->b.begin() + 10);
#else
	copy(p, p + sizeof(Int), os->b.begin() + 10);
#endif    
	    
	//
	// Send the reply.
	//
	os->i = os->b.begin();
	traceReply("sending reply", *os, _logger, _traceLevels);
	_transceiver->write(*os, _endpoint->timeout());
    }
    catch(const LocalException& ex)
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	setState(StateClosed, ex);
    }

    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

	assert(_state > StateNotValidated);

	try
	{
	    if(--_dispatchCount == 0)
	    {
		notifyAll();
	    }
	    
	    if(_state == StateClosing && _dispatchCount == 0)
	    {
		initiateShutdown();
	    }
	}
	catch(const LocalException& ex)
	{
	    setState(StateClosed, ex);
	}
    }
}

void
Ice::Connection::sendNoResponse()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    
    assert(_state > StateNotValidated);

    try
    {
	if(--_dispatchCount == 0)
	{
	    notifyAll();
	}

	if(_state == StateClosing && _dispatchCount == 0)
	{
	    initiateShutdown();
	}
    }
    catch(const LocalException& ex)
    {
	setState(StateClosed, ex);
    }
}

#endif

EndpointPtr
Ice::Connection::endpoint() const
{
    return _endpoint; // No mutex protection necessary, _endpoint is immutable.
}

#if defined(ICEE_BLOCKING_CLIENT) && !defined(ICEE_PURE_BLOCKING_CLIENT)
bool
Ice::Connection::blocking() const
{
    return _blocking;
}
#endif

#ifndef ICEE_PURE_CLIENT

void
Ice::Connection::setAdapter(const ObjectAdapterPtr& adapter)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(_exception.get())
    {
	_exception->ice_throw();
    }
    
    assert(_state < StateClosing);

    _adapter = adapter;

    if(_adapter)
    {
	_servantManager = _adapter->getServantManager();
    }
    else
    {
	_servantManager = 0;
    }

    //
    // We never change the thread pool with which we were initially
    // registered, even if we add or remove an object adapter.
    //
}

ObjectAdapterPtr
Ice::Connection::getAdapter() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    return _adapter;
}

#endif

ObjectPrx
Ice::Connection::createProxy(const Identity& ident) const
{
    //
    // Create a reference and return a reverse proxy for this
    // reference.
    //
    vector<ConnectionPtr> connections;
    connections.push_back(const_cast<Connection*>(this));
    ReferencePtr ref = _instance->referenceFactory()->create(ident, Context(), "", Reference::ModeTwoway, connections);
    return _instance->proxyFactory()->referenceToProxy(ref);
}

void
Ice::Connection::exception(const LocalException& ex)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    setState(StateClosed, ex);
}

string
Ice::Connection::type() const
{
    return _type; // No mutex lock, _type is immutable.
}

Ice::Int
Ice::Connection::timeout() const
{
    return _endpoint->timeout(); // No mutex lock, _endpoint is immutable.
}

string
Ice::Connection::toString() const
{
    return _desc; // No mutex lock, _desc is immutable.
}

#ifndef ICEE_PURE_CLIENT
Ice::Connection::Connection(const InstancePtr& instance,
			    const TransceiverPtr& transceiver,
			    const EndpointPtr& endpoint,
			    const ObjectAdapterPtr& adapter) :
#else
Ice::Connection::Connection(const InstancePtr& instance,
			    const TransceiverPtr& transceiver,
			    const EndpointPtr& endpoint) :
#endif

    _instance(instance),
    _transceiver(transceiver),
    _desc(transceiver->toString()),
    _type(transceiver->type()),
    _endpoint(endpoint),
#ifndef ICEE_PURE_CLIENT
    _adapter(adapter),
#endif
    _logger(_instance->logger()), // Cached for better performance.
    _traceLevels(_instance->traceLevels()), // Cached for better performance.
    _warn(_instance->properties()->getPropertyAsInt("Ice.Warn.Connections") > 0),
    _requestHdr(headerSize + sizeof(Int), 0),
#ifndef ICEE_PURE_CLIENT
    _replyHdr(headerSize, 0),
#endif
#ifndef ICEE_PURE_BLOCKING_CLIENT
    _nextRequestId(1),
    _requestsHint(_requests.end()),
#endif
#ifdef ICEE_HAS_BATCH
    _requestBatchHdr(headerSize + sizeof(Int), 0),
    _batchStream(_instance.get()),
    _batchStreamInUse(false),
    _batchRequestNum(0),
#endif
    _dispatchCount(0),
    _state(StateNotValidated),
    _stateTime(IceUtil::Time::now())
{
#if defined(ICEE_BLOCKING_CLIENT) && !defined(ICEE_PURE_BLOCKING_CLIENT)
#  ifdef ICEE_PURE_CLIENT
    _blocking = _instance->properties()->getPropertyAsInt("Ice.Blocking") > 0;
#  else
    _blocking = _instance->properties()->getPropertyAsInt("Ice.Blocking") > 0 && !_adapter;
#  endif
#endif

    vector<Byte>& requestHdr = const_cast<vector<Byte>&>(_requestHdr);
    requestHdr[0] = magic[0];
    requestHdr[1] = magic[1];
    requestHdr[2] = magic[2];
    requestHdr[3] = magic[3];
    requestHdr[4] = protocolMajor;
    requestHdr[5] = protocolMinor;
    requestHdr[6] = encodingMajor;
    requestHdr[7] = encodingMinor;
    requestHdr[8] = requestMsg;
    requestHdr[9] = 0;

#ifdef ICEE_HAS_BATCH
    vector<Byte>& requestBatchHdr = const_cast<vector<Byte>&>(_requestBatchHdr);
    requestBatchHdr[0] = magic[0];
    requestBatchHdr[1] = magic[1];
    requestBatchHdr[2] = magic[2];
    requestBatchHdr[3] = magic[3];
    requestBatchHdr[4] = protocolMajor;
    requestBatchHdr[5] = protocolMinor;
    requestBatchHdr[6] = encodingMajor;
    requestBatchHdr[7] = encodingMinor;
    requestBatchHdr[8] = requestBatchMsg;
    requestBatchHdr[9] = 0;
#endif

#ifndef ICEE_PURE_CLIENT
    vector<Byte>& replyHdr = const_cast<vector<Byte>&>(_replyHdr);
    replyHdr[0] = magic[0];
    replyHdr[1] = magic[1];
    replyHdr[2] = magic[2];
    replyHdr[3] = magic[3];
    replyHdr[4] = protocolMajor;
    replyHdr[5] = protocolMinor;
    replyHdr[6] = encodingMajor;
    replyHdr[7] = encodingMinor;
    replyHdr[8] = replyMsg;
    replyHdr[9] = 0;

    if(_adapter)
    {
	_servantManager = _adapter->getServantManager();
    }
#endif

#ifdef ICEE_PURE_BLOCKING_CLIENT
    validate();
#else
#  ifdef ICEE_BLOCKING_CLIENT
    if(_blocking)
    {
    	validate();
    }
    else
#  endif
    {
        __setNoDelete(true);
        try
        {
	    //
	    // If we are in thread per connection mode, create the thread
	    // for this connection.
	    //
	    _threadPerConnection = new ThreadPerConnection(this);
	    _threadPerConnection->start(_instance->threadPerConnectionStackSize());
        }
        catch(const Ice::Exception& ex)
        {
	    {
	        Error out(_logger);
	        out << "cannot create thread for connection:\n" << ex.toString();
	    }
	
	    try
	    {
	        _transceiver->close();
	    }
	    catch(const LocalException&)
	    {
	        // Here we ignore any exceptions in close().
	    }
	
	    __setNoDelete(false);
	    ex.ice_throw();
        }
        __setNoDelete(false);
    }
#endif
}

Ice::Connection::~Connection()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    
    assert(_state == StateClosed);
    assert(!_transceiver);
#ifndef ICEE_PURE_BLOCKING_CLIENT
    assert(_dispatchCount == 0);
    assert(!_threadPerConnection);
#endif
}

void
Ice::Connection::validate()
{
#ifndef ICEE_PURE_CLIENT
    bool active;

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	    
        assert(_state == StateNotValidated);
        
        if(_adapter)
        {
    	    active = true; // The server side has the active role for connection validation.
        }
        else
        {
    	    active = false; // The client side has the passive role for connection validation.
        }
    }
#endif

    try
    {
        Int timeout;
        if(_instance->defaultsAndOverrides()->overrideConnectTimeout)
        {
    	    timeout = _instance->defaultsAndOverrides()->overrideConnectTimeoutValue;
        }
        else
        {
    	    timeout = _endpoint->timeout();
        }
        
#ifndef ICEE_PURE_CLIENT
        if(active)
        {
    	    BasicStream os(_instance.get());
    	    os.writeBlob(magic, sizeof(magic));
    	    os.write(protocolMajor);
    	    os.write(protocolMinor);
    	    os.write(encodingMajor);
    	    os.write(encodingMinor);
    	    os.write(validateConnectionMsg);
    	    os.write(static_cast<Byte>(0)); // Compression status (always zero for validate connection).
    	    os.write(headerSize); // Message size.
    	    os.i = os.b.begin();
    	    traceHeader("sending validate connection", os, _logger, _traceLevels);
    	    try
    	    {
    	        _transceiver->write(os, timeout);
    	    }
    	    catch(const TimeoutException&)
    	    {
    	        throw ConnectTimeoutException(__FILE__, __LINE__);
    	    }
        }
        else
#endif
        {
    	    BasicStream is(_instance.get());
    	    is.b.resize(headerSize);
    	    is.i = is.b.begin();
    	    try
    	    {
    	        _transceiver->read(is, timeout);
    	    }
    	    catch(const TimeoutException&)
    	    {
    	        throw ConnectTimeoutException(__FILE__, __LINE__);
    	    }
    	    assert(is.i == is.b.end());
    	    is.i = is.b.begin();
    	    ByteSeq m(sizeof(magic), 0);
    	    is.readBlob(m, static_cast<Int>(sizeof(magic)));
    	    if(!equal(m.begin(), m.end(), magic))
    	    {
    	        BadMagicException ex(__FILE__, __LINE__);
    	        ex.badMagic = m;
    	        throw ex;
    	    }
    	    Byte pMajor;
    	    Byte pMinor;
    	    is.read(pMajor);
    	    is.read(pMinor);
    	    if(pMajor != protocolMajor)
    	    {
    	        UnsupportedProtocolException ex(__FILE__, __LINE__);
    	        ex.badMajor = static_cast<unsigned char>(pMajor);
    	        ex.badMinor = static_cast<unsigned char>(pMinor);
    	        ex.major = static_cast<unsigned char>(protocolMajor);
    	        ex.minor = static_cast<unsigned char>(protocolMinor);
    	        throw ex;
    	    }
    	    Byte eMajor;
    	    Byte eMinor;
    	    is.read(eMajor);
    	    is.read(eMinor);
    	    if(eMajor != encodingMajor)
    	    {
    	        UnsupportedEncodingException ex(__FILE__, __LINE__);
    	        ex.badMajor = static_cast<unsigned char>(eMajor);
    	        ex.badMinor = static_cast<unsigned char>(eMinor);
    	        ex.major = static_cast<unsigned char>(encodingMajor);
    	        ex.minor = static_cast<unsigned char>(encodingMinor);
    	        throw ex;
    	    }
    	    Byte messageType;
    	    is.read(messageType);
    	    if(messageType != validateConnectionMsg)
    	    {
    	        throw ConnectionNotValidatedException(__FILE__, __LINE__);
    	    }
            Byte compress;
            is.read(compress); // Ignore compression status for validate connection.
    	    Int size;
	    is.read(size);
	    if(size != headerSize)
	    {
	        throw IllegalMessageSizeException(__FILE__, __LINE__);
	    }
	    traceHeader("received validate connection", is, _logger, _traceLevels);
        }
    }
    catch(const LocalException& ex)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        setState(StateClosed, ex);
        assert(_exception.get());
        _exception->ice_throw();
    }

#ifdef ICEE_PURE_CLIENT
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	
	//
	// We start out in active state.
	//
	setState(StateActive);
    }
#else
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	
	//
	// We start out in holding state.
	//
	setState(StateHolding);
    }
#endif
}

void
Ice::Connection::setState(State state, const LocalException& ex)
{
    //
    // If setState() is called with an exception, then only closed and
    // closing states are permissible.
    //
    assert(state == StateClosing || state == StateClosed);

    if(_state == state) // Don't switch twice.
    {
	return;
    }

    if(!_exception.get())
    {
	//
	// If we are in closed state, an exception must be set.
	//
	assert(_state != StateClosed);

	_exception.reset(dynamic_cast<LocalException*>(ex.ice_clone()));

	if(_warn)
	{
	    //
	    // We don't warn if we are not validated.
	    //
	    if(_state > StateNotValidated)
	    {
		//
		// Don't warn about certain expected exceptions.
		//
		if(!(dynamic_cast<const CloseConnectionException*>(_exception.get()) ||
		     dynamic_cast<const ForcedCloseConnectionException*>(_exception.get()) ||
		     dynamic_cast<const CommunicatorDestroyedException*>(_exception.get()) ||
#ifndef ICEE_PURE_CLIENT
		     dynamic_cast<const ObjectAdapterDeactivatedException*>(_exception.get()) ||
#endif
		     (dynamic_cast<const ConnectionLostException*>(_exception.get()) && _state == StateClosing)))
		{
		    Warning out(_logger);
		    out << "connection exception:\n" << (*_exception.get()).toString() << "\n" << _desc;
		}
	    }
	}
    }

    //
    // We must set the new state before we notify requests of any
    // exceptions. Otherwise new requests may retry on a connection
    // that is not yet marked as closed or closing.
    //
    setState(state);
}

void
Ice::Connection::setState(State state)
{
    if(_state == state) // Don't switch twice.
    {
	return;
    }
    
    switch(state)
    {
	case StateNotValidated:
	{
	    assert(false);
	    break;
	}

	case StateActive:
	{
	    //
            // Can only switch from holding or not validated to
            // active.
	    //
#ifdef ICEE_PURE_CLIENT
	    if(_state != StateNotValidated)
	    {
		return;
	    }
#else
	    if(_state != StateHolding && _state != StateNotValidated)
	    {
		return;
	    }
#endif
	    break;
	}
	
#ifndef ICEE_PURE_CLIENT
	case StateHolding:
	{
	    //
	    // Can only switch from active or not validated to
	    // holding.
	    //
	    if(_state != StateActive && _state != StateNotValidated)
	    {
		return;
	    }
	    break;
	}
#endif

	case StateClosing:
	{
	    //
	    // Can't change back from closed.
	    //
	    if(_state == StateClosed)
	    {
		return;
	    }
	    break;
	}
	
	case StateClosed:
	{
	    //
	    // If we are in thread per connection mode, we
	    // shutdown both for reading and writing. This will
	    // unblock and read call with an exception. The thread
	    // per connection then closes the transceiver.
	    //
	    _transceiver->shutdownReadWrite();
	    break;
	}
    }

    _state = state;
    _stateTime = IceUtil::Time::now();

    notifyAll();

    if(_state == StateClosing && _dispatchCount == 0)
    {
	try
	{
	    initiateShutdown();
	}
	catch(const LocalException& ex)
	{
	    setState(StateClosed, ex);
	}

#ifdef ICEE_BLOCKING_CLIENT
	if(_state != StateClosed
#  ifndef ICEE_PURE_BLOCKING_CLIENT
	   && _blocking
#  endif
	  )
	{
	    try
	    {
	        _transceiver->close();
	    }
	    catch(const LocalException&)
	    {
	    }
	    _transceiver = 0;
	    _state = StateClosed;
	}
#endif
    }
}

void
Ice::Connection::initiateShutdown() const
{
    assert(_state == StateClosing);
    assert(_dispatchCount == 0);

    IceUtil::Mutex::Lock sendSync(_sendMutex);

    //
    // Before we shut down, we send a close connection message.
    //
    BasicStream os(_instance.get());
    os.writeBlob(magic, sizeof(magic));
    os.write(protocolMajor);
    os.write(protocolMinor);
    os.write(encodingMajor);
    os.write(encodingMinor);
    os.write(closeConnectionMsg);
    os.write((Byte)0); // Compression status: compression not supported.
    os.write(headerSize); // Message size.

    //
    // Send the message.
    //
    os.i = os.b.begin();
    traceHeader("sending close connection", os, _logger, _traceLevels);
    _transceiver->write(os, _endpoint->timeout());

    //
    // The CloseConnection message should be sufficient. Closing the
    // write end of the socket is probably an artifact of how things
    // were done in IIOP. In fact, shutting down the write end of the
    // socket causes problems on Windows by preventing the peer from
    // using the socket.  For example, the peer is no longer able to
    // continue writing a large message after the socket is shutdown.
    //
    //_transceiver->shutdownWrite();
}

void
Ice::Connection::parseMessage(BasicStream& stream, Int& requestId
#ifndef ICEE_PURE_CLIENT
			      ,Int& invokeNum, ServantManagerPtr& servantManager, ObjectAdapterPtr& adapter
#endif
)
{
    assert(_state > StateNotValidated && _state < StateClosed);
    
    try
    {
	//
	// We don't need to check magic and version here. This has
	// already been done by the ThreadPerConnection,
	// which provides us with the stream.
	//
	assert(stream.i == stream.b.end());
	stream.i = stream.b.begin() + 8;
	Byte messageType;
	stream.read(messageType);

	Byte compress;
	stream.read(compress);
	if(compress == 2)
	{
	    FeatureNotSupportedException ex(__FILE__, __LINE__);
	    ex.unsupportedFeature = "compression";
	    throw ex;
	}
	stream.i = stream.b.begin() + headerSize;
    
#ifdef ICEE_BLOCKING_CLIENT
#  ifndef ICEE_PURE_BLOCKING_CLIENT
	if(_blocking)
	{
#  endif
	    switch(messageType)
	    {
	        case closeConnectionMsg:
	        {
		    traceHeader("received close connection", stream, _logger, _traceLevels);
		    setState(StateClosed, CloseConnectionException(__FILE__, __LINE__));
		    break;
	        }

	        case replyMsg:
	        {
		    traceReply("received reply", stream, _logger, _traceLevels);
	    
	    	    Int reqId;
		    stream.read(reqId);
		    if(reqId != requestId)
		    {
		        throw UnknownRequestIdException(__FILE__, __LINE__);
		    }
		    break;
		}			
		

	        default:
	        {
		    traceHeader("received unexpected message\n"
			        "(invalid, closing connection)",
			        stream, _logger, _traceLevels);
		    throw UnknownMessageException(__FILE__, __LINE__);
		    break;
	        }
	    }
#  ifndef ICEE_PURE_BLOCKING_CLIENT
	}
	else
#  endif
#endif
#ifndef ICEE_PURE_BLOCKING_CLIENT
	{
	    switch(messageType)
	    {
	        case closeConnectionMsg:
	        {
		    traceHeader("received close connection", stream, _logger, _traceLevels);
		    setState(StateClosed, CloseConnectionException(__FILE__, __LINE__));
		    break;
	        }
	
#ifndef ICEE_PURE_CLIENT
	        case requestMsg:
	        {
		    if(_state == StateClosing)
		    {
		        traceRequest("received request during closing\n"
				     "(ignored by server, client will retry)",
				     stream, _logger, _traceLevels);
		    }
		    else
		    {
		        traceRequest("received request", stream, _logger, _traceLevels);
		        stream.read(requestId);
		        invokeNum = 1;
		        servantManager = _servantManager;
		        adapter = _adapter;
		        ++_dispatchCount;
		    }
		    break;
	        }
	
	        case requestBatchMsg:
	        {
		    if(_state == StateClosing)
		    {
		        traceBatchRequest("received batch request during closing\n"
				          "(ignored by server, client will retry)",
				          stream, _logger, _traceLevels);
		    }
		    else
		    {
		        traceBatchRequest("received batch request", stream, _logger, _traceLevels);
		        stream.read(invokeNum);
		        if(invokeNum < 0)
		        {
			    invokeNum = 0;
			    throw NegativeSizeException(__FILE__, __LINE__);
		        }
		        servantManager = _servantManager;
		        adapter = _adapter;
		        _dispatchCount += invokeNum;
		    }
		    break;
	        }
#endif
	
	        case replyMsg:
	        {
		    traceReply("received reply", stream, _logger, _traceLevels);
	    
		    stream.read(requestId);
	    
		    map<Int, Outgoing*>::iterator p = _requests.end();
	        
		    if(_requestsHint != _requests.end())
		    {
		        if(_requestsHint->first == requestId)
		        {
			    p = _requestsHint;
		        }
		    }
	    
		    if(p == _requests.end())
		    {
		        p = _requests.find(requestId);
		    }
	    
		    if(p == _requests.end())
		    {
		        throw UnknownRequestIdException(__FILE__, __LINE__);
		    }
	    
		    if(p != _requests.end())
		    {
		        p->second->finished(stream);
		
		        if(p == _requestsHint)
		        {
			    _requests.erase(p++);
			    _requestsHint = p;
		        }
		        else
		        {
			    _requests.erase(p);
		        }
		    }
	    
		    break;
	        }
	
	        case validateConnectionMsg:
	        {
		    traceHeader("received validate connection", stream, _logger, _traceLevels);
		    if(_warn)
		    {
		        Warning out(_logger);
		        out << "ignoring unexpected validate connection message:\n" << _desc;
		    }
		    break;
	        }
	
	        default:
	        {
		    traceHeader("received unknown message\n"
			        "(invalid, closing connection)",
			        stream, _logger, _traceLevels);
		    throw UnknownMessageException(__FILE__, __LINE__);
		    break;
	        }
	    }
	}
#endif
    }
    catch(const LocalException& ex)
    {
	setState(StateClosed, ex);
    }
}

#ifndef ICEE_PURE_CLIENT
void
Ice::Connection::invokeAll(BasicStream& stream, Int invokeNum, Int requestId,
			    const ServantManagerPtr& servantManager, const ObjectAdapterPtr& adapter)
{
    //
    // Note: In contrast to other private or protected methods, this
    // operation must be called *without* the mutex locked.
    //

    try
    {
	while(invokeNum > 0)
	{
	    //
	    // Prepare the invocation.
	    //
	    bool response = requestId != 0;
	    Incoming in(_instance.get(), this, adapter, response);
	    BasicStream* is = in.is();
	    stream.swap(*is);
	    BasicStream* os = in.os();
	    
	    //
	    // Prepare the response if necessary.
	    //
	    if(response)
	    {
		assert(invokeNum == 1); // No further invocations if a response is expected.
		os->writeBlob(_replyHdr);
		
		//
		// Add the request ID.
		//
		os->write(requestId);
	    }

	    in.invoke(servantManager);
	    
	    //
	    // If there are more invocations, we need the stream back.
	    //
	    if(--invokeNum > 0)
	    {
		stream.swap(*is);
	    }
	}
    }
    catch(const LocalException& ex)
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	setState(StateClosed, ex);
    }
    catch(const std::exception& ex)
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	UnknownException uex(__FILE__, __LINE__);
	uex.unknown = string("std::exception: ") + ex.what();
	setState(StateClosed, uex);
    }
    catch(...)
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	UnknownException uex(__FILE__, __LINE__);
	uex.unknown = "unknown c++ exception";
	setState(StateClosed, uex);
    }

    //
    // If invoke() above raised an exception, and therefore neither
    // sendResponse() nor sendNoResponse() has been called, then we
    // must decrement _dispatchCount here.
    //
    if(invokeNum > 0)
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	assert(_dispatchCount > 0);
	_dispatchCount -= invokeNum;
	assert(_dispatchCount >= 0);
	if(_dispatchCount == 0)
	{
	    notifyAll();
	}
    }
}
#endif

void
Ice::Connection::readStream(IceInternal::BasicStream& stream)
{
    try
    {
        stream.b.resize(headerSize);
        stream.i = stream.b.begin();
        _transceiver->read(stream,
#ifdef ICEE_PURE_BLOCKING_CLIENT
			   _endpoint->timeout()
#else
#  ifdef ICEE_BLOCKING_CLIENT
	                   _blocking ? _endpoint->timeout() :
#  endif
			   -1
#endif
			   );
    
        ptrdiff_t pos = stream.i - stream.b.begin();
        assert(pos >= headerSize);
        stream.i = stream.b.begin();
        ByteSeq m(sizeof(magic), 0);
        stream.readBlob(m, static_cast<Int>(sizeof(magic)));
        if(!equal(m.begin(), m.end(), magic))
        {
    	    BadMagicException ex(__FILE__, __LINE__);
    	    ex.badMagic = m;
    	    throw ex;
        }
        Byte pMajor;
        Byte pMinor;
        stream.read(pMajor);
        stream.read(pMinor);
        if(pMajor != protocolMajor)
        {
    	    UnsupportedProtocolException ex(__FILE__, __LINE__);
    	    ex.badMajor = static_cast<unsigned char>(pMajor);
    	    ex.badMinor = static_cast<unsigned char>(pMinor);
    	    ex.major = static_cast<unsigned char>(protocolMajor);
    	    ex.minor = static_cast<unsigned char>(protocolMinor);
    	    throw ex;
        }
        Byte eMajor;
        Byte eMinor;
        stream.read(eMajor);
        stream.read(eMinor);
        if(eMajor != encodingMajor)
        {
    	    UnsupportedEncodingException ex(__FILE__, __LINE__);
    	    ex.badMajor = static_cast<unsigned char>(eMajor);
    	    ex.badMinor = static_cast<unsigned char>(eMinor);
    	    ex.major = static_cast<unsigned char>(encodingMajor);
    	    ex.minor = static_cast<unsigned char>(encodingMinor);
    	    throw ex;
        }
        Byte messageType;
        stream.read(messageType);
        Byte compress;
        stream.read(compress);
        Int size;
        stream.read(size);
        if(size < headerSize)
        {
    	    throw IllegalMessageSizeException(__FILE__, __LINE__);
        }
        if(size > static_cast<Int>(_instance->messageSizeMax()))
        {
    	    throw MemoryLimitException(__FILE__, __LINE__);
        }
        if(size > static_cast<Int>(stream.b.size()))
        {
    	    stream.b.resize(size);
        }
        stream.i = stream.b.begin() + pos;
    
        if(stream.i != stream.b.end())
        {
            _transceiver->read(stream,
#ifdef ICEE_PURE_BLOCKING_CLIENT
			       _endpoint->timeout()
#else
#  ifdef ICEE_BLOCKING_CLIENT
	                       _blocking ? _endpoint->timeout() :
#  endif
			       -1
#endif
			       );
    	    assert(stream.i == stream.b.end());
        }
    }
    catch(const LocalException& ex)
    {
        exception(ex);
    }
}

#ifndef ICEE_PURE_BLOCKING_CLIENT

void
Ice::Connection::run()
{
    //
    // The thread-per-connection must validate and activate this connection,
    // and not in the connection factory. Please see the comments in the connection
    // factory for details.
    //
    try
    {
        validate();
    }
    catch(const LocalException&)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        
        assert(_state == StateClosed);
        
        //
        // We must make sure that nobody is sending when we close
        // the transceiver.
        //
        IceUtil::Mutex::Lock sendSync(_sendMutex);
        
        try
        {
    	    _transceiver->close();
        }
        catch(const LocalException&)
        {
    	    // Here we ignore any exceptions in close().
        }
        
        _transceiver = 0;
        notifyAll();
        return;
    }
	
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	setState(StateActive);
    }

    bool closed = false;

    while(!closed)
    {
	//
	// We must accept new connections outside the thread
	// synchronization, because we use blocking accept.
	//

	BasicStream stream(_instance.get());
	readStream(stream);

	Int requestId = 0;
#ifndef ICEE_PURE_CLIENT
	Int invokeNum = 0;
	ServantManagerPtr servantManager;
	ObjectAdapterPtr adapter;
#endif
	
	auto_ptr<LocalException> exception;
	
	map<Int, Outgoing*> requests;

	{
	    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

#ifndef ICEE_PURE_CLIENT
	    while(_state == StateHolding)
	    {
		wait();
	    }
#endif
	    if(_state != StateClosed)
	    {
#ifndef ICEE_PURE_CLIENT
		parseMessage(stream, requestId, invokeNum, servantManager, adapter);
#else
		parseMessage(stream, requestId);
#endif
	    }

	    //
            // parseMessage() can close the connection, so we must
            // check for closed state again.
	    //
	    if(_state == StateClosed)
	    {
		//
		// We must make sure that nobody is sending when we close
		// the transceiver.
		//
		IceUtil::Mutex::Lock sendSync(_sendMutex);
		
		try
		{
		    _transceiver->close();
		}
		catch(const LocalException& ex)
		{
		    exception.reset(dynamic_cast<LocalException*>(ex.ice_clone()));
		}
		
		_transceiver = 0;
		notifyAll();

		//
		// We cannot simply return here. We have to make sure
		// that all requests are notified about the closed 
		// connection below.
		//
		closed = true;
	    }

	    if(_state == StateClosed || _state == StateClosing)
	    {
		requests.swap(_requests);
		_requestsHint = _requests.end();
	    }
	}

	//
	// Method invocation (or multiple invocations for batch
	// messages) must be done outside the thread synchronization,
	// so that nested calls are possible.
	//
#ifndef ICEE_PURE_CLIENT
	invokeAll(stream, invokeNum, requestId, servantManager, adapter);
#endif
	for(map<Int, Outgoing*>::iterator p = requests.begin(); p != requests.end(); ++p)
	{
	    p->second->finished(*_exception.get()); // The exception is immutable at this point.
	}
	
	if(exception.get())
	{
	    assert(closed);
	    exception->ice_throw();
	}    
    }
}

Ice::Connection::ThreadPerConnection::ThreadPerConnection(const ConnectionPtr& connection) :
    _connection(connection)
{
}

void
Ice::Connection::ThreadPerConnection::run()
{
    try
    {
	_connection->run();
    }
    catch(const Exception& ex)
    {	
	Error out(_connection->_instance->logger());
	out << "exception in thread per connection:\n" << _connection->toString() << ex.toString(); 
    }
    catch(const std::exception& ex)
    {
	Error out(_connection->_instance->logger());
	out << "std::exception in thread per connection:\n" << _connection->toString() << ex.what();
    }
    catch(...)
    {
	Error out(_connection->_instance->logger());
	out << "unknown exception in thread per connection:\n" << _connection->toString();
    }

    _connection = 0; // Resolve cyclic dependency.
}

#endif // ICEE_PURE_BLOCKING_CLIENT
