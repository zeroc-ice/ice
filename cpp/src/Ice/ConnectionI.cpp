// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/ConnectionI.h>
#include <Ice/Instance.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Properties.h>
#include <Ice/TraceUtil.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/Transceiver.h>
#include <Ice/ThreadPool.h>
#include <Ice/ConnectionMonitor.h>
#include <Ice/ObjectAdapterI.h> // For getThreadPool() and getServantManager().
#include <Ice/Endpoint.h>
#include <Ice/Outgoing.h>
#include <Ice/OutgoingAsync.h>
#include <Ice/Incoming.h>
#include <Ice/LocalException.h>
#include <Ice/Protocol.h>
#include <Ice/ReferenceFactory.h> // For createProxy().
#include <Ice/ProxyFactory.h> // For createProxy().
#include <bzlib.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(ConnectionI* p) { p->__incRef(); }
void IceInternal::decRef(ConnectionI* p) { p->__decRef(); }

void
Ice::ConnectionI::validate()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(_instance->threadPerConnection() &&
       _threadPerConnection->getThreadControl() != IceUtil::ThreadControl())
    {
	//
	// In thread per connection mode, this connection's thread
	// will take care of connection validation. Therefore all we
	// have to do here is to wait until this thread has completed
	// validation.
	//
	while(_state == StateNotValidated)
	{
	    wait();
	}

	if(_state >= StateClosing)
	{
	    assert(_exception.get());
	    _exception->ice_throw();
	}

	return;
    }

    assert(_state == StateNotValidated);
    
    if(!_endpoint->datagram()) // Datagram connections are always implicitly validated.
    {
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
	    
	    if(_adapter)
	    {
		IceUtil::Mutex::Lock sendSync(_sendMutex);

		//
		// Incoming connections play the active role with respect to
		// connection validation.
		//
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
	    {
		//
		// Outgoing connections play the passive role with respect to
		// connection validation.
		//
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
	    setState(StateClosed, ex);
	    assert(_exception.get());
	    _exception->ice_throw();
	}
    }

    if(_acmTimeout > 0)
    {
	_acmAbsoluteTimeout = IceUtil::Time::now() + IceUtil::Time::seconds(_acmTimeout);
    }

    //
    // We start out in holding state.
    //
    setState(StateHolding);
}

void
Ice::ConnectionI::activate()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    setState(StateActive);
}

void
Ice::ConnectionI::hold()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    setState(StateHolding);
}

void
Ice::ConnectionI::destroy(DestructionReason reason)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    switch(reason)
    {
	case ObjectAdapterDeactivated:
	{
	    setState(StateClosing, ObjectAdapterDeactivatedException(__FILE__, __LINE__));
	    break;
	}

	case CommunicatorDestroyed:
	{
	    setState(StateClosing, CommunicatorDestroyedException(__FILE__, __LINE__));
	    break;
	}
    }
}

void
Ice::ConnectionI::close(bool force)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(force)
    {
	setState(StateClosed, ForcedCloseConnectionException(__FILE__, __LINE__));
    }
    else
    {
	setState(StateClosing, CloseConnectionException(__FILE__, __LINE__));
    }
}

bool
Ice::ConnectionI::isDestroyed() const
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
Ice::ConnectionI::isFinished() const
{
    IceUtil::ThreadPtr threadPerConnection;

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

	if(_transceiver != 0 || _dispatchCount != 0 ||
	   (_threadPerConnection &&
	    _threadPerConnection->getThreadControl() != IceUtil::ThreadControl() &&
	    _threadPerConnection->getThreadControl().isAlive()))
	{
	    return false;
	}

	assert(_state == StateClosed);

	threadPerConnection = _threadPerConnection;
	_threadPerConnection = 0;
    }

    if(threadPerConnection && threadPerConnection->getThreadControl() != IceUtil::ThreadControl())
    {
	threadPerConnection->getThreadControl().join();
    }

    return true;
}

void
Ice::ConnectionI::waitUntilHolding() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    while(_state < StateHolding || _dispatchCount > 0)
    {
	wait();
    }
}

void
Ice::ConnectionI::waitUntilFinished()
{
    IceUtil::ThreadPtr threadPerConnection;

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

	threadPerConnection = _threadPerConnection;
	_threadPerConnection = 0;
    }

    if(threadPerConnection && threadPerConnection->getThreadControl() != IceUtil::ThreadControl())
    {
	threadPerConnection->getThreadControl().join();
    }
}

void
Ice::ConnectionI::monitor()
{
    IceUtil::Monitor<IceUtil::Mutex>::TryLock sync(*this);
    
    if(!sync.acquired())
    {
	return;
    }
    
    if(_state != StateActive)
    {
	return;
    }
    
    //
    // Check for timed out async requests.
    //
    for(map<Int, AsyncRequest>::iterator p = _asyncRequests.begin(); p != _asyncRequests.end(); ++p)
    {
	if(p->second.t > IceUtil::Time() && p->second.t <= IceUtil::Time::now())
	{
	    setState(StateClosed, TimeoutException(__FILE__, __LINE__));
	    return;
	}
    }
    
    //
    // Active connection management for idle connections.
    //
    if(_acmTimeout > 0 &&
       _requests.empty() && _asyncRequests.empty() &&
       !_batchStreamInUse && _batchStream.b.empty() &&
       _dispatchCount == 0)
    {
	if(IceUtil::Time::now() >= _acmAbsoluteTimeout)
	{
	    setState(StateClosing, ConnectionTimeoutException(__FILE__, __LINE__));
	    return;
	}
    }
}

