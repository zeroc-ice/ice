// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/UUID.h>
#include <Ice/ObjectAdapterI.h>
#include <Ice/ObjectAdapterFactory.h>
#include <Ice/Instance.h>
#include <Ice/Proxy.h>
#include <Ice/ProxyFactory.h>
#include <Ice/ReferenceFactory.h>
#include <Ice/EndpointI.h>
#include <Ice/EndpointFactoryManager.h>
#include <Ice/ConnectionFactory.h>
#include <Ice/ServantManager.h>
#include <Ice/RouterInfo.h>
#include <Ice/LocalException.h>
#include <Ice/Properties.h>
#include <Ice/Functional.h>
#include <Ice/LocatorInfo.h>
#include <Ice/Locator.h>
#include <Ice/LoggerUtil.h>
#include <Ice/ThreadPool.h>
#include <Ice/Communicator.h>
#include <Ice/Router.h>
#include <Ice/IdentityUtil.h>
#include <Ice/DefaultsAndOverrides.h>

#ifdef _WIN32
#   include <sys/timeb.h>
#else
#   include <sys/time.h>
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;

string
Ice::ObjectAdapterI::getName() const
{
    //
    // No mutex lock necessary, _name is immutable.
    //
    return _name;
}

CommunicatorPtr
Ice::ObjectAdapterI::getCommunicator() const
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    checkForDeactivation();

    return _communicator;
}

void
Ice::ObjectAdapterI::activate()
{
    LocatorInfoPtr locatorInfo;
    bool registerProcess = false;
    string serverId;
    CommunicatorPtr communicator;
    bool printAdapterReady = false;

    {    
	IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
	
	checkForDeactivation();

	if(!_printAdapterReadyDone)
	{
	    locatorInfo = _locatorInfo;
            registerProcess = 
	        _instance->initializationData().properties->getPropertyAsInt(_name + ".RegisterProcess") > 0;
            serverId = _instance->initializationData().properties->getProperty("Ice.ServerId");
	    printAdapterReady =
	        _instance->initializationData().properties->getPropertyAsInt("Ice.PrintAdapterReady") > 0;
            communicator = _communicator;
	    _printAdapterReadyDone = true;
	}
	
	for_each(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(),
		 Ice::voidMemFun(&IncomingConnectionFactory::activate));	
    }

    if(registerProcess || !_id.empty())
    {
	//
	// We must get and call on the locator registry outside the thread
	// synchronization to avoid deadlocks. (we can't make remote calls
	// within the OA synchronization because the remote call will
	// indirectly call isLocal() on this OA with the OA factory
	// locked).
	//
	LocatorRegistryPrx locatorRegistry;
	if(locatorInfo)
	{
	    //
	    // TODO: This might throw if we can't connect to the
	    // locator. Shall we raise a special exception for the
	    // activate operation instead of a non obvious network
	    // exception?
	    //
	    locatorRegistry = locatorInfo->getLocatorRegistry();
	}
	
	if(locatorRegistry && !_id.empty())
	{
	    try
	    {
		Identity ident;
		ident.name = "dummy";
		if(_replicaGroupId.empty())
		{
		    locatorRegistry->setAdapterDirectProxy(_id, createDirectProxy(ident));
		}
		else
		{
		    locatorRegistry->setReplicatedAdapterDirectProxy(_id, _replicaGroupId, createDirectProxy(ident));
		}
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
	    catch(const InvalidReplicaGroupIdException&)
	    {
		NotRegisteredException ex(__FILE__, __LINE__);
		ex.kindOfObject = "replica group";
		ex.id = _replicaGroupId;
		throw ex;
	    }
	    catch(const AdapterAlreadyActiveException&)
	    {
		ObjectAdapterIdInUseException ex(__FILE__, __LINE__);
		ex.id = _id;
		throw ex;
	    }
	}	    

	if(registerProcess)
	{
	    if(!locatorRegistry)
	    {
		Warning out(communicator->getLogger());
		out << "object adapter `" << _name << "' cannot register the process without a locator registry";
	    }
	    else if(serverId.empty())
	    {
		Warning out(communicator->getLogger());
		out << "object adapter `" << _name << "' cannot register the process without a value for Ice.ServerId";
	    }
	    else
	    {
		try
		{
		    ProcessPtr servant = new ProcessI(communicator);
		    Ice::ObjectPrx proxy = createDirectProxy(addWithUUID(servant)->ice_getIdentity());
		    locatorRegistry->setServerProcessProxy(serverId, ProcessPrx::uncheckedCast(proxy));
		}
		catch(const ObjectAdapterDeactivatedException&)
		{
		    // IGNORE: The object adapter is already inactive.
		}
		catch(const ServerNotFoundException&)
		{
		    NotRegisteredException ex(__FILE__, __LINE__);
		    ex.kindOfObject = "server";
		    ex.id = serverId;
		    throw ex;
		}
	    }
	}
    }

    if(printAdapterReady)
    {
	cout << _name << " ready" << endl;
    }
}

void
Ice::ObjectAdapterI::hold()
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    checkForDeactivation();
	
    for_each(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(),
	     Ice::voidMemFun(&IncomingConnectionFactory::hold));
}
    
