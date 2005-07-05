// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/OutgoingConnectionFactory.h>
#include <IceE/Connection.h>
#include <IceE/Instance.h>
#include <IceE/LoggerUtil.h>
#include <IceE/TraceLevels.h>
#include <IceE/DefaultsAndOverrides.h>
#include <IceE/Connector.h>
#include <IceE/Endpoint.h>
#include <IceE/RouterInfo.h>
#include <IceE/LocalException.h>
#include <IceE/Functional.h>
#ifndef ICEE_NO_ROUTER
#  include <IceE/Reference.h>
#endif
#include <list>

using namespace std;
using namespace IceE;
using namespace IceEInternal;

void IceEInternal::incRef(OutgoingConnectionFactory* p) { p->__incRef(); }
void IceEInternal::decRef(OutgoingConnectionFactory* p) { p->__decRef(); }

void
IceEInternal::OutgoingConnectionFactory::destroy()
{
    IceE::Monitor<IceE::Mutex>::Lock sync(*this);

    if(_destroyed)
    {
	return;
    }

#ifdef _STLP_BEGIN_NAMESPACE
    // voidbind2nd is an STLport extension for broken compilers in IceE/Functional.h
    for_each(_connections.begin(), _connections.end(),
	     voidbind2nd(IceE::secondVoidMemFun1<EndpointPtr, Connection, Connection::DestructionReason>
			 (&Connection::destroy), Connection::CommunicatorDestroyed));
#else
    for_each(_connections.begin(), _connections.end(),
	     bind2nd(IceE::secondVoidMemFun1<const EndpointPtr, Connection, Connection::DestructionReason>
		     (&Connection::destroy), Connection::CommunicatorDestroyed));
#endif

    _destroyed = true;
    notifyAll();
}

void
IceEInternal::OutgoingConnectionFactory::waitUntilFinished()
{
    multimap<EndpointPtr, ConnectionPtr> connections;

    {
	IceE::Monitor<IceE::Mutex>::Lock sync(*this);
	
	//
	// First we wait until the factory is destroyed. We also wait
	// until there are no pending connections anymore. Only then
	// we can be sure the _connections contains all connections.
	//
	while(!_destroyed || !_pending.empty())
	{
	    wait();
	}
	
	//
	// We want to wait until all connections are finished outside the
	// thread synchronization.
	//
	connections.swap(_connections);
    }

    for_each(connections.begin(), connections.end(),
	     IceE::secondVoidMemFun<const EndpointPtr, Connection>(&Connection::waitUntilFinished));
}

