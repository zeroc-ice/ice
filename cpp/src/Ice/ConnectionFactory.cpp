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

#include <Ice/ConnectionFactory.h>
#include <Ice/Connection.h>
#include <Ice/Instance.h>
#include <Ice/LoggerUtil.h>
#include <Ice/TraceLevels.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/Properties.h>
#include <Ice/Transceiver.h>
#include <Ice/Connector.h>
#include <Ice/Acceptor.h>
#include <Ice/ThreadPool.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/Reference.h>
#include <Ice/Endpoint.h>
#include <Ice/RouterInfo.h>
#include <Ice/LocalException.h>
#include <Ice/Functional.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(OutgoingConnectionFactory* p) { p->__incRef(); }
void IceInternal::decRef(OutgoingConnectionFactory* p) { p->__decRef(); }

void IceInternal::incRef(IncomingConnectionFactory* p) { p->__incRef(); }
void IceInternal::decRef(IncomingConnectionFactory* p) { p->__decRef(); }

void
IceInternal::OutgoingConnectionFactory::destroy()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(!_instance)
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

    _instance = 0;
    notifyAll();
}

void
IceInternal::OutgoingConnectionFactory::waitUntilFinished()
{
    ::IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);

    //
    // First we wait until the factory is destroyed.
    //
    while(_instance)
    {
	wait();
    }

    //
    // Now we wait for until the destruction of each connection is
    // finished.
    //
    for_each(_connections.begin(), _connections.end(),
	     Ice::secondConstVoidMemFun<EndpointPtr, Connection>(&Connection::waitUntilFinished));

    //
    // We're done, now we can throw away all connections.
    //
    _connections.clear();
}

ConnectionPtr
IceInternal::OutgoingConnectionFactory::create(const vector<EndpointPtr>& endpoints)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(!endpoints.empty());

    //
    // Reap connections for which destruction has completed.
    //
    std::map<EndpointPtr, ConnectionPtr>::iterator p = _connections.begin();
    while(p != _connections.end())
    {
	if(p->second->isFinished())
	{
	    _connections.erase(p++);
	}
	else
	{
	    ++p;
	}
    }

    //
    // Search for existing connections.
    //
    DefaultsAndOverridesPtr defaultsAndOverrides = _instance->defaultsAndOverrides();
    vector<EndpointPtr>::const_iterator q;
    for(q = endpoints.begin(); q != endpoints.end(); ++q)
    {
	EndpointPtr endpoint = *q;
	if(defaultsAndOverrides->overrideTimeout)
	{
	    endpoint = endpoint->timeout(defaultsAndOverrides->overrideTimeoutValue);
	}

	map<EndpointPtr, ConnectionPtr>::const_iterator r = _connections.find(endpoint);
	if(r != _connections.end())
	{
	    //
	    // Don't return connections for which destruction has been
	    // initiated.
	    //
	    if(!r->second->isDestroyed())
	    {
		return r->second;
	    }
	}
    }

    //
    // No connections exist, try to create one.
    //
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    LoggerPtr logger = _instance->logger();

    ConnectionPtr connection;
    auto_ptr<LocalException> exception;
    q = endpoints.begin();
    while(q != endpoints.end())
    {
	EndpointPtr endpoint = *q;
	if(defaultsAndOverrides->overrideTimeout)
	{
	    endpoint = endpoint->timeout(defaultsAndOverrides->overrideTimeoutValue);
	}

	try
	{
	    TransceiverPtr transceiver = endpoint->clientTransceiver();
	    if(!transceiver)
	    {
		ConnectorPtr connector = endpoint->connector();
		assert(connector);
		transceiver = connector->connect(endpoint->timeout());
		assert(transceiver);
	    }	    
	    connection = new Connection(_instance, transceiver, endpoint, 0);
	    connection->validate();
	    connection->activate();
	    _connections.insert(make_pair(endpoint, connection));
	    break;
	}
	catch(const LocalException& ex)
	{
	    exception = auto_ptr<LocalException>(dynamic_cast<LocalException*>(ex.ice_clone()));
	}

	++q;

	if(traceLevels->retry >= 2)
	{
	    Trace out(logger, traceLevels->retryCat);
	    out << "connection to endpoint failed";
	    if(q != endpoints.end())
	    {
		out << ", trying next endpoint\n";
	    }
	    else
	    {
		out << " and no more endpoints to try\n";
	    }
	    out << *exception.get();
	}
    }

    if(!connection)
    {
	assert(exception.get());
	exception->ice_throw();
    }

    return connection;
}

