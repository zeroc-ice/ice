// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
#include <Ice/ConnectionFactory.h>
#include <Ice/RouterInfo.h>
#include <Ice/Exception.h>
#include <Ice/Properties.h>
#include <Ice/Functional.h>

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
    return _name; // _name is immutable
}

CommunicatorPtr
Ice::ObjectAdapterI::getCommunicator()
{
    return _instance->communicator(); // _instance is immutable
}

void
Ice::ObjectAdapterI::activate()
{
    IceUtil::Mutex::Lock sync(*this);

    if (_deactivated)
    {
	throw ObjectAdapterDeactivatedException(__FILE__, __LINE__);
    }

    for_each(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(),
	     Ice::voidMemFun(&IncomingConnectionFactory::activate));

    if (!_printAdapterReadyDone)
    {
	if (atoi(_instance->properties()->getProperty("Ice.PrintAdapterReady").c_str()) > 0)
	{
	    cout << _name << " ready" << endl;
	}

	_printAdapterReadyDone = true;
    }
}

void
Ice::ObjectAdapterI::hold()
{
    IceUtil::Mutex::Lock sync(*this);

    if (_deactivated)
    {
	throw ObjectAdapterDeactivatedException(__FILE__, __LINE__);
    }

    for_each(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(),
	     Ice::voidMemFun(&IncomingConnectionFactory::hold));
}

void
Ice::ObjectAdapterI::deactivate()
{
    IceUtil::Mutex::Lock sync(*this);

    if (_deactivated)
    {
	//
        // Ignore deactivation requests if the Object Adapter has
        // already been deactivated.
	//
	return;
    }

    for_each(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(),
	     Ice::voidMemFun(&IncomingConnectionFactory::destroy));
    _incomingConnectionFactories.clear();

    _instance->outgoingConnectionFactory()->removeAdapter(this);

    _activeServantMap.clear();
    _activeServantMapHint = _activeServantMap.end();

    for_each(_locatorMap.begin(), _locatorMap.end(),
	     Ice::secondVoidMemFun<string, ServantLocator>(&ServantLocator::deactivate));
    _locatorMap.clear();
    _locatorMapHint = _locatorMap.end();

    _deactivated = true;
}

ObjectPrx
Ice::ObjectAdapterI::add(const ObjectPtr& object, const Identity& ident)
{
    IceUtil::Mutex::Lock sync(*this);

    if (_deactivated)
    {
	throw ObjectAdapterDeactivatedException(__FILE__, __LINE__);
    }

    _activeServantMapHint = _activeServantMap.insert(_activeServantMapHint, make_pair(ident, object));

    return newProxy(ident);
}

ObjectPrx
Ice::ObjectAdapterI::addWithUUID(const ObjectPtr& object)
{
    IceUtil::Mutex::Lock sync(*this);

    if (_deactivated)
    {
	throw ObjectAdapterDeactivatedException(__FILE__, __LINE__);
    }

    Identity ident;
    ident.name = IceUtil::generateUUID();

    _activeServantMapHint = _activeServantMap.insert(_activeServantMapHint, make_pair(ident, object));

    return newProxy(ident);
}

void
Ice::ObjectAdapterI::remove(const Identity& ident)
{
    IceUtil::Mutex::Lock sync(*this);

    if (_deactivated)
    {
	throw ObjectAdapterDeactivatedException(__FILE__, __LINE__);
    }

    _activeServantMap.erase(ident);
    _activeServantMapHint = _activeServantMap.end();
}

void
Ice::ObjectAdapterI::addServantLocator(const ServantLocatorPtr& locator, const string& prefix)
{
    IceUtil::Mutex::Lock sync(*this);

    if (_deactivated)
    {
	throw ObjectAdapterDeactivatedException(__FILE__, __LINE__);
    }

    _locatorMapHint = _locatorMap.insert(_locatorMapHint, make_pair(prefix, locator));
}