//
// TODO: Should not be a member function of Connection.
//
void
Ice::ConnectionI::prepareRequest(BasicStream* os)
{
    os->writeBlob(_requestHdr);
}

void
Ice::ConnectionI::sendRequest(BasicStream* os, Outgoing* out, bool compress)
{
    Int requestId;

    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

	assert(!(out && _endpoint->datagram())); // Twoway requests cannot be datagrams.
	
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
	    const Byte* p = reinterpret_cast<const Byte*>(&requestId);
#ifdef ICE_BIG_ENDIAN
	    reverse_copy(p, p + sizeof(Int), os->b.begin() + headerSize);
#else
	    copy(p, p + sizeof(Int), os->b.begin() + headerSize);
#endif

	    //
	    // Add to the requests map.
	    //
	    _requestsHint = _requests.insert(_requests.end(), pair<const Int, Outgoing*>(requestId, out));
	}
		
	if(_acmTimeout > 0)
	{
	    _acmAbsoluteTimeout = IceUtil::Time::now() + IceUtil::Time::seconds(_acmTimeout);
	}
    }

    try
    {
	IceUtil::Mutex::Lock sendSync(_sendMutex);

	if(!_transceiver) // Has the transceiver already been closed?
	{
	    assert(_exception.get()); 
	    _exception->ice_throw(); // The exception is immutable at this point.
	}
	
	if(compress && os->b.size() >= 100) // Only compress messages larger than 100 bytes.
	{
	    //
	    // Message compressed. Request compressed response, if any.
	    //
	    os->b[9] = 2;
	    
	    //
	    // Do compression.
	    //
	    BasicStream cstream(_instance.get());
	    doCompress(*os, cstream);
	    
	    //
	    // Send the request.
	    //
	    os->i = os->b.begin();
	    traceRequest("sending request", *os, _logger, _traceLevels);
	    cstream.i = cstream.b.begin();
	    _transceiver->write(cstream, _endpoint->timeout());
	}
	else
	{
	    if(compress)
	    {
		//
		// Message not compressed. Request compressed response, if any.
		//
		os->b[9] = 1;
	    }

	    //
	    // No compression, just fill in the message size.
	    //
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

void
Ice::ConnectionI::sendAsyncRequest(BasicStream* os, const OutgoingAsyncPtr& out, bool compress)
{
    Int requestId;

    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

	assert(!_endpoint->datagram()); // Twoway requests cannot be datagrams, and async implies twoway.

	if(_exception.get())
	{
	    _exception->ice_throw();
	}

	assert(_state > StateNotValidated);
	assert(_state < StateClosing);
	
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
	const Byte* p = reinterpret_cast<const Byte*>(&requestId);
#ifdef ICE_BIG_ENDIAN
	reverse_copy(p, p + sizeof(Int), os->b.begin() + headerSize);
#else
	copy(p, p + sizeof(Int), os->b.begin() + headerSize);
#endif
	
	//
	// Add to the async requests map.
	//
	struct AsyncRequest asyncRequest;
	asyncRequest.p = out;
	if(_endpoint->timeout() > 0)
	{
	    asyncRequest.t = IceUtil::Time::now() + IceUtil::Time::milliSeconds(_endpoint->timeout());
	}
	_asyncRequestsHint = _asyncRequests.insert(_asyncRequests.end(),
						   pair<const Int, AsyncRequest>(requestId, asyncRequest));
	
	if(_acmTimeout > 0)
	{
	    _acmAbsoluteTimeout = IceUtil::Time::now() + IceUtil::Time::seconds(_acmTimeout);
	}
    }

    try
    {
	IceUtil::Mutex::Lock sendSync(_sendMutex);

	if(!_transceiver) // Has the transceiver already been closed?
	{
	    assert(_exception.get());
	    _exception->ice_throw(); // The exception is immutable at this point.
	}

	if(compress && os->b.size() >= 100) // Only compress messages larger than 100 bytes.
	{
	    //
	    // Message compressed. Request compressed response, if any.
	    //
	    os->b[9] = 2;
	    
	    //
	    // Do compression.
	    //
	    BasicStream cstream(_instance.get());
	    doCompress(*os, cstream);
	    
	    //
	    // Send the request.
	    //
	    os->i = os->b.begin();
	    traceRequest("sending asynchronous request", *os, _logger, _traceLevels);
	    cstream.i = cstream.b.begin();
	    _transceiver->write(cstream, _endpoint->timeout());
	}
	else
	{
	    if(compress)
	    {
		//
		// Message not compressed. Request compressed response, if any.
		//
		os->b[9] = 1;
	    }

	    //
	    // No compression, just fill in the message size.
	    //
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
	    traceRequest("sending asynchronous request", *os, _logger, _traceLevels);
	    _transceiver->write(*os, _endpoint->timeout());
	}
    }
    catch(const LocalException& ex)
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	setState(StateClosed, ex);
	assert(_exception.get());

	//
	// If the request has already been removed from the async
	// request map, we are out of luck. It would mean that
	// finished() has been called already, and therefore the
	// exception has been set using the
	// OutgoingAsync::__finished() callback. In this case, we
	// cannot throw the exception here, because we must not both
	// raise an exception and have OutgoingAsync::__finished()
	// called with an exception. This means that in some rare
	// cases, a request will not be retried even though it
	// could. But I honestly don't know how I could avoid this,
	// without a very elaborate and complex design, which would be
	// bad for performance.
	//
	map<Int, AsyncRequest>::iterator p = _asyncRequests.find(requestId);
	if(p != _asyncRequests.end())
	{
	    if(p == _asyncRequestsHint)
	    {
		_asyncRequests.erase(p++);
		_asyncRequestsHint = p;
	    }
	    else
	    {
		_asyncRequests.erase(p);
	    }

	    _exception->ice_throw();
	}
    }
}

