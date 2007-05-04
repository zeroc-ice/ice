// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/DisableWarnings.h>
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
#include <IceE/TraceLevels.h>

#ifndef ICEE_PURE_CLIENT
#   include <IceE/Incoming.h>
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(Connection* p) { return p; }

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

void
Ice::Connection::activate()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    setState(StateActive);
}

#ifndef ICEE_PURE_CLIENT
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
	   || _dispatchCount != 0 || (_threadPerConnection && _threadPerConnection->isAlive())
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

void
Ice::Connection::throwException() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(_exception.get())
    {
        assert(_state >= StateClosing);
        _exception->ice_throw();
    }
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

void
Ice::Connection::sendRequest(BasicStream* os, Outgoing* out)
{
    bool requestSent = false;
    try
    {
	Lock sendSync(_sendMonitor);
	if(!_transceiver)
	{
	    assert(_exception.get());
	    //
	    // If the connection is closed before we even have a chance
	    // to send our request, we always try to send the request
	    // again.
	    //
	    throw LocalExceptionWrapper(*_exception.get(), true);
	}

	Int requestId;
	if(out)
	{
	    //
	    // Create a new unique request ID.
	    //
	    requestId = _nextRequestId++;
	    if(requestId <= 0)
	    {
		_nextRequestId = 1;
		requestId = _nextRequestId++;
	    }
	    
	    //
	    // Fill in the request ID.
	    //
	    Byte* dest = &(os->b[0]) + headerSize;
#ifdef ICE_BIG_ENDIAN
	    const Byte* src = reinterpret_cast<const Byte*>(&requestId) + sizeof(Ice::Int) - 1;
	    *dest++ = *src--;
	    *dest++ = *src--;
	    *dest++ = *src--;
	    *dest = *src;
#else
	    const Byte* src = reinterpret_cast<const Byte*>(&requestId);
	    *dest++ = *src++;
	    *dest++ = *src++;
	    *dest++ = *src++;
	    *dest = *src;
#endif

#ifndef ICEE_PURE_BLOCKING_CLIENT
	    if(!_blocking)
	    {
		_requestsHint = _requests.insert(_requests.end(), pair<const Int, Outgoing*>(requestId, out));
	    }
#endif
	}

	//
	// Fill in the message size.
	//
	const Int sz = static_cast<Int>(os->b.size());
	Byte* dest = &(os->b[0]) + 10;
#ifdef ICE_BIG_ENDIAN
	const Byte* src = reinterpret_cast<const Byte*>(&sz) + sizeof(Ice::Int) - 1;
	*dest++ = *src--;
	*dest++ = *src--;
	*dest++ = *src--;
	*dest = *src;
#else
	const Byte* src = reinterpret_cast<const Byte*>(&sz);
	*dest++ = *src++;
	*dest++ = *src++;
	*dest++ = *src++;
	*dest = *src;
#endif
	    
	//
	// Send the request.
	//
	os->i = os->b.begin();
	if(_traceLevels->protocol >= 1)
	{
	    traceRequest("sending request", *os, _logger, _traceLevels);
	}
	_transceiver->write(*os);	
	requestSent = true;

	if(!out)
	{
	    return;
	}
	
#ifndef ICEE_PURE_BLOCKING_CLIENT
	if(_blocking)
	{
#endif
	    //
	    // Re-use the stream for reading the reply.
	    //
	    os->reset();
		
	    Int receivedRequestId = 0;
#ifndef ICEE_PURE_CLIENT
	    Int invokeNum = 0;
	    readStreamAndParseMessage(*os, receivedRequestId, invokeNum);
	    if(invokeNum > 0)
	    {
		throwUnknownMessageException(__FILE__, __LINE__);
	    }
	    else if(requestId != receivedRequestId)
	    {
		throwUnknownRequestIdException(__FILE__, __LINE__);
	    }
#else
	    readStreamAndParseMessage(*os, receivedRequestId);
	    if(requestId != receivedRequestId)
	    {
		throwUnknownRequestIdException(__FILE__, __LINE__);
	    }
#endif
	    out->finished(*os);
#ifndef ICEE_PURE_BLOCKING_CLIENT
	}
	else
	{
	    //
	    // Wait until the request has completed, or until the request times out.
	    //
	    Int tout = timeout();
	    IceUtil::Time expireTime;
	    if(tout > 0)
	    {
		expireTime = IceUtil::Time::now() + IceUtil::Time::milliSeconds(tout);
	    }
	    
	    while(out->state() == Outgoing::StateInProgress)
	    {
		if(tout > 0)
		{           
		    IceUtil::Time now = IceUtil::Time::now();
		    if(now < expireTime)
		    {
			_sendMonitor.timedWait(expireTime - now);
		    }
		    
		    //
		    // Make sure we woke up because of timeout and not another response.
		    //
		    if(out->state() == Outgoing::StateInProgress && IceUtil::Time::now() > expireTime)
		    {	
			throw TimeoutException(__FILE__, __LINE__);
		    }
		}
		else
		{
		    _sendMonitor.wait();
		}
	    }
	}
#endif
    }
    catch(const LocalException& ex)
    {
	{
	    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	    setState(StateClosed, ex);
	    assert(_exception.get());
	    if(!requestSent)
	    {
		_exception->ice_throw();
	    }
	}
	
	//
	// If the request was already sent, we don't throw directly
	// but instead we set the Outgoing object exception with
	// finished().  Throwing directly would break "at-most-once"
	// (see also comment in Outgoing.invoke())
	//
	IceUtil::Monitor<IceUtil::Mutex>::Lock sendSync(_sendMonitor);
#ifndef ICEE_PURE_BLOCKING_CLIENT
	if(_blocking)
	{
#endif
	    out->finished(ex);
#ifndef ICEE_PURE_BLOCKING_CLIENT
	}
	else
	{
	    while(out->state() == Outgoing::StateInProgress)
	    {
		_sendMonitor.wait(); // Wait for the thread to propagate the exception to the Outgoing object.
	    }
	}
#endif
    }
}

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
	    _batchStream.writeBlob(requestBatchHdr, sizeof(requestBatchHdr));
	}
	catch(const LocalException& ex)
	{
	    setState(StateClosed, ex);
	    ex.ice_throw();
	}
    }

    _batchStreamInUse = true;
    _batchMarker = _batchStream.b.size();
    _batchStream.swap(*os);

    //
    // The batch stream now belongs to the caller, until
    // finishBatchRequest() or abortBatchRequest() is called.
    //
}

