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

#include <Ice/Connection.h>
#include <Ice/Instance.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Properties.h>
#include <Ice/TraceUtil.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/Transceiver.h>
#include <Ice/ThreadPool.h>
#include <Ice/ObjectAdapter.h>
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

    return _transceiver == 0;
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
IceInternal::Connection::waitUntilFinished() const
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    while(_transceiver || _dispatchCount > 0)
    {
	wait();
    }
}

void
IceInternal::Connection::validate()
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

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
		os.write(protocolVersion);
		os.write(encodingVersion);
		os.write(validateConnectionMsg);
		os.write(headerSize); // Message size.
		os.i = os.b.begin();
		traceHeader("sending validate connection", os, _logger, _traceLevels);
		_transceiver->write(os, _endpoint->timeout());
	    }
	    else
	    {
		//
		// Outgoing connection play the passive role with respect to
		// connection validation.
		//
		BasicStream is(_instance.get());
		is.b.resize(headerSize);
		is.i = is.b.begin();
		_transceiver->read(is, _endpoint->timeout());
		assert(is.i == is.b.end());
		assert(is.i - is.b.begin() >= headerSize);
		is.i = is.b.begin();
		Byte protVer;
		is.read(protVer);
		if(protVer != protocolVersion)
		{
		    throw UnsupportedProtocolException(__FILE__, __LINE__);
		}
		Byte encVer;
		is.read(encVer);
		if(encVer != encodingVersion)
		{
		    throw UnsupportedEncodingException(__FILE__, __LINE__);
		}
		Byte messageType;
		is.read(messageType);
		if(messageType != validateConnectionMsg)
		{
		    throw ConnectionNotValidatedException(__FILE__, __LINE__);
		}
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
    if(_proxyCount == 0 && !_adapter)
    {
	assert(_requests.empty());
	assert(_asyncRequests.empty());
	setState(StateClosing, CloseConnectionException(__FILE__, __LINE__));
    }
}

void
IceInternal::Connection::prepareRequest(BasicStream* os)
{
    os->writeBlob(_requestHdr);
}

void
IceInternal::Connection::sendRequest(Outgoing* out, bool oneway, bool compress)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    if(_exception.get())
    {
	_exception->ice_throw();
    }
    assert(_state < StateClosing);

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
	    const Byte* p;
	    p = reinterpret_cast<const Byte*>(&requestId);
	    copy(p, p + sizeof(Int), os->b.begin() + headerSize);
	}

	if(os->b.size() < 100) // Don't compress if message size is smaller than 100 bytes.
	{
	    compress = false;
	}
	else
	{
	    if(_defaultsAndOverrides->overrideCompress)
	    {
		compress = _defaultsAndOverrides->overrideCompressValue;
	    }
	}

	if(compress)
	{
	    //
	    // Change message type.
	    //
	    os->b[2] = compressedRequestMsg;

	    //
	    // Do compression.
	    //
	    BasicStream cstream(_instance.get());
	    doCompress(*os, cstream);
	    
	    //
	    // Send the request.
	    //
	    os->i = os->b.begin();
	    traceRequest("sending compressed request", *os, _logger, _traceLevels);
	    cstream.i = cstream.b.begin();
	    _transceiver->write(cstream, _endpoint->timeout());
	}
	else
	{
	    //
	    // No compression, just fill in the message size.
	    //
	    const Byte* p;
	    Int sz = os->b.size();
	    p = reinterpret_cast<const Byte*>(&sz);
	    copy(p, p + sizeof(Int), os->b.begin() + 3);

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
	_requestsHint = _requests.insert(_requests.end(), make_pair(requestId, out));
    }
}