void
Ice::ConnectionI::prepareBatchRequest(BasicStream* os)
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
Ice::ConnectionI::finishBatchRequest(BasicStream* os, bool compress)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    //
    // Get the batch stream back and increment the number of requests
    // in the batch.
    //
    _batchStream.swap(*os);
    ++_batchRequestNum;

    //
    // We compress the whole batch if there is at least one compressed
    // message.
    //
    if(compress)
    {
	_batchRequestCompress = true;
    }
    
    //
    // Notify about the batch stream not being in use anymore.
    //
    assert(_batchStreamInUse);
    _batchStreamInUse = false;
    notifyAll();
}

void
Ice::ConnectionI::abortBatchRequest()
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
    _batchRequestCompress = false;

    //
    // Notify about the batch stream not being in use
    // anymore.
    //
    assert(_batchStreamInUse);
    _batchStreamInUse = false;
    notifyAll();
}

void
Ice::ConnectionI::flushBatchRequests()
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

	if(_acmTimeout > 0)
	{
	    _acmAbsoluteTimeout = IceUtil::Time::now() + IceUtil::Time::seconds(_acmTimeout);
	}

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
	
	if(_batchRequestCompress && _batchStream.b.size() >= 100) // Only compress messages larger than 100 bytes.
	{
	    //
	    // Message compressed. Request compressed response, if any.
	    //
	    _batchStream.b[9] = 2;
	    
	    //
	    // Do compression.
	    //
	    BasicStream cstream(_instance.get());
	    doCompress(_batchStream, cstream);
	    
	    //
	    // Send the batch request.
	    //
	    _batchStream.i = _batchStream.b.begin();
	    traceBatchRequest("sending batch request", _batchStream, _logger, _traceLevels);
	    cstream.i = cstream.b.begin();
	    _transceiver->write(cstream, _endpoint->timeout());
	}
	else
	{
	    if(_batchRequestCompress)
	    {
		//
		// Message not compressed. Request compressed response, if any.
		//
		_batchStream.b[9] = 1;
	    }

	    //
	    // No compression, just fill in the message size.
	    //
	    Int sz = static_cast<Int>(_batchStream.b.size());
	    const Byte* p = reinterpret_cast<const Byte*>(&sz);
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
    }
    catch(const LocalException& ex)
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	setState(StateClosed, ex);
	assert(_exception.get());

	//
	// Since batch requests are all oneways (or datagrams), we
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
	_batchRequestCompress = false;
	_batchStreamInUse = false;
	notifyAll();
    }
}

void
Ice::ConnectionI::sendResponse(BasicStream* os, Byte compressFlag)
{
    try
    {
	IceUtil::Mutex::Lock sendSync(_sendMutex);

	if(!_transceiver) // Has the transceiver already been closed?
	{
	    assert(_exception.get());
	    _exception->ice_throw(); // The exception is immutable at this point.
	}

	//
	// Only compress if compression was requested by the client,
	// and if the message is larger than 100 bytes.
	//
	if(compressFlag > 0 && os->b.size() >= 100)
	{
	    //
	    // Message compressed. Request compressed response, if any.
	    //
	    os->b[9] = 2;

	    //
	    // Do compression.
	    //
	    BasicStream cstream(_instance.get());
	    doCompress(*os, cstream);
	    
	    //
	    // Send the reply.
	    //
	    os->i = os->b.begin();
	    traceReply("sending reply", *os, _logger, _traceLevels);
	    cstream.i = cstream.b.begin();
	    _transceiver->write(cstream, _endpoint->timeout());
	}
	else
	{
	    if(compressFlag > 0)
	    {
		//
		// Message not compressed. Request compressed response, if any.
		//
		os->b[9] = 1;
	    }

	    //
	    // No compression, just fill in the message size.
	    //
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
	    
	    if(_acmTimeout > 0)
	    {
		_acmAbsoluteTimeout = IceUtil::Time::now() + IceUtil::Time::seconds(_acmTimeout);
	    }
	}
	catch(const LocalException& ex)
	{
	    setState(StateClosed, ex);
	}
    }
}

void
Ice::ConnectionI::sendNoResponse()
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

EndpointPtr
Ice::ConnectionI::endpoint() const
{
    return _endpoint; // No mutex protection necessary, _endpoint is immutable.
}