void
Ice::Connection::finishBatchRequest(BasicStream* os)
{
    bool autoflush = false;
    vector<Ice::Byte> lastRequest;

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

        //
        // Get the batch stream back.
        //
        _batchStream.swap(*os);

        if(_batchAutoFlush)
        {
            Lock sendSync(_sendMonitor);

            if(!_transceiver)
            {
                assert(_exception.get());
                _exception->ice_throw(); // The exception is immutable at this point.
            }

            //
            // Throw memory limit exception if the first message added causes us to 
            // go over limit. Otherwise put aside the marshalled message that caused
            // limit to be exceeded and rollback stream to the marker.
            //
            if(_batchStream.b.size() > _instance->messageSizeMax())
            {
                if(_batchRequestNum == 0)
                {
                    resetBatch(true);
                    throwMemoryLimitException(__FILE__, __LINE__);
                }
                vector<Ice::Byte>(_batchStream.b.begin() + _batchMarker, _batchStream.b.end()).swap(lastRequest);
                _batchStream.b.resize(_batchMarker);
                autoflush = true;
            }
        }

        if(!autoflush)
        {
            //
            // Increment the number of requests in the batch.
            //
            ++_batchRequestNum;

            //
            // Notify about the batch stream not being in use anymore.
            //
            assert(_batchStreamInUse);
            _batchStreamInUse = false;
            notifyAll();
        }
    }

    if(autoflush)
    {
        //
        // We have to keep _batchStreamInUse set until after we insert the
        // saved marshalled data into a new stream.
        //
        flushBatchRequestsInternal(true);

        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

        //
        // Throw memory limit exception if the message that caused us to go over
        // limit causes us to exceed the limit by itself.
        //
        if(sizeof(requestBatchHdr) + lastRequest.size() >  _instance->messageSizeMax())
        {
            resetBatch(true);
            throwMemoryLimitException(__FILE__, __LINE__);
        }

        //
        // Start a new batch with the last message that caused us to 
        // go over the limit.
        //
        try
        {
            _batchStream.writeBlob(requestBatchHdr, sizeof(requestBatchHdr));
            _batchStream.writeBlob(&lastRequest[0], lastRequest.size());
        }
        catch(const LocalException& ex)
        {
            setState(StateClosed, ex);
            ex.ice_throw();
        }

        //
        // Notify that the batch stream not in use anymore.
        //
        ++_batchRequestNum;
        _batchStreamInUse = false;
        notifyAll();
    }
}

