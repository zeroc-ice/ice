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
#include <Ice/ObjectAdapterI.h> // For getThreadPool().
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

    if(_destroyed)
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
	     bind2nd(Ice::secondVoidMemFun1<const EndpointPtr, Connection, Connection::DestructionReason>
		     (&Connection::destroy), Connection::CommunicatorDestroyed));
#endif

    _destroyed = true;
    notifyAll();
}

void
IceInternal::OutgoingConnectionFactory::waitUntilFinished()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    //
    // First we wait until the factory is destroyed.
    //
    while(!_destroyed || !_pending.empty())
    {
	wait();
    }

    //
    // Now we wait until the destruction of each connection has
    // finished.
    //
    for_each(_connections.begin(), _connections.end(),
	     Ice::secondVoidMemFun<const EndpointPtr, Connection>(&Connection::waitUntilFinished));

    //
    // We're done, now we can throw away all connections.
    //
    _connections.clear();
}

ConnectionPtr
IceInternal::OutgoingConnectionFactory::create(const vector<EndpointPtr>& endpts)
{
    assert(!endpts.empty());
    vector<EndpointPtr> endpoints = endpts;

    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

	if(_destroyed)
	{
	    throw CommunicatorDestroyedException(__FILE__, __LINE__);
	}

	//
	// Reap connections for which destruction has completed.
	//
	std::multimap<EndpointPtr, ConnectionPtr>::iterator p = _connections.begin();
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
	// Modify endpoints with overrides.
	//
	DefaultsAndOverridesPtr defaultsAndOverrides = _instance->defaultsAndOverrides();
	vector<EndpointPtr>::iterator q;
	for(q = endpoints.begin(); q != endpoints.end(); ++q)
	{
	    if(defaultsAndOverrides->overrideTimeout)
	    {
		*q = (*q)->timeout(defaultsAndOverrides->overrideTimeoutValue);
	    }

	    if(defaultsAndOverrides->overrideCompress)
	    {
		*q = (*q)->compress(defaultsAndOverrides->overrideCompressValue);
	    }
	}

	//
	// Search for existing connections.
	//
	for(q = endpoints.begin(); q != endpoints.end(); ++q)
	{
	    pair<multimap<EndpointPtr, ConnectionPtr>::iterator,
		 multimap<EndpointPtr, ConnectionPtr>::iterator> r = _connections.equal_range(*q);
	    
	    while(r.first != r.second)
	    {
		//
		// Don't return connections for which destruction has
		// been initiated.
		//
		if(!r.first->second->isDestroyed())
		{
		    return r.first->second;
		}

		++r.first;
	    }
	}

	//
	// If some other thread is currently trying to establish a
	// connection to any of our endpoints, we wait until this
	// thread is finished.
	//
	bool searchAgain = false;
	while(!_destroyed)
	{
	    for(q = endpoints.begin(); q != endpoints.end(); ++q)
	    {
		if(_pending.find(*q) != _pending.end())
		{
		    break;
		}
	    }

	    if(q == endpoints.end())
	    {
		break;
	    }

	    searchAgain = true;

	    wait();
	}

	if(_destroyed)
	{
	    throw CommunicatorDestroyedException(__FILE__, __LINE__);
	}

	//
	// Search for existing connections again if we waited above,
	// as new connections might have been added in the meantime.
	//
	if(searchAgain)
	{	
	    for(q = endpoints.begin(); q != endpoints.end(); ++q)
	    {
		pair<multimap<EndpointPtr, ConnectionPtr>::iterator,
 		     multimap<EndpointPtr, ConnectionPtr>::iterator> r = _connections.equal_range(*q);
		
		while(r.first != r.second)
		{
		    //
		    // Don't return connections for which destruction has
		    // been initiated.
		    //
		    if(!r.first->second->isDestroyed())
		    {
			return r.first->second;
		    }

		    ++r.first;
		}
	    }
	}

	//
	// No connection to any of our endpoints exists yet, so we
	// will try to create one. To avoid that other threads try to
	// create connections to the same endpoints, we add our
	// endpoints to _pending.
	//
	_pending.insert(endpoints.begin(), endpoints.end());
    }

    ConnectionPtr connection;
    auto_ptr<LocalException> exception;
    
    vector<EndpointPtr>::const_iterator q;
    for(q = endpoints.begin(); q != endpoints.end(); ++q)
    {
	EndpointPtr endpoint = *q;
	
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
	    break;
	}
	catch(const LocalException& ex)
	{
	    exception = auto_ptr<LocalException>(dynamic_cast<LocalException*>(ex.ice_clone()));
	}
	
	TraceLevelsPtr traceLevels = _instance->traceLevels();
	if(traceLevels->retry >= 2)
	{
	    Trace out(_instance->logger(), traceLevels->retryCat);

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
    
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	
	//
	// Signal other threads that we are done with trying to
	// establish connections to our endpoints.
	//
	for(q = endpoints.begin(); q != endpoints.end(); ++q)
	{
	    _pending.erase(*q);
	}
	notifyAll();

	if(!connection)
	{
	    assert(exception.get());
	    exception->ice_throw();
	}
	else
	{
	    _connections.insert(_connections.end(), pair<const EndpointPtr, ConnectionPtr>(connection->endpoint(), connection));

	    if(_destroyed)
	    {
		connection->destroy(Connection::CommunicatorDestroyed);
		throw CommunicatorDestroyedException(__FILE__, __LINE__);
	    }
	    else
	    {
		connection->activate();
	    }
	}
    }

    assert(connection);
    return connection;
}