void
Ice::ConnectionI::setAdapter(const ObjectAdapterPtr& adapter)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(_exception.get())
    {
	_exception->ice_throw();
    }
    
    assert(_state < StateClosing);

    //
    // Before we set an adapter (or reset it) we wait until the
    // dispatch count with any old adapter is zero.
    //
    // A deadlock can occur if we wait while an operation is
    // outstanding on this adapter that holds a lock while
    // calling this function (e.g., __getDelegate).
    //
    // In order to avoid such a deadlock, we only wait if the new
    // adapter is different than the current one.
    //
    // TODO: Verify that this fix solves all cases.
    //
    if(_adapter.get() != adapter.get())
    {
	while(_dispatchCount > 0)
	{
	    wait();
	}

	//
	// We never change the thread pool with which we were initially
	// registered, even if we add or remove an object adapter.
	//

	_adapter = adapter;
	if(_adapter)
	{
	    _servantManager = dynamic_cast<ObjectAdapterI*>(_adapter.get())->getServantManager();
	}
	else
	{
	    _servantManager = 0;
	}
    }
}

ObjectAdapterPtr
Ice::ConnectionI::getAdapter() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    return _adapter;
}

ObjectPrx
Ice::ConnectionI::createProxy(const Identity& ident) const
{
    //
    // Create a reference and return a reverse proxy for this
    // reference.
    //
    vector<ConnectionIPtr> connections;
    connections.push_back(const_cast<ConnectionI*>(this));
    ReferencePtr ref = _instance->referenceFactory()->create(ident, Context(), "", Reference::ModeTwoway, connections);
    return _instance->proxyFactory()->referenceToProxy(ref);
}

bool
Ice::ConnectionI::datagram() const
{
    assert(!_instance->threadPerConnection()); // Only for use with a thread pool.
    return _endpoint->datagram(); // No mutex protection necessary, _endpoint is immutable.
}

bool
Ice::ConnectionI::readable() const
{
    assert(!_instance->threadPerConnection()); // Only for use with a thread pool.
    return true;
}

void
Ice::ConnectionI::read(BasicStream& stream)
{
    assert(!_instance->threadPerConnection()); // Only for use with a thread pool.

    _transceiver->read(stream, 0);

    //
    // Updating _acmAbsoluteTimeout is too expensive here, because we
    // would have to acquire a lock just for this purpose. Instead, we
    // update _acmAbsoluteTimeout in message().
    //
}

void
Ice::ConnectionI::message(BasicStream& stream, const ThreadPoolPtr& threadPool)
{
    assert(!_instance->threadPerConnection()); // Only for use with a thread pool.

    Byte compress = 0;
    Int requestId = 0;
    Int invokeNum = 0;
    ServantManagerPtr servantManager;
    ObjectAdapterPtr adapter;
    OutgoingAsyncPtr outAsync;

    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

	//
	// We must promote within the synchronization, otherwise there
	// could be various race conditions with close connection
	// messages and other messages.
	//
	threadPool->promoteFollower();

	if(_state != StateClosed)
	{
	    parseMessage(stream, invokeNum, requestId, compress, servantManager, adapter, outAsync);
	}

	//
	// parseMessage() can close the connection, so we must check
	// for closed state again.
	//
	if(_state == StateClosed)
	{
	    return;
	}
    }

    //
    // Asynchronous replies must be handled outside the thread
    // synchronization, so that nested calls are possible.
    //
    if(outAsync)
    {
	outAsync->__finished(stream);
    }

    //
    // Method invocation (or multiple invocations for batch messages)
    // must be done outside the thread synchronization, so that nested
    // calls are possible.
    //
    invokeAll(stream, invokeNum, requestId, compress, servantManager, adapter);
}

void
Ice::ConnectionI::finished(const ThreadPoolPtr& threadPool)
{
    assert(!_instance->threadPerConnection()); // Only for use with a thread pool.

    threadPool->promoteFollower();

    auto_ptr<LocalException> exception;
    
    map<Int, Outgoing*> requests;
    map<Int, AsyncRequest> asyncRequests;

    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	
	if(_state == StateActive || _state == StateClosing)
	{
	    registerWithPool();
	}
	else if(_state == StateClosed)
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
		exception = auto_ptr<LocalException>(dynamic_cast<LocalException*>(ex.ice_clone()));
	    }

	    _transceiver = 0;
	    notifyAll();
	}

	if(_state == StateClosed || _state == StateClosing)
	{
	    requests.swap(_requests);
	    _requestsHint = _requests.end();

	    asyncRequests.swap(_asyncRequests);
	    _asyncRequestsHint = _asyncRequests.end();
	}
    }

    for(map<Int, Outgoing*>::iterator p = requests.begin(); p != requests.end(); ++p)
    {
	p->second->finished(*_exception.get()); // The exception is immutable at this point.
    }

    for(map<Int, AsyncRequest>::iterator q = asyncRequests.begin(); q != asyncRequests.end(); ++q)
    {
	q->second.p->__finished(*_exception.get()); // The exception is immutable at this point.
    }

    if(exception.get())
    {
	exception->ice_throw();
    }    
}

