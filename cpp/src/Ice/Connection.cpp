// **********************************************************************
//
// Copyright (c) 2003
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

#include <Ice/Connection.h>
#include <Ice/Instance.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Properties.h>
#include <Ice/TraceUtil.h>
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
#include <bzlib.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(Connection* p) { p->__incRef(); }
void IceInternal::decRef(Connection* p) { p->__decRef(); }

void
IceInternal::Connection::activate()
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
    setState(StateActive);
}

void
IceInternal::Connection::hold()
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
    setState(StateHolding);
}

void
IceInternal::Connection::destroy(DestructionReason reason)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

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

bool
IceInternal::Connection::isDestroyed() const
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    return _state >= StateClosing;
}

bool
IceInternal::Connection::isFinished() const
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    return _transceiver == 0 && _dispatchCount == 0;
}

void
IceInternal::Connection::waitUntilHolding() const
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    while(_state < StateHolding || _dispatchCount > 0)
    {
	wait();
    }
}

void
IceInternal::Connection::waitUntilFinished()
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    while(_transceiver || _dispatchCount > 0)
    {
	if(_endpoint->timeout() >= 0)
	{
	    if(!timedWait(IceUtil::Time::milliSeconds(_endpoint->timeout())))
	    {
		setState(StateClosed, CloseTimeoutException(__FILE__, __LINE__));
		assert(_dispatchCount == 0);
		// No return here, we must still wait until _transceiver becomes null.
	    }
	}
	else
	{
	    wait();
	}
    }

    assert(_state == StateClosed);
}

void
IceInternal::Connection::monitor()
{
    
    IceUtil::Monitor<IceUtil::RecMutex>::TryLock sync(*this);
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
    for(map<Int, OutgoingAsyncPtr>::iterator p = _asyncRequests.begin(); p != _asyncRequests.end(); ++p)
    {
	if(p->second->__timedOut())
	{
	    setState(StateClosed, TimeoutException(__FILE__, __LINE__));
	    return;
	}
    }
    
    //
    // Active connection management for idle connections.
    //
    // TODO: Hack: ACM for incoming connections doesn't work right
    // with AMI.
    //
    if(_acmTimeout > 0 && closeOK() && !_adapter)
    {
	if(IceUtil::Time::now() >= _acmAbsoluteTimeout)
	{
	    setState(StateClosing, ConnectionTimeoutException(__FILE__, __LINE__));
	    return;
	}
    }
}

void
IceInternal::Connection::validate()
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    if(_exception.get())
    {
	_exception->ice_throw();
    }

    if(_state != StateNotValidated)
    {
	return;
    }

    if(!_endpoint->datagram()) // Datagram connections are always implicitly validated.
    {
	try
	{
	    if(_adapter)
	    {
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
		os.write((Byte)1); // Compression status.
		os.write(headerSize); // Message size.
		os.i = os.b.begin();
		traceHeader("sending validate connection", os, _logger, _traceLevels);
		_transceiver->write(os, _endpoint->timeout());
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
		_transceiver->read(is, _endpoint->timeout());
		assert(is.i == is.b.end());
		assert(is.i - is.b.begin() >= headerSize);
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

		//
		// We only check the major version number here. The minor version
		// number is irrelevant -- no matter what minor version number is offered
		// by the server, we can be certain that the server supports at least minor version 0.
		// As the client, we are obliged to never produce a message with a minor
		// version number that is larger than what the server can understand, but we don't
		// care if the server understands more than we do.
		//
		// Note: Once we add minor versions, we need to modify the client side to never produce
		// a message with a minor number that is greater than what the server can handle. Similarly,
		// the server side will have to be modified so it never replies with a minor version that is
		// greater than what the client can handle.
		//
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

		//
		// The same applies here as above -- only the major version number
		// of the encoding is relevant.
		//
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
                is.read(compress);

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

    //
    // We only print warnings after successful connection validation.
    //
    _warn = _instance->properties()->getPropertyAsInt("Ice.Warn.Connections") > 0;

    //
    // We only use active connection management after successful
    // connection validation. We don't use active connection
    // management for datagram connections at all, because such
    // "virtual connections" cannot be reestablished.
    //
    if(!_endpoint->datagram())
    {
	_acmTimeout = _instance->properties()->getPropertyAsInt("Ice.ConnectionIdleTime");
	if(_acmTimeout > 0)
	{
	    _acmAbsoluteTimeout = IceUtil::Time::now() + IceUtil::Time::seconds(_acmTimeout);
	}
    }

    //
    // We start out in holding state.
    //
    setState(StateHolding);
}

void
IceInternal::Connection::incProxyCount()
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
    assert(_proxyCount >= 0);
    ++_proxyCount;
}

void
IceInternal::Connection::decProxyCount()
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
    assert(_proxyCount > 0);
    --_proxyCount;

    if(_proxyCount == 0 && !_adapter && closeOK())
    {
	setState(StateClosing, CloseConnectionException(__FILE__, __LINE__));
    }
}