void
Ice::Connection::abortBatchRequest()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    
    //
    // Reset the batch stream. We cannot save old requests
    // in the batch stream, as they might be corrupted due to
    // incomplete marshaling.
    //
    resetBatch(true);
}

void
Ice::Connection::flushBatchRequests()
{
    flushBatchRequestsInternal(false);
}

void
Ice::Connection::flushBatchRequestsInternal(bool ignoreInUse)
{
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

        if(!ignoreInUse)
        {
	    while(_batchStreamInUse && !_exception.get())
	    {
	        wait();
	    }
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
	Lock sendSync(_sendMonitor);

	if(!_transceiver) // Has the transceiver already been closed?
	{
	    assert(_exception.get());
	    _exception->ice_throw(); // The exception is immutable at this point.
	}

	//
	// Fill in the number of requests in the batch.
	//
	Byte* dest = &(_batchStream.b[0]) + headerSize;
#ifdef ICE_BIG_ENDIAN
	const Byte* src = reinterpret_cast<const Byte*>(&_batchRequestNum) + sizeof(Ice::Int) - 1;
	*dest++ = *src--;
	*dest++ = *src--;
	*dest++ = *src--;
	*dest = *src;
#else
	const Byte* src = reinterpret_cast<const Byte*>(&_batchRequestNum);
	*dest++ = *src++;
	*dest++ = *src++;
	*dest++ = *src++;
	*dest = *src;
#endif
	
	const Int sz = static_cast<Int>(_batchStream.b.size());
	dest = &(_batchStream.b[0]) + 10;
#ifdef ICE_BIG_ENDIAN
	src = reinterpret_cast<const Byte*>(&sz) + sizeof(Ice::Int) - 1;
	*dest++ = *src--;
	*dest++ = *src--;
	*dest++ = *src--;
	*dest = *src;
#else
	src = reinterpret_cast<const Byte*>(&sz);
	*dest++ = *src++;
	*dest++ = *src++;
	*dest++ = *src++;
	*dest = *src;
#endif
	    
	//
	// Send the batch request.
	//
	_batchStream.i = _batchStream.b.begin();
	if(_traceLevels->protocol >= 1)
	{
	    traceBatchRequest("sending batch request", _batchStream, _logger, _traceLevels);
	}
	_transceiver->write(_batchStream);
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
        resetBatch(!ignoreInUse);
    }
}