void
Ice::ConnectionI::exception(const LocalException& ex)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    setState(StateClosed, ex);
}

string
Ice::ConnectionI::type() const
{
    return _type; // No mutex lock, _type is immutable.
}

Ice::Int
Ice::ConnectionI::timeout() const
{
    return _endpoint->timeout(); // No mutex lock, _endpoint is immutable.
}

string
Ice::ConnectionI::toString() const
{
    return _desc; // No mutex lock, _desc is immutable.
}

Ice::ConnectionI::ConnectionI(const InstancePtr& instance,
			      const TransceiverPtr& transceiver,
			      const EndpointPtr& endpoint,
			      const ObjectAdapterPtr& adapter) :
    EventHandler(instance),
    _transceiver(transceiver),
    _desc(transceiver->toString()),
    _type(transceiver->type()),
    _endpoint(endpoint),
    _adapter(adapter),
    _logger(_instance->logger()), // Cached for better performance.
    _traceLevels(_instance->traceLevels()), // Cached for better performance.
    _registeredWithPool(false),
    _warn(_instance->properties()->getPropertyAsInt("Ice.Warn.Connections") > 0),
    _acmTimeout(_endpoint->datagram() ?	0 : _instance->connectionIdleTime()),
    _requestHdr(headerSize + sizeof(Int), 0),
    _requestBatchHdr(headerSize + sizeof(Int), 0),
    _replyHdr(headerSize, 0),
    _nextRequestId(1),
    _requestsHint(_requests.end()),
    _asyncRequestsHint(_asyncRequests.end()),
    _batchStream(_instance.get()),
    _batchStreamInUse(false),
    _batchRequestNum(0),
    _batchRequestCompress(false),
    _dispatchCount(0),
    _state(StateNotValidated),
    _stateTime(IceUtil::Time::now())
{
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

    ObjectAdapterI* adapterImpl = _adapter ? dynamic_cast<ObjectAdapterI*>(_adapter.get()) : 0;
    if(adapterImpl)
    {
	_servantManager = adapterImpl->getServantManager();
    }

    __setNoDelete(true);
    try
    {
	if(!_instance->threadPerConnection())
	{
	    //
	    // Only set _threadPool if we really need it, i.e., if we are
	    // not in thread per connection mode. Thread pools have lazy
	    // initialization in Instance, and we don't want them to be
	    // created if they are not needed.
	    //
	    if(adapterImpl)
	    {
		const_cast<ThreadPoolPtr&>(_threadPool) = adapterImpl->getThreadPool();
	    }
	    else
	    {
		const_cast<ThreadPoolPtr&>(_threadPool) = _instance->clientThreadPool();
	    }
	}
	else
	{
	    //
	    // If we are in thread per connection mode, create the thread
	    // for this connection.
	    //
	    _threadPerConnection = new ThreadPerConnection(this);
	    _threadPerConnection->start(_instance->threadPerConnectionStackSize());
	}
    }
    catch(const IceUtil::Exception& ex)
    {
	{
	    Error out(_logger);
	    if(_instance->threadPerConnection())
	    {
		out << "cannot create thread for connection:\n" << ex;
	    }
	    else
	    {
		out << "cannot create thread pool for connection:\n" << ex;
	    }
	}
	
	_state = StateClosed;
	
	try
	{
	    _transceiver->close();
	}
	catch(const LocalException&)
	{
	    // Here we ignore any exceptions in close().
	}
	_transceiver = 0;
	_threadPerConnection = 0;
	
	__setNoDelete(false);
	throw;
    }
    __setNoDelete(false);
}

Ice::ConnectionI::~ConnectionI()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    
    assert(_state == StateClosed);
    assert(!_transceiver);
    assert(_dispatchCount == 0);
    assert(!_threadPerConnection);
}