void
Ice::ObjectAdapterI::waitForHold()
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    checkForDeactivation();

    for_each(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(),
	     Ice::constVoidMemFun(&IncomingConnectionFactory::waitUntilHolding));
}

void
Ice::ObjectAdapterI::deactivate()
{
    vector<IncomingConnectionFactoryPtr> incomingConnectionFactories;
    OutgoingConnectionFactoryPtr outgoingConnectionFactory;

    {
	IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
	
	//
	// Ignore deactivation requests if the object adapter has already
	// been deactivated.
	//
	if(_deactivated)
	{
	    return;
	}

	if(_routerInfo)
	{
	    //
	    // Remove entry from the router manager.
	    //
	    _instance->routerManager()->erase(_routerInfo->getRouter());

	    //
	    //  Clear this object adapter with the router.
	    //
	    _routerInfo->setAdapter(0);

	    //
	    // Update all existing outgoing connections.
	    //
	    _instance->outgoingConnectionFactory()->setRouterInfo(_routerInfo);
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
	     Ice::voidMemFun(&IncomingConnectionFactory::destroy));
    
    //
    // Must be called outside the thread synchronization, because
    // changing the object adapter might block if there are still
    // requests being dispatched.
    //
    outgoingConnectionFactory->removeAdapter(this);
}

void
Ice::ObjectAdapterI::waitForDeactivate()
{
    {
	IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

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
	     Ice::voidMemFun(&IncomingConnectionFactory::waitUntilFinished));

    //
    // Now it's also time to clean up our servants and servant
    // locators.
    //
    if(_servantManager)
    {
	_servantManager->destroy();
    }

    //
    // Destroy the thread pool.
    //
    if(_threadPool)
    {
	_threadPool->destroy();
	_threadPool->joinWithAllThreads();
    }

    ObjectAdapterFactoryPtr objectAdapterFactory;

    {
	IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

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
	_threadPool = 0;
	_servantManager = 0;
	_communicator = 0;
	_routerEndpoints.clear();
	_routerInfo = 0;
	_publishedEndpoints.clear();
	_locatorInfo = 0;

	objectAdapterFactory = _objectAdapterFactory;
	_objectAdapterFactory = 0;
    }

    if(objectAdapterFactory)
    {
        objectAdapterFactory->removeObjectAdapter(_name);
    }
}

void
Ice::ObjectAdapterI::destroy()
{
    deactivate();
    waitForDeactivate();
}

ObjectPrx
Ice::ObjectAdapterI::add(const ObjectPtr& object, const Identity& ident)
{
    return addFacet(object, ident, "");
}

ObjectPrx
Ice::ObjectAdapterI::addFacet(const ObjectPtr& object, const Identity& ident, const string& facet)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    checkForDeactivation();
    checkIdentity(ident);

    _servantManager->addServant(object, ident, facet);

    return newProxy(ident, facet);
}

ObjectPrx
Ice::ObjectAdapterI::addWithUUID(const ObjectPtr& object)
{
    return addFacetWithUUID(object, "");
}

ObjectPrx
Ice::ObjectAdapterI::addFacetWithUUID(const ObjectPtr& object, const string& facet)
{
    Identity ident;
    ident.name = IceUtil::generateUUID();
    return addFacet(object, ident, facet);
}

ObjectPtr
Ice::ObjectAdapterI::remove(const Identity& ident)
{
    return removeFacet(ident, "");
}

ObjectPtr
Ice::ObjectAdapterI::removeFacet(const Identity& ident, const string& facet)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    checkForDeactivation();
    checkIdentity(ident);

    return _servantManager->removeServant(ident, facet);
}

FacetMap
Ice::ObjectAdapterI::removeAllFacets(const Identity& ident)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    checkForDeactivation();
    checkIdentity(ident);

    return _servantManager->removeAllFacets(ident);
}

ObjectPtr
Ice::ObjectAdapterI::find(const Identity& ident) const
{
    return findFacet(ident, "");
}

