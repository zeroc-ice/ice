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

#include <IceUtil/UUID.h>
#include <Ice/ObjectAdapterI.h>
#include <Ice/ServantLocator.h>
#include <Ice/Instance.h>
#include <Ice/Proxy.h>
#include <Ice/ProxyFactory.h>
#include <Ice/ReferenceFactory.h>
#include <Ice/Endpoint.h>
#include <Ice/EndpointFactoryManager.h>
#include <Ice/ConnectionFactory.h>
#include <Ice/RouterInfo.h>
#include <Ice/LocalException.h>
#include <Ice/Properties.h>
#include <Ice/Functional.h>
#include <Ice/LocatorInfo.h>
#include <Ice/Locator.h>
#include <Ice/LoggerUtil.h>
#include <Ice/IdentityUtil.h>

#ifdef _WIN32
#   include <sys/timeb.h>
#else
#   include <sys/time.h>
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;

string
Ice::ObjectAdapterI::getName()
{
    //
    // No mutex lock necessary, _name is immutable.
    //
    return _name;
}

CommunicatorPtr
Ice::ObjectAdapterI::getCommunicator()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    checkForDeactivation();

    return _communicator;
}

void
Ice::ObjectAdapterI::activate()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    checkForDeactivation();
    
    if(!_printAdapterReadyDone)
    {
	if(_locatorInfo && !_id.empty())
	{
	    Identity ident;
	    ident.name = "dummy";
	    
	    //
	    // TODO: This might throw if we can't connect to the
	    // locator. Shall we raise a special exception for the
	    // activate operation instead of a non obvious network
	    // exception?
	    //
	    try
	    {
		_locatorInfo->getLocatorRegistry()->setAdapterDirectProxy(_id, newDirectProxy(ident));
	    }
	    catch(const Ice::AdapterNotFoundException&)
	    {
		NotRegisteredException ex(__FILE__, __LINE__);
		ex.kindOfObject = "object adapter";
		ex.id = _id;
		throw ex;
	    }
	    catch(const Ice::AdapterAlreadyActiveException&)
	    {
		ObjectAdapterIdInUseException ex(__FILE__, __LINE__);
		ex.id = _id;
		throw ex;
	    }
	}
    }

    for_each(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(),
	     Ice::voidMemFun(&IncomingConnectionFactory::activate));

    if(!_printAdapterReadyDone)
    {
	if(_instance->properties()->getPropertyAsInt("Ice.PrintAdapterReady") > 0)
	{
	    cout << _name << " ready" << endl;
	}
	
	_printAdapterReadyDone = true;
    }
}

void
Ice::ObjectAdapterI::hold()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    checkForDeactivation();
	
    for_each(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(),
	     Ice::voidMemFun(&IncomingConnectionFactory::hold));
}
    
void
Ice::ObjectAdapterI::waitForHold()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    checkForDeactivation();

    for_each(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(),
	     Ice::constVoidMemFun(&IncomingConnectionFactory::waitUntilHolding));
}

void
Ice::ObjectAdapterI::deactivate()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    
    //
    // Ignore deactivation requests if the object adapter has already
    // been deactivated.
    //
    if(!_instance)
    {
	return;
    }
    
    for_each(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(),
	     Ice::voidMemFun(&IncomingConnectionFactory::destroy));
    
    _instance->outgoingConnectionFactory()->removeAdapter(this);
    
    _instance = 0;
    _communicator = 0;

    notifyAll();
}

void
Ice::ObjectAdapterI::waitForDeactivate()
{
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

	//
	// First we wait for deactivation of the adapter itself, and for
	// the return of all direct method calls using this adapter.
	//
	while(_instance || _directCount > 0)
	{
	    wait();
	}

	//
	// If some other thread is currently deactivating, we wait
	// until this thread is finished.
	//
	while(_waitForDeactivate)
	{
	    wait();
	}
	_waitForDeactivate = true;
    }

    //
    // Now we wait for until all incoming connection factories are
    // finished.
    //
    for_each(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(),
	     Ice::voidMemFun(&IncomingConnectionFactory::waitUntilFinished));
    
    //
    // We're done, now we can throw away all incoming connection
    // factories.
    //
    _incomingConnectionFactories.clear();

    //
    // Now it's also time to clean up the active servant map.
    //
    _activeServantMap.clear();
    _activeServantMapHint = _activeServantMap.end();

    //
    // And the servant locators, too.
    //
    for(map<string, ServantLocatorPtr>::iterator p = _locatorMap.begin(); p != _locatorMap.end(); ++p)
    {
	try
	{
	    p->second->deactivate();
	}
	catch(const Exception& ex)
	{
	    Error out(_logger);
	    out << "exception during locator deactivation:\n"
		<< "object adapter: `" << _name << "'\n"
		<< "locator prefix: `" << p->first << "'\n"
		<< ex;
	}
	catch(...)
	{
	    Error out(_logger);
	    out << "unknown exception during locator deactivation:\n"
		<< "object adapter: `" << _name << "'\n"
		<< "locator prefix: `" << p->first << "'";
	}
    }

    _locatorMap.clear();
    _locatorMapHint = _locatorMap.end();

    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

	//
	// Signal that waiting is complete.
	//
	_waitForDeactivate = false;
	notifyAll();
    }
}