void
Ice::ConnectionI::setState(State state, const LocalException& ex)
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

	_exception = auto_ptr<LocalException>(dynamic_cast<LocalException*>(ex.ice_clone()));

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
		     dynamic_cast<const ConnectionTimeoutException*>(_exception.get()) ||
		     dynamic_cast<const CommunicatorDestroyedException*>(_exception.get()) ||
		     dynamic_cast<const ObjectAdapterDeactivatedException*>(_exception.get()) ||
		     (dynamic_cast<const ConnectionLostException*>(_exception.get()) && _state == StateClosing)))
		{
		    Warning out(_logger);
		    out << "connection exception:\n" << *_exception.get() << '\n' << _desc;
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
Ice::ConnectionI::setState(State state)
{
    //
    // We don't want to send close connection messages if the endpoint
    // only supports oneway transmission from client to server.
    //
    if(_endpoint->datagram() && state == StateClosing)
    {
	state = StateClosed;
    }

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
	    if(_state != StateHolding && _state != StateNotValidated)
	    {
		return;
	    }
	    if(!_instance->threadPerConnection())
	    {
		registerWithPool();
	    }
	    break;
	}
	
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
	    if(!_instance->threadPerConnection())
	    {
		unregisterWithPool();
	    }
	    break;
	}

	case StateClosing:
	{
	    //
	    // Can't change back from closed.
	    //
	    if(_state == StateClosed)
	    {
		return;
	    }
	    if(!_instance->threadPerConnection())
	    {
		registerWithPool(); // We need to continue to read in closing state.
	    }
	    break;
	}
	
	case StateClosed:
	{
	    if(_instance->threadPerConnection())
	    {
		//
		// If we are in thread per connection mode, we
		// shutdown both for reading and writing. This will
		// unblock and read call with an exception. The thread
		// per connection then closes the transceiver.
		//
		_transceiver->shutdownReadWrite();
	    }
	    else if(_state == StateNotValidated)
	    {
		//
		// If we change from not validated we can close right
		// away.
		//
		assert(!_registeredWithPool);
		
		//
		// We must make sure that nobody is sending when we
		// close the transceiver.
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
		//notifyAll(); // We notify already below.
	    }
	    else
	    {
		//
		// Otherwise we first must make sure that we are
		// registered, then we unregister, and let finished()
		// do the close.
		//
		registerWithPool();
		unregisterWithPool();

		//
		// We must prevent any further writes when _state == StateClosed.
		// However, functions such as sendResponse cannot acquire the main
		// mutex in order to check _state. Therefore we shut down the write
		// end of the transceiver, which causes subsequent write attempts
		// to fail with an exception.
		//
		_transceiver->shutdownWrite();
	    }
	    break;
	}
    }

    //
    // We only register with the connection monitor if our new state
    // is StateActive. Otherwise we unregister with the connection
    // monitor, but only if we were registered before, i.e., if our
    // old state was StateActive.
    //
    ConnectionMonitorPtr connectionMonitor = _instance->connectionMonitor();
    if(connectionMonitor)
    {
	if(state == StateActive)
	{
	    connectionMonitor->add(this);
	}
	else if(_state == StateActive)
	{
	    connectionMonitor->remove(this);
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
    }
}

void
Ice::ConnectionI::initiateShutdown() const
{
    assert(_state == StateClosing);
    assert(_dispatchCount == 0);

    if(!_endpoint->datagram())
    {
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
	os.write((Byte)1); // Compression status: compression supported but not used.
	os.write(headerSize); // Message size.

	//
	// Send the message.
	//
	os.i = os.b.begin();
	traceHeader("sending close connection", os, _logger, _traceLevels);
	_transceiver->write(os, _endpoint->timeout());
	//
	// The CloseConnection message should be sufficient. Closing the write
	// end of the socket is probably an artifact of how things were done
	// in IIOP. In fact, shutting down the write end of the socket causes
	// problems on Windows by preventing the peer from using the socket.
	// For example, the peer is no longer able to continue writing a large
	// message after the socket is shutdown.
	//
	//_transceiver->shutdownWrite();
    }
}

void
Ice::ConnectionI::registerWithPool()
{
    assert(!_instance->threadPerConnection()); // Only for use with a thread pool.

    if(!_registeredWithPool)
    {
	_threadPool->_register(_transceiver->fd(), this);
	_registeredWithPool = true;
    }
}

void
Ice::ConnectionI::unregisterWithPool()
{
    assert(!_instance->threadPerConnection()); // Only for use with a thread pool.

    if(_registeredWithPool)
    {
	_threadPool->unregister(_transceiver->fd());
	_registeredWithPool = false;
    }
}

static string
getBZ2Error(int bzError)
{
    if(bzError == BZ_RUN_OK)
    {
	return ": BZ_RUN_OK";
    }
    else if(bzError == BZ_FLUSH_OK)
    {
	return ": BZ_FLUSH_OK";
    }
    else if(bzError == BZ_FINISH_OK)
    {
	return ": BZ_FINISH_OK";
    }
    else if(bzError == BZ_STREAM_END)
    {
	return ": BZ_STREAM_END";
    }
    else if(bzError == BZ_CONFIG_ERROR)
    {
	return ": BZ_CONFIG_ERROR";
    }
    else if(bzError == BZ_SEQUENCE_ERROR)
    {
	return ": BZ_SEQUENCE_ERROR";
    }
    else if(bzError == BZ_PARAM_ERROR)
    {
	return ": BZ_PARAM_ERROR";
    }
    else if(bzError == BZ_MEM_ERROR)
    {
	return ": BZ_MEM_ERROR";
    }
    else if(bzError == BZ_DATA_ERROR)
    {
	return ": BZ_DATA_ERROR";
    }
    else if(bzError == BZ_DATA_ERROR_MAGIC)
    {
	return ": BZ_DATA_ERROR_MAGIC";
    }
    else if(bzError == BZ_IO_ERROR)
    {
	return ": BZ_IO_ERROR";
    }
    else if(bzError == BZ_UNEXPECTED_EOF)
    {
	return ": BZ_UNEXPECTED_EOF";
    }
    else if(bzError == BZ_OUTBUFF_FULL)
    {
	return ": BZ_OUTBUFF_FULL";
    }
    else
    {
	return "";
    }
}

