// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/ObjectAdapter.h>
#include <IceE/UUID.h>
#include <IceE/Instance.h>
#include <IceE/ProxyFactory.h>
#include <IceE/ReferenceFactory.h>
#include <IceE/EndpointFactory.h>
#include <IceE/IncomingConnectionFactory.h>
#include <IceE/OutgoingConnectionFactory.h>
#include <IceE/ServantManager.h>
#include <IceE/LocalException.h>
#include <IceE/Properties.h>
#include <IceE/Functional.h>
#ifndef ICEE_NO_LOCATOR
#    include <IceE/LocatorInfo.h>
#    include <IceE/Locator.h>
#endif
#ifndef ICEE_NO_ROUTER
#    include <IceE/RouterInfo.h>
#    include <IceE/Router.h>
#    include <IceE/Endpoint.h>
#endif
#include <IceE/LoggerUtil.h>

#include <ctype.h>

using namespace std;
using namespace IceE;
using namespace IceEInternal;

bool
IceE::operator==(const ::IceE::ObjectAdapter& l, const ::IceE::ObjectAdapter& r)
{
    return l == r;
}

bool
IceE::operator!=(const ::IceE::ObjectAdapter& l, const ::IceE::ObjectAdapter& r)
{
    return l != r;
}

bool
IceE::operator<(const ::IceE::ObjectAdapter& l, const ::IceE::ObjectAdapter& r)
{
    return l < r;
}

void
IceEInternal::incRef(::IceE::ObjectAdapter* p)
{
    p->__incRef();
}

void
IceEInternal::decRef(::IceE::ObjectAdapter* p)
{
    p->__decRef();
}

string
IceE::ObjectAdapter::getName() const
{
    //
    // No mutex lock necessary, _name is immutable.
    //
    return _name;
}

CommunicatorPtr
IceE::ObjectAdapter::getCommunicator() const
{
    IceE::Monitor<IceE::RecMutex>::Lock sync(*this);

    checkForDeactivation();

    return _communicator;
}

void
IceE::ObjectAdapter::activate()
{
#ifndef ICEE_NO_LOCATOR
    LocatorRegistryPrx locatorRegistry;
#endif
    bool printAdapterReady = false;

    {    
	IceE::Monitor<IceE::RecMutex>::Lock sync(*this);
	
	checkForDeactivation();

	if(!_printAdapterReadyDone)
	{
#ifndef ICEE_NO_LOCATOR
	    if(_locatorInfo && !_id.empty())
	    {
		locatorRegistry = _locatorInfo->getLocatorRegistry();
	    }
#endif
	    printAdapterReady = _instance->properties()->getPropertyAsInt("IceE.PrintAdapterReady") > 0;
	    _printAdapterReadyDone = true;
	}
	
	for_each(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(),
		 IceE::voidMemFun(&IncomingConnectionFactory::activate));	
    }

    //
    // We must call on the locator registry outside the thread
    // synchronization, to avoid deadlocks.
    //
#ifndef ICEE_NO_LOCATOR
    if(locatorRegistry)
    {
	//
	// TODO: This might throw if we can't connect to the
	// locator. Shall we raise a special exception for the
	// activate operation instead of a non obvious network
	// exception?
	//
	try
	{
	    Identity ident;
	    ident.name = "dummy";
	    locatorRegistry->setAdapterDirectProxy(_id, createDirectProxy(ident));
	}
	catch(const ObjectAdapterDeactivatedException&)
	{
	    // IGNORE: The object adapter is already inactive.
	}
	catch(const AdapterNotFoundException&)
	{
	    NotRegisteredException ex(__FILE__, __LINE__);
	    ex.kindOfObject = "object adapter";
	    ex.id = _id;
	    throw ex;
	}
	catch(const AdapterAlreadyActiveException&)
	{
	    ObjectAdapterIdInUseException ex(__FILE__, __LINE__);
	    ex.id = _id;
	    throw ex;
	}
    }
#endif

    if(printAdapterReady)
    {
	printf("%s ready\n", _name.c_str());
	fflush(stdout);
    }
}

