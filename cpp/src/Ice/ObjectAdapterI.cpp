// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/TraceLevels.h>
#include <Ice/PropertyNames.h>

#ifdef _WIN32
#   include <sys/timeb.h>
#else
#   include <sys/time.h>
#endif

#ifdef __BCPLUSPLUS__
#  include <iterator>
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
    return _noConfig ? string("") : _name;
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
    bool printAdapterReady = false;

    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
        
        checkForDeactivation();

        //
        // If the one off initializations of the adapter are already
        // done, we just need to activate the incoming connection
        // factories and we're done.
        //
        if(_activateOneOffDone)
        {
            for_each(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(),
                     Ice::voidMemFun(&IncomingConnectionFactory::activate));
            return;
        }
        
        //
        // One off initializations of the adapter: update the locator
        // registry and print the "adapter ready" message. We set the
        // _waitForActivate flag to prevent deactivation from other
        // threads while these one off initializations are done.
        //
        _waitForActivate = true;

        locatorInfo = _locatorInfo;
        if(!_noConfig)
        {
            PropertiesPtr properties = _instance->initializationData().properties;
            printAdapterReady = properties->getPropertyAsInt("Ice.PrintAdapterReady") > 0;
            registerProcess = properties->getPropertyAsInt(_name + ".RegisterProcess") > 0;
        }
    }

    try
    {
        Ice::Identity dummy;
        dummy.name = "dummy";
        updateLocatorRegistry(locatorInfo, createDirectProxy(dummy), registerProcess);
    }
    catch(const Ice::LocalException&)
    {
        //
        // If we couldn't update the locator registry, we let the
        // exception go through and don't activate the adapter to
        // allow to user code to retry activating the adapter
        // later.
        //
        {
            IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
            _waitForActivate = false;
            notifyAll();
        }
        throw;
    }

    if(printAdapterReady)
    {
        cout << _name << " ready" << endl;
    }

    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
        assert(!_deactivated); // Not possible if _waitForActivate = true;

        //
        // Signal threads waiting for the activation.
        //
        _waitForActivate = false;
        notifyAll();

        _activateOneOffDone = true;

        for_each(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(),
                 Ice::voidMemFun(&IncomingConnectionFactory::activate));
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
    LocatorInfoPtr locatorInfo;
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

        //
        // Wait for activation to complete. This is necessary to not 
        // get out of order locator updates.
        //
        while(_waitForActivate)
        {
            wait();
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
        }
        
        incomingConnectionFactories = _incomingConnectionFactories;
        outgoingConnectionFactory = _instance->outgoingConnectionFactory();
        locatorInfo = _locatorInfo;

        _deactivated = true;

        notifyAll();
    }

    try
    {
        updateLocatorRegistry(locatorInfo, 0, false);
    }
    catch(const Ice::LocalException&)
    {
        //
        // We can't throw exceptions in deactivate so we ignore
        // failures to update the locator registry.
        //
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
    vector<IceInternal::IncomingConnectionFactoryPtr> incomingConnectionFactories;

    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

        if(_destroyed)
        {
            return;
        }

        //
        // Wait for deactivation of the adapter itself, and for
        // the return of all direct method calls using this adapter.
        //
        while(!_deactivated || _directCount > 0)
        {
            wait();
        }

        incomingConnectionFactories = _incomingConnectionFactories;
    }

    //
    // Now we wait until all incoming connection factories are
    // finished.
    //
    for_each(incomingConnectionFactories.begin(), incomingConnectionFactories.end(),
             Ice::voidMemFun(&IncomingConnectionFactory::waitUntilFinished));
}

bool
Ice::ObjectAdapterI::isDeactivated() const
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    return _deactivated;
}