void
Ice::ConnectionI::doCompress(BasicStream& uncompressed, BasicStream& compressed)
{
    const Byte* p;

    //
    // Compress the message body, but not the header.
    //
    unsigned int uncompressedLen = static_cast<unsigned int>(uncompressed.b.size() - headerSize);
    unsigned int compressedLen = static_cast<unsigned int>(uncompressedLen * 1.01 + 600);
    compressed.b.resize(headerSize + sizeof(Int) + compressedLen);
    int bzError = BZ2_bzBuffToBuffCompress(reinterpret_cast<char*>(&compressed.b[0]) + headerSize + sizeof(Int),
					   &compressedLen,
					   reinterpret_cast<char*>(&uncompressed.b[0]) + headerSize,
					   uncompressedLen,
					   1, 0, 0);
    if(bzError != BZ_OK)
    {
	CompressionException ex(__FILE__, __LINE__);
	ex.reason = "BZ2_bzBuffToBuffCompress failed" + getBZ2Error(bzError);
	throw ex;
    }
    compressed.b.resize(headerSize + sizeof(Int) + compressedLen);
    
    //
    // Write the size of the compressed stream into the header of the
    // uncompressed stream. Since the header will be copied, this size
    // will also be in the header of the compressed stream.
    //
    Int compressedSize = static_cast<Int>(compressed.b.size());
    p = reinterpret_cast<const Byte*>(&compressedSize);
#ifdef ICE_BIG_ENDIAN
    reverse_copy(p, p + sizeof(Int), uncompressed.b.begin() + 10);
#else
    copy(p, p + sizeof(Int), uncompressed.b.begin() + 10);
#endif

    //
    // Add the size of the uncompressed stream before the message body
    // of the compressed stream.
    //
    Int uncompressedSize = static_cast<Int>(uncompressed.b.size());
    p = reinterpret_cast<const Byte*>(&uncompressedSize);
#ifdef ICE_BIG_ENDIAN
    reverse_copy(p, p + sizeof(Int), compressed.b.begin() + headerSize);
#else
    copy(p, p + sizeof(Int), compressed.b.begin() + headerSize);
#endif
    
    //
    // Copy the header from the uncompressed stream to the compressed one.
    //
    copy(uncompressed.b.begin(), uncompressed.b.begin() + headerSize, compressed.b.begin());
}

void
Ice::ConnectionI::doUncompress(BasicStream& compressed, BasicStream& uncompressed)
{
    Int uncompressedSize;
    compressed.i = compressed.b.begin() + headerSize;
    compressed.read(uncompressedSize);
    if(uncompressedSize <= headerSize)
    {
	throw IllegalMessageSizeException(__FILE__, __LINE__);
    }

    uncompressed.resize(uncompressedSize);
    unsigned int uncompressedLen = uncompressedSize - headerSize;
    unsigned int compressedLen = static_cast<unsigned int>(compressed.b.size() - headerSize - sizeof(Int));
    int bzError = BZ2_bzBuffToBuffDecompress(reinterpret_cast<char*>(&uncompressed.b[0]) + headerSize,
					     &uncompressedLen,
					     reinterpret_cast<char*>(&compressed.b[0]) + headerSize + sizeof(Int),
					     compressedLen,
					     0, 0);
    if(bzError != BZ_OK)
    {
	CompressionException ex(__FILE__, __LINE__);
	ex.reason = "BZ2_bzBuffToBuffCompress failed" + getBZ2Error(bzError);
	throw ex;
    }

    copy(compressed.b.begin(), compressed.b.begin() + headerSize, uncompressed.b.begin());
}