ObjectPrx
Ice::ObjectAdapterI::add(const ObjectPtr& object, const Identity& ident)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    checkForDeactivation();
    checkIdentity(ident);

    if((_activeServantMapHint != _activeServantMap.end() && _activeServantMapHint->first == ident)
       || _activeServantMap.find(ident) != _activeServantMap.end())
    {
	AlreadyRegisteredException ex(__FILE__, __LINE__);
	ex.kindOfObject = "servant";
	ex.id = identityToString(ident);
	throw ex;
    }

    _activeServantMapHint = _activeServantMap.insert(_activeServantMapHint, make_pair(ident, object));

    return newProxy(ident);
}

ObjectPrx
Ice::ObjectAdapterI::addWithUUID(const ObjectPtr& object)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    checkForDeactivation();

    Identity ident;
    ident.name = IceUtil::generateUUID();

    _activeServantMapHint = _activeServantMap.insert(_activeServantMapHint, make_pair(ident, object));

    return newProxy(ident);
}

void
Ice::ObjectAdapterI::remove(const Identity& ident)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    checkForDeactivation();
    checkIdentity(ident);

    ObjectDict::iterator p = _activeServantMap.find(ident);
    if(p == _activeServantMap.end())
    {
	NotRegisteredException ex(__FILE__, __LINE__);
	ex.kindOfObject = "servant";
	ex.id = identityToString(ident);
	throw ex;
    }

    _activeServantMap.erase(p);
    _activeServantMapHint = _activeServantMap.end();
}

void
Ice::ObjectAdapterI::addServantLocator(const ServantLocatorPtr& locator, const string& prefix)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    checkForDeactivation();

    if((_locatorMapHint != _locatorMap.end() && _locatorMapHint->first == prefix)
       || _locatorMap.find(prefix) != _locatorMap.end())
    {
	AlreadyRegisteredException ex(__FILE__, __LINE__);
	ex.kindOfObject = "servant locator";
	ex.id = prefix;
	throw ex;
    }

    _locatorMapHint = _locatorMap.insert(_locatorMapHint, make_pair(prefix, locator));
}

void
Ice::ObjectAdapterI::removeServantLocator(const string& prefix)
{
    ServantLocatorPtr locator;

    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

	checkForDeactivation();

	map<string, ServantLocatorPtr>::iterator p = _locatorMap.end();
    
	if(_locatorMapHint != _locatorMap.end())
	{
	    if(_locatorMapHint->first == prefix)
	    {
		p = _locatorMapHint;
	    }
	}
    
	if(p == _locatorMap.end())
	{
	    p = _locatorMap.find(prefix);
	    if (p == _locatorMap.end())
	    {
		NotRegisteredException ex(__FILE__, __LINE__);
		ex.kindOfObject = "servant locator";
		ex.id = prefix;
		throw ex;
	    }
	}
	assert(p != _locatorMap.end());
    
	locator = p->second;

	if(p == _locatorMapHint)
	{
	    _locatorMap.erase(p++);
	    _locatorMapHint = p;
	}
	else
	{
	    _locatorMap.erase(p);
	}
    }

    locator->deactivate();
}

ServantLocatorPtr
Ice::ObjectAdapterI::findServantLocator(const string& prefix)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    
    checkForDeactivation();

    map<string, ServantLocatorPtr>::iterator p = _locatorMap.end();
    
    if(_locatorMapHint != _locatorMap.end())
    {
	if(_locatorMapHint->first == prefix)
	{
	    p = _locatorMapHint;
	}
    }
    
    if(p == _locatorMap.end())
    {
	p = _locatorMap.find(prefix);
    }
    
    if(p != _locatorMap.end())
    {
	_locatorMapHint = p;
	return p->second;
    }
    else
    {
	return 0;
    }
}

ObjectPtr
Ice::ObjectAdapterI::identityToServant(const Identity& ident)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    checkForDeactivation();
    
    //
    // Don't call checkIdentity. We simply want null to returned
    // (e.g., for Direct, Incoming) in case the identity is incorrect
    // and therefore no servant can be found.
    //
    /*
    checkIdentity(ident);
    */

    if(_activeServantMapHint != _activeServantMap.end())
    {
	if(_activeServantMapHint->first == ident)
	{
	    return _activeServantMapHint->second;
	}
    }
    
    ObjectDict::iterator p = _activeServantMap.find(ident);
    if(p != _activeServantMap.end())
    {
	_activeServantMapHint = p;
	return p->second;
    }
    else
    {
	return 0;
    }
}