void
IceInternal::OutgoingConnectionFactory::setRouter(const RouterPrx& router)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    RouterInfoPtr routerInfo = _instance->routerManager()->get(router);
    if(routerInfo)
    {
	//
	// Search for connections to the router's client proxy
	// endpoints, and update the object adapter for such
	// connections, so that callbacks from the router can be
	// received over such connections.
	//
	ObjectPrx proxy = routerInfo->getClientProxy();
	ObjectAdapterPtr adapter = routerInfo->getAdapter();
	DefaultsAndOverridesPtr defaultsAndOverrides = _instance->defaultsAndOverrides();
	vector<EndpointPtr>::const_iterator p;
	for(p = proxy->__reference()->endpoints.begin(); p != proxy->__reference()->endpoints.end(); ++p)
	{
	    EndpointPtr endpoint = *p;
	    if(defaultsAndOverrides->overrideTimeout)
	    {
		endpoint = endpoint->timeout(defaultsAndOverrides->overrideTimeoutValue);
	    }

	    map<EndpointPtr, ConnectionPtr>::const_iterator q = _connections.find(endpoint);
	    if(q != _connections.end())
	    {
		q->second->setAdapter(adapter);
	    }
	}
    }    
}

void
IceInternal::OutgoingConnectionFactory::removeAdapter(const ObjectAdapterPtr& adapter)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    
    if(!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    
    for(map<EndpointPtr, ConnectionPtr>::const_iterator p = _connections.begin(); p != _connections.end(); ++p)
    {
	if(p->second->getAdapter() == adapter)
	{
	    p->second->setAdapter(0);
	}
    }
}

IceInternal::OutgoingConnectionFactory::OutgoingConnectionFactory(const InstancePtr& instance) :
    _instance(instance)
{
}

IceInternal::OutgoingConnectionFactory::~OutgoingConnectionFactory()
{
    assert(!_instance);
    assert(_connections.empty());
}

void
IceInternal::IncomingConnectionFactory::activate()
{
    ::IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);
    setState(StateActive);
}

void
IceInternal::IncomingConnectionFactory::hold()
{
    ::IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);
    setState(StateHolding);
}

void
IceInternal::IncomingConnectionFactory::destroy()
{
    ::IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);
    setState(StateClosed);
}

void
IceInternal::IncomingConnectionFactory::waitUntilHolding() const
{
    ::IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);

    //
    // First we wait until the connection factory itself is in holding
    // state.
    //
    while(_state < StateHolding)
    {
	wait();
    }

    //
    // Now we wait until each connection is in holding state.
    //
    for_each(_connections.begin(), _connections.end(), Ice::constVoidMemFun(&Connection::waitUntilHolding));
}

void
IceInternal::IncomingConnectionFactory::waitUntilFinished()
{
    ::IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);

    //
    // First we wait until the factory is destroyed.
    //
    while(_acceptor)
    {
	wait();
    }

    //
    // Now we wait for until the destruction of each connection is
    // finished.
    //
    for_each(_connections.begin(), _connections.end(), Ice::constVoidMemFun(&Connection::waitUntilFinished));

    //
    // We're done, now we can throw away all connections.
    //
    _connections.clear();
}

EndpointPtr
IceInternal::IncomingConnectionFactory::endpoint() const
{
    // No mutex protection necessary, _endpoint is immutable.
    return _endpoint;
}

bool
IceInternal::IncomingConnectionFactory::equivalent(const EndpointPtr& endp) const
{
    if(_transceiver)
    {
	return endp->equivalent(_transceiver);
    }
    
    assert(_acceptor);
    return endp->equivalent(_acceptor);
}

list<ConnectionPtr>
IceInternal::IncomingConnectionFactory::connections() const
{
    ::IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);

    list<ConnectionPtr> result;

    //
    // Only copy connections which have not been destroyed.
    //
    remove_copy_if(_connections.begin(), _connections.end(), back_inserter(result),
		   ::Ice::constMemFun(&Connection::isDestroyed));

    return result;
}

bool
IceInternal::IncomingConnectionFactory::readable() const
{
    return false;
}

void
IceInternal::IncomingConnectionFactory::read(BasicStream&)
{
    assert(false); // Must not be called.
}

void
IceInternal::IncomingConnectionFactory::message(BasicStream&, const ThreadPoolPtr& threadPool)
{
    ::IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);

    if(_state != StateActive)
    {
	IceUtil::ThreadControl::yield();
	threadPool->promoteFollower();
	return;
    }
    
    //
    // Reap connections for which destruction has completed.
    //
    _connections.erase(remove_if(_connections.begin(), _connections.end(),
				 ::Ice::constMemFun(&Connection::isFinished)),
		       _connections.end());

    //
    // Now accept a new connection.
    //
    TransceiverPtr transceiver;
    try
    {
	transceiver = _acceptor->accept(0);
    }
    catch(const SocketException&)
    {
        // TODO: bandaid. Takes care of SSL Handshake problems during
        // creation of a Transceiver. Ignore, nothing we can do here.
	threadPool->promoteFollower();
	return;
    }
    catch(const TimeoutException&)
    {
	// Ignore timeouts.
	threadPool->promoteFollower();
	return;
    }
    catch(const LocalException& ex)
    {
	if(_warn)
	{
	    Warning out(_instance->logger());
	    out << "connection exception:\n" << ex << '\n' << _acceptor->toString();
	}
        setState(StateClosed);
	threadPool->promoteFollower();
	return;
    }
    catch(...)
    {
	threadPool->promoteFollower();
	throw;
    }

    //
    // We must promote a follower after we accepted the new
    // connection.
    //
    threadPool->promoteFollower();

    //
    // Create and activate a connection object for the connection.
    //
    try
    {
	assert(transceiver);
	ConnectionPtr connection = new Connection(_instance, transceiver, _endpoint, _adapter);
	connection->validate();
	connection->activate();
	_connections.push_back(connection);
    }
    catch(const LocalException&)
    {
	//
	// Ignore all exceptions while creating or activating the
	// connection object. Warning or error messages for such
	// exceptions must be printed directly in the connection
	// object code.
	//
    }
}