void
Ice::ConnectionI::parseMessage(BasicStream& stream, Int& invokeNum, Int& requestId, Byte& compress,
			       ServantManagerPtr& servantManager, ObjectAdapterPtr& adapter,
			       OutgoingAsyncPtr& outAsync)
{
    assert(_state > StateNotValidated && _state < StateClosed);
    
    if(_acmTimeout > 0)
    {
	_acmAbsoluteTimeout = IceUtil::Time::now() + IceUtil::Time::seconds(_acmTimeout);
    }
    
    try
    {
	//
	// We don't need to check magic and version here. This has
	// already been done by the ThreadPool or ThreadPerConnection,
	// which provides us with the stream.
	//
	assert(stream.i == stream.b.end());
	stream.i = stream.b.begin() + 8;
	Byte messageType;
	stream.read(messageType);
	stream.read(compress);
	if(compress == 2)
	{
	    BasicStream ustream(_instance.get());
	    doUncompress(stream, ustream);
	    stream.b.swap(ustream.b);
	}
	stream.i = stream.b.begin() + headerSize;
    
	switch(messageType)
	{
	    case closeConnectionMsg:
	    {
		traceHeader("received close connection", stream, _logger, _traceLevels);
		if(_endpoint->datagram() && _warn)
		{
		    Warning out(_logger);
		    out << "ignoring close connection message for datagram connection:\n" << _desc;
		}
		else
		{
		    setState(StateClosed, CloseConnectionException(__FILE__, __LINE__));
		}
		break;
	    }
	
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
	
	    case replyMsg:
	    {
		traceReply("received reply", stream, _logger, _traceLevels);
	    
		stream.read(requestId);
	    
		map<Int, Outgoing*>::iterator p = _requests.end();
		map<Int, AsyncRequest>::iterator q = _asyncRequests.end();
	    
		if(_requestsHint != _requests.end())
		{
		    if(_requestsHint->first == requestId)
		    {
			p = _requestsHint;
		    }
		}
	    
		if(p == _requests.end())
		{
		    if(_asyncRequestsHint != _asyncRequests.end())
		    {
			if(_asyncRequestsHint->first == requestId)
			{
			    q = _asyncRequestsHint;
			}
		    }
		}
	    
		if(p == _requests.end() && q == _asyncRequests.end())
		{
		    p = _requests.find(requestId);
		}
	    
		if(p == _requests.end() && q == _asyncRequests.end())
		{
		    q = _asyncRequests.find(requestId);
		}
	    
		if(p == _requests.end() && q == _asyncRequests.end())
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
		else
		{
		    assert(q != _asyncRequests.end());
		
		    outAsync = q->second.p;
		
		    if(q == _asyncRequestsHint)
		    {
			_asyncRequests.erase(q++);
			_asyncRequestsHint = q;
		    }
		    else
		    {
			_asyncRequests.erase(q);
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
    catch(const LocalException& ex)
    {
	setState(StateClosed, ex);
    }
}

void
Ice::ConnectionI::invokeAll(BasicStream& stream, Int invokeNum, Int requestId, Byte compress,
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
	    bool response = !_endpoint->datagram() && requestId != 0;
	    Incoming in(_instance.get(), this, adapter, response, compress);
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

void
Ice::ConnectionI::run()
{
    try
    {
	//
	// First we must validate and activate this connection. This must
	// be done here, and not in the connection factory. Please see the
	// comments in the connection factory for details.
	//
	validate();
    }
    catch(const LocalException&)
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

	assert(_state == StateClosed);

	//
	// We must make sure that nobody is sending when we close the
	// transceiver.
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
    
    activate();

    const bool warnUdp = _instance->properties()->getPropertyAsInt("Ice.Warn.Datagrams") > 0;

    bool closed = false;

    while(!closed)
    {
	//
	// We must accept new connections outside the thread
	// synchronization, because we use blocking accept.
	//

	BasicStream stream(_instance.get());

	try
	{
	    stream.b.resize(headerSize);
	    stream.i = stream.b.begin();
	    _transceiver->read(stream, -1);
	
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
		if(_endpoint->datagram())
		{
		    if(warnUdp)
		    {
			Warning out(_instance->logger());
			out << "DatagramLimitException: maximum size of " << pos << " exceeded";
		    }
		    throw DatagramLimitException(__FILE__, __LINE__);
		}
		else
		{
		    _transceiver->read(stream, -1);
		    assert(stream.i == stream.b.end());
		}
	    }
	}
	catch(const DatagramLimitException&) // Expected.
	{
	    continue;
	}
	catch(const LocalException& ex)
	{
	    exception(ex);
	}

	Byte compress = 0;
	Int requestId = 0;
	Int invokeNum = 0;
	ServantManagerPtr servantManager;
	ObjectAdapterPtr adapter;
	OutgoingAsyncPtr outAsync;
	
	auto_ptr<LocalException> exception;
	
	map<Int, Outgoing*> requests;
	map<Int, AsyncRequest> asyncRequests;

	{
	    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

	    while(_state == StateHolding)
	    {
		wait();
	    }
	    
	    if(_state != StateClosed)
	    {
		parseMessage(stream, invokeNum, requestId, compress, servantManager, adapter, outAsync);
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
		    exception = auto_ptr<LocalException>(dynamic_cast<LocalException*>(ex.ice_clone()));
		}
		
		_transceiver = 0;
		notifyAll();

		//
		// We cannot simply return here. We have to make sure
		// that all requests (regular and async) are notified
		// about the closed connection below.
		//
		closed = true;
	    }

	    if(_state == StateClosed || _state == StateClosing)
	    {
		requests.swap(_requests);
		_requestsHint = _requests.end();
		
		asyncRequests.swap(_asyncRequests);
		_asyncRequestsHint = _asyncRequests.end();
	    }
	}

	//
	// Asynchronous replies must be handled outside the thread
	// synchronization, so that nested calls are possible.
	//
	if(outAsync)
	{
	    outAsync->__finished(stream);
	}
	
	//
	// Method invocation (or multiple invocations for batch messages)
	// must be done outside the thread synchronization, so that nested
	// calls are possible.
	//
	invokeAll(stream, invokeNum, requestId, compress, servantManager, adapter);

	for(map<Int, Outgoing*>::iterator p = requests.begin(); p != requests.end(); ++p)
	{
	    p->second->finished(*_exception.get()); // The exception is immutable at this point.
	}
	
	for(map<Int, AsyncRequest>::iterator q = asyncRequests.begin(); q != asyncRequests.end(); ++q)
	{
	    q->second.p->__finished(*_exception.get()); // The exception is immutable at this point.
	}

	if(exception.get())
	{
	    assert(closed);
	    exception->ice_throw();
	}    
    }
}

Ice::ConnectionI::ThreadPerConnection::ThreadPerConnection(const ConnectionIPtr& connection) :
    _connection(connection)
{
}

void
Ice::ConnectionI::ThreadPerConnection::run()
{
    try
    {
	_connection->run();
    }
    catch(const Exception& ex)
    {	
	Error out(_connection->_instance->logger());
	out << "exception in thread per connection:\n" << _connection->toString() << ex; 
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