void
Ice::ObjectAdapterI::destroy()
{
    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

        //
        // Another thread is in the process of destroying the object
        // adapter. Wait for it to finish.
        //
        while(_destroying)
        {
            wait();
        }

        //
        // Object adapter is already destroyed.
        //
        if(_destroyed)
        {
            return;
        }

        _destroying = true;
    }

    //
    // Deactivate and wait for completion.
    //
    deactivate();
    waitForDeactivate();

    //
    // Now it's also time to clean up our servants and servant
    // locators.
    //
    _servantManager->destroy();

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
        // Signal that destroy is complete.
        //
        _destroying = false;
        _destroyed = true;
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
    ReferencePtr ref = _instance->referenceFactory()->create(ident, _instance->getDefaultContext(), 
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

void
Ice::ObjectAdapterI::refreshPublishedEndpoints()
{
    LocatorInfoPtr locatorInfo;
    bool registerProcess = false;
    vector<EndpointIPtr> oldPublishedEndpoints;

    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

        checkForDeactivation();

        oldPublishedEndpoints = _publishedEndpoints;
        _publishedEndpoints = parsePublishedEndpoints();

        locatorInfo = _locatorInfo;
        if(!_noConfig)
        {
            registerProcess =
                _instance->initializationData().properties->getPropertyAsInt(_name + ".RegisterProcess") > 0;
        }
    }

    try
    {
        Ice::Identity dummy;
        dummy.name = "dummy";
        updateLocatorRegistry(locatorInfo, createDirectProxy(dummy), registerProcess);
    }
    catch(const Ice::LocalException& ex)
    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

        //
        // Restore the old published endpoints.
        //
        _publishedEndpoints = oldPublishedEndpoints;
        ex.ice_throw();
    }
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
            // adapter id or replica group id.
            //
            return ir->getAdapterId() == _id || ir->getAdapterId() == _replicaGroupId;
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

    assert(_instance); // Must not be called after destroy().

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
    // destroy().

    // Not check for deactivation here!

    assert(_instance); // Must not be called after destroy().

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
    //
    // No mutex lock necessary, _servantManager is immutable.
    //
    return _servantManager;
}

bool
Ice::ObjectAdapterI::getThreadPerConnection() const
{
    //
    // No mutex lock necessary, _threadPerConnection is immutable.
    //
    return _threadPerConnection;
}

size_t
Ice::ObjectAdapterI::getThreadPerConnectionStackSize() const
{
    //
    // No mutex lock necessary, _threadPerConnectionStackSize is immutable.
    //
    return _threadPerConnectionStackSize;
}