void
Ice::ObjectAdapterI::removeServantLocator(const string& prefix)
{
    IceUtil::Mutex::Lock sync(*this);

    if (_deactivated)
    {
	throw ObjectAdapterDeactivatedException(__FILE__, __LINE__);
    }

    map<string, ServantLocatorPtr>::iterator p = _locatorMap.end();
    
    if (_locatorMapHint != _locatorMap.end())
    {
	if (_locatorMapHint->first == prefix)
	{
	    p = _locatorMapHint;
	}
    }
    
    if (p == _locatorMap.end())
    {
	p = _locatorMap.find(prefix);
    }
    
    if (p != _locatorMap.end())
    {
	p->second->deactivate();

	if (p == _locatorMapHint)
	{
	    _locatorMap.erase(p++);
	    _locatorMapHint = p;
	}
	else
	{
	    _locatorMap.erase(p);
	}
    }
}

ServantLocatorPtr
Ice::ObjectAdapterI::findServantLocator(const string& prefix)
{
    IceUtil::Mutex::Lock sync(*this);
    
    if (_deactivated)
    {
	throw ObjectAdapterDeactivatedException(__FILE__, __LINE__);
    }

    map<string, ServantLocatorPtr>::iterator p = _locatorMap.end();
    
    if (_locatorMapHint != _locatorMap.end())
    {
	if (_locatorMapHint->first == prefix)
	{
	    p = _locatorMapHint;
	}
    }
    
    if (p == _locatorMap.end())
    {
	p = _locatorMap.find(prefix);
    }
    
    if (p != _locatorMap.end())
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
    IceUtil::Mutex::Lock sync(*this);

    if (_activeServantMapHint != _activeServantMap.end())
    {
	if (_activeServantMapHint->first == ident)
	{
	    return _activeServantMapHint->second;
	}
    }
    
    ObjectDict::iterator p = _activeServantMap.find(ident);
    if (p != _activeServantMap.end())
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
    IceUtil::Mutex::Lock sync(*this);
    
    if (_deactivated)
    {
	throw ObjectAdapterDeactivatedException(__FILE__, __LINE__);
    }
    
    return newProxy(ident);
}

ObjectPrx
Ice::ObjectAdapterI::createReverseProxy(const Identity& ident)
{
    IceUtil::Mutex::Lock sync(*this);
    
    if (_deactivated)
    {
	throw ObjectAdapterDeactivatedException(__FILE__, __LINE__);
    }
    
    //
    // Create a reference and return a reverse proxy for this reference.
    //
    vector<EndpointPtr> endpoints;
    ReferencePtr ref = _instance->referenceFactory()->create(ident, "", Reference::ModeTwoway, false,
							     endpoints, endpoints, 0, this);
    return _instance->proxyFactory()->referenceToProxy(ref);
}