void
Ice::Connection::resetBatch(bool resetInUse)
{
    BasicStream dummy(_instance.get(), _instance->messageSizeMax(),
#ifdef ICEE_HAS_WSTRING
        _instance->initializationData().stringConverter, _instance->initializationData().wstringConverter,
#endif
        _batchAutoFlush);
    _batchStream.swap(dummy);
    _batchRequestNum = 0;
    _batchMarker = 0;

    //
    // Notify about the batch stream not being in use
    // anymore.
    //
    if(resetInUse)
    {
        assert(_batchStreamInUse);
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
	Lock sendSync(_sendMonitor);

	if(!_transceiver) // Has the transceiver already been closed?
	{
	    assert(_exception.get());
	    _exception->ice_throw(); // The exception is immutable at this point.
	}


	const Int sz = static_cast<Int>(os->b.size());
	Byte* dest = &(os->b[0]) + 10;
#ifdef ICE_BIG_ENDIAN
	const Byte* src = reinterpret_cast<const Byte*>(&sz) + sizeof(Ice::Int) - 1;
	*dest++ = *src--;
	*dest++ = *src--;
	*dest++ = *src--;
	*dest = *src;
#else
	const Byte* src = reinterpret_cast<const Byte*>(&sz);
	*dest++ = *src++;
	*dest++ = *src++;
	*dest++ = *src++;
	*dest = *src;
#endif
	    
	//
	// Send the reply.
	//
	os->i = os->b.begin();
	if(_traceLevels->protocol >= 1)
	{
	    traceReply("sending reply", *os, _logger, _traceLevels);
	}
	_transceiver->write(*os);
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
	    assert(_dispatchCount > 0);
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
	assert(_dispatchCount > 0);
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

#ifndef ICEE_PURE_CLIENT

void
Ice::Connection::setAdapter(const ObjectAdapterPtr& adapter)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(_blocking)
    {
        throw FeatureNotSupportedException(__FILE__, __LINE__, "setAdapter with blocking connection");
    }

    //
    // Wait for all the incoming to be dispatched. We can't modify the
    // _adapter and _servantManager if there's incoming because the
    // Incoming object is using plain pointers for these objects.
    //
    while(_dispatchCount > 0)
    {
	wait();
    }

    if(_exception.get())
    {
	_exception->ice_throw();
    }
    
    assert(_state < StateClosing);

    _in.setAdapter(adapter);
}

ObjectAdapterPtr
Ice::Connection::getAdapter() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    return _in.getAdapter();
}

ObjectPrx
Ice::Connection::createProxy(const Identity& ident) const
{
    //
    // Create a reference and return a reverse proxy for this
    // reference.
    //
    vector<ConnectionPtr> connections;
    connections.push_back(const_cast<Connection*>(this));
    ReferencePtr ref = _instance->referenceFactory()->create(ident, Ice::Context(), "", ReferenceModeTwoway, 
							     connections);
    return _instance->proxyFactory()->referenceToProxy(ref);
}

#endif

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
	_logger(_instance->initializationData().logger), // Cached for better performance.
	_traceLevels(_instance->traceLevels()), // Cached for better performance.
	_warn(_instance->initializationData().properties->getPropertyAsInt("Ice.Warn.Connections") > 0),
#ifndef ICEE_PURE_CLIENT
	_in(_instance.get(), this, _stream, adapter),
#endif
#ifndef ICEE_PURE_BLOCKING_CLIENT
	_stream(_instance.get(), _instance->messageSizeMax()
#ifdef ICEE_HAS_WSTRING
                , _instance->initializationData().stringConverter, _instance->initializationData().wstringConverter
#endif
                ),
#endif
#ifdef ICEE_HAS_BATCH
        _batchAutoFlush(
            _instance->initializationData().properties->getPropertyAsIntWithDefault("Ice.BatchAutoFlush", 1) > 0),
	_batchStream(_instance.get(), _instance->messageSizeMax(),
#ifdef ICEE_HAS_WSTRING
                     _instance->initializationData().stringConverter, _instance->initializationData().wstringConverter,
#endif
                     _batchAutoFlush),
	_batchStreamInUse(false),
	_batchRequestNum(0),
        _batchMarker(0),