Ice::ObjectAdapterI::ObjectAdapterI(const InstancePtr& instance, const CommunicatorPtr& communicator,
                                    const ObjectAdapterFactoryPtr& objectAdapterFactory, const string& name,
                                    const string& endpointInfo, const RouterPrx& router, bool noConfig) :
    _deactivated(false),
    _instance(instance),
    _communicator(communicator),
    _objectAdapterFactory(objectAdapterFactory),
    _servantManager(new ServantManager(instance, name)),
    _activateOneOffDone(false),
    _name(name),
    _directCount(0),
    _waitForActivate(false),
    _destroying(false),
    _destroyed(false),
    _noConfig(noConfig),
    _threadPerConnection(false),
    _threadPerConnectionStackSize(0)
{
    if(_noConfig)
    {
        return;
    }

    PropertiesPtr properties = instance->initializationData().properties;
    StringSeq unknownProps;
    bool noProps = filterProperties(unknownProps);

    //
    // Warn about unknown object adapter properties.
    //
    if(unknownProps.size() != 0 && properties->getPropertyAsIntWithDefault("Ice.Warn.UnknownProperties", 1) > 0)
    {
        Warning out(_instance->initializationData().logger);
        out << "found unknown properties for object adapter '" << _name << "':";
        for(unsigned int i = 0; i < unknownProps.size(); ++i)
        {
            out << "\n    " << unknownProps[i];
        }
    }

    //
    // Make sure named adapter has some configuration
    //
    if(endpointInfo.empty() && router == 0 && noProps)
    {
        InitializationException ex(__FILE__, __LINE__);
        ex.reason = "object adapter \"" + _name + "\" requires configuration.";
        throw ex;
    }

    const_cast<string&>(_id) = properties->getProperty(_name + ".AdapterId");
    const_cast<string&>(_replicaGroupId) = properties->getProperty(_name + ".ReplicaGroupId");

    __setNoDelete(true);
    try
    {
        _threadPerConnection = properties->getPropertyAsInt(_name + ".ThreadPerConnection") > 0;

        int threadPoolSize = properties->getPropertyAsInt(_name + ".ThreadPool.Size");
        int threadPoolSizeMax = properties->getPropertyAsInt(_name + ".ThreadPool.SizeMax");
        if(_threadPerConnection && (threadPoolSize > 0 || threadPoolSizeMax > 0))
        {
            InitializationException ex(__FILE__, __LINE__);
            ex.reason = "object adapter \"" + _name + "\" cannot be configured for both\n"
                "thread pool and thread per connection";
            throw ex;
        }

        if(!_threadPerConnection && threadPoolSize == 0 && threadPoolSizeMax == 0)
        {
            _threadPerConnection = _instance->threadPerConnection();
        }

        if(_threadPerConnection)
        {
            int stackSize = 
                properties->getPropertyAsIntWithDefault(_name + ".ThreadPerConnection.StackSize",
                                                        static_cast<Int>(_instance->threadPerConnectionStackSize()));
            if(stackSize < 0)
            {
                stackSize = 0;
            }
            _threadPerConnectionStackSize = stackSize;
        }

        //
        // Create the per-adapter thread pool, if necessary. This is done before the creation of the incoming
        // connection factory as the thread pool is needed during creation for the call to incFdsInUse.
        //
        if(threadPoolSize > 0 || threadPoolSizeMax > 0)
        {
            _threadPool = new ThreadPool(_instance, _name + ".ThreadPool", 0);
        }

        if(!router)
        {
            const_cast<RouterPrx&>(router) = RouterPrx::uncheckedCast(
                _instance->proxyFactory()->propertyToProxy(_name + ".Router"));
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
                                                     _instance->identityToString(router->ice_getIdentity()));
                }

                //
                // Add the router's server proxy endpoints to this object
                // adapter.
                //
                vector<EndpointIPtr> endpoints = _routerInfo->getServerEndpoints();
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
            vector<EndpointIPtr> endpoints;
            if(endpointInfo.empty())
            {
                endpoints = parseEndpoints(properties->getProperty(_name + ".Endpoints"));
            }
            else
            {
                endpoints = parseEndpoints(endpointInfo);
            }
            for(vector<EndpointIPtr>::iterator p = endpoints.begin(); p != endpoints.end(); ++p)
            {
                _incomingConnectionFactories.push_back(new IncomingConnectionFactory(instance, *p, this, _name));
            }
            if(endpoints.empty())
            {
                TraceLevelsPtr tl = _instance->traceLevels();
                if(tl->network >= 2)
                {
                    Trace out(_instance->initializationData().logger, tl->networkCat);
                    out << "created adapter `" << name << "' without endpoints";
                }
            }

            //
            // Parse the published endpoints.
            //
            _publishedEndpoints = parsePublishedEndpoints();
        }

        if(!properties->getProperty(_name + ".Locator").empty())
        {
            setLocator(LocatorPrx::uncheckedCast(_instance->proxyFactory()->propertyToProxy(_name + ".Locator")));
        }
        else
        {
            setLocator(_instance->referenceFactory()->getDefaultLocator());
        }
    }
    catch(...)
    {
        destroy();
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
        out << "object adapter `" << getName() << "' has not been deactivated";
    }
    else if(!_destroyed)
    {
        Warning out(_instance->initializationData().logger);
        out << "object adapter `" << getName() << "' has not been destroyed";
    }
    else
    {
        //assert(!_servantManager); // We don't clear this reference, it needs to be immutable.
        assert(!_threadPool);
        assert(!_communicator);
        assert(_incomingConnectionFactories.empty());
        assert(_directCount == 0);
        assert(!_waitForActivate);
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
        ident, _instance->getDefaultContext(), facet, Reference::ModeTwoway, false,
        _instance->defaultsAndOverrides()->defaultPreferSecure, endpoints, 0,
        _instance->defaultsAndOverrides()->defaultCollocationOptimization, true,
        _instance->defaultsAndOverrides()->defaultEndpointSelection, _instance->threadPerConnection());
    return _instance->proxyFactory()->referenceToProxy(ref);

}