void
IceInternal::Connection::prepareRequest(BasicStream* os)
{
    os->writeBlob(_requestHdr);
}

void
IceInternal::Connection::sendRequest(Outgoing* out, bool oneway)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    if(_exception.get())
    {
	_exception->ice_throw();
    }
    assert(_state > StateNotValidated && _state < StateClosing);

    Int requestId;

    try
    {
	BasicStream* os = out->os();
	
	//
	// Fill in the request ID.
	//
	if(!_endpoint->datagram() && !oneway)
	{
	    requestId = _nextRequestId++;
	    if(requestId <= 0)
	    {
		_nextRequestId = 1;
		requestId = _nextRequestId++;
	    }
	    const Byte* p = reinterpret_cast<const Byte*>(&requestId);
#ifdef ICE_BIG_ENDIAN
	    reverse_copy(p, p + sizeof(Int), os->b.begin() + headerSize);
#else
	    copy(p, p + sizeof(Int), os->b.begin() + headerSize);
#endif
	}

	bool compress;
	if(os->b.size() < 100) // Don't compress if message size is smaller than 100 bytes.
	{
	    compress = false;
	}
	else
	{
	    compress = _endpoint->compress();
	}

	if(compress)
	{
	    //
	    // Set compression status.
	    //
	    os->b[9] = 2; // Message is compressed.

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
	setState(StateClosed, ex);
	assert(_exception.get());
	_exception->ice_throw();
    }
    
    //
    // Only add to the request map if there was no exception, and if
    // the operation is not oneway.
    //
    if(!_endpoint->datagram() && !oneway)
    {
	_requestsHint = _requests.insert(_requests.end(), pair<const Int, Outgoing*>(requestId, out));
    }

    if(_acmTimeout > 0)
    {
	_acmAbsoluteTimeout = IceUtil::Time::now() + IceUtil::Time::seconds(_acmTimeout);
    }
}

void
IceInternal::Connection::sendAsyncRequest(const OutgoingAsyncPtr& out)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    if(_exception.get())
    {
	_exception->ice_throw();
    }
    assert(_state > StateNotValidated && _state < StateClosing);

    Int requestId;

    try
    {
	BasicStream* os = out->__os();
	
	//
	// Fill in the request ID.
	//
	requestId = _nextRequestId++;
	if(requestId <= 0)
	{
	    _nextRequestId = 1;
	    requestId = _nextRequestId++;
	}
	const Byte* p = reinterpret_cast<const Byte*>(&requestId);
#ifdef ICE_BIG_ENDIAN
	reverse_copy(p, p + sizeof(Int), os->b.begin() + headerSize);
#else
	copy(p, p + sizeof(Int), os->b.begin() + headerSize);
#endif

	bool compress;
	if(os->b.size() < 100) // Don't compress if message size is smaller than 100 bytes.
	{
	    compress = false;
	}
	else
	{
	    compress = _endpoint->compress();
	}

	if(compress)
	{
	    //
	    // Set compression status.
	    //
	    os->b[9] = 2; // Message is compressed.

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
	    //
	    // No compression, just fill in the message size.
	    //
	    Int sz = static_cast<Int>(os->b.size());
	    p = reinterpret_cast<const Byte*>(&sz);
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
	setState(StateClosed, ex);
	assert(_exception.get());
	_exception->ice_throw();
    }
    
    //
    // Only add to the request map if there was no exception.
    //
    _asyncRequestsHint = _asyncRequests.insert(_asyncRequests.end(), pair<const Int, OutgoingAsyncPtr>(requestId, out));

    if(_acmTimeout > 0)
    {
	_acmAbsoluteTimeout = IceUtil::Time::now() + IceUtil::Time::seconds(_acmTimeout);
    }
}