#endif
	_dispatchCount(0),
	_state(StateNotValidated),
	_stateTime(IceUtil::Time::now()),
	_nextRequestId(1)
#ifndef ICEE_PURE_BLOCKING_CLIENT
      , _requestsHint(_requests.end())
#endif
{
#ifndef ICEE_PURE_BLOCKING_CLIENT
#  ifdef ICEE_PURE_CLIENT
    _blocking = _instance->initializationData().properties->getPropertyAsInt("Ice.Blocking") > 0;
#  else
    _blocking = _instance->initializationData().properties->getPropertyAsInt("Ice.Blocking") > 0 && !adapter;
#  endif
    if(_blocking)
    {
	_transceiver->setTimeouts(_endpoint->timeout(), _endpoint->timeout());
    }
    else
    {
#ifdef _WIN32
	//
	// On Windows, the recv() call doesn't return if the socket is
	// shutdown. We use the timeout to not block indefinitely.
	// 
	_transceiver->setTimeouts(_endpoint->timeout(), _endpoint->timeout());
#else
	_transceiver->setTimeouts(-1, _endpoint->timeout());
#endif
    }
#else
    _transceiver->setTimeouts(_endpoint->timeout(), _endpoint->timeout());
#endif

#ifdef ICEE_PURE_BLOCKING_CLIENT
    validate();
#else
    if(_blocking)
    {
    	validate();
    }
    else
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
    assert(_dispatchCount == 0);
#ifndef ICEE_PURE_BLOCKING_CLIENT
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
	    
	//
	// The connection might already be closed (e.g.: the communicator 
	// was destroyed or object adapter deactivated.)
	//
	assert(_state == StateNotValidated || _state == StateClosed);
	if(_state == StateClosed)
	{
	    assert(_exception.get());
	    _exception->ice_throw();
	}
        
        if(_in.getAdapter())
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
    	    BasicStream os(_instance.get(), _instance->messageSizeMax()
#ifdef ICEE_HAS_WSTRING
                           , _instance->initializationData().stringConverter,
                           _instance->initializationData().wstringConverter
#endif
                          );
	    os.write(magic[0]);
	    os.write(magic[1]);
	    os.write(magic[2]);
	    os.write(magic[3]);
    	    os.write(protocolMajor);
    	    os.write(protocolMinor);
    	    os.write(encodingMajor);
    	    os.write(encodingMinor);
    	    os.write(validateConnectionMsg);
    	    os.write(static_cast<Byte>(0)); // Compression status (always zero for validate connection).
    	    os.write(headerSize); // Message size.
    	    os.i = os.b.begin();
	    if(_traceLevels->protocol >= 1)
	    {
		traceHeader("sending validate connection", os, _logger, _traceLevels);
	    }
    	    try
    	    {
    	        _transceiver->writeWithTimeout(os, timeout);
    	    }
    	    catch(const TimeoutException&)
    	    {
    	        throw ConnectTimeoutException(__FILE__, __LINE__);
    	    }
        }
        else