void
IceInternal::OutgoingConnectionFactory::setRouter(const RouterPrx& router)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(_destroyed)
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

	    if(defaultsAndOverrides->overrideCompress)
	    {
		endpoint = endpoint->compress(defaultsAndOverrides->overrideCompressValue);
	    }

	    pair<multimap<EndpointPtr, ConnectionPtr>::iterator,
		 multimap<EndpointPtr, ConnectionPtr>::iterator> r = _connections.equal_range(endpoint);
	    
	    while(r.first != r.second)
	    {
		r.first->second->setAdapter(adapter);
		++r.first;
	    }
	}
    }    
}

void
IceInternal::OutgoingConnectionFactory::removeAdapter(const ObjectAdapterPtr& adapter)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    
    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    
    for(multimap<EndpointPtr, ConnectionPtr>::const_iterator p = _connections.begin(); p != _connections.end(); ++p)
    {
	if(p->second->getAdapter() == adapter)
	{
	    p->second->setAdapter(0);
	}
    }
}

IceInternal::OutgoingConnectionFactory::OutgoingConnectionFactory(const InstancePtr& instance) :
    _instance(instance),
    _destroyed(false)
{
}

IceInternal::OutgoingConnectionFactory::~OutgoingConnectionFactory()
{
    assert(_destroyed);
    assert(_connections.empty());
}

void
IceInternal::IncomingConnectionFactory::activate()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    setState(StateActive);
}

void
IceInternal::IncomingConnectionFactory::hold()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    setState(StateHolding);
}

void
IceInternal::IncomingConnectionFactory::destroy()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    setState(StateClosed);
}

void
IceInternal::IncomingConnectionFactory::waitUntilHolding() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

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
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    //
    // First we wait until the factory is destroyed.
    //
    while(_acceptor)
    {
	wait();
    }

    //
    // Now we wait until the destruction of each connection has
    // finished.
    //
    for_each(_connections.begin(), _connections.end(), Ice::voidMemFun(&Connection::waitUntilFinished));

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
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    list<ConnectionPtr> result;

    //
    // Only copy connections which have not been destroyed.
    //
    remove_copy_if(_connections.begin(), _connections.end(), back_inserter(result),
		   Ice::constMemFun(&Connection::isDestroyed));

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
    ConnectionPtr connection;

    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	
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
				     Ice::constMemFun(&Connection::isFinished)),
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
	    // Warn about other Ice local exceptions.
	    if(_warn)
	    {
		Warning out(_instance->logger());
		out << "connection exception:\n" << ex << '\n' << _acceptor->toString();
	    }
	    threadPool->promoteFollower();
	    return;
	}
	catch(...)
	{
	    threadPool->promoteFollower();
	    throw;
	}
	
	//
	// We must promote a follower after we accepted a new connection.
	//
	threadPool->promoteFollower();
	
	//
	// Create a connection object for the connection.
	//
	assert(transceiver);
	connection = new Connection(_instance, transceiver, _endpoint, _adapter);
	_connections.push_back(connection);
    }
    
    assert(connection);

    //
    // We validate outside the thread synchronization, to not block
    // the factory.
    //
    try
    {
	connection->validate();
    }
    catch(const LocalException&)
    {
	//
	// Ignore all exceptions while activating or validating the
	// connection object. Warning or error messages for such
	// exceptions must be printed directly in the connection
	// object code.
	//
    }

    //
    // The factory must be active at this point, so we activate the
    // connection, too.
    //
    connection->activate();
}

void
IceInternal::IncomingConnectionFactory::finished(const ThreadPoolPtr& threadPool)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

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
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

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

    if(defaultsAndOverrides->overrideCompress)
    {
	const_cast<EndpointPtr&>(_endpoint) = _endpoint->compress(defaultsAndOverrides->overrideCompressValue);
    }

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
	    for_each(_connections.begin(), _connections.end(), Ice::voidMemFun(&Connection::activate));
	    break;
	}
	
	case StateHolding:
	{
	    if(_state != StateActive) // Can only switch from active to holding.
	    {
		return;
	    }
	    unregisterWithPool();
	    for_each(_connections.begin(), _connections.end(), Ice::voidMemFun(&Connection::hold));
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
    if(_acceptor && !_registeredWithPool)
    {
	dynamic_cast<ObjectAdapterI*>(_adapter.get())->getThreadPool()->_register(_acceptor->fd(), this);
	_registeredWithPool = true;
    }
}

void
IceInternal::IncomingConnectionFactory::unregisterWithPool()
{
    if(_acceptor && _registeredWithPool)
    {
	dynamic_cast<ObjectAdapterI*>(_adapter.get())->getThreadPool()->unregister(_acceptor->fd());
	_registeredWithPool = false;
    }
}