void
IceInternal::IncomingConnectionFactory::finished(const ThreadPoolPtr& threadPool)
{
    ::IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);

    threadPool->promoteFollower();
    
    if(_state == StateActive)
    {
	registerWithPool();
    }
    else if(_state == StateClosed)
    {
	_acceptor->close();
	_acceptor = 0;
	notifyAll();
    }
}

void
IceInternal::IncomingConnectionFactory::exception(const LocalException&)
{
    assert(false); // Must not be called.
}

string
IceInternal::IncomingConnectionFactory::toString() const
{
    ::IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);

    if(_transceiver)
    {
	return _transceiver->toString();
    }
    
    assert(_acceptor);
    return _acceptor->toString();
}

IceInternal::IncomingConnectionFactory::IncomingConnectionFactory(const InstancePtr& instance,
								  const EndpointPtr& endpoint,
								  const ObjectAdapterPtr& adapter) :
    EventHandler(instance),
    _endpoint(endpoint),
    _adapter(adapter),
    _registeredWithPool(false),
    _warn(_instance->properties()->getPropertyAsInt("Ice.Warn.Connections") > 0),
    _state(StateHolding)
{
    DefaultsAndOverridesPtr defaultsAndOverrides = _instance->defaultsAndOverrides();
    if(defaultsAndOverrides->overrideTimeout)
    {
	const_cast<EndpointPtr&>(_endpoint) = _endpoint->timeout(defaultsAndOverrides->overrideTimeoutValue);
    }

    try
    {
	const_cast<TransceiverPtr&>(_transceiver) = _endpoint->serverTransceiver(const_cast<EndpointPtr&>(_endpoint));
	if(_transceiver)
	{
	    ConnectionPtr connection = new Connection(_instance, _transceiver, _endpoint, _adapter);
	    connection->validate();
	    _connections.push_back(connection);
	}
	else
	{
	    _acceptor = _endpoint->acceptor(const_cast<EndpointPtr&>(_endpoint));
	    assert(_acceptor);
	    _acceptor->listen();
	}
    }
    catch(...)
    {
	setState(StateClosed);
	throw;
    }
}

IceInternal::IncomingConnectionFactory::~IncomingConnectionFactory()
{
    assert(_state == StateClosed);
    assert(!_acceptor);
    assert(_connections.empty());
}

void
IceInternal::IncomingConnectionFactory::setState(State state)
{
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
	    for_each(_connections.begin(), _connections.end(), ::Ice::voidMemFun(&Connection::activate));
	    break;
	}
	
	case StateHolding:
	{
	    if(_state != StateActive) // Can only switch from active to holding.
	    {
		return;
	    }
	    unregisterWithPool();
	    for_each(_connections.begin(), _connections.end(), ::Ice::voidMemFun(&Connection::hold));
	    break;
	}
	
	case StateClosed:
	{
	    //
	    // If we come from holding state, we first need to
	    // register again before we unregister.
	    //
	    if(_state == StateHolding)
	    {
		registerWithPool();
	    }
	    unregisterWithPool();

#ifdef _STLP_BEGIN_NAMESPACE
	    // voidbind2nd is an STLport extension for broken compilers in IceUtil/Functional.h
	    for_each(_connections.begin(), _connections.end(),
		     voidbind2nd(Ice::voidMemFun1(&Connection::destroy), Connection::ObjectAdapterDeactivated));
#else
	    for_each(_connections.begin(), _connections.end(),
		     bind2nd(Ice::voidMemFun1(&Connection::destroy), Connection::ObjectAdapterDeactivated));
#endif
	    break;
	}
    }

    _state = state;
    notifyAll();
}

void
IceInternal::IncomingConnectionFactory::registerWithPool()
{
    if(_acceptor)
    {
	if(!_registeredWithPool)
	{
	    if(!_serverThreadPool) // Lazy initialization.
	    {
		const_cast<ThreadPoolPtr&>(_serverThreadPool) = _instance->serverThreadPool();
		assert(_serverThreadPool);
	    }
	    _serverThreadPool->_register(_acceptor->fd(), this);
	    _registeredWithPool = true;
	}
    }
}

void
IceInternal::IncomingConnectionFactory::unregisterWithPool()
{
    if(_acceptor)
    {
	if(_registeredWithPool)
	{
	    assert(_serverThreadPool);
	    _serverThreadPool->unregister(_acceptor->fd());
	    _registeredWithPool = false;
	}
    }
}
