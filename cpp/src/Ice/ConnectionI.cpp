// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/ConnectionI.h>
#include <Ice/Instance.h>
#include <Ice/Logger.h>
#include <Ice/Properties.h>
#include <Ice/TraceUtil.h>
#include <Ice/Transceiver.h>
#include <Ice/ThreadPool.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/Endpoint.h>
#include <Ice/Outgoing.h>
#include <Ice/Incoming.h>
#include <Ice/Exception.h>
#include <Ice/Protocol.h>
#include <Ice/Functional.h>
#include <Ice/SecurityException.h> // TODO: bandaid, see below.

using namespace std;
using namespace Ice;
using namespace IceInternal;

Ice::ConnectionI::ConnectionI(const InstancePtr& instance,
			      const TransceiverPtr& transceiver,
			      const EndpointPtr& endpoint,
			      const ObjectAdapterPtr& adapter) :
    EventHandler(instance),
    _transceiver(transceiver),
    _endpoint(endpoint),
    _adapter(adapter),
    _logger(instance->logger()),
    _traceLevels(instance->traceLevels()),
    _threadPool(instance->threadPool()),
    _nextRequestId(1),
    _requestsHint(_requests.end()),
    _batchStream(instance),
    _responseCount(0),
    _state(StateHolding)
{
    _warn = atoi(_instance->properties()->getProperty("Ice.ConnectionWarnings").c_str()) > 0 ? true : false;
}

Ice::ConnectionI::~ConnectionI()
{
    assert(_state == StateClosed);
}

bool
Ice::ConnectionI::destroyed() const
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);
    return _state >= StateClosing;
}

void
Ice::ConnectionI::hold()
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);
    setState(StateHolding);
}

void
Ice::ConnectionI::activate()
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);
    setState(StateActive);
}

void
Ice::ConnectionI::prepareRequest(Outgoing* out)
{
    BasicStream* os = out->os();
    os->write(protocolVersion);
    os->write(encodingVersion);
    os->write(requestMsg);
    os->write(Int(0)); // Message size (placeholder)
    os->write(Int(0)); // Request ID (placeholder)
}

void
Ice::ConnectionI::sendRequest(Outgoing* out, bool oneway)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);

    if (_exception.get())
    {
	_exception->ice_throw();
    }
    assert(_state < StateClosing);
    
    Int requestId;

    try
    {
	BasicStream* os = out->os();
	os->i = os->b.begin();
	
	//
	// Fill in the message size and request ID
	//
	const Byte* p;
	Int sz = os->b.size();
	p = reinterpret_cast<Byte*>(&sz);
	copy(p, p + sizeof(Int), os->i + 3);
	if (!_endpoint->oneway() && !oneway)
	{
	    requestId = _nextRequestId++;
	    if (requestId <= 0)
	    {
		_nextRequestId = 1;
		requestId = _nextRequestId++;
	    }		
	    p = reinterpret_cast<Byte*>(&requestId);
	    copy(p, p + sizeof(Int), os->i + headerSize);
	}
	traceRequest("sending request", *os, _logger, _traceLevels);
	_transceiver->write(*os, _endpoint->timeout());
    }
    catch (const LocalException& ex)
    {
	setState(StateClosed, ex);
	ex.ice_throw();
    }
    
    //
    // Only add to the request map if there was no exception, and if
    // the operation is not oneway.
    //
    if (!_endpoint->oneway() && !oneway)
    {
	_requestsHint = _requests.insert(_requests.end(), make_pair(requestId, out));
    }
}

void
Ice::ConnectionI::prepareBatchRequest(Outgoing* out)
{
    lock();

    if (_exception.get())
    {
	unlock();
	_exception->ice_throw();
    }
    assert(_state < StateClosing);

    //
    // The Connection now belongs to `out', until finishBatchRequest()
    // is called.
    //

    if (_batchStream.b.empty())
    {
	_batchStream.write(protocolVersion);
	_batchStream.write(encodingVersion);
	_batchStream.write(requestBatchMsg);
	_batchStream.write(Int(0)); // Message size (placeholder)
    }

    //
    // Give the batch stream to `out', until finishBatchRequest() is
    // called.
    //
    _batchStream.swap(*out->os());
}

void
Ice::ConnectionI::finishBatchRequest(Outgoing* out)
{
    if (_exception.get())
    {
	unlock();
	_exception->ice_throw();
    }
    assert(_state < StateClosing);

    _batchStream.swap(*out->os()); // Get the batch stream back 
    unlock(); // Give the Connection back
}

void
Ice::ConnectionI::abortBatchRequest()
{
    setState(StateClosed, AbortBatchRequestException(__FILE__, __LINE__));
    unlock(); // Give the Connection back
}