void
IceInternal::Connection::sendAsyncRequest(const OutgoingAsyncPtr& out, bool compress)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    if(_exception.get())
    {
	_exception->ice_throw();
    }
    assert(_state < StateClosing);

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
	const Byte* p;
	p = reinterpret_cast<const Byte*>(&requestId);
	copy(p, p + sizeof(Int), os->b.begin() + headerSize);

	if(os->b.size() < 100) // Don't compress if message size is smaller than 100 bytes.
	{
	    compress = false;
	}
	else
	{
	    if(_defaultsAndOverrides->overrideCompress)
	    {
		compress = _defaultsAndOverrides->overrideCompressValue;
	    }
	}

	if(compress)
	{
	    //
	    // Change message type.
	    //
	    os->b[2] = compressedRequestMsg;

	    //
	    // Do compression.
	    //
	    BasicStream cstream(_instance.get());
	    doCompress(*os, cstream);
	    
	    //
	    // Send the request.
	    //
	    os->i = os->b.begin();
	    traceRequest("sending compressed asynchronous request", *os, _logger, _traceLevels);
	    cstream.i = cstream.b.begin();
	    _transceiver->write(cstream, _endpoint->timeout());
	}
	else
	{
	    //
	    // No compression, just fill in the message size.
	    //
	    const Byte* p;
	    Int sz = os->b.size();
	    p = reinterpret_cast<const Byte*>(&sz);
	    copy(p, p + sizeof(Int), os->b.begin() + 3);

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
    _asyncRequestsHint = _asyncRequests.insert(_asyncRequests.end(), make_pair(requestId, out));
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
    assert(_state < StateClosing);

    //
    // The Connection now belongs to the caller, until
    // finishBatchRequest() is called.
    //

    if(_batchStream.b.empty())
    {
	_batchStream.writeBlob(_requestBatchHdr);
    }

    //
    // Give the batch stream to the caller, until finishBatchRequest()
    // or abortBatchRequest() is called.
    //
    _batchStream.swap(*os);
}

