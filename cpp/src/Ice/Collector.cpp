// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Collector.h>
#include <Ice/Instance.h>
#include <Ice/Logger.h>
#include <Ice/Properties.h>
#include <Ice/Transceiver.h>
#include <Ice/Acceptor.h>
#include <Ice/Connection.h>
#include <Ice/ThreadPool.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/Endpoint.h>
#include <Ice/Exception.h>
#include <Ice/Functional.h>
#include <Ice/SecurityException.h> // TODO: bandaid, see below.

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(CollectorFactory* p) { p->__incRef(); }
void IceInternal::decRef(CollectorFactory* p) { p->__decRef(); }

IceInternal::CollectorFactory::CollectorFactory(const InstancePtr& instance,
						const EndpointPtr& endpoint,
						const ObjectAdapterPtr& adapter) :
    EventHandler(instance),
    _endpoint(endpoint),
    _adapter(adapter),
    _state(StateHolding)
{
    _warn = atoi(_instance->properties()->getProperty("Ice.ConnectionWarnings").c_str()) > 0 ? true : false;

    try
    {
	_transceiver = _endpoint->serverTransceiver(_endpoint);
	if (_transceiver)
	{
	    ConnectionPtr connection = new Connection(_instance, _transceiver, _endpoint, _adapter);
	    _connections.push_back(connection);
	}
	else
	{
	    _acceptor = _endpoint->acceptor(_endpoint);
	    assert(_acceptor);
	    _acceptor->listen();
	    _threadPool = _instance->threadPool();
	}
    }
    catch (...)
    {
	setState(StateClosed);
	throw;
    }
}

IceInternal::CollectorFactory::~CollectorFactory()
{
    assert(_state == StateClosed);
}

void
IceInternal::CollectorFactory::destroy()
{
    JTCSyncT<JTCMutex> sync(*this);
    setState(StateClosed);
}

void
IceInternal::CollectorFactory::hold()
{
    JTCSyncT<JTCMutex> sync(*this);
    setState(StateHolding);
}

void
IceInternal::CollectorFactory::activate()
{
    JTCSyncT<JTCMutex> sync(*this);
    setState(StateActive);
}

EndpointPtr
IceInternal::CollectorFactory::endpoint() const
{
    return _endpoint;
}

bool
IceInternal::CollectorFactory::equivalent(const EndpointPtr& endp) const
{
    if (_transceiver)
    {
	return endp->equivalent(_transceiver);
    }
    
    assert(_acceptor);
    return endp->equivalent(_acceptor);
}

bool
IceInternal::CollectorFactory::server() const
{
    return true;
}

bool
IceInternal::CollectorFactory::readable() const
{
    return false;
}

void
IceInternal::CollectorFactory::read(BasicStream&)
{
    assert(false); // Must not be called
}

void
IceInternal::CollectorFactory::message(BasicStream&)
{
    JTCSyncT<JTCMutex> sync(*this);

    if (_state != StateActive)
    {
	_threadPool->promoteFollower();
	JTCThread::yield();
	return;
    }
    
    //
    // First reap destroyed connections
    //
    // Can't use _connections.remove_if(constMemFun(...)), because VC++
    // doesn't support member templates :-(
    _connections.erase(remove_if(_connections.begin(), _connections.end(), ::Ice::constMemFun(&Connection::destroyed)),
		      _connections.end());

    //
    // Now accept a new connection and create a new ConnectionPtr
    //
    try
    {
	TransceiverPtr transceiver = _acceptor->accept(0);
	ConnectionPtr connection = new Connection(_instance, transceiver, _endpoint, _adapter);
	connection->activate();
	_connections.push_back(connection);
    }
    catch (const IceSecurity::SecurityException& ex)
    {
        // TODO: bandaid. Takes care of SSL Handshake problems during
        // creation of a Transceiver. Ignore, nothing we can do here.
        warning(ex);
    }
    catch (const SocketException& ex)
    {
        // TODO: bandaid. Takes care of SSL Handshake problems during
        // creation of a Transceiver. Ignore, nothing we can do here.
        warning(ex);
    }
    catch (const TimeoutException&)
    {
	// Ignore timeouts
    }
    catch (const LocalException& ex)
    {
	warning(ex);
        setState(StateClosed);
    }

    _threadPool->promoteFollower();
}

void
IceInternal::CollectorFactory::exception(const LocalException&)
{
    assert(false); // Must not be called
}

void
IceInternal::CollectorFactory::finished()
{
    JTCSyncT<JTCMutex> sync(*this);
    assert(_state == StateClosed);
    _acceptor->shutdown();
    clearBacklog();
    _acceptor->close();
}

bool
IceInternal::CollectorFactory::tryDestroy()
{
    //
    // Do nothing. We don't want collector factories to be closed by
    // active connection management.
    //
    return false;
}

void
IceInternal::CollectorFactory::setState(State state)
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

	    if (_threadPool)
	    {
		_threadPool->_register(_acceptor->fd(), this);
	    }

	    for_each(_connections.begin(), _connections.end(), ::Ice::voidMemFun(&Connection::activate));
	    break;
	}
	
	case StateHolding:
	{
	    if (_state != StateActive) // Can only switch from active to holding
	    {
		return;
	    }

	    if (_threadPool)
	    {
		_threadPool->unregister(_acceptor->fd(), false);
	    }

	    for_each(_connections.begin(), _connections.end(), ::Ice::voidMemFun(&Connection::hold));
	    break;
	}
	
	case StateClosed:
	{
	    if (_threadPool)
	    {
		//
		// If we come from holding state, we first need to
		// register again before we unregister.
		//
		if (_state == StateHolding)
		{
		    _threadPool->_register(_acceptor->fd(), this);
		}
		_threadPool->unregister(_acceptor->fd(), true);
	    }
	    for_each(_connections.begin(), _connections.end(),
		     bind2nd(Ice::voidMemFun1(&Connection::destroy), Connection::ObjectAdapterDeactivated));
	    _connections.clear();
	    break;
	}
    }

    _state = state;
}

void
IceInternal::CollectorFactory::clearBacklog()
{
    //
    // Clear listen() backlog properly by accepting all queued
    // connections, and then shutting them down.
    //
    while (true)
    {
	try
	{
	    TransceiverPtr transceiver = _acceptor->accept(0);
	    ConnectionPtr connection = new Connection(_instance, transceiver, _endpoint, _adapter);
	    connection->exception(ObjectAdapterDeactivatedException(__FILE__, __LINE__));
	}
	catch (const Exception&)
	{
	    break;
	}
    }
}

void
IceInternal::CollectorFactory::warning(const LocalException& ex) const
{
    if (_warn)
    {
	ostringstream s;
	s << "connection exception:\n" << ex << '\n' << _acceptor->toString();
	_instance->logger()->warning(s.str());
    }
}