void
Ice::ConnectionI::flushBatchRequest()
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);

    if (_exception.get())
    {
	_exception->ice_throw();
    }
    assert(_state < StateClosing);
    
    try
    {
	if(_batchStream.b.empty())
	{
	    return; // Nothing to send
	}

	_batchStream.i = _batchStream.b.begin();
	
	//
	// Fill in the message size
	//
	const Byte* p;
	Int sz = _batchStream.b.size();
	p = reinterpret_cast<Byte*>(&sz);
	copy(p, p + sizeof(Int), _batchStream.i + 3);
	traceBatchRequest("sending batch request", _batchStream, _logger, _traceLevels);
	_transceiver->write(_batchStream, _endpoint->timeout());

	//
	// Reset _batchStream so that new batch messages can be sent.
	//
	BasicStream dummy(_instance);
	_batchStream.swap(dummy);
	assert(_batchStream.b.empty());
    }
    catch (const LocalException& ex)
    {
	setState(StateClosed, ex);
	ex.ice_throw();
    }
}

int
Ice::ConnectionI::timeout() const
{
    return _endpoint->timeout();
}

bool
Ice::ConnectionI::server() const
{
    return _adapter;
}

bool
Ice::ConnectionI::readable() const
{
    return true;
}

void
Ice::ConnectionI::read(BasicStream& stream)
{
    _transceiver->read(stream, 0);
}

void
Ice::ConnectionI::message(BasicStream& stream)
{
    bool invoke = false;
    bool batch = false;

    {
	JTCSyncT<JTCRecursiveMutex> sync(*this);
	
	_threadPool->promoteFollower();
	
	if (_state == StateClosed)
	{
	    JTCThread::yield();
	    return;
	}


	Byte messageType;

	try
	{
	    assert(stream.i == stream.b.end());
	    stream.i = stream.b.begin() + 2;
	    stream.read(messageType);
	    stream.i = stream.b.begin() + headerSize;
	    
	    switch (messageType)
	    {
		case requestMsg:
		{
		    if (_state == StateClosing)
		    {
			traceRequest("received request during closing\n"
				     "(ignored by server, client will retry)",
				     stream, _logger, _traceLevels);
		    }
		    else
		    {
			traceRequest("received request", stream, _logger, _traceLevels);
			invoke = true;
		    }
		    break;
		}
		
		case requestBatchMsg:
		{
		    if (_state == StateClosing)
		    {
			traceBatchRequest("received batch request during closing\n"
					  "(ignored by server, client will retry)",
					  stream, _logger, _traceLevels);
		    }
		    else
		    {
			traceBatchRequest("received batch request", stream, _logger, _traceLevels);
			invoke = true;
			batch = true;
		    }
		    break;
		}
		
		case replyMsg:
		{
		    traceReply("received reply", stream, _logger, _traceLevels);
		    Int requestId;
		    stream.read(requestId);
		    
		    map<Int, Outgoing*>::iterator p = _requests.end();
		    
		    if (_requestsHint != _requests.end())
		    {
			if (_requestsHint->first == requestId)
			{
			    p = _requestsHint;
			}
		    }
		    
		    if (_requestsHint == _requests.end())
		    {
			p = _requests.find(requestId);
		    }
		    
		    if (p == _requests.end())
		    {
			throw UnknownRequestIdException(__FILE__, __LINE__);
		    }
		    p->second->finished(stream);
		    _requests.erase(p);
		    _requestsHint = _requests.end();
		    break;
		}
		
		case closeConnectionMsg:
		{
		    traceHeader("received close connection", stream, _logger, _traceLevels);
		    throw CloseConnectionException(__FILE__, __LINE__);
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
	catch (const LocalException& ex)
	{
	    setState(StateClosed, ex);
	    return;
	}
    }

    //
    // Method invocation must be done outside the thread
    // synchronization, so that nested callbacks are possible.
    //
    if (invoke)
    {
	Incoming in(_instance, _adapter);
	BasicStream* is = in.is();
	BasicStream* os = in.os();
	stream.swap(*is);
	
	bool response = false;

	try
	{
	    if (!batch)
	    {
		Int requestId;
		stream.read(requestId);
		if (!_endpoint->oneway() && requestId != 0) // 0 means oneway
		{
		    response = true;
		    ++_responseCount;
		    os->write(protocolVersion);
		    os->write(encodingVersion);
		    os->write(replyMsg);
		    os->write(Int(0)); // Message size (placeholder)
		    os->write(requestId);
		}
	    }
	    
	    do
	    {
		try
		{
		    in.invoke();
		}
		catch (const Exception& ex)
		{
		    JTCSyncT<JTCRecursiveMutex> sync(*this);
		    warning(ex);
		}
		catch (...)
		{
		    assert(false); // Should not happen
		}
	    }
	    while (batch && is->i < is->b.end());
	}
	catch (const LocalException& ex)
	{
	    JTCSyncT<JTCRecursiveMutex> sync(*this);
	    setState(StateClosed, ex);
	    return;
	}
	
	if (response)
	{
	    JTCSyncT<JTCRecursiveMutex> sync(*this);
	    
	    try
	    {
		
		if (_state == StateClosed)
		{
		    return;
		}
		
		os->i = os->b.begin();
		
		//
		// Fill in the message size
		//
		const Byte* p;
		Int sz = os->b.size();
		p = reinterpret_cast<Byte*>(&sz);
		copy(p, p + sizeof(Int), os->i + 3);
		
		traceReply("sending reply", *os, _logger, _traceLevels);
		_transceiver->write(*os, _endpoint->timeout());
		
		--_responseCount;
		
		if (_state == StateClosing && _responseCount == 0)
		{
		    closeConnection();
		}
	    }
	    catch (const LocalException& ex)
	    {
		setState(StateClosed, ex);
		return;
	    }
	}
    }
}

void
Ice::ConnectionI::exception(const LocalException& ex)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);
    setState(StateClosed, ex);
}

void
Ice::ConnectionI::finished()
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);
    assert(_state == StateClosed);
    _transceiver->close();
}