ObjectPrx
Ice::ObjectAdapterI::newIndirectProxy(const Identity& ident, const string& facet, const string& id) const
{
    //
    // Create an indirect reference with the given adapter id.
    //
    ReferencePtr ref = _instance->referenceFactory()->create(
        ident, _instance->getDefaultContext(), facet, Reference::ModeTwoway, false, 
        _instance->defaultsAndOverrides()->defaultPreferSecure, id, 0, 
        _locatorInfo, _instance->defaultsAndOverrides()->defaultCollocationOptimization, true,
        _instance->defaultsAndOverrides()->defaultEndpointSelection, _instance->threadPerConnection(),
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
        ex.name = getName();
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
Ice::ObjectAdapterI::parseEndpoints(const string& endpts) const
{
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
        EndpointIPtr endp = _instance->endpointFactoryManager()->create(s, true);
        if(endp == 0)
        {
            EndpointParseException ex(__FILE__, __LINE__);
            ex.str = s;
            throw ex;
        }
        endpoints.push_back(endp);

        ++end;
    }

    return endpoints;
}

std::vector<EndpointIPtr>
ObjectAdapterI::parsePublishedEndpoints()
{
    //
    // Parse published endpoints. If set, these are used in proxies
    // instead of the connection factory endpoints. 
    //
    string endpts = _communicator->getProperties()->getProperty(_name + ".PublishedEndpoints");
    vector<EndpointIPtr> endpoints = parseEndpoints(endpts);
    if(endpoints.empty())
    {
        transform(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(), 
                  back_inserter(endpoints), Ice::constMemFun(&IncomingConnectionFactory::endpoint));
    }

    //
    // Expand any endpoints that may be listening on INADDR_ANY to 
    // include actual addresses in the published endpoints.
    //
    vector<EndpointIPtr> expandedEndpoints;
    for(unsigned int i = 0; i < endpoints.size(); ++i)
    {
        vector<EndpointIPtr> endps = endpoints[i]->expand();
        expandedEndpoints.insert(expandedEndpoints.end(), endps.begin(), endps.end());
    }
    return expandedEndpoints;
}

void
ObjectAdapterI::updateLocatorRegistry(const IceInternal::LocatorInfoPtr& locatorInfo,
                                      const Ice::ObjectPrx& proxy,
                                      bool registerProcess)
{
    if(!registerProcess && _id.empty())
    {
        return; // Nothing to update.
    }

    //
    // We must get and call on the locator registry outside the thread
    // synchronization to avoid deadlocks. (we can't make remote calls
    // within the OA synchronization because the remote call will
    // indirectly call isLocal() on this OA with the OA factory
    // locked).
    //
    // TODO: This might throw if we can't connect to the
    // locator. Shall we raise a special exception for the activate
    // operation instead of a non obvious network exception?
    //
    LocatorRegistryPrx locatorRegistry = locatorInfo ? locatorInfo->getLocatorRegistry() : LocatorRegistryPrx();
    string serverId;
    if(registerProcess)
    {
        assert(_instance);
        serverId = _instance->initializationData().properties->getProperty("Ice.ServerId");

        if(!locatorRegistry)
        {
            Warning out(_instance->initializationData().logger);
            out << "object adapter `" << getName() << "' cannot register the process without a locator registry";
        }
        else if(serverId.empty())
        {
            Warning out(_instance->initializationData().logger);
            out << "object adapter `" << getName() << "' cannot register the process without a value for Ice.ServerId";
        }
    }

    if(!locatorRegistry)
    {
        return;
    }

    if(!_id.empty())
    {
        try
        {
            if(_replicaGroupId.empty())
            {
                locatorRegistry->setAdapterDirectProxy(_id, proxy);
            }
            else
            {
                locatorRegistry->setReplicatedAdapterDirectProxy(_id, _replicaGroupId, proxy);
            }
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

    if(registerProcess && !serverId.empty())
    {
        try
        {
            ProcessPtr servant = new ProcessI(_communicator);
            Ice::ObjectPrx process = createDirectProxy(addWithUUID(servant)->ice_getIdentity());
            locatorRegistry->setServerProcessProxy(serverId, ProcessPrx::uncheckedCast(process));
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

bool
Ice::ObjectAdapterI::filterProperties(StringSeq& unknownProps)
{
    static const string suffixes[] = 
    { 
        "AdapterId",
        "Endpoints",
        "Locator",
        "PublishedEndpoints",
        "RegisterProcess",
        "ReplicaGroupId",
        "Router",
        "ThreadPerConnection",
        "ThreadPerConnection.StackSize",
        "ThreadPool.Size",
        "ThreadPool.SizeMax",
        "ThreadPool.SizeWarn",
        "ThreadPool.StackSize"
    };

    //
    // Do not create unknown properties list if Ice prefix, ie Ice, Glacier2, etc
    //
    bool addUnknown = true;
    string prefix = _name + ".";
    for(const char** i = IceInternal::PropertyNames::clPropNames; *i != 0; ++i)
    {
        string icePrefix = string(*i) + ".";
        if(prefix.find(icePrefix) == 0)
        {
            addUnknown = false;
            break;
        }
    }

    bool noProps = true;
    PropertyDict props = _instance->initializationData().properties->getPropertiesForPrefix(prefix);
    PropertyDict::const_iterator p;
    for(p = props.begin(); p != props.end(); ++p)
    {
        bool valid = false;
        for(unsigned int i = 0; i < sizeof(suffixes)/sizeof(*suffixes); ++i)
        {
            string prop = prefix + suffixes[i];
            if(p->first == prop)
            {
                noProps = false;
                valid = true;
                break;
            }
        }

        if(!valid && addUnknown)
        {
            unknownProps.push_back(p->first);
        }
    }

    return noProps;
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