ObjectPtr
Ice::ObjectAdapterI::proxyToServant(const ObjectPrx& proxy)
{
    ReferencePtr ref = proxy->__reference();
    return identityToServant(ref->identity);
}

ObjectPrx
Ice::ObjectAdapterI::createProxy(const Identity& ident)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    checkForDeactivation();
    checkIdentity(ident);

    return newProxy(ident);
}

ObjectPrx
Ice::ObjectAdapterI::createDirectProxy(const Identity& ident)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    
    checkForDeactivation();
    checkIdentity(ident);

    return newDirectProxy(ident);
}

ObjectPrx
Ice::ObjectAdapterI::createReverseProxy(const Identity& ident)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    
    checkForDeactivation();
    checkIdentity(ident);

    //
    // Create a reference and return a reverse proxy for this reference.
    //
    vector<EndpointPtr> endpoints;
    ReferencePtr ref = _instance->referenceFactory()->create(ident, vector<string>(), Reference::ModeTwoway,
							     false, false, "", endpoints, 0, 0, this, true);
    return _instance->proxyFactory()->referenceToProxy(ref);
}

void
Ice::ObjectAdapterI::addRouter(const RouterPrx& router)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    
    checkForDeactivation();

    RouterInfoPtr routerInfo = _instance->routerManager()->get(router);
    if(routerInfo)
    {
	//
	// Add the router's server proxy endpoints to this object
	// adapter.
	//
	ObjectPrx proxy = routerInfo->getServerProxy();

	{
	    IceUtil::Mutex::Lock routerEndpointsSync(_routerEndpointsMutex);
	    copy(proxy->__reference()->endpoints.begin(), proxy->__reference()->endpoints.end(),
		 back_inserter(_routerEndpoints));
	    sort(_routerEndpoints.begin(), _routerEndpoints.end()); // Must be sorted.
	    _routerEndpoints.erase(unique(_routerEndpoints.begin(), _routerEndpoints.end()), _routerEndpoints.end());
	}

	//
	// Associate this object adapter with the router. This way,
	// new outgoing connections to the router's client proxy will
	// use this object adapter for callbacks.
	//
	routerInfo->setAdapter(this);

	//
	// Also modify all existing outgoing connections to the
	// router's client proxy to use this object adapter for
	// callbacks.
	//	
	_instance->outgoingConnectionFactory()->setRouter(routerInfo->getRouter());
    }
}

void
Ice::ObjectAdapterI::setLocator(const LocatorPrx& locator)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    
    checkForDeactivation();

    _locatorInfo = _instance->locatorManager()->get(locator);
}

list<ConnectionPtr>
Ice::ObjectAdapterI::getIncomingConnections() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    checkForDeactivation();

    list<ConnectionPtr> connections;
    vector<IncomingConnectionFactoryPtr>::const_iterator p;
    for(p = _incomingConnectionFactories.begin(); p != _incomingConnectionFactories.end(); ++p)
    {
	list<ConnectionPtr> cons = (*p)->connections();
	connections.splice(connections.end(), cons);
    }
    return connections;
}

void
Ice::ObjectAdapterI::incDirectCount()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
 
    checkForDeactivation();

    assert(_directCount >= 0);
    ++_directCount;
}

void
Ice::ObjectAdapterI::decDirectCount()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    //
    // The object adapter may already have been deactivated when the
    // direct count is decremented, thus there is no check for prior
    // deactivation.
    //

    assert(_directCount > 0);
    if(--_directCount == 0)
    {
	notifyAll();
    }    
}

Ice::ObjectAdapterI::ObjectAdapterI(const InstancePtr& instance, const CommunicatorPtr& communicator,
				    const string& name, const string& endpts, const string& id) :
    _instance(instance),
    _communicator(communicator),
    _printAdapterReadyDone(false),
    _name(name),
    _id(id),
    _logger(instance->logger()),
    _activeServantMapHint(_activeServantMap.end()),
    _locatorMapHint(_locatorMap.end()),
    _directCount(0),
    _waitForDeactivate(false)
{
    string s(endpts);
    transform(s.begin(), s.end(), s.begin(), ::tolower);

    __setNoDelete(true);
    try
    {
	string::size_type beg;
	string::size_type end = 0;

	while(end < s.length())
	{
	    const string delim = " \t\n\r";
	    
	    beg = s.find_first_not_of(delim, end);
	    if(beg == string::npos)
	    {
		break;
	    }

	    end = s.find(':', beg);
	    if(end == string::npos)
	    {
		end = s.length();
	    }
	    
	    if(end == beg)
	    {
		break;
	    }
	    
	    string es = s.substr(beg, end - beg);
	   
	    //
	    // Don't store the endpoint in the adapter. The Collector
	    // might change it, for example, to fill in the real port
	    // number if a zero port number is given.
	    //
	    EndpointPtr endp = _instance->endpointFactoryManager()->create(es);
	    _incomingConnectionFactories.push_back(new IncomingConnectionFactory(instance, endp, this));

	    ++end;
	}
    }
    catch(...)
    {
	deactivate();
	__setNoDelete(false);
	throw;
    }
    __setNoDelete(false);  
}

