// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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

ConnectionPtr
IceInternal::OutgoingConnectionFactory::create(const vector<EndpointPtr>& endpoints)
{
    IceUtil::Mutex::Lock sync(*this);

    if(!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(!endpoints.empty());

    //
    // Reap destroyed connections.
    //
    std::map<EndpointPtr, ConnectionPtr>::iterator p = _connections.begin();
    while(p != _connections.end())
    {
	if(p->second->destroyed())
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
	    return r->second;
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
	    connection->activate();
	    _connections.insert(make_pair(endpoint, connection));
	    break;
	}
	catch(const SocketException& ex)
	{
	    exception = auto_ptr<LocalException>(dynamic_cast<LocalException*>(ex.ice_clone()));
	}
	catch(const DNSException& ex)
	{
	    exception = auto_ptr<LocalException>(dynamic_cast<LocalException*>(ex.ice_clone()));
	}
	catch(const TimeoutException& ex)
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
    IceUtil::Mutex::Lock sync(*this);

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
    IceUtil::Mutex::Lock sync(*this);
    
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
}

void
IceInternal::OutgoingConnectionFactory::destroy()
{
    IceUtil::Mutex::Lock sync(*this);

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
    _connections.clear();
    _instance = 0;
}

void
IceInternal::IncomingConnectionFactory::hold()
{
    ::IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);
    setState(StateHolding);
}

void
IceInternal::IncomingConnectionFactory::activate()
{
    ::IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);
    setState(StateActive);
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

    //
    // Reap destroyed connections.
    //
    list<ConnectionPtr>& connections = const_cast<list<ConnectionPtr>& >(_connections);
    connections.erase(remove_if(connections.begin(), connections.end(), ::Ice::constMemFun(&Connection::destroyed)),
		      connections.end());

    return _connections;
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

    threadPool->promoteFollower();

    if(_state != StateActive)
    {
	IceUtil::ThreadControl::yield();
	return;
    }
    
    //
    // Reap destroyed connections.
    //
    _connections.erase(remove_if(_connections.begin(), _connections.end(), ::Ice::constMemFun(&Connection::destroyed)),
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
	return;
    }
    catch(const TimeoutException&)
    {
	// Ignore timeouts.
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
	return;
    }

    //
    // Create and activate a connection object for the connection.
    //
    try
    {
	assert(transceiver);
	ConnectionPtr connection = new Connection(_instance, transceiver, _endpoint, _adapter);
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

	//
        // Break cyclic object dependency. This is necessary, because
        // the object adapter never clears the list of incoming
        // connections it keeps.
	//
	_adapter = 0;

	notifyAll(); // For waitUntilFinished().
    }
}

void
IceInternal::IncomingConnectionFactory::exception(const LocalException&)
{
    assert(false); // Must not be called.
}

IceInternal::IncomingConnectionFactory::IncomingConnectionFactory(const InstancePtr& instance,
								  const EndpointPtr& endpoint,
								  const ObjectAdapterPtr& adapter) :
    EventHandler(instance),
    _endpoint(endpoint),
    _adapter(adapter),
    _state(StateHolding),
    _registeredWithPool(false)    
{
    DefaultsAndOverridesPtr defaultsAndOverrides = _instance->defaultsAndOverrides();
    if(defaultsAndOverrides->overrideTimeout)
    {
	_endpoint = _endpoint->timeout(defaultsAndOverrides->overrideTimeoutValue);
    }

    _warn = _instance->properties()->getPropertyAsInt("Ice.ConnectionWarnings") > 0;

    try
    {
	_transceiver = _endpoint->serverTransceiver(_endpoint);
	if(_transceiver)
	{
	    ConnectionPtr connection = new Connection(_instance, _transceiver, _endpoint, _adapter);
	    _connections.push_back(connection);

	    //
	    // We don't need an adapter anymore if we don't use an
	    // acceptor. So we break cyclic object dependency
	    // now. This is necessary, because the object adapter
	    // never clears the list of incoming connections it keeps.
	    //
	    _adapter = 0;
	}
	else
	{
	    _acceptor = _endpoint->acceptor(_endpoint);
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
    assert(!_adapter);
}

void
IceInternal::IncomingConnectionFactory::destroy()
{
    ::IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);
    setState(StateClosed);
}

void
IceInternal::IncomingConnectionFactory::waitUntilFinished()
{
    ::IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);

    while(_state != StateClosed || _adapter)
    {
	wait();
    }
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
	    if(_state != StateActive) // Can only switch from active to holding
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
	    _connections.clear();

	    break;
	}
    }

    _state = state;
}

void
IceInternal::IncomingConnectionFactory::registerWithPool()
{
    if(_acceptor)
    {
	if(!_registeredWithPool)
	{
	    if(!_serverThreadPool)
	    {
		_serverThreadPool = _instance->serverThreadPool();
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
