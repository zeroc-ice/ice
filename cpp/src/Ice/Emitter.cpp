// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Emitter.h>
#include <Ice/Instance.h>
#include <Ice/Logger.h>
#include <Ice/TraceLevels.h>
#include <Ice/TraceUtil.h>
#include <Ice/Transceiver.h>
#include <Ice/Connector.h>
#include <Ice/ThreadPool.h>
#include <Ice/Endpoint.h>
#include <Ice/Outgoing.h>
#include <Ice/LocalException.h>
#include <Ice/Functional.h>
#include <sstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(Emitter* p) { p->__incRef(); }
void IceInternal::decRef(Emitter* p) { p->__decRef(); }

void IceInternal::incRef(EmitterFactory* p) { p->__incRef(); }
void IceInternal::decRef(EmitterFactory* p) { p->__decRef(); }

void
IceInternal::Emitter::destroy()
{
    exception(CommunicatorDestroyedException(__FILE__, __LINE__));
}

bool
IceInternal::Emitter::destroyed() const
{
    JTCSyncT<JTCMutex> sync(*this);
    return _state >= StateClosing;
}

void
IceInternal::Emitter::prepareRequest(Outgoing* out)
{
    Stream* os = out->os();
    os->write(bigendian);
    os->write(Byte(0)); // Protocol version
    os->write(Byte(0)); // Encoding version
    os->write(Byte(0)); // Message type = Request
    os->write(Int(0)); // Message size (placeholder)
    os->write(Int(0)); // Request ID (placeholder)
}

void
IceInternal::Emitter::sendRequest(Outgoing* out, bool oneway)
{
    JTCSyncT<JTCMutex> sync(*this);

    if (_exception.get())
    {
	_exception->raise();
    }

    assert(_state == StateActive);

    Int requestId;

    try
    {
	Stream* os = out->os();
	os->i = os->b.begin();
	
	//
	// Fill in the message size and request ID
	//
	const Byte* p;
	Int sz = os->b.size();
	p = reinterpret_cast<Byte*>(&sz);
	copy(p, p + sizeof(Int), os->i + 4);
	if (!_endpoint->oneway() && !oneway)
	{
	    requestId = _nextRequestId++;
	    if (requestId == 0) // 0 means oneway
	    {
		requestId = _nextRequestId++;
	    }		
	    p = reinterpret_cast<Byte*>(&requestId);
	    copy(p, p + sizeof(Int), os->i + 8);
	}
	traceRequest("sending request", *os, _logger, _traceLevels);
	_transceiver->write(*os, _endpoint->timeout());
    }
    catch(const LocalException& ex)
    {
	setState(StateClosed, ex);
	ex.raise();
    }

    //
    // Only add to the request map if there was no exception, and if
    // the operation is twoway.
    //
    if (!_endpoint->oneway() && !oneway)
    {
	_requests.insert(_requests.end(), make_pair(requestId, out));
    }
}

int
IceInternal::Emitter::timeout() const
{
    return _endpoint->timeout();
}

bool
IceInternal::Emitter::server() const
{
    return true;
}

bool
IceInternal::Emitter::readable() const
{
    return true;
}

void
IceInternal::Emitter::read(Stream& stream)
{
    _transceiver->read(stream, 0);
}