void
IceInternal::Connection::prepareBatchRequest(BasicStream* os)
{
    lock();

    if(_exception.get())
    {
	unlock();
	_exception->ice_throw();
    }
    assert(_state > StateNotValidated && _state < StateClosing);

    if(_batchStream.b.empty())
    {
	try
	{
	    _batchStream.writeBlob(_requestBatchHdr);
	}
	catch(const LocalException& ex)
	{
	    setState(StateClosed, ex);
	    unlock();
	    ex.ice_throw();
	}
    }

    _batchStream.swap(*os);

    //
    // The Connection and _batchStream now belongs to the caller,
    // until finishBatchRequest() or abortBatchRequest() is called.
    //
}

void
IceInternal::Connection::finishBatchRequest(BasicStream* os)
{
    if(_exception.get())
    {
	unlock();
	_exception->ice_throw();
    }
    assert(_state > StateNotValidated && _state < StateClosing);

    _batchStream.swap(*os); // Get the batch stream back.
    ++_batchRequestNum; // Increment the number of requests in the batch.
    unlock(); // Give the Connection back.
}

void
IceInternal::Connection::abortBatchRequest()
{
    setState(StateClosed, AbortBatchRequestException(__FILE__, __LINE__));
    unlock(); // Give the Connection back.
}

void
IceInternal::Connection::flushBatchRequest()
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    if(_exception.get())
    {
	_exception->ice_throw();
    }
    assert(_state > StateNotValidated && _state < StateClosing);
    
    if(!_batchStream.b.empty())
    {
	try
	{
	    _batchStream.i = _batchStream.b.begin();
	    
	    //
	    // Fill in the number of requests in the batch.
	    //
	    const Byte* p = reinterpret_cast<const Byte*>(&_batchRequestNum);
#ifdef ICE_BIG_ENDIAN
            reverse_copy(p, p + sizeof(Int), _batchStream.b.begin() + headerSize);
#else
            copy(p, p + sizeof(Int), _batchStream.b.begin() + headerSize);
#endif

	    bool compress;
	    if(_batchStream.b.size() < 100) // Don't compress if message size is smaller than 100 bytes.
	    {
		compress = false;
	    }
	    else
	    {
		compress = _endpoint->compress();
	    }
	    
	    if(compress)
	    {
		//
		// Set compression status.
		//
		_batchStream.b[9] = 2; // Message is compressed.
		
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
		//
		// No compression, just fill in the message size.
		//
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
	    
	    //
	    // Reset _batchStream and _batchRequestNum, so that new batch
	    // messages can be sent.
	    //
	    BasicStream dummy(_instance.get());
	    _batchStream.swap(dummy);
	    assert(_batchStream.b.empty());
	    _batchRequestNum = 0;
	}
	catch(const LocalException& ex)
	{
	    setState(StateClosed, ex);
	    assert(_exception.get());
	    _exception->ice_throw();
	}
	
	if(_acmTimeout > 0)
	{
	    _acmAbsoluteTimeout = IceUtil::Time::now() + IceUtil::Time::seconds(_acmTimeout);
	}
    }
    
    if(_proxyCount == 0 && !_adapter && closeOK())
    {
	setState(StateClosing, CloseConnectionException(__FILE__, __LINE__));
    }
}

void
IceInternal::Connection::sendResponse(BasicStream* os, Byte compressFlag)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
    
    try
    {
	if(_state == StateClosed)
	{
	    assert(_dispatchCount == 0);
	    return;
	}
	
	if(--_dispatchCount == 0)
	{
	    notifyAll();
	}

	bool compress;
	if(os->b.size() < 100) // Don't compress if message size is smaller than 100 bytes.
	{
	    compress = false;
	}
	else
	{
	    compress = _endpoint->compress() && compressFlag > 0;
	}
	
	if(compress)
	{
	    //
	    // Set compression status.
	    //
	    os->b[9] = 2; // Message is compressed.

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
	
	if(_state == StateClosing && _dispatchCount == 0)
	{
	    initiateShutdown();
	}
    }
    catch(const LocalException& ex)
    {
	setState(StateClosed, ex);
    }

    if(_acmTimeout > 0)
    {
	_acmAbsoluteTimeout = IceUtil::Time::now() + IceUtil::Time::seconds(_acmTimeout);
    }
}

