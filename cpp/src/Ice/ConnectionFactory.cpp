// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/ConnectionFactory.h>
#include <Ice/Connection.h>
#include <Ice/Instance.h>
#include <Ice/Logger.h>
#include <Ice/TraceLevels.h>
#include <Ice/Properties.h>
#include <Ice/Transceiver.h>
#include <Ice/Connector.h>
#include <Ice/Acceptor.h>
#include <Ice/ThreadPool.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/Endpoint.h>
#include <Ice/Exception.h>
#include <Ice/Functional.h>
#include <Ice/SecurityException.h> // TODO: bandaid, see below.

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(OutgoingConnectionFactory* p) { p->__incRef(); }
void IceInternal::decRef(OutgoingConnectionFactory* p) { p->__decRef(); }

void IceInternal::incRef(IncomingConnectionFactory* p) { p->__incRef(); }
void IceInternal::decRef(IncomingConnectionFactory* p) { p->__decRef(); }

IceInternal::OutgoingConnectionFactory::OutgoingConnectionFactory(const InstancePtr& instance) :
    _instance(instance)
{
}

IceInternal::OutgoingConnectionFactory::~OutgoingConnectionFactory()
{
    assert(!_instance);
}

ConnectionPtr
IceInternal::OutgoingConnectionFactory::create(const vector<EndpointPtr>& endpoints)
{
    IceUtil::Mutex::Lock sync(*this);

    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(!endpoints.empty());

    //
    // First reap destroyed connections
    //
    std::map<EndpointPtr, ConnectionPtr>::iterator p = _connections.begin();
    while (p != _connections.end())
    {
	if (p->second->destroyed())
	{
	    std::map<EndpointPtr, ConnectionPtr>::iterator p2 = p;	    
	    ++p;
	    _connections.erase(p2);
	}
	else
	{
	    ++p;
	}
    }

    //
    // Search for existing connections
    //
    vector<EndpointPtr>::const_iterator q;
    for (q = endpoints.begin(); q != endpoints.end(); ++q)
    {
	map<EndpointPtr, ConnectionPtr>::const_iterator r = _connections.find(*q);
	if (r != _connections.end())
	{
	    return r->second;
	}
    }

    //
    // No connections exist, try to create one
    //
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    LoggerPtr logger = _instance->logger();

    ConnectionPtr connection;
    auto_ptr<LocalException> exception;
    q = endpoints.begin();
    while (q != endpoints.end())
    {
	try
	{
	    TransceiverPtr transceiver = (*q)->clientTransceiver();
	    if (!transceiver)
	    {
		ConnectorPtr connector = (*q)->connector();
		assert(connector);
		transceiver = connector->connect((*q)->timeout());
		assert(transceiver);
	    }	    
	    connection = new Connection(_instance, transceiver, *q, 0);
	    connection->activate();
	    _connections.insert(make_pair(*q, connection));
	    break;
	}
	catch (const SocketException& ex)
	{
	    exception = auto_ptr<LocalException>(dynamic_cast<LocalException*>(ex.ice_clone()));
	}
	catch (const IceSecurity::SecurityException& ex) // TODO: bandaid to make retry w/ ssl work.
	{
	    exception = auto_ptr<LocalException>(dynamic_cast<LocalException*>(ex.ice_clone()));
	}
	catch (const DNSException& ex)
	{
	    exception = auto_ptr<LocalException>(dynamic_cast<LocalException*>(ex.ice_clone()));
	}
	catch (const TimeoutException& ex)
	{
	    exception = auto_ptr<LocalException>(dynamic_cast<LocalException*>(ex.ice_clone()));
	}

	++q;

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
    }

    if (!connection)
    {
	assert(exception.get());
	exception->ice_throw();
    }

    return connection;
}