ObjectPtr
Ice::ObjectAdapterI::findFacet(const Identity& ident, const string& facet) const
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    checkForDeactivation();
    checkIdentity(ident);

    return _servantManager->findServant(ident, facet);
}

FacetMap
Ice::ObjectAdapterI::findAllFacets(const Identity& ident) const
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    checkForDeactivation();
    checkIdentity(ident);

    return _servantManager->findAllFacets(ident);
}

ObjectPtr
Ice::ObjectAdapterI::findByProxy(const ObjectPrx& proxy) const
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    checkForDeactivation();

    ReferencePtr ref = proxy->__reference();
    return findFacet(ref->getIdentity(), ref->getFacet());
}

void
Ice::ObjectAdapterI::addServantLocator(const ServantLocatorPtr& locator, const string& prefix)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    checkForDeactivation();

    _servantManager->addServantLocator(locator, prefix);
}

ServantLocatorPtr
Ice::ObjectAdapterI::findServantLocator(const string& prefix) const
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    checkForDeactivation();

    return _servantManager->findServantLocator(prefix);
}

ObjectPrx
Ice::ObjectAdapterI::createProxy(const Identity& ident) const
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    checkForDeactivation();
    checkIdentity(ident);

    return newProxy(ident, "");
}

ObjectPrx
Ice::ObjectAdapterI::createDirectProxy(const Identity& ident) const
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
    
    checkForDeactivation();
    checkIdentity(ident);

    return newDirectProxy(ident, "");
}

ObjectPrx
Ice::ObjectAdapterI::createIndirectProxy(const Identity& ident) const
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
    
    checkForDeactivation();
    checkIdentity(ident);

    return newIndirectProxy(ident, "", _id);
}

ObjectPrx
Ice::ObjectAdapterI::createReverseProxy(const Identity& ident) const
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
    
    checkForDeactivation();
    checkIdentity(ident);

    //
    // Get all incoming connections for this object adapter.
    //
    vector<ConnectionIPtr> connections;
    vector<IncomingConnectionFactoryPtr>::const_iterator p;
    for(p = _incomingConnectionFactories.begin(); p != _incomingConnectionFactories.end(); ++p)
    {
	list<ConnectionIPtr> cons = (*p)->connections();
	copy(cons.begin(), cons.end(), back_inserter(connections));
    }

    //
    // Create a reference and return a reverse proxy for this
    // reference.
    //
    ReferencePtr ref = _instance->referenceFactory()->create(ident, _instance->initializationData().defaultContext, 
    							     "", Reference::ModeTwoway, connections);
    return _instance->proxyFactory()->referenceToProxy(ref);
}

void
Ice::ObjectAdapterI::setLocator(const LocatorPrx& locator)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
    
    checkForDeactivation();

    _locatorInfo = _instance->locatorManager()->get(locator);
}

bool
Ice::ObjectAdapterI::isLocal(const ObjectPrx& proxy) const
{
    ReferencePtr ref = proxy->__reference();
    vector<EndpointIPtr>::const_iterator p;
    vector<EndpointIPtr> endpoints;

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

	//
	// Get Locator endpoint information for indirect references.
	//
	LocatorInfoPtr info = ir->getLocatorInfo();
	if(info)
	{
	    bool isCached;
	    endpoints = info->getEndpoints(ir, ir->getLocatorCacheTimeout(), isCached);
	}
	else
	{
	    return false;
	}
    }
    else
    {
	endpoints = ref->getEndpoints();
    }

    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
    checkForDeactivation();

    //
    // Proxies which have at least one endpoint in common with the
    // endpoints used by this object adapter's incoming connection
    // factories are considered local.
    //
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
    if(_routerInfo && _routerInfo->getRouter() == proxy->ice_getRouter())
    {
        for(p = endpoints.begin(); p != endpoints.end(); ++p)
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
Ice::ObjectAdapterI::flushBatchRequests()
{
    vector<IncomingConnectionFactoryPtr> f;
    {
	IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
	f = _incomingConnectionFactories;
    }
    for_each(f.begin(), f.end(), Ice::voidMemFun(&IncomingConnectionFactory::flushBatchRequests));
}

void
Ice::ObjectAdapterI::incDirectCount()
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
 
    checkForDeactivation();

    assert(_directCount >= 0);
    ++_directCount;
}

void
Ice::ObjectAdapterI::decDirectCount()
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    // Not check for deactivation here!

    assert(_instance); // Must not be called after waitForDeactivate().

    assert(_directCount > 0);
    if(--_directCount == 0)
    {
	notifyAll();
    }    
}