void
IceInternal::Connection::sendNoResponse()
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
    
    try
    {
	if(_state == StateClosed)
	{
	    assert(_dispatchCount == 0);
	    return;
	}
	
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

int
IceInternal::Connection::timeout() const
{
    // No mutex protection necessary, _endpoint is immutable.
    return _endpoint->timeout();
}

EndpointPtr
IceInternal::Connection::endpoint() const
{
    // No mutex protection necessary, _endpoint is immutable.
    return _endpoint;
}

void
IceInternal::Connection::setAdapter(const ObjectAdapterPtr& adapter)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    //
    // We are registered with a thread pool in active and closing
    // mode. However, we only change subscription if we're in active
    // mode, and thus ignore closing mode here.
    //
    if(_state == StateActive)
    {
	if(adapter && !_adapter)
	{
	    //
	    // Client is now server.
	    //
	    unregisterWithPool();
	}
	
	if(!adapter && _adapter)
	{
	    //
	    // Server is now client.
	    //
	    unregisterWithPool();
	}
    }

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

ObjectAdapterPtr
IceInternal::Connection::getAdapter() const
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
    return _adapter;
}

bool
IceInternal::Connection::readable() const
{
    return true;
}

void
IceInternal::Connection::read(BasicStream& stream)
{
    if(_transceiver)
    {
	_transceiver->read(stream, 0);
    }

    //
    // Updating _acmAbsoluteTimeout is too expensive here, because we
    // would have to acquire a lock just for this purpose. Instead, we
    // update _acmAbsoluteTimeout in message().
    //
}

// used for the COMPILERFIX below
static void
setAbsoluteTimeout(int timeout, IceUtil::Time& result)
{
    result = IceUtil::Time::now() + IceUtil::Time::seconds(timeout);
}

void
IceInternal::Connection::message(BasicStream& stream, const ThreadPoolPtr& threadPool)
{
    OutgoingAsyncPtr outAsync;

    Int invoke = 0;
    Int requestId = 0;
    Byte compress = 0;

    {
	IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
	
	threadPool->promoteFollower();
	
	if(_state == StateClosed)
	{
	    IceUtil::ThreadControl::yield();
	    return;
	}
	
//	if(_acmTimeout > 0)
//	{
//	    _acmAbsoluteTimeout = IceUtil::Time::now() + IceUtil::Time::seconds(_acmTimeout);
//	}
// COMPILERFIX without this change VC6 sp5 + processor pack generates code that crashed on exceptions
	if(_acmTimeout > 0)
	{
	    setAbsoluteTimeout(_acmTimeout, _acmAbsoluteTimeout);
	}

	try
	{
	    assert(stream.i == stream.b.end());
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
	    if(pMajor != protocolMajor
	       || static_cast<unsigned char>(pMinor) > static_cast<unsigned char>(protocolMinor))
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
	    if(eMajor != encodingMajor
	       || static_cast<unsigned char>(eMinor) > static_cast<unsigned char>(encodingMinor))
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

	    //
	    // Uncompress if necessary.
	    //
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
			invoke = 1;
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
			stream.read(invoke);
			if(invoke < 0)
			{
			    throw NegativeSizeException(__FILE__, __LINE__);
			}
			_dispatchCount += invoke;
		    }
		    break;
		}
		
		case replyMsg:
		{
                    traceReply("received reply", stream, _logger, _traceLevels);

		    stream.read(requestId);
		    
		    map<Int, Outgoing*>::iterator p = _requests.end();
		    map<Int, OutgoingAsyncPtr>::iterator q = _asyncRequests.end();
		    
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

			outAsync = q->second;
			
			if(q == _asyncRequestsHint)
			{
			    _asyncRequests.erase(q++);
			    _asyncRequestsHint = q;
			}
			else
			{
			    _asyncRequests.erase(q);
			}

			if(_proxyCount == 0 && !_adapter && closeOK())
			{
			    setState(StateClosing, CloseConnectionException(__FILE__, __LINE__));
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
			out << "ignoring unexpected validate connection message:\n"
			    << _transceiver->toString();
		    }
		    break;
		}
		
		case closeConnectionMsg:
		{
		    traceHeader("received close connection", stream, _logger, _traceLevels);
		    if(_endpoint->datagram())
		    {
			if(_warn)
			{
			    Warning out(_logger);
			    out << "ignoring close connection message for datagram connection:\n"
				<< _transceiver->toString();
			}
		    }
		    else
		    {
			throw CloseConnectionException(__FILE__, __LINE__);
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
    // Method invocation must be done outside the thread
    // synchronization, so that nested calls are possible.
    //
    if(invoke > 0)
    {
	//
	// Prepare the invocation.
	//
	bool response = !_endpoint->datagram() && requestId != 0;
	Incoming in(_instance.get(), this, _adapter, response, compress);
	BasicStream* is = in.is();
	stream.swap(*is);
	BasicStream* os = in.os();

	try
	{
	    //
	    // Prepare the response if necessary.
	    //
	    if(response)
	    {
		assert(invoke == 1);
		os->writeBlob(_replyHdr);

		//
		// Add the request ID.
		//
		os->write(requestId);
	    }
	    
	    //
	    // Do the invocation, or multiple invocations for batch
	    // messages.
	    //
	    while(invoke-- > 0)
	    {
		in.invoke(_servantManager);
	    }
	}
	catch(const LocalException& ex)
	{
	    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
	    setState(StateClosed, ex);
	}
    }
}

void
IceInternal::Connection::finished(const ThreadPoolPtr& threadPool)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    threadPool->promoteFollower();

    if(_state == StateActive || _state == StateClosing)
    {
	registerWithPool();
    }
    else if(_state == StateClosed && _transceiver)
    {
	_transceiver->close();
	_transceiver = 0;
	notifyAll();
    }
}

void
IceInternal::Connection::exception(const LocalException& ex)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
    setState(StateClosed, ex);
}