ConnectionPtr
IceEInternal::OutgoingConnectionFactory::create(const vector<EndpointPtr>& endpts)
{
    assert(!endpts.empty());
    vector<EndpointPtr> endpoints = endpts;

    {
	IceE::Monitor<IceE::Mutex>::Lock sync(*this);

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
	vector<EndpointPtr>::iterator q;
	for(q = endpoints.begin(); q != endpoints.end(); ++q)
	{
	    if(_instance->defaultsAndOverrides()->overrideTimeout)
	    {
		*q = (*q)->timeout(_instance->defaultsAndOverrides()->overrideTimeoutValue);
	    }
	}

	//
	// Search for existing connections.
	//
	vector<EndpointPtr>::const_iterator r;
	for(q = endpoints.begin(), r = endpts.begin(); q != endpoints.end(); ++q, ++r)
	{
	    pair<multimap<EndpointPtr, ConnectionPtr>::iterator,
		 multimap<EndpointPtr, ConnectionPtr>::iterator> pr = _connections.equal_range(*q);
	    
	    while(pr.first != pr.second)
	    {
		//
		// Don't return connections for which destruction has
		// been initiated.
		//
		if(!pr.first->second->isDestroyed())
		{
		    return pr.first->second;
		}

		++pr.first;
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
	    for(q = endpoints.begin(), r = endpts.begin(); q != endpoints.end(); ++q, ++r)
	    {
		pair<multimap<EndpointPtr, ConnectionPtr>::iterator,
 		     multimap<EndpointPtr, ConnectionPtr>::iterator> pr = _connections.equal_range(*q);
		
		while(pr.first != pr.second)
		{
		    //
		    // Don't return connections for which destruction has
		    // been initiated.
		    //
		    if(!pr.first->second->isDestroyed())
		    {
			return pr.first->second;
		    }

		    ++pr.first;
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
    vector<EndpointPtr>::const_iterator r;
    for(q = endpoints.begin(), r = endpts.begin(); q != endpoints.end(); ++q, ++r)
    {
	EndpointPtr endpoint = *q;
	
	try
	{
	    ConnectorPtr connector = endpoint->connector();
	    assert(connector);
	    
	    Int timeout;
	    if(_instance->defaultsAndOverrides()->overrideConnectTimeout)
	    {
		timeout = _instance->defaultsAndOverrides()->overrideConnectTimeoutValue;
	    }
	    // It is not necessary to check for overrideTimeout,
	    // the endpoint has already been modified with this
	    // override, if set.
	    else
	    {
		timeout = endpoint->timeout();
	    }
	    
	    TransceiverPtr transceiver = connector->connect(timeout);
	    assert(transceiver);

#ifdef ICEE_PURE_CLIENT
	    connection = new Connection(_instance, transceiver, endpoint);
#else
	    connection = new Connection(_instance, transceiver, endpoint, 0);
#endif
	    //
	    // Wait for the connection to be validated by the
	    // connection thread.  Once the connection has been
	    // validated it will be activated also.
	    //
	    connection->waitForValidation();
	    break;
	}
	catch(const LocalException& ex)
	{
	    exception = auto_ptr<LocalException>(dynamic_cast<LocalException*>(ex.ice_clone()));

	    //
	    // If a connection object was constructed, then validate()
	    // must have raised the exception.
	    //
	    if(connection)
	    {
	        connection->waitUntilFinished(); // We must call waitUntilFinished() for cleanup.
		connection = 0;
	    }
	}
	
	TraceLevelsPtr traceLevels = _instance->traceLevels();
	if(traceLevels->retry >= 2)
	{
	    Trace out(_instance->logger(), traceLevels->retryCat);

	    out << "connection to endpoint failed";
	    if(q + 1 != endpoints.end())
	    {
		out << ", trying next endpoint\n";
	    }
	    else
	    {
		out << " and no more endpoints to try\n";
	    }
	    out << (*exception.get()).toString();
	}
    }
    
    {
	IceE::Monitor<IceE::Mutex>::Lock sync(*this);
	
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
	    _connections.insert(_connections.end(),
				pair<const EndpointPtr, ConnectionPtr>(connection->endpoint(), connection));

	    if(_destroyed)
	    {
		connection->destroy(Connection::CommunicatorDestroyed);
		throw CommunicatorDestroyedException(__FILE__, __LINE__);
	    }
	}
    }

    assert(connection);
    return connection;
}

#ifndef ICEE_NO_ROUTER

void
IceEInternal::OutgoingConnectionFactory::setRouter(const RouterPrx& router)
{
    IceE::Monitor<IceE::Mutex>::Lock sync(*this);

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
	vector<EndpointPtr> endpoints = proxy->__reference()->getEndpoints();
	vector<EndpointPtr>::const_iterator p;
	for(p = endpoints.begin(); p != endpoints.end(); ++p)
	{
	    EndpointPtr endpoint = *p;

	    //
	    // Modify endpoints with overrides.
	    //
	    if(_instance->defaultsAndOverrides()->overrideTimeout)
	    {
		endpoint = endpoint->timeout(_instance->defaultsAndOverrides()->overrideTimeoutValue);
	    }

	    pair<multimap<EndpointPtr, ConnectionPtr>::iterator,
		 multimap<EndpointPtr, ConnectionPtr>::iterator> pr = _connections.equal_range(endpoint);
	    
	    while(pr.first != pr.second)
	    {
		try
		{
		    pr.first->second->setAdapter(adapter);
		}
		catch(const IceE::LocalException&)
		{
		    //
		    // Ignore, the connection is being closed or closed.
		    //
		}
		++pr.first;
	    }
	}
    }    
}

#endif

#ifndef ICEE_NO_BATCH
void
IceEInternal::OutgoingConnectionFactory::flushBatchRequests()
{
    list<ConnectionPtr> c;

    {
	IceE::Monitor<IceE::Mutex>::Lock sync(*this);

	for(std::multimap<EndpointPtr, ConnectionPtr>::const_iterator p = _connections.begin();
	    p != _connections.end();
	    ++p)
	{
	    c.push_back(p->second);
	}
    }

    for(list<ConnectionPtr>::const_iterator p = c.begin(); p != c.end(); ++p)
    {
	try
	{
	    (*p)->flushBatchRequests();
	}
	catch(const LocalException&)
	{
	    // Ignore.
	}
    }
}
#endif

IceEInternal::OutgoingConnectionFactory::OutgoingConnectionFactory(const InstancePtr& instance) :
    _instance(instance),
    _destroyed(false)
{
}

IceEInternal::OutgoingConnectionFactory::~OutgoingConnectionFactory()
{
    assert(_destroyed);
    assert(_connections.empty());
}

#ifndef ICEE_PURE_CLIENT

void
IceEInternal::OutgoingConnectionFactory::removeAdapter(const ObjectAdapterPtr& adapter)
{
    IceE::Monitor<IceE::Mutex>::Lock sync(*this);
    
    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    
    for(multimap<EndpointPtr, ConnectionPtr>::const_iterator p = _connections.begin(); p != _connections.end(); ++p)
    {
	if(p->second->getAdapter() == adapter)
	{
	    try
	    {
		p->second->setAdapter(0);
	    }
	    catch(const IceE::LocalException&)
	    {
		//
		// Ignore, the connection is being closed or closed.
		//
	    }
	}
    }
}

#endif