void
IceInternal::Emitter::message(Stream& stream)
{
    JTCSyncT<JTCMutex> sync(*this);

    _threadPool->promoteFollower();
    
    if (_state != StateActive)
    {
	return;
    }

    try
    {
	assert(stream.i == stream.b.end());
	stream.i = stream.b.begin() + 3;
	Byte messageType;
	stream.read(messageType);
	stream.i = stream.b.begin() + 8;
	
	switch (messageType)
	{
	    case 0: // Request
	    {
		traceRequest("received request on the client side\n"
			     "(invalid, closing connection)",
			     stream, _logger, _traceLevels);
		throw InvalidMessageException(__FILE__, __LINE__);
		break;
	    }
	    
	    case 1: // Reply
	    {
		traceReply("received reply", stream, _logger, _traceLevels);
		Int requestId;
		stream.read(requestId);
		map<Int, Outgoing*>::iterator p = _requests.find(requestId);
		if (p == _requests.end())
		{
		    throw UnknownRequestIdException(__FILE__, __LINE__);
		}
		p->second->finished(stream);
		_requests.erase(p);
		break;
	    }
	    
	    case 2: // CloseConnection
	    {
		traceHeader("received close connection",
			    stream, _logger, _traceLevels);
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
    catch(const LocalException& ex)
    {
	setState(StateClosed, ex);
	return;
    }
}

void
IceInternal::Emitter::exception(const LocalException& ex)
{
    JTCSyncT<JTCMutex> sync(*this);
    setState(StateClosed, ex);
}

void
IceInternal::Emitter::finished()
{
    JTCSyncT<JTCMutex> sync(*this);
    _transceiver->close();
}

IceInternal::Emitter::Emitter(const InstancePtr& instance,
			      const TransceiverPtr& transceiver,
			      const EndpointPtr& endpoint) :
    EventHandler(instance),
    _transceiver(transceiver),
    _endpoint(endpoint),
    _nextRequestId(1),
    _state(StateActive)
{
#ifndef ICE_NO_TRACE
    _traceLevels = _instance->traceLevels();
    _logger = _instance->logger();
#endif

    if (!_endpoint->oneway())
    {
	_threadPool = _instance->threadPool();
	_threadPool->_register(_transceiver->fd(), this);
    }
}

IceInternal::Emitter::~Emitter()
{
    assert(_state == StateClosed);
}

void
IceInternal::Emitter::setState(State state, const LocalException& ex)
{
    if (_state == state) // Don't switch twice
    {
	return;
    }

    switch (state)
    {
	case StateActive:
	{
	    return; // Can't switch back to holding state
	}
	
	case StateClosed:
	{
	    if (_threadPool)
		_threadPool->unregister(_transceiver->fd());
	    else
		_transceiver->close();
	    break;
	}
    }
    
    if (!_exception.get())
    {
	_exception = auto_ptr<LocalException>(ex.clone());
    }

    for (std::map< ::Ice::Int, Outgoing*>::iterator p = _requests.begin(); p != _requests.end(); ++p)
    {
	p->second->finished(*_exception.get());
    }
    _requests.clear();
    
    _state = state;
}

EmitterPtr
IceInternal::EmitterFactory::create(const vector<EndpointPtr>& endpoints)
{
    JTCSyncT<JTCMutex> sync(*this);

    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(!endpoints.empty());

    //
    // First reap destroyed emitters
    //
    std::map<EndpointPtr, EmitterPtr>::iterator p = _emitters.begin();
    while (p != _emitters.end())
    {
	if (p -> second -> destroyed())
	{
	    std::map<EndpointPtr, EmitterPtr>::iterator p2 = p;	    
	    ++p;
	    _emitters.erase(p2);
	}
	else
	{
	    ++p;
	}
    }

    //
    // Search for existing emitters
    //
    vector<EndpointPtr>::const_iterator q;
    for (q = endpoints.begin(); q != endpoints.end(); ++q)
    {
	map<EndpointPtr, EmitterPtr>::const_iterator r = _emitters.find(*q);
	if (r != _emitters.end())
	{
	    return r->second;
	}
    }

    //
    // No emitters exist, try to create one
    //
#ifndef ICE_NO_TRACE
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    LoggerPtr logger = _instance->logger();
#endif

    EmitterPtr emitter;
    auto_ptr<LocalException> exception;
    q = endpoints.begin();
    while (q != endpoints.end())
    {
	try
	{
	    TransceiverPtr transceiver = (*q)->clientTransceiver(_instance);
	    if (!transceiver)
	    {
		ConnectorPtr connector = (*q)->connector(_instance);
		assert(connector);
		transceiver = connector->connect((*q)->timeout());
		assert(transceiver);
	    }	    
	    emitter = new Emitter(_instance, transceiver, *q);
	    _emitters.insert(make_pair(*q, emitter));
	    break;
	}
	catch (const SocketException& ex)
	{
	    exception = auto_ptr<LocalException>(ex.clone());
	}
	catch (const DNSException& ex)
	{
	    exception = auto_ptr<LocalException>(ex.clone());
	}

	++q;

#ifndef ICE_NO_TRACE
	if (traceLevels->retry >= 2)
	{
	    ostringstream s;
	    s << "connection to endpoint failed";
	    if (q != endpoints.end())
	    {
		s << ", trying next endpoint\n";
	    }
	    else
	    {
		s << " and no more endpoints to try\n";
	    }
	    s << *exception.get();
	    logger->trace(traceLevels->retryCat, s.str());
	}
#endif	
    }

    if (!emitter)
    {
	assert(exception.get());
	exception -> raise();
    }

    return emitter;
}

IceInternal::EmitterFactory::EmitterFactory(const InstancePtr& instance) :
    _instance(instance)
{
}

IceInternal::EmitterFactory::~EmitterFactory()
{
    assert(!_instance);
}

void
IceInternal::EmitterFactory::destroy()
{
    JTCSyncT<JTCMutex> sync(*this);

    if (!_instance)
    {
	return;
    }

    for_each(_emitters.begin(), _emitters.end(), secondVoidMemFun<EndpointPtr, Emitter>(&Emitter::destroy));
    _emitters.clear();
    _instance = 0;
}