string
IceInternal::Connection::toString() const
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
    assert(_transceiver);
    return _transceiver->toString();
}

bool
IceInternal::Connection::operator==(const Connection& r) const
{
    return this == &r;
}

bool
IceInternal::Connection::operator!=(const Connection& r) const
{
    return this != &r;
}

bool
IceInternal::Connection::operator<(const Connection& r) const
{
    return this < &r;
}

IceInternal::Connection::Connection(const InstancePtr& instance,
				    const TransceiverPtr& transceiver,
				    const EndpointPtr& endpoint,
				    const ObjectAdapterPtr& adapter) :
    EventHandler(instance),
    _transceiver(transceiver),
    _endpoint(endpoint),
    _adapter(adapter),
    _logger(_instance->logger()), // Cached for better performance.
    _traceLevels(_instance->traceLevels()), // Cached for better performance.
    _registeredWithPool(false),
    _warn(false),
    _acmTimeout(0),
    _requestHdr(headerSize + sizeof(Int), 0),
    _requestBatchHdr(headerSize + sizeof(Int), 0),
    _replyHdr(headerSize, 0),
    _nextRequestId(1),
    _requestsHint(_requests.end()),
    _asyncRequestsHint(_asyncRequests.end()),
    _batchStream(_instance.get()),
    _batchRequestNum(0),
    _dispatchCount(0),
    _proxyCount(0),
    _state(StateNotValidated)
{
    if(_adapter)
    {
	_servantManager = dynamic_cast<ObjectAdapterI*>(_adapter.get())->getServantManager();
    }
    else
    {
	_servantManager = 0;
    }

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
    requestHdr[9] = 1; // Default compression status: compression supported but not used.

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
    requestBatchHdr[9] = 1; // Default compression status: compression supported but not used.

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
    replyHdr[9] = 1; // Default compression status: compression supported but not used.
}

IceInternal::Connection::~Connection()
{
    assert(_state == StateClosed);
    assert(!_transceiver);
    assert(_dispatchCount == 0);
    assert(_proxyCount == 0);
}