/*
bool
Ice::ConnectionI::tryDestroy()
{
    bool isLocked = trylock();
    if(!isLocked)
    {
	return false;
    }

    _threadPool->promoteFollower();

    try
    {
	setState(StateClosing);
    }
    catch (...)
    {
	unlock();
	throw;
    }
    
    unlock();    
    return true;
}
*/

InternetAddress
Ice::ConnectionI::getLocalAddress()
{
    assert(false); // TODO: Not implemented yet.
    return InternetAddress();
}

InternetAddress
Ice::ConnectionI::getRemoteAddress()
{
    assert(false); // TODO: Not implemented yet.
    return InternetAddress();
}

ProtocolInfoPtr
Ice::ConnectionI::getProtocolInfo()
{
    assert(false); // TODO: Not implemented yet.
    return 0;
}

void
Ice::ConnectionI::setState(State state, const LocalException& ex)
{
    if (_state == state) // Don't switch twice
    {
	return;
    }

    if (!_exception.get())
    {
	_exception = auto_ptr<LocalException>(dynamic_cast<LocalException*>(ex.ice_clone()));

	//
	// Don't warn about certain expected exceptions.
	//
	if (!dynamic_cast<const CloseConnectionException*>(&ex) &&
	    !dynamic_cast<const CommunicatorDestroyedException*>(&ex) &&
	    !dynamic_cast<const ObjectAdapterDeactivatedException*>(&ex))
	{
	    warning(ex);
	}
    }
    
    for (std::map< ::Ice::Int, Outgoing*>::iterator p = _requests.begin(); p != _requests.end(); ++p)
    {
	p->second->finished(*_exception.get());
    }
    _requests.clear();
    _requestsHint = _requests.end();

    setState(state);
}

void
Ice::ConnectionI::setState(State state)
{
    if (_state == state) // Don't switch twice
    {
	return;
    }
    
    switch (state)
    {
	case StateActive:
	{
	    if (_state != StateHolding) // Can only switch from holding to active
	    {
		return;
	    }
	    _threadPool->_register(_transceiver->fd(), this);
	    break;
	}
	
	case StateHolding:
	{
	    if (_state != StateActive) // Can only switch from active to holding
	    {
		return;
	    }
	    _threadPool->unregister(_transceiver->fd(), false);
	    break;
	}

	case StateClosing:
	{
	    if (_state == StateClosed) // Can't change back from closed
	    {
		return;
	    }
	    if (_state == StateHolding)
	    {
		//
		// We need to continue to read data in closing state
		//
		_threadPool->_register(_transceiver->fd(), this);
	    }
	    break;
	}
	
	case StateClosed:
	{
	    if (_state == StateHolding)
	    {
		//
		// If we come from holding state, we first need to
		// register again before we unregister, so that
		// finished() is called correctly.
		//
		_threadPool->_register(_transceiver->fd(), this);
	    }
	    _threadPool->unregister(_transceiver->fd(), true);
	}
    }

    _state = state;

    if (_state == StateClosing && _responseCount == 0)
    {
	try
	{
	    closeConnection();
	}
	catch (const LocalException& ex)
	{
	    setState(StateClosed, ex);
	}
    }
}

void
Ice::ConnectionI::closeConnection()
{
    BasicStream os(_instance);
    os.write(protocolVersion);
    os.write(encodingVersion);
    os.write(closeConnectionMsg);
    os.write(headerSize); // Message size
    os.i = os.b.begin();
    traceHeader("sending close connection", os, _logger, _traceLevels);
    _transceiver->write(os, _endpoint->timeout());
    _transceiver->shutdown();
}

void
Ice::ConnectionI::warning(const Exception& ex) const
{
    if (_warn)
    {
	ostringstream s;
	s << "server exception:\n" << ex << '\n' << _transceiver->toString();
	_logger->warning(s.str());
    }
}