void
IceE::ObjectAdapter::hold()
{
    IceE::Monitor<IceE::RecMutex>::Lock sync(*this);

    checkForDeactivation();
	
    for_each(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(),
	     IceE::voidMemFun(&IncomingConnectionFactory::hold));
}
    
void
IceE::ObjectAdapter::waitForHold()
{
    IceE::Monitor<IceE::RecMutex>::Lock sync(*this);

    checkForDeactivation();

    for_each(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(),
	     IceE::constVoidMemFun(&IncomingConnectionFactory::waitUntilHolding));
}

void
IceE::ObjectAdapter::deactivate()
{
    vector<IncomingConnectionFactoryPtr> incomingConnectionFactories;
    OutgoingConnectionFactoryPtr outgoingConnectionFactory;

    {
	IceE::Monitor<IceE::RecMutex>::Lock sync(*this);
	
	//
	// Ignore deactivation requests if the object adapter has already
	// been deactivated.
	//
	if(_deactivated)
	{
	    return;
	}

        incomingConnectionFactories = _incomingConnectionFactories;
	outgoingConnectionFactory = _instance->outgoingConnectionFactory();
    
	_deactivated = true;
	
	notifyAll();
    }

    //
    // Must be called outside the thread synchronization, because
    // Connection::destroy() might block when sending a CloseConnection
    // message.
    //
    for_each(incomingConnectionFactories.begin(), incomingConnectionFactories.end(),
	     IceE::voidMemFun(&IncomingConnectionFactory::destroy));
    
    //
    // Must be called outside the thread synchronization, because
    // changing the object adapter might block if there are still
    // requests being dispatched.
    //
    outgoingConnectionFactory->removeAdapter(this);
}