Ice::ObjectAdapterI::~ObjectAdapterI()
{
    if(_instance)
    {
	Warning out(_instance->logger());
	out << "object adapter `" << _name << "' has not been deactivated";
    }
    else
    {
	assert(!_communicator);
	assert(_incomingConnectionFactories.empty());
	assert(_activeServantMap.empty());
	assert(_locatorMap.empty());
	assert(_directCount == 0);
	assert(!_waitForDeactivate);
    }
}

ObjectPrx
Ice::ObjectAdapterI::newProxy(const Identity& ident) const
{
    checkForDeactivation();

    if(_id.empty())
    {
	return newDirectProxy(ident);
    }
    else
    {
	//
	// Create a reference with the adapter id.
	//
	vector<EndpointPtr> endpoints;
	ReferencePtr ref = _instance->referenceFactory()->create(ident, vector<string>(), Reference::ModeTwoway,
								 false, false, _id, endpoints, 0, 0, 0, true);

	//
	// Return a proxy for the reference. 
	//
	return _instance->proxyFactory()->referenceToProxy(ref);
    }
}

ObjectPrx
Ice::ObjectAdapterI::newDirectProxy(const Identity& ident) const
{
    checkForDeactivation();

    vector<EndpointPtr> endpoints;

    // 
    // First we add all endpoints from all incoming connection
    // factories.
    //
    transform(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(), back_inserter(endpoints),
	      Ice::constMemFun(&IncomingConnectionFactory::endpoint));
    
    //
    // Now we also add the endpoints of the router's server proxy, if
    // any. This way, object references created by this object adapter
    // will also point to the router's server proxy endpoints.
    //
    {
	IceUtil::Mutex::Lock routerEndpointsSync(_routerEndpointsMutex);
	copy(_routerEndpoints.begin(), _routerEndpoints.end(), back_inserter(endpoints));
    }
    
    //
    // Create a reference and return a proxy for this reference.
    //
    ReferencePtr ref = _instance->referenceFactory()->create(ident, vector<string>(), Reference::ModeTwoway,
							     false, false, "", endpoints, 0, 0, 0, true);
    return _instance->proxyFactory()->referenceToProxy(ref);

}

bool
Ice::ObjectAdapterI::isLocal(const ObjectPrx& proxy) const
{
    checkForDeactivation();

    ReferencePtr ref = proxy->__reference();
    vector<EndpointPtr>::const_iterator p;

    if(!ref->adapterId.empty())
    {
	//
	// Proxy is local if the reference adapter id matches this
	// adapter id.
	//
	return ref->adapterId == _id;
    }

    //
    // Proxies which have at least one endpoint in common with the
    // endpoints used by this object adapter's incoming connection
    // factories are considered local.
    //
    for(p = ref->endpoints.begin(); p != ref->endpoints.end(); ++p)
    {
	vector<IncomingConnectionFactoryPtr>::const_iterator q;
	for(q = _incomingConnectionFactories.begin(); q != _incomingConnectionFactories.end(); ++q)
	{
	    if((*q)->equivalent(*p))
	    {
		return true;
	    }
	}
    }

    {
	IceUtil::Mutex::Lock routerEndpointsSync(_routerEndpointsMutex);
	
	//
	// Proxies which have at least one endpoint in common with the
	// router's server proxy endpoints (if any), are also considered
	// local.
	//
	for(p = ref->endpoints.begin(); p != ref->endpoints.end(); ++p)
	{
	    if(binary_search(_routerEndpoints.begin(), _routerEndpoints.end(), *p)) // _routerEndpoints is sorted.
	    {
		return true;
	    }
	}
    }
	
    return false;
}

void
Ice::ObjectAdapterI::checkForDeactivation() const
{
    if(!_instance)
    {
	ObjectAdapterDeactivatedException ex(__FILE__, __LINE__);
	ex.name = _name;
	throw ex;
    }
}

void
Ice::ObjectAdapterI::checkIdentity(const Identity& ident)
{
    if(ident.name.size() == 0)
    {
        IllegalIdentityException e(__FILE__, __LINE__);
        e.id = ident;
        throw e;
    }
}