#endif
        {
    	    BasicStream is(_instance.get(), _instance->messageSizeMax()
#ifdef ICEE_HAS_WSTRING
                           , _instance->initializationData().stringConverter,
                           _instance->initializationData().wstringConverter
#endif
                          );
    	    is.b.resize(headerSize);
    	    is.i = is.b.begin();
    	    try
    	    {
    	        _transceiver->readWithTimeout(is, timeout);
    	    }
    	    catch(const TimeoutException&)
    	    {
    	        throw ConnectTimeoutException(__FILE__, __LINE__);
    	    }
    	    assert(is.i == is.b.end());
    	    is.i = is.b.begin();
	    Ice::Byte m[4];
	    is.read(m[0]);
	    is.read(m[1]);
	    is.read(m[2]);
	    is.read(m[3]);
	    if(m[0] != magic[0] || m[1] != magic[1] || m[2] != magic[2] || m[3] != magic[3])
	    {
		throwBadMagicException(__FILE__, __LINE__, Ice::ByteSeq(&m[0], &m[0] + sizeof(m)));
	    }
    	    Byte pMajor;
    	    Byte pMinor;
    	    is.read(pMajor);
    	    is.read(pMinor);
    	    if(pMajor != protocolMajor)
    	    {
    	        throwUnsupportedProtocolException(__FILE__, __LINE__, pMajor, pMinor, protocolMajor, protocolMinor);
    	    }
    	    Byte eMajor;
    	    Byte eMinor;
    	    is.read(eMajor);
    	    is.read(eMinor);
    	    if(eMajor != encodingMajor)
    	    {
		throwUnsupportedEncodingException(__FILE__, __LINE__, eMajor, eMinor, encodingMajor, encodingMinor);
    	    }
    	    Byte messageType;
    	    is.read(messageType);
    	    if(messageType != validateConnectionMsg)
    	    {
    	        throwConnectionNotValidatedException(__FILE__, __LINE__);
    	    }
            Byte compress;
            is.read(compress); // Ignore compression status for validate connection.
    	    Int size;
	    is.read(size);
	    if(size != headerSize)
	    {
	        throwIllegalMessageSizeException(__FILE__, __LINE__);
	    }
	    if(_traceLevels->protocol >= 1)
	    {
		traceHeader("received validate connection", is, _logger, _traceLevels);
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

#ifdef ICEE_PURE_CLIENT
    activate();
#else
    hold();
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
	// We shutdown both for reading and writing. This will
	// unblock and read call with an exception. The thread
	// per connection then closes the transceiver.
	//
	_transceiver->shutdownReadWrite();

	//
 	// In blocking mode, we close the transceiver now.
	//
#ifndef ICEE_PURE_BLOCKING_CLIENT
	if(_blocking)
#endif
	{
	    Lock sync(_sendMonitor);
	    try
	    {
		_transceiver->close();
	    }
	    catch(const Ice::LocalException&)
	    {
	    }
	    _transceiver = 0;
	}
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

#ifndef ICEE_PURE_BLOCKING_CLIENT
	    if(_blocking)
#endif
	    {
		setState(StateClosed);
	    }
	}
	catch(const LocalException& ex)
	{
	    setState(StateClosed, ex);
	}
    }
}