void
IceInternal::Connection::finishBatchRequest(BasicStream* os)
{
    if(_exception.get())
    {
	unlock();
	_exception->ice_throw();
    }
    assert(_state < StateClosing);

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
IceInternal::Connection::flushBatchRequest(bool compress)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    if(_exception.get())
    {
	_exception->ice_throw();
    }
    assert(_state < StateClosing);
    
    try
    {
	if(_batchStream.b.empty())
	{
	    return; // Nothing to send.
	}

	_batchStream.i = _batchStream.b.begin();
	
	//
	// Fill in the number of requests in the batch.
	//
	const Byte* p;
	p = reinterpret_cast<const Byte*>(&_batchRequestNum);
	copy(p, p + sizeof(Int), _batchStream.b.begin() + headerSize);

	if(_batchStream.b.size() < 100) // Don't compress if message size is smaller than 100 bytes.
	{
	    compress = false;
	}
	else
	{
	    if(_defaultsAndOverrides->overrideCompress)
	    {
		compress = _defaultsAndOverrides->overrideCompressValue;
	    }
	}

	if(compress)
	{
	    //
	    // Change message type.
	    //
	    _batchStream.b[2] = compressedRequestBatchMsg;

	    //
	    // Do compression.
	    //
	    BasicStream cstream(_instance.get());
	    doCompress(_batchStream, cstream);
	    
	    //
	    // Send the batch request.
	    //
	    _batchStream.i = _batchStream.b.begin();
	    traceBatchRequest("sending compressed batch request", _batchStream, _logger, _traceLevels);
	    cstream.i = cstream.b.begin();
	    _transceiver->write(cstream, _endpoint->timeout());
	}
	else
	{
	    //
	    // No compression, just fill in the message size.
	    //
	    const Byte* p;
	    Int sz = _batchStream.b.size();
	    p = reinterpret_cast<const Byte*>(&sz);
	    copy(p, p + sizeof(Int), _batchStream.b.begin() + 3);

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
}

void
IceInternal::Connection::sendResponse(BasicStream* os, bool compress)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
    
    try
    {
	if(--_dispatchCount == 0)
	{
	    notifyAll();
	}

	if(_state == StateClosed)
	{
	    return;
	}
	
	if(os->b.size() < 100) // Don't compress if message size is smaller than 100 bytes.
	{
	    compress = false;
	}
	else
	{
	    if(_defaultsAndOverrides->overrideCompress)
	    {
		compress = _defaultsAndOverrides->overrideCompressValue;
	    }
	}
	
	if(compress)
	{
	    //
	    // Change message type.
	    //
	    os->b[2] = compressedReplyMsg;
	    
	    //
	    // Do compression.
	    //
	    BasicStream cstream(_instance.get());
	    doCompress(*os, cstream);
	    
	    //
	    // Send the reply.
	    //
	    os->i = os->b.begin();
	    traceReply("sending compressed reply", *os, _logger, _traceLevels);
	    cstream.i = cstream.b.begin();
	    _transceiver->write(cstream, _endpoint->timeout());
	}
	else
	{
	    //
	    // No compression, just fill in the message size.
	    //
	    const Byte* p;
	    Int sz = os->b.size();
	    p = reinterpret_cast<const Byte*>(&sz);
	    copy(p, p + sizeof(Int), os->b.begin() + 3);
	    
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
}

void
IceInternal::Connection::sendNoResponse()
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
    
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
    _transceiver->read(stream, 0);
}

void
IceInternal::Connection::message(BasicStream& stream, const ThreadPoolPtr& threadPool)
{
    OutgoingAsyncPtr outAsync;

    Int invoke = 0;
    Int requestId = 0;
    bool compress = false;

    {
	IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
	
	threadPool->promoteFollower();
	
	if(_state == StateClosed)
	{
	    IceUtil::ThreadControl::yield();
	    return;
	}

	Byte messageType;

	try
	{
	    assert(stream.i == stream.b.end());
	    stream.i = stream.b.begin() + 2;
	    stream.read(messageType);

	    //
	    // Uncompress if necessary.
	    //
	    if(messageType == compressedRequestMsg ||
	       messageType == compressedRequestBatchMsg ||
	       messageType == compressedReplyMsg)
	    {
		BasicStream ustream(_instance.get());
		doUncompress(stream, ustream);
		stream.b.swap(ustream.b);
		compress = true;
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
		
		case compressedRequestMsg:
		{
		    if(_state == StateClosing)
		    {
			traceRequest("received compressed request during closing\n"
				     "(ignored by server, client will retry)",
				     stream, _logger, _traceLevels);
		    }
		    else
		    {
			traceRequest("received compressed request", stream, _logger, _traceLevels);
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
		
		case compressedRequestBatchMsg:
		{
		    if(_state == StateClosing)
		    {
			traceBatchRequest("received compressed batch request during closing\n"
					  "(ignored by server, client will retry)",
					  stream, _logger, _traceLevels);
		    }
		    else
		    {
			traceBatchRequest("received compressed batch request", stream, _logger, _traceLevels);
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
		case compressedReplyMsg:
		{
		    if(messageType == compressedReplyMsg)
		    {
			traceReply("received compressed reply", stream, _logger, _traceLevels);
		    }
		    else
		    {
			traceReply("received reply", stream, _logger, _traceLevels);
		    }

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
		in.invoke();
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
    else if(_state == StateClosed)
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

IceInternal::Connection::Connection(const InstancePtr& instance,
				    const TransceiverPtr& transceiver,
				    const EndpointPtr& endpoint,
				    const ObjectAdapterPtr& adapter) :
    EventHandler(instance),
    _transceiver(transceiver),
    _endpoint(endpoint),
    _adapter(adapter),
    _logger(_instance->logger()),
    _traceLevels(_instance->traceLevels()),
    _defaultsAndOverrides(_instance->defaultsAndOverrides()),
    _registeredWithPool(false),
    _warn(false),
    _requestHdr(headerSize + 4, 0),
    _requestBatchHdr(headerSize + 4, 0),
    _replyHdr(headerSize, 0),
    _nextRequestId(1),
    _requestsHint(_requests.end()),
    _asyncRequestsHint(_asyncRequests.end()),
    _batchStream(_instance.get()),
    _batchRequestNum(0),
    _dispatchCount(0),
    _proxyCount(0),
    _state(StateHolding)
{
    vector<Byte>& requestHdr = const_cast<vector<Byte>&>(_requestHdr);
    requestHdr[0] = protocolVersion;
    requestHdr[1] = encodingVersion;
    requestHdr[2] = requestMsg;

    vector<Byte>& requestBatchHdr = const_cast<vector<Byte>&>(_requestBatchHdr);
    requestBatchHdr[0] = protocolVersion;
    requestBatchHdr[1] = encodingVersion;
    requestBatchHdr[2] = requestBatchMsg;

    vector<Byte>& replyHdr = const_cast<vector<Byte>&>(_replyHdr);
    replyHdr[0] = protocolVersion;
    replyHdr[1] = encodingVersion;
    replyHdr[2] = replyMsg;
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
		 dynamic_cast<const CommunicatorDestroyedException*>(_exception.get()) ||
		 dynamic_cast<const ObjectAdapterDeactivatedException*>(_exception.get()) ||
		 (dynamic_cast<const ConnectionLostException*>(_exception.get()) && _state == StateClosing)))
	    {
		Warning out(_logger);
		out << "connection exception:\n" << *_exception.get() << '\n' << _transceiver->toString();
		out << '\n' << (int)_state << " -> " << (int)state;
	    }
	}
    }
    
    for(std::map< ::Ice::Int, Outgoing*>::iterator p = _requests.begin(); p != _requests.end(); ++p)
    {
	p->second->finished(*_exception.get());
    }
    _requests.clear();
    _requestsHint = _requests.end();

    for(std::map< ::Ice::Int, OutgoingAsyncPtr>::iterator q = _asyncRequests.begin(); q != _asyncRequests.end(); ++q)
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
	case StateActive:
	{
	    if(_state != StateHolding) // Can only switch from holding to active.
	    {
		return;
	    }
	    registerWithPool();
	    break;
	}
	
	case StateHolding:
	{
	    if(_state != StateActive) // Can only switch from active to holding.
	    {
		return;
	    }
	    unregisterWithPool();
	    break;
	}

	case StateClosing:
	{
	    if(_state == StateClosed) // Can't change back from closed.
	    {
		return;
	    }
	    if(_state == StateHolding)
	    {
		//
		// We need to continue to read data in closing state.
		//
		registerWithPool();
	    }
	    break;
	}
	
	case StateClosed:
	{
	    if(_state == StateHolding)
	    {
		//
		// If we come from holding state, we first need to
		// register again before we unregister, so that
		// finished() is called correctly.
		//
		registerWithPool();
	    }
	    unregisterWithPool();
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
	os.write(protocolVersion);
	os.write(encodingVersion);
	os.write(closeConnectionMsg);
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
	    if(!_serverThreadPool) // Lazy initialization.
	    {
		const_cast<ThreadPoolPtr&>(_serverThreadPool) = _instance->serverThreadPool();
		assert(_serverThreadPool);
	    }
	    _serverThreadPool->_register(_transceiver->fd(), this);
	}
	else
	{
	    if(!_clientThreadPool) // Lazy initialization.
	    {
		const_cast<ThreadPoolPtr&>(_clientThreadPool) = _instance->clientThreadPool();
		assert(_clientThreadPool);
	    }
	    _clientThreadPool->_register(_transceiver->fd(), this);
	}

	_registeredWithPool = true;
    }
}

void
IceInternal::Connection::unregisterWithPool()
{
    if(_registeredWithPool)
    {
	if(_adapter)
	{
	    assert(_serverThreadPool);
	    _serverThreadPool->unregister(_transceiver->fd());
	}
	else
	{
	    assert(_clientThreadPool);
	    _clientThreadPool->unregister(_transceiver->fd());
	}

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
IceInternal::Connection::doCompress(BasicStream& uncompressed, BasicStream& compressed)
{
    const Byte* p;

    //
    // Compress the message body, but not the header.
    //
    unsigned int uncompressedLen = uncompressed.b.size() - headerSize;
    unsigned int compressedLen = static_cast<int>(uncompressedLen * 1.01 + 600);
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
    Int compressedSize = compressed.b.size();
    p = reinterpret_cast<const Byte*>(&compressedSize);
    copy(p, p + sizeof(Int), uncompressed.b.begin() + 3);

    //
    // Add the size of the uncompressed stream before the message body
    // of the compressed stream.
    //
    Int uncompressedSize = uncompressed.b.size();
    p = reinterpret_cast<const Byte*>(&uncompressedSize);
    copy(p, p + sizeof(Int), compressed.b.begin() + headerSize);
    
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
    unsigned int compressedLen = compressed.b.size() - headerSize - sizeof(Int);
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