void
IceInternal::Connection::setState(State state, const LocalException& ex)
{
    if(_state == state) // Don't switch twice.
    {
	return;
    }

    if(!_exception.get())
    {
	_exception = auto_ptr<LocalException>(dynamic_cast<LocalException*>(ex.ice_clone()));

	if(_warn)
	{
	    //
	    // Don't warn about certain expected exceptions.
	    //
	    if(!(dynamic_cast<const CloseConnectionException*>(_exception.get()) ||
		 dynamic_cast<const ConnectionTimeoutException*>(_exception.get()) ||
		 dynamic_cast<const CommunicatorDestroyedException*>(_exception.get()) ||
		 dynamic_cast<const ObjectAdapterDeactivatedException*>(_exception.get()) ||
		 (dynamic_cast<const ConnectionLostException*>(_exception.get()) && _state == StateClosing)))
	    {
		Warning out(_logger);
		out << "connection exception:\n" << *_exception.get() << '\n' << _transceiver->toString();
	    }
	}
    }
    
    for(map<Int, Outgoing*>::iterator p = _requests.begin(); p != _requests.end(); ++p)
    {
	p->second->finished(*_exception.get());
    }
    _requests.clear();
    _requestsHint = _requests.end();

    for(map<Int, OutgoingAsyncPtr>::iterator q = _asyncRequests.begin(); q != _asyncRequests.end(); ++q)
    {
	q->second->__finished(*_exception.get());
    }
    _asyncRequests.clear();
    _asyncRequestsHint = _asyncRequests.end();

    setState(state);
}

void
IceInternal::Connection::setState(State state)
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
	    registerWithPool();
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
	    unregisterWithPool();
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
	    registerWithPool(); // We need to continue to read in closing state.
	    break;
	}
	
	case StateClosed:
	{
	    //
	    // If we do a hard close, all outstanding requests are
	    // disregarded.
	    //
	    _dispatchCount = 0;

	    //
	    // If we change from not validated, we can close right
	    // away. Otherwise we first must make sure that we are
	    // registered, then we unregister, and let finished() do
	    // the close.
	    //
	    if(_state == StateNotValidated)
	    {
		assert(!_registeredWithPool);
		_transceiver->close();
		_transceiver = 0;
	    }
	    else
	    {
		registerWithPool();
		unregisterWithPool();
	    }
	    break;
	}
    }

    _state = state;
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
IceInternal::Connection::initiateShutdown() const
{
    assert(_state == StateClosing);
    assert(_dispatchCount == 0);

    if(!_endpoint->datagram())
    {
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
	os.i = os.b.begin();
	traceHeader("sending close connection", os, _logger, _traceLevels);
	_transceiver->write(os, _endpoint->timeout());
	_transceiver->shutdown();
    }
}

void
IceInternal::Connection::registerWithPool()
{
    if(!_registeredWithPool)
    {
	if(_adapter)
	{
	    dynamic_cast<ObjectAdapterI*>(_adapter.get())->getThreadPool()->_register(_transceiver->fd(), this);
	}
	else
	{
	    _instance->clientThreadPool()->_register(_transceiver->fd(), this);
	}

	_registeredWithPool = true;

	ConnectionMonitorPtr connectionMonitor = _instance->connectionMonitor();
	if(connectionMonitor)
	{
	    connectionMonitor->add(this);
	}
    }
}

void
IceInternal::Connection::unregisterWithPool()
{
    if(_registeredWithPool)
    {
	if(_adapter)
	{
	    dynamic_cast<ObjectAdapterI*>(_adapter.get())->getThreadPool()->unregister(_transceiver->fd());
	}
	else
	{
	    _instance->clientThreadPool()->unregister(_transceiver->fd());
	}

	_registeredWithPool = false;

	ConnectionMonitorPtr connectionMonitor = _instance->connectionMonitor();
	if(connectionMonitor)
	{
	    connectionMonitor->remove(this);
	}
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
IceInternal::Connection::doCompress(BasicStream& uncompressed, BasicStream& compressed)
{
    const Byte* p;

    //
    // Compress the message body, but not the header.
    //
    unsigned int uncompressedLen = static_cast<unsigned int>(uncompressed.b.size() - headerSize);
    unsigned int compressedLen = static_cast<unsigned int>(uncompressedLen * 1.01 + 600);
    compressed.b.resize(headerSize + sizeof(Int) + compressedLen);
    int bzError = BZ2_bzBuffToBuffCompress(&compressed.b[0] + headerSize + sizeof(Int), &compressedLen,
					   &uncompressed.b[0] + headerSize, uncompressedLen,
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
IceInternal::Connection::doUncompress(BasicStream& compressed, BasicStream& uncompressed)
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
    int bzError = BZ2_bzBuffToBuffDecompress(&uncompressed.b[0] + headerSize,
					     &uncompressedLen,
					     &compressed.b[0] + headerSize + sizeof(Int),
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

bool
IceInternal::Connection::closeOK() const
{
    return
	_requests.empty() &&
	_asyncRequests.empty() &&
	_batchStream.b.empty() &&
	_dispatchCount == 0;
}