ThreadPoolPtr
Ice::ObjectAdapterI::getThreadPool() const
{
    // No mutex lock necessary, _threadPool and _instance are
    // immutable after creation until they are removed in
    // waitForDeactivate().

    // Not check for deactivation here!

    assert(_instance); // Must not be called after waitForDeactivate().

    if(_threadPool)
    {
	return _threadPool;
    }
    else
    {
	return _instance->serverThreadPool();
    }
}

ServantManagerPtr
Ice::ObjectAdapterI::getServantManager() const
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    return _servantManager;
}

Ice::ObjectAdapterI::ObjectAdapterI(const InstancePtr& instance, const CommunicatorPtr& communicator,
				    const ObjectAdapterFactoryPtr& objectAdapterFactory, const string& name,
				    const string& endpointInfo, const RouterPrx& router) :
    _deactivated(false),
    _instance(instance),
    _communicator(communicator),
    _objectAdapterFactory(objectAdapterFactory),
    _servantManager(new ServantManager(instance, name)),
    _printAdapterReadyDone(false),
    _name(name),
    _id(instance->initializationData().properties->getProperty(name + ".AdapterId")),
    _replicaGroupId(instance->initializationData().properties->getProperty(name + ".ReplicaGroupId")),
    _directCount(0),
    _waitForDeactivate(false)
{
    __setNoDelete(true);
    try
    {
    	if(!router)
	{
	    string routerStr = _instance->initializationData().properties->getProperty(_name + ".Router");
	    if(!routerStr.empty())
	    {
	        const_cast<RouterPrx&>(router) =
		    RouterPrx::uncheckedCast(_instance->proxyFactory()->stringToProxy(routerStr));
	    }
	}
	if(router)
	{
            _routerInfo = _instance->routerManager()->get(router);
            if(_routerInfo)
            {
	        //
		// Make sure this router is not already registered with another adapter.
		//
	        if(_routerInfo->getAdapter())
		{
		    throw AlreadyRegisteredException(__FILE__, __LINE__, "object adapter with router", 
		    				     identityToString(router->ice_getIdentity()));
		}

	        //
	        // Add the router's server proxy endpoints to this object
	        // adapter.
	        //
	        ObjectPrx proxy = _routerInfo->getServerProxy();
	        vector<EndpointIPtr> endpoints = proxy->__reference()->getEndpoints();
	        copy(endpoints.begin(), endpoints.end(), back_inserter(_routerEndpoints));
	        sort(_routerEndpoints.begin(), _routerEndpoints.end()); // Must be sorted.
	        _routerEndpoints.erase(unique(_routerEndpoints.begin(), _routerEndpoints.end()),
				       _routerEndpoints.end());

	        //
	        // Associate this object adapter with the router. This way,
	        // new outgoing connections to the router's client proxy will
	        // use this object adapter for callbacks.
	        //
	        _routerInfo->setAdapter(this);

	        //
	        // Also modify all existing outgoing connections to the
	        // router's client proxy to use this object adapter for
	        // callbacks.
	        //	
	        _instance->outgoingConnectionFactory()->setRouterInfo(_routerInfo);
            }
	}
	else
	{
	    //
	    // Parse the endpoints, but don't store them in the adapter.
	    // The connection factory might change it, for example, to
	    // fill in the real port number.
	    //
	    vector<EndpointIPtr> endpoints = parseEndpoints(endpointInfo);
	    for(vector<EndpointIPtr>::iterator p = endpoints.begin(); p != endpoints.end(); ++p)
    	    {
	        _incomingConnectionFactories.push_back(new IncomingConnectionFactory(instance, *p, this));
	    }

	    //
	    // Parse published endpoints. If set, these are used in proxies
	    // instead of the connection factory endpoints. 
	    //
	    string endpts = _instance->initializationData().properties->getProperty(name + ".PublishedEndpoints");
	    _publishedEndpoints = parseEndpoints(endpts);
	    if(_publishedEndpoints.empty())
    	    {
	        transform(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(), 
	    	          back_inserter(_publishedEndpoints), Ice::constMemFun(&IncomingConnectionFactory::endpoint));
	    }

	    //
	    // Filter out any endpoints that are not meant to be published.
	    //
	    _publishedEndpoints.erase(remove_if(_publishedEndpoints.begin(), _publishedEndpoints.end(),
				      not1(Ice::constMemFun(&EndpointI::publish))), _publishedEndpoints.end());
	}

	string locator = _instance->initializationData().properties->getProperty(_name + ".Locator");
	if(!locator.empty())
	{
	    setLocator(LocatorPrx::uncheckedCast(_instance->proxyFactory()->stringToProxy(locator)));
	}
	else
	{
	    setLocator(_instance->referenceFactory()->getDefaultLocator());
	}

	if(!_instance->threadPerConnection())
	{
	    int size = _instance->initializationData().properties->getPropertyAsInt(_name + ".ThreadPool.Size");
	    int sizeMax = _instance->initializationData().properties->getPropertyAsInt(_name + ".ThreadPool.SizeMax");
	    if(size > 0 || sizeMax > 0)
	    {
		_threadPool = new ThreadPool(_instance, _name + ".ThreadPool", 0);
	    }
	}
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

Ice::ObjectAdapterI::~ObjectAdapterI()
{
    if(!_deactivated)
    {
	Warning out(_instance->initializationData().logger);
	out << "object adapter `" << _name << "' has not been deactivated";
    }
    else if(_instance)
    {
	Warning out(_instance->initializationData().logger);
	out << "object adapter `" << _name << "' deactivation had not been waited for";
    }
    else
    {
	assert(!_threadPool);
	assert(!_servantManager);
	assert(!_communicator);
	assert(_incomingConnectionFactories.empty());
	assert(_directCount == 0);
	assert(!_waitForDeactivate);
    }
}

ObjectPrx
Ice::ObjectAdapterI::newProxy(const Identity& ident, const string& facet) const
{
    if(_id.empty())
    {
	return newDirectProxy(ident, facet);
    }
    else if(_replicaGroupId.empty())
    {
	return newIndirectProxy(ident, facet, _id);
    }
    else
    {
	return newIndirectProxy(ident, facet, _replicaGroupId);
    }
}

ObjectPrx
Ice::ObjectAdapterI::newDirectProxy(const Identity& ident, const string& facet) const
{
    vector<EndpointIPtr> endpoints = _publishedEndpoints;

    //
    // Now we also add the endpoints of the router's server proxy, if
    // any. This way, object references created by this object adapter
    // will also point to the router's server proxy endpoints.
    //
    copy(_routerEndpoints.begin(), _routerEndpoints.end(), back_inserter(endpoints));
    
    //
    // Create a reference and return a proxy for this reference.
    //
    ReferencePtr ref = _instance->referenceFactory()->create(
	ident, _instance->initializationData().defaultContext, facet, Reference::ModeTwoway, false, endpoints, 0,
	_instance->defaultsAndOverrides()->defaultCollocationOptimization);
    return _instance->proxyFactory()->referenceToProxy(ref);

}

ObjectPrx
Ice::ObjectAdapterI::newIndirectProxy(const Identity& ident, const string& facet, const string& id) const
{
    //
    // Create an indirect reference with the given adapter id.
    //
    ReferencePtr ref = _instance->referenceFactory()->create(
	ident, _instance->initializationData().defaultContext, facet, Reference::ModeTwoway, false, id, 0, 
	_locatorInfo, _instance->defaultsAndOverrides()->defaultCollocationOptimization,
	_instance->defaultsAndOverrides()->defaultLocatorCacheTimeout);
    
    //
    // Return a proxy for the reference. 
    //
    return _instance->proxyFactory()->referenceToProxy(ref);
}

void
Ice::ObjectAdapterI::checkForDeactivation() const
{
    if(_deactivated)
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

vector<EndpointIPtr>
Ice::ObjectAdapterI::parseEndpoints(const string& str) const
{
    string endpts = str;
    transform(endpts.begin(), endpts.end(), endpts.begin(), ::tolower);

    string::size_type beg;
    string::size_type end = 0;

    vector<EndpointIPtr> endpoints;
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
	EndpointIPtr endp = _instance->endpointFactoryManager()->create(s);
	if(endp == 0)
	{
	    EndpointParseException ex(__FILE__, __LINE__);
	    ex.str = s;
	    throw ex;
	}
	vector<EndpointIPtr> endps = endp->expand(true);
	endpoints.insert(endpoints.end(), endps.begin(), endps.end());

	++end;
    }

    return endpoints;
}

Ice::ObjectAdapterI::ProcessI::ProcessI(const CommunicatorPtr& communicator) :
    _communicator(communicator)
{
}

void
Ice::ObjectAdapterI::ProcessI::shutdown(const Current&)
{
    _communicator->shutdown();
}

void
Ice::ObjectAdapterI::ProcessI::writeMessage(const string& message, Int fd, const Current&)
{
    switch(fd)
    {
	case 1:
	{
	    cout << message << endl;
	    break;
	}
	case 2:
	{
	    cerr << message << endl;
	    break;
	}
    }
}