void
IceInternal::OutgoingConnectionFactory::destroy()
{
    IceUtil::Mutex::Lock sync(*this);

    if (!_instance)
    {
	return;
    }

#ifdef _STLP_BEGIN_NAMESPACE
    // voidbind2nd is an STLport extension for broken compilers in IceUtil/Functional.h
    for_each(_connections.begin(), _connections.end(),
	     voidbind2nd(Ice::secondVoidMemFun1<EndpointPtr, Connection, Connection::DestructionReason>
			 (&Connection::destroy), Connection::CommunicatorDestroyed));
#else
    for_each(_connections.begin(), _connections.end(),
	     bind2nd(Ice::secondVoidMemFun1<EndpointPtr, Connection, Connection::DestructionReason>
		     (&Connection::destroy), Connection::CommunicatorDestroyed));
#endif
    _connections.clear();
    _instance = 0;
}

IceInternal::IncomingConnectionFactory::IncomingConnectionFactory(const InstancePtr& instance,
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

IceInternal::IncomingConnectionFactory::~IncomingConnectionFactory()
{
    assert(_state == StateClosed);
}

void
IceInternal::IncomingConnectionFactory::destroy()
{
    IceUtil::Mutex::Lock sync(*this);
    setState(StateClosed);
}

void
IceInternal::IncomingConnectionFactory::hold()
{
    IceUtil::Mutex::Lock sync(*this);
    setState(StateHolding);
}

void
IceInternal::IncomingConnectionFactory::activate()
{
    IceUtil::Mutex::Lock sync(*this);
    setState(StateActive);
}

EndpointPtr
IceInternal::IncomingConnectionFactory::endpoint() const
{
    return _endpoint;
}

bool
IceInternal::IncomingConnectionFactory::equivalent(const EndpointPtr& endp) const
{
    if (_transceiver)
    {
	return endp->equivalent(_transceiver);
    }
    
    assert(_acceptor);
    return endp->equivalent(_acceptor);
}

bool
IceInternal::IncomingConnectionFactory::server() const
{
    return true;
}

bool
IceInternal::IncomingConnectionFactory::readable() const
{
    return false;
}

void
IceInternal::IncomingConnectionFactory::read(BasicStream&)
{
    assert(false); // Must not be called
}

void
IceInternal::IncomingConnectionFactory::message(BasicStream&)
{
    IceUtil::Mutex::Lock sync(*this);

    if (_state != StateActive)
    {
	_threadPool->promoteFollower();
	IceUtil::ThreadControl::yield();
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
IceInternal::IncomingConnectionFactory::exception(const LocalException&)
{
    assert(false); // Must not be called
}

void
IceInternal::IncomingConnectionFactory::finished()
{
    IceUtil::Mutex::Lock sync(*this);
    assert(_state == StateClosed);
    _acceptor->shutdown();
    clearBacklog();
    _acceptor->close();
}

bool
IceInternal::IncomingConnectionFactory::tryDestroy()
{
    //
    // Do nothing. We don't want collector factories to be closed by
    // active connection management.
    //
    return false;
}

void
IceInternal::IncomingConnectionFactory::setState(State state)
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
#ifdef _STLP_BEGIN_NAMESPACE
	    // voidbind2nd is an STLport extension for broken compilers in IceUtil/Functional.h
	    for_each(_connections.begin(), _connections.end(),
		     voidbind2nd(Ice::voidMemFun1(&Connection::destroy), Connection::ObjectAdapterDeactivated));
#else
	    for_each(_connections.begin(), _connections.end(),
		     bind2nd(Ice::voidMemFun1(&Connection::destroy), Connection::ObjectAdapterDeactivated));
#endif
	    _connections.clear();
	    break;
	}
    }

    _state = state;
}

void
IceInternal::IncomingConnectionFactory::clearBacklog()
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
IceInternal::IncomingConnectionFactory::warning(const LocalException& ex) const
{
    if (_warn)
    {
	ostringstream s;
	s << "connection exception:\n" << ex << '\n' << _acceptor->toString();
	_instance->logger()->warning(s.str());
    }
}