void
Ice::Connection::initiateShutdown() const
{
    assert(_state == StateClosing);
    assert(_dispatchCount == 0);

    Lock sendSync(_sendMonitor);

    //
    // Before we shut down, we send a close connection message.
    //
    BasicStream os(_instance.get(), _instance->messageSizeMax()
#ifdef ICEE_HAS_WSTRING
                   , _instance->initializationData().stringConverter, _instance->initializationData().wstringConverter
#endif
                  );

    os.write(magic[0]);
    os.write(magic[1]);
    os.write(magic[2]);
    os.write(magic[3]);
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
    if(_traceLevels->protocol >= 1)
    {
	traceHeader("sending close connection", os, _logger, _traceLevels);
    }
    _transceiver->write(os);

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
#ifndef ICEE_PURE_CLIENT
Ice::Connection::readStreamAndParseMessage(IceInternal::BasicStream& stream, Int& requestId, Int& invokeNum)
#else
Ice::Connection::readStreamAndParseMessage(IceInternal::BasicStream& stream, Int& requestId)
#endif
{
    //
    // Read the header.
    //
    stream.b.resize(headerSize);
    stream.i = stream.b.begin();
    _transceiver->read(stream);
    
    ptrdiff_t pos = stream.i - stream.b.begin();
    assert(pos >= headerSize);
    stream.i = stream.b.begin();
    const Ice::Byte* header;
    stream.readBlob(header, headerSize);
    if(header[0] != magic[0] || header[1] != magic[1] || header[2] != magic[2] || header[3] != magic[3])
    {
	throwBadMagicException(__FILE__, __LINE__, Ice::ByteSeq(&header[0], &header[0] + sizeof(magic)));
    }
    if(header[4] != protocolMajor)
    {
	throwUnsupportedProtocolException(__FILE__, __LINE__, header[4], header[5], protocolMajor, protocolMinor);
    }
    if(header[6] != encodingMajor)
    {
	throwUnsupportedEncodingException(__FILE__, __LINE__, header[6], header[7], encodingMajor, encodingMinor);
    }
    const Byte messageType = header[8];
    if(header[9] == 2)
    {
	throw FeatureNotSupportedException(__FILE__, __LINE__, "compression");
    }
    
    Int size;
    stream.i -= sizeof(Int);
    stream.read(size);
    if(size < headerSize)
    {
	throwIllegalMessageSizeException(__FILE__, __LINE__);
    }
    if(size > static_cast<Int>(_instance->messageSizeMax()))
    {
	throwMemoryLimitException(__FILE__, __LINE__);
    }
    if(size > static_cast<Int>(stream.b.size()))
    {
	stream.b.resize(size);
    }
    stream.i = stream.b.begin() + pos;
    
    if(stream.i != stream.b.end())
    {
	_transceiver->read(stream);
    }

    assert(stream.i == stream.b.end());
    stream.i = stream.b.begin() + headerSize;
    
    switch(messageType)
    {
    case closeConnectionMsg:
    {
	if(_traceLevels->protocol >= 1)
	{
	    traceHeader("received close connection", stream, _logger, _traceLevels);
	}
	throw CloseConnectionException(__FILE__, __LINE__);
	break;
    }
	
    case replyMsg:
    {
	if(_traceLevels->protocol >= 1)
	{
	    traceReply("received reply", stream, _logger, _traceLevels);
	}
	stream.read(requestId);
	break;
    }			
	
#ifndef ICEE_PURE_CLIENT
    case requestMsg:
    {
	if(_traceLevels->protocol >= 1)
	{
	    traceRequest("received request", stream, _logger, _traceLevels);
	}
	stream.read(requestId);
	invokeNum = 1;
	break;
    }
	
    case requestBatchMsg:
    {
	if(_traceLevels->protocol >= 1)
	{
	    traceBatchRequest("received batch request", stream, _logger, _traceLevels);
	}
	stream.read(invokeNum);
	if(invokeNum < 0)
	{
	    invokeNum = 0;
	    throwNegativeSizeException(__FILE__, __LINE__);
	}
	break;
    }
#endif
	
    case validateConnectionMsg:
    {
	if(_traceLevels->protocol >= 1)
	{
	    traceHeader("received validate connection", stream, _logger, _traceLevels);
	}
	if(_warn)
	{
	    Warning out(_logger);
	    out << "ignoring unexpected validate connection message:\n" << _desc;
	}
	break;
    }
	
    default:
    {
	if(_traceLevels->protocol >= 1)
	{
	    traceHeader("received unknown message\n(invalid, closing connection)", stream, _logger, _traceLevels);
	}
	throwUnknownMessageException(__FILE__, __LINE__);
	break;
    }
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
	Lock sync(*this);
        assert(_state == StateClosed);

	Lock sendSync(_sendMonitor);
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

    activate();

    bool closed = false;

    while(!closed)
    {
	Int requestId = 0;
#ifndef ICEE_PURE_CLIENT
	Int invokeNum = 0;
	_in.os()->reset();
#endif
	_stream.reset();

	//
	// Read and parse the next message. We don't need to lock the
	// send monitor here as we have the guarantee that
	// _transceiver won't be set to 0 by another thread, the
	// thread per connection is the only thread that can set
	// _transceiver to 0.
	//
	try
	{
#ifndef ICEE_PURE_CLIENT
	    readStreamAndParseMessage(_stream, requestId, invokeNum);
#else
	    readStreamAndParseMessage(_stream, requestId);
#endif
	}
#ifdef _WIN32
	catch(const Ice::TimeoutException&)
	{
	    //
	    // See the comment in the Connection constructor. This is
	    // necessary to not block in recv() indefinitely.
	    //
	    continue;
	}
#endif
	catch(const Ice::LocalException& ex)
	{
	    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	    setState(StateClosed, ex);
	}

	{
	    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	    if(_state != StateClosed)
	    {
#ifndef ICEE_PURE_CLIENT
		if(invokeNum > 0) // We received a request or a batch request
		{
		    if(_state == StateClosing)
		    {
			if(_traceLevels->protocol >= 1)
			{
			    string req = invokeNum > 1 ? "received batch request" : "received request";
			    req += " during closing\n(ignored by server, client will retry)";
			    traceRequest( req.c_str(), _stream, _logger, _traceLevels);
			}
			invokeNum = 0;
		    }
		    _dispatchCount += invokeNum;
		}
		else 
#endif
		    if(requestId > 0)
		{
		    //
		    // The message is a reply, we search the Outgoing object waiting
		    // for this reply and pass it the stream before to notify the
		    // send monitor to wake up threads waiting for replies.
		    //
		    try
		    {
			Lock sync(_sendMonitor);
			
			map<Int, Outgoing*>::iterator p = _requests.end();
			if(p != _requestsHint)
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
			    throwUnknownRequestIdException(__FILE__, __LINE__);
			}
			
			p->second->finished(_stream);
			
			if(p == _requestsHint)
			{
			    _requests.erase(p++);
			    _requestsHint = p;
			}
			else
			{
			    _requests.erase(p);
			}
			_sendMonitor.notifyAll(); // Wake up threads waiting in sendRequest()
		    }
		    catch(const Ice::LocalException& ex)
		    {
			setState(StateClosed, ex);
		    }
		}
	    }

#ifndef ICEE_PURE_CLIENT
	    while(_state == StateHolding)
	    {
		wait();
	    }
#endif

	    if(_state == StateClosed)
	    {
		Lock sync(_sendMonitor);
		try
		{
		    _transceiver->close();
		}
		catch(const LocalException&)
		{
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
		Lock sync(_sendMonitor);
		assert(_exception.get());
		for(map<Int, Outgoing*>::iterator p = _requests.begin(); p != _requests.end(); ++p)
		{
		    p->second->finished(*_exception.get()); // The exception is immutable at this point.
		}
		_requests.clear();
		_sendMonitor.notifyAll(); // Wake up threads waiting in sendRequest()
	    }
	}

	//
	// Method invocation (or multiple invocations for batch
	// messages) must be done outside the thread synchronization,
	// so that nested calls are possible.
	//
#ifndef ICEE_PURE_CLIENT
	try
	{
	    for(;invokeNum > 0; --invokeNum)
	    {
		//
		// Prepare the response if necessary.
		//
		const bool response = requestId != 0;
		if(response)
		{
		    assert(invokeNum == 1); // No further invocations if a response is expected.

		    //
		    // Add the reply header and request id.
		    //
		    BasicStream* os = _in.os();
		    os->writeBlob(replyHdr, sizeof(replyHdr));		
		    os->write(requestId);
		}
	    
		//
		// Dispatch the incoming request.
		//
		_in.invoke(response, requestId);
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
#endif
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
	Error out(_connection->_logger);
	out << "exception in thread per connection:\n" << _connection->toString() << ex.toString(); 
    }
    catch(const std::exception& ex)
    {
	Error out(_connection->_logger);
	out << "std::exception in thread per connection:\n" << _connection->toString() << ex.what();
    }
    catch(...)
    {
	Error out(_connection->_logger);
	out << "unknown exception in thread per connection:\n" << _connection->toString();
    }

    _connection = 0; // Resolve cyclic dependency.
}

#endif // ICEE_PURE_BLOCKING_CLIENT