void
Ice::ObjectAdapterI::addRouter(const RouterPrx& router)
{
    IceUtil::Mutex::Lock sync(*this);
    
    if (_deactivated)
    {
	throw ObjectAdapterDeactivatedException(__FILE__, __LINE__);
    }

    RouterInfoPtr routerInfo = _instance->routerManager()->get(router);
    if (routerInfo)
    {
	//
	// Add the router's server proxy endpoints to this object
	// adapter.
	//
	ObjectPrx proxy = routerInfo->getServerProxy();
	copy(proxy->__reference()->endpoints.begin(), proxy->__reference()->endpoints.end(),
	     back_inserter(_routerEndpoints));
	sort(_routerEndpoints.begin(), _routerEndpoints.end()); // Must be sorted.
	_routerEndpoints.erase(unique(_routerEndpoints.begin(), _routerEndpoints.end()), _routerEndpoints.end());

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

list<ConnectionPtr>
Ice::ObjectAdapterI::getIncomingConnections() const
{
    IceUtil::Mutex::Lock sync(*this);

    list<ConnectionPtr> connections;

    vector<IncomingConnectionFactoryPtr>::const_iterator p;
    for (p = _incomingConnectionFactories.begin(); p != _incomingConnectionFactories.end(); ++p)
    {
	list<ConnectionPtr> cons = (*p)->connections();
	connections.splice(connections.end(), cons);
    }

    return connections;
}

Ice::ObjectAdapterI::ObjectAdapterI(const InstancePtr& instance, const string& name, const string& endpts) :
    _instance(instance),
    _deactivated(false),
    _printAdapterReadyDone(false),
    _name(name),
    _activeServantMapHint(_activeServantMap.end()),
    _locatorMapHint(_locatorMap.end())
{
    string s(endpts);
    transform(s.begin(), s.end(), s.begin(), tolower);

    __setNoDelete(true);
    try
    {
	string::size_type beg;
	string::size_type end = 0;

	while (end < s.length())
	{
	    static const string delim = " \t\n\r";
	    
	    beg = s.find_first_not_of(delim, end);
	    if (beg == string::npos)
	    {
		break;
	    }

	    end = s.find(':', beg);
	    if (end == string::npos)
	    {
		end = s.length();
	    }
	    
	    if (end == beg)
	    {
		break;
	    }
	    
	    string es = s.substr(beg, end - beg);
	   
	    //
	    // Don't store the endpoint in the adapter. The Collector
	    // might change it, for example, to fill in the real port
	    // number if a zero port number is given.
	    //
	    EndpointPtr endp = Endpoint::endpointFromString(instance, es);
	    _incomingConnectionFactories.push_back(new IncomingConnectionFactory(instance, endp, this));

	    end = end + 1;
	}
    }
    catch(...)
    {
	if (!_deactivated)
	{
	    deactivate();
	}
	__setNoDelete(false);
	throw;
    }
    __setNoDelete(false);
  
//
// Object Adapters without incoming connection factories are
// permissible, as such Object Adapters can still be used with a
// router. (See addRouter.)
//
/*
    if (_incomingConnectionFactories.empty())
    {
	throw EndpointParseException(__FILE__, __LINE__);
    }
*/
}

Ice::ObjectAdapterI::~ObjectAdapterI()
{
    if (!_deactivated)
    {
	deactivate();
    }
}

ObjectPrx
Ice::ObjectAdapterI::newProxy(const Identity& ident) const
{
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
    copy(_routerEndpoints.begin(), _routerEndpoints.end(), back_inserter(endpoints));
    
    //
    // Create a reference and return a proxy for this reference.
    //
    ReferencePtr ref = _instance->referenceFactory()->create(ident, "", Reference::ModeTwoway, false,
							     endpoints, endpoints, 0, 0);
    return _instance->proxyFactory()->referenceToProxy(ref);
}

bool
Ice::ObjectAdapterI::isLocal(const ObjectPrx& proxy) const
{
    ReferencePtr ref = proxy->__reference();
    vector<EndpointPtr>::const_iterator p;

    //
    // Proxies which have at least one endpoint in common with the
    // endpoints used by this object adapter's incoming connection
    // factories are considered local.
    //
    for (p = ref->endpoints.begin(); p != ref->endpoints.end(); ++p)
    {
	vector<IncomingConnectionFactoryPtr>::const_iterator q;
	for (q = _incomingConnectionFactories.begin(); q != _incomingConnectionFactories.end(); ++q)
	{
	    if ((*q)->equivalent(*p))
	    {
		return true;
	    }
	}
    }

    //
    // Proxies which have at least one endpoint in common with the
    // router's server proxy endpoints (if any), are also considered
    // local.
    //
    for (p = ref->endpoints.begin(); p != ref->endpoints.end(); ++p)
    {
	if (binary_search(_routerEndpoints.begin(), _routerEndpoints.end(), *p)) // _routerEndpoints is sorted.
	{
	    return true;
	}
    }

    return false;
}