void
IceE::ObjectAdapter::waitForDeactivate()
{
    {
	IceE::Monitor<IceE::RecMutex>::Lock sync(*this);

	//
	// First we wait for deactivation of the adapter itself, and for
	// the return of all direct method calls using this adapter.
	//
	while(!_deactivated || _directCount > 0)
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
    // Now we wait until all incoming connection factories are
    // finished.
    //
    for_each(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(),
	     IceE::voidMemFun(&IncomingConnectionFactory::waitUntilFinished));

    //
    // Now it's also time to clean up our servants and servant
    // locators.
    //
    if(_servantManager)
    {
	_servantManager->destroy();
    }

    {
	IceE::Monitor<IceE::RecMutex>::Lock sync(*this);

	//
	// Signal that waiting is complete.
	//
	_waitForDeactivate = false;
	notifyAll();

	//
	// We're done, now we can throw away all incoming connection
	// factories.
	//
	_incomingConnectionFactories.clear();

	//
	// Remove object references (some of them cyclic).
	//
	_instance = 0;
	_servantManager = 0;
	_communicator = 0;
    }
}

ObjectPrx
IceE::ObjectAdapter::add(const ObjectPtr& object, const Identity& ident)
{
    return addFacet(object, ident, "");
}

ObjectPrx
IceE::ObjectAdapter::addFacet(const ObjectPtr& object, const Identity& ident, const string& facet)
{
    IceE::Monitor<IceE::RecMutex>::Lock sync(*this);

    checkForDeactivation();
    checkIdentity(ident);

    _servantManager->addServant(object, ident, facet);

    return newProxy(ident, facet);
}

ObjectPrx
IceE::ObjectAdapter::addWithUUID(const ObjectPtr& object)
{
    return addFacetWithUUID(object, "");
}

ObjectPrx
IceE::ObjectAdapter::addFacetWithUUID(const ObjectPtr& object, const string& facet)
{
    Identity ident;
    ident.name = IceE::generateUUID();
    return addFacet(object, ident, facet);
}

ObjectPtr
IceE::ObjectAdapter::remove(const Identity& ident)
{
    return removeFacet(ident, "");
}

ObjectPtr
IceE::ObjectAdapter::removeFacet(const Identity& ident, const string& facet)
{
    IceE::Monitor<IceE::RecMutex>::Lock sync(*this);

    checkForDeactivation();
    checkIdentity(ident);

    return _servantManager->removeServant(ident, facet);
}

FacetMap
IceE::ObjectAdapter::removeAllFacets(const Identity& ident)
{
    IceE::Monitor<IceE::RecMutex>::Lock sync(*this);

    checkForDeactivation();
    checkIdentity(ident);

    return _servantManager->removeAllFacets(ident);
}

ObjectPtr
IceE::ObjectAdapter::find(const Identity& ident) const
{
    return findFacet(ident, "");
}

ObjectPtr
IceE::ObjectAdapter::findFacet(const Identity& ident, const string& facet) const
{
    IceE::Monitor<IceE::RecMutex>::Lock sync(*this);

    checkForDeactivation();
    checkIdentity(ident);

    return _servantManager->findServant(ident, facet);
}

FacetMap
IceE::ObjectAdapter::findAllFacets(const Identity& ident) const
{
    IceE::Monitor<IceE::RecMutex>::Lock sync(*this);

    checkForDeactivation();
    checkIdentity(ident);

    return _servantManager->findAllFacets(ident);
}

ObjectPtr
IceE::ObjectAdapter::findByProxy(const ObjectPrx& proxy) const
{
    IceE::Monitor<IceE::RecMutex>::Lock sync(*this);

    checkForDeactivation();

    ReferencePtr ref = proxy->__reference();
    return findFacet(ref->getIdentity(), ref->getFacet());
}

ObjectPrx
IceE::ObjectAdapter::createProxy(const Identity& ident) const
{
    IceE::Monitor<IceE::RecMutex>::Lock sync(*this);

    checkForDeactivation();
    checkIdentity(ident);

    return newProxy(ident, "");
}

ObjectPrx
IceE::ObjectAdapter::createDirectProxy(const Identity& ident) const
{
    IceE::Monitor<IceE::RecMutex>::Lock sync(*this);
    
    checkForDeactivation();
    checkIdentity(ident);

    return newDirectProxy(ident, "");
}

ObjectPrx
IceE::ObjectAdapter::createReverseProxy(const Identity& ident) const
{
    IceE::Monitor<IceE::RecMutex>::Lock sync(*this);
    
    checkForDeactivation();
    checkIdentity(ident);

    //
    // Get all incoming connections for this object adapter.
    //
    vector<ConnectionPtr> connections;
    vector<IncomingConnectionFactoryPtr>::const_iterator p;
    for(p = _incomingConnectionFactories.begin(); p != _incomingConnectionFactories.end(); ++p)
    {
	list<ConnectionPtr> cons = (*p)->connections();
	copy(cons.begin(), cons.end(), back_inserter(connections));
    }

    //
    // Create a reference and return a reverse proxy for this
    // reference.
    //
    vector<EndpointPtr> endpoints;
    ReferencePtr ref = _instance->referenceFactory()->create(ident, Context(), "", Reference::ModeTwoway, connections);
    return _instance->proxyFactory()->referenceToProxy(ref);
}


void
IceE::ObjectAdapter::addRouter(const RouterPrx& router)
{
#ifndef ICEE_NO_ROUTER
    IceE::Monitor<IceE::RecMutex>::Lock sync(*this);
    
    checkForDeactivation();

    RouterInfoPtr routerInfo = _instance->routerManager()->get(router);
    if(routerInfo)
    {
	//
	// Add the router's server proxy endpoints to this object
	// adapter.
	//
	ObjectPrx proxy = routerInfo->getServerProxy();
	vector<EndpointPtr> endpoints = proxy->__reference()->getEndpoints();
	copy(endpoints.begin(), endpoints.end(), back_inserter(_routerEndpoints));
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
#endif
}



void
IceE::ObjectAdapter::setLocator(const LocatorPrx& locator)
{
#ifndef ICEE_NO_LOCATOR
    IceE::Monitor<IceE::RecMutex>::Lock sync(*this);
    
    checkForDeactivation();

    _locatorInfo = _instance->locatorManager()->get(locator);
#endif
}

bool
IceE::ObjectAdapter::isLocal(const ObjectPrx& proxy) const
{
    IceE::Monitor<IceE::RecMutex>::Lock sync(*this);

    checkForDeactivation();

    ReferencePtr ref = proxy->__reference();
    vector<EndpointPtr>::const_iterator p;

#ifndef ICEE_NO_LOCATOR
    IndirectReferencePtr ir = IndirectReferencePtr::dynamicCast(ref);
    if(ir)
    {
	if(!ir->getAdapterId().empty())
	{
	    //
	    // Proxy is local if the reference adapter id matches this
	    // adapter id.
	    //
	    return ir->getAdapterId() == _id;
	}
	return false;
    }
#endif

    //
    // Proxies which have at least one endpoint in common with the
    // endpoints used by this object adapter's incoming connection
    // factories are considered local.
    //
    vector<EndpointPtr> endpoints = ref->getEndpoints();
    for(p = endpoints.begin(); p != endpoints.end(); ++p)
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

    //
    // Proxies which have at least one endpoint in common with the
    // router's server proxy endpoints (if any), are also considered
    // local.
    //
#ifndef ICEE_NO_ROUTER
    for(p = endpoints.begin(); p != endpoints.end(); ++p)
    {
	if(binary_search(_routerEndpoints.begin(), _routerEndpoints.end(), *p)) // _routerEndpoints is sorted.
	{
	    return true;
	}
    }
#endif
	
    return false;
}

void
IceE::ObjectAdapter::flushBatchRequests()
{
    vector<IncomingConnectionFactoryPtr> f;
    {
	IceE::Monitor<IceE::RecMutex>::Lock sync(*this);
	f = _incomingConnectionFactories;
    }
    for_each(f.begin(), f.end(), IceE::voidMemFun(&IncomingConnectionFactory::flushBatchRequests));
}

void
IceE::ObjectAdapter::incDirectCount()
{
    IceE::Monitor<IceE::RecMutex>::Lock sync(*this);
 
    checkForDeactivation();

    assert(_directCount >= 0);
    ++_directCount;
}

void
IceE::ObjectAdapter::decDirectCount()
{
    IceE::Monitor<IceE::RecMutex>::Lock sync(*this);

    // Not check for deactivation here!

    assert(_instance); // Must not be called after waitForDeactivate().

    assert(_directCount > 0);
    if(--_directCount == 0)
    {
	notifyAll();
    }    
}

ServantManagerPtr
IceE::ObjectAdapter::getServantManager() const
{
    // No mutex lock necessary, _instance is
    // immutable after creation until it is removed in
    // waitForDeactivate().

    // Not check for deactivation here!

    assert(_instance); // Must not be called after waitForDeactivate().

    return _servantManager;
}

IceE::ObjectAdapter::ObjectAdapter(const InstancePtr& instance, const CommunicatorPtr& communicator,
				    const string& name) :
    _deactivated(false),
    _instance(instance),
    _communicator(communicator),
    _servantManager(new ServantManager(instance, name)),
    _printAdapterReadyDone(false),
    _name(name),
    _id(instance->properties()->getProperty(name + ".AdapterId")),
    _directCount(0),
    _waitForDeactivate(false)
{
    __setNoDelete(true);
    try
    {
	//
	// Parse the endpoints, but don't store them in the adapter.
	// The connection factory might change it, for example, to
	// fill in the real port number.
	//
	string endpts = _instance->properties()->getProperty(name + ".Endpoints");
	vector<EndpointPtr> endpoints = parseEndpoints(endpts);
	for(vector<EndpointPtr>::iterator p = endpoints.begin(); p != endpoints.end(); ++p)
	{
	    _incomingConnectionFactories.push_back(new IncomingConnectionFactory(_instance, *p, this));
	}

	//
	// Parse published endpoints. These are used in proxies
	// instead of the connection factory endpoints.
	//
	endpts = _instance->properties()->getProperty(name + ".PublishedEndpoints");
	_publishedEndpoints = parseEndpoints(endpts);

#ifndef ICEE_NO_ROUTER
	string router = _instance->properties()->getProperty(_name + ".Router");
	if(!router.empty())
	{
	    addRouter(RouterPrx::uncheckedCast(_instance->proxyFactory()->stringToProxy(router)));
	}
#endif
	
#ifndef ICEE_NO_LOCATOR
	string locator = _instance->properties()->getProperty(_name + ".Locator");
	if(!locator.empty())
	{
	    setLocator(LocatorPrx::uncheckedCast(_instance->proxyFactory()->stringToProxy(locator)));
	}
	else
	{
	    setLocator(_instance->referenceFactory()->getDefaultLocator());
	}
#endif
    }
    catch(...)
    {
	deactivate();
	waitForDeactivate();
	__setNoDelete(false);
	throw;
    }
    __setNoDelete(false);  
}

IceE::ObjectAdapter::~ObjectAdapter()
{
    if(!_deactivated)
    {
	Warning out(_instance->logger());
	out << "object adapter `" << _name << "' has not been deactivated";
    }
    else if(_instance)
    {
	Warning out(_instance->logger());
	out << "object adapter `" << _name << "' deactivation had not been waited for";
    }
    else
    {
	assert(!_servantManager);
	assert(!_communicator);
	assert(_incomingConnectionFactories.empty());
	assert(_directCount == 0);
	assert(!_waitForDeactivate);
    }
}

ObjectPrx
IceE::ObjectAdapter::newProxy(const Identity& ident, const string& facet) const
{
#ifndef ICEE_NO_LOCATOR
    if(_id.empty())
    {
#endif
	return newDirectProxy(ident, facet);
#ifndef ICEE_NO_LOCATOR
    }
    else
    {
	//
	// Create a reference with the adapter id.
	//
	ReferencePtr ref = _instance->referenceFactory()->create(ident, Context(), facet,
								 Reference::ModeTwoway, _id
#ifndef ICEE_NO_ROUTER
								 , 0
#endif
								 , _locatorInfo
								 );

	//
	// Return a proxy for the reference. 
	//
	return _instance->proxyFactory()->referenceToProxy(ref);
    }
#endif
}

ObjectPrx
IceE::ObjectAdapter::newDirectProxy(const Identity& ident, const string& facet) const
{
    vector<EndpointPtr> endpoints;

    // 
    // Use the published endpoints, otherwise use the endpoints from all
    // incoming connection factories.
    //
    if(!_publishedEndpoints.empty())
    {
	endpoints = _publishedEndpoints;
    }
    else
    {
	transform(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(), back_inserter(endpoints),
		  IceE::constMemFun(&IncomingConnectionFactory::endpoint));
    }
    
    //
    // Now we also add the endpoints of the router's server proxy, if
    // any. This way, object references created by this object adapter
    // will also point to the router's server proxy endpoints.
    //
#ifndef ICEE_NO_ROUTER
    copy(_routerEndpoints.begin(), _routerEndpoints.end(), back_inserter(endpoints));
#endif
    
    //
    // Create a reference and return a proxy for this reference.
    //
    ReferencePtr ref = _instance->referenceFactory()->create(ident, Context(), facet, Reference::ModeTwoway,
							     endpoints
#ifndef ICEE_NO_ROUTER
							     , 0
#endif
							     );
    return _instance->proxyFactory()->referenceToProxy(ref);

}

void
IceE::ObjectAdapter::checkForDeactivation() const
{
    if(_deactivated)
    {
	ObjectAdapterDeactivatedException ex(__FILE__, __LINE__);
	ex.name = _name;
	throw ex;
    }
}

void
IceE::ObjectAdapter::checkIdentity(const Identity& ident)
{
    if(ident.name.size() == 0)
    {
        IllegalIdentityException e(__FILE__, __LINE__);
        e.id = ident;
        throw e;
    }
}

vector<EndpointPtr>
IceE::ObjectAdapter::parseEndpoints(const string& str) const
{
    string endpts = str;
    transform(endpts.begin(), endpts.end(), endpts.begin(), ::tolower);

    string::size_type beg;
    string::size_type end = 0;

    vector<EndpointPtr> endpoints;
    while(end < endpts.length())
    {
	const string delim = " \t\n\r";
	
	beg = endpts.find_first_not_of(delim, end);
	if(beg == string::npos)
	{
	    break;
	}

	end = endpts.find(':', beg);
	if(end == string::npos)
	{
	    end = endpts.length();
	}
	
	if(end == beg)
	{
	    ++end;
	    continue;
	}
	
	string s = endpts.substr(beg, end - beg);
	EndpointPtr endp = _instance->endpointFactory()->create(s);
	endpoints.push_back(endp);

	++end;
    }

    return endpoints;
}
