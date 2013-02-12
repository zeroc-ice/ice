// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/UUID.h>
#include <Ice/ObjectAdapterI.h>
#include <Ice/ObjectAdapterFactory.h>
#include <Ice/Instance.h>
#include <Ice/ConnectionMonitor.h>
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

#include <iterator>

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
        // If some threads are waiting on waitForHold(), we set this
        // flag to ensure the threads will start again the wait for
        // all the incoming connection factories.
        //
        _waitForHoldRetry = _waitForHold > 0;

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
    while(true)
    {
        vector<IncomingConnectionFactoryPtr> incomingConnectionFactories;
        {
            IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
            
            checkForDeactivation();
            
            incomingConnectionFactories = _incomingConnectionFactories;

            ++_waitForHold;
        }
        
        for_each(incomingConnectionFactories.begin(), incomingConnectionFactories.end(),
                 Ice::constVoidMemFun(&IncomingConnectionFactory::waitUntilHolding));
        
        {
            IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
            if(--_waitForHold == 0)
            {
                notifyAll();
            }
            
            //
            // If we don't need to retry, we're done. Otherwise, we wait until 
            // all the waiters finish waiting on the connections and we try 
            // again waiting on all the conncetions. This is necessary in the 
            // case activate() is called by another thread while waitForHold()
            // waits on the some connection, if we didn't retry, waitForHold() 
            // could return only after waiting on a subset of the connections.
            //
            if(!_waitForHoldRetry)
            {
                return;
            }
            else
            {
                while(_waitForHold > 0)
                {
                    checkForDeactivation();
                    wait();
                }
                _waitForHoldRetry = false;
            }
        }
    }
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
        _routerEndpoints.clear();
        _routerInfo = 0;
        _publishedEndpoints.clear();
        _locatorInfo = 0;
        _reference = 0;

        objectAdapterFactory = _objectAdapterFactory;
        _objectAdapterFactory = 0;
    }

    if(objectAdapterFactory)
    {
        objectAdapterFactory->removeObjectAdapter(this);
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

void
Ice::ObjectAdapterI::addDefaultServant(const ObjectPtr& servant, const string& category)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    checkForDeactivation();

    _servantManager->addDefaultServant(servant, category);
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
Ice::ObjectAdapterI::removeDefaultServant(const string& category)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    checkForDeactivation();

    return _servantManager->removeDefaultServant(category);
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

ObjectPtr
Ice::ObjectAdapterI::findDefaultServant(const string& category) const
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    checkForDeactivation();

    return _servantManager->findDefaultServant(category);
}

void
Ice::ObjectAdapterI::addServantLocator(const ServantLocatorPtr& locator, const string& prefix)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    checkForDeactivation();

    _servantManager->addServantLocator(locator, prefix);
}

ServantLocatorPtr
Ice::ObjectAdapterI::removeServantLocator(const string& prefix)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    checkForDeactivation();

    return _servantManager->removeServantLocator(prefix);
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
    catch(const Ice::LocalException&)
    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

        //
        // Restore the old published endpoints.
        //
        _publishedEndpoints = oldPublishedEndpoints;
        throw;
    }
}

EndpointSeq
Ice::ObjectAdapterI::getEndpoints() const
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    EndpointSeq endpoints;
    transform(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(), 
              back_inserter(endpoints), Ice::constMemFun(&IncomingConnectionFactory::endpoint));
    return endpoints;
}

EndpointSeq
Ice::ObjectAdapterI::getPublishedEndpoints() const
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    EndpointSeq endpoints;
    copy(_publishedEndpoints.begin(), _publishedEndpoints.end(), back_inserter(endpoints));
    return endpoints;
}

bool
Ice::ObjectAdapterI::isLocal(const ObjectPrx& proxy) const
{
    //
    // NOTE: it's important that isLocal() doesn't perform any blocking operations as 
    // it can be called for AMI invocations if the proxy has no delegate set yet.
    //

    ReferencePtr ref = proxy->__reference();
    if(ref->isWellKnown())
    {
        //
        // Check the active servant map to see if the well-known
        // proxy is for a local object.
        //
        return _servantManager->hasServant(ref->getIdentity());
    }
    else if(ref->isIndirect())
    {
        //
        // Proxy is local if the reference adapter id matches this
        // adapter id or replica group id.
        //
        return ref->getAdapterId() == _id || ref->getAdapterId() == _replicaGroupId;
    }
    else
    {
        vector<EndpointIPtr> endpoints = ref->getEndpoints();

        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
        checkForDeactivation();
        
        //
        // Proxies which have at least one endpoint in common with the
        // endpoints used by this object adapter are considered local.
        //
        for(vector<EndpointIPtr>::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p)
        {
            for(vector<IncomingConnectionFactoryPtr>::const_iterator q = _incomingConnectionFactories.begin(); 
                q != _incomingConnectionFactories.end(); ++q)
            {
                if((*p)->equivalent((*q)->endpoint()))
                {
                    return true;
                }
            }

            for(vector<EndpointIPtr>::const_iterator r = _publishedEndpoints.begin();
                r != _publishedEndpoints.end(); ++r)
            {
                if((*p)->equivalent(*r))
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
            for(vector<EndpointIPtr>::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p)
            {
                for(vector<EndpointIPtr>::const_iterator r = _routerEndpoints.begin(); r != _routerEndpoints.end(); ++r)
                {
                    if((*p)->equivalent(*r))
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

void
Ice::ObjectAdapterI::flushAsyncBatchRequests(const CommunicatorBatchOutgoingAsyncPtr& outAsync)
{
    vector<IncomingConnectionFactoryPtr> f;
    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
        f = _incomingConnectionFactories;
    }

    for(vector<IncomingConnectionFactoryPtr>::const_iterator p = f.begin(); p != f.end(); ++p)
    {
        (*p)->flushAsyncBatchRequests(outAsync);
    }
}

void
Ice::ObjectAdapterI::updateConnectionObservers()
{
    vector<IncomingConnectionFactoryPtr> f;
    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
        f = _incomingConnectionFactories;
    }
    for_each(f.begin(), f.end(), Ice::voidMemFun(&IncomingConnectionFactory::updateConnectionObservers));
}

void
Ice::ObjectAdapterI::updateThreadObservers()
{
    ThreadPoolPtr threadPool;
    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
        threadPool = _threadPool;
    }
    if(threadPool)
    {
        threadPool->updateObservers();
    }
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

Ice::Int
Ice::ObjectAdapterI::getACM() const
{
    // Not check for deactivation here!

    assert(_instance); // Must not be called after destroy().

    if(_hasAcmTimeout)
    {
        return _acmTimeout;
    }
    else
    {
        return _instance->serverACM();
    }
}

//
// COMPILERFIX: The ObjectAdapterI setup is broken out into a separate initialize
// function because when it was part of the constructor C++Builder 2010 apps would
// crash if an execption was thrown from any calls within the constructor.
//
Ice::ObjectAdapterI::ObjectAdapterI(const InstancePtr& instance, const CommunicatorPtr& communicator,
                                    const ObjectAdapterFactoryPtr& objectAdapterFactory, const string& name,
                                    /*const RouterPrx& router,*/ bool noConfig) :
    _deactivated(false),
    _instance(instance),
    _communicator(communicator),
    _objectAdapterFactory(objectAdapterFactory),
    _hasAcmTimeout(false),
    _acmTimeout(0),
    _servantManager(new ServantManager(instance, name)),
    _activateOneOffDone(false),
    _name(name),
    _directCount(0),
    _waitForActivate(false),
    _waitForHold(0),
    _waitForHoldRetry(false),
    _destroying(false),
    _destroyed(false),
    _noConfig(noConfig)
{
}

void
Ice::ObjectAdapterI::initialize(const RouterPrx& router)
{
    if(_noConfig)
    {
        _reference = _instance->referenceFactory()->create("dummy -t", "");
        return;
    }

    PropertiesPtr properties = _instance->initializationData().properties;
    StringSeq unknownProps;
    bool noProps = filterProperties(unknownProps);

    //
    // Warn about unknown object adapter properties.
    //
    if(unknownProps.size() != 0 && properties->getPropertyAsIntWithDefault("Ice.Warn.UnknownProperties", 1) > 0)
    {
        Warning out(_instance->initializationData().logger);
        out << "found unknown properties for object adapter `" << _name << "':";
        for(unsigned int i = 0; i < unknownProps.size(); ++i)
        {
            out << "\n    " << unknownProps[i];
        }
    }

    try
    {
        //
        // Make sure named adapter has some configuration
        //
        if(router == 0 && noProps)
        {
            InitializationException ex(__FILE__, __LINE__);
            ex.reason = "object adapter `" + _name + "' requires configuration";
            throw ex;
        }

        const_cast<string&>(_id) = properties->getProperty(_name + ".AdapterId");
        const_cast<string&>(_replicaGroupId) = properties->getProperty(_name + ".ReplicaGroupId");

        //
        // Setup a reference to be used to get the default proxy options
        // when creating new proxies. By default, create twoway proxies.
        //
        string proxyOptions = properties->getPropertyWithDefault(_name + ".ProxyOptions", "-t");
        try
        {
            _reference = _instance->referenceFactory()->create("dummy " + proxyOptions, "");
        }
        catch(const ProxyParseException&)
        {
            InitializationException ex(__FILE__, __LINE__);
            ex.reason = "invalid proxy options `" + proxyOptions + "' for object adapter `" + _name + "'";
            throw ex;
        }

        int threadPoolSize = properties->getPropertyAsInt(_name + ".ThreadPool.Size");
        int threadPoolSizeMax = properties->getPropertyAsInt(_name + ".ThreadPool.SizeMax");
        bool hasPriority = properties->getProperty(_name + ".ThreadPool.ThreadPriority") != "";

        //
        // Create the per-adapter thread pool, if necessary. This is done before the creation of the incoming
        // connection factory as the thread pool is needed during creation for the call to incFdsInUse.
        //
        if(threadPoolSize > 0 || threadPoolSizeMax > 0 || hasPriority)
        {
            _threadPool = new ThreadPool(_instance, _name + ".ThreadPool", 0);
        }
        
        _hasAcmTimeout = properties->getProperty(_name + ".ACM") != "";
        if(_hasAcmTimeout)
        {
            _acmTimeout = properties->getPropertyAsInt(_name + ".ACM");
            _instance->connectionMonitor()->checkIntervalForACM(_acmTimeout);
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
            vector<EndpointIPtr> endpoints = parseEndpoints(properties->getProperty(_name + ".Endpoints"), true);
            for(vector<EndpointIPtr>::iterator p = endpoints.begin(); p != endpoints.end(); ++p)
            {

                IncomingConnectionFactoryPtr factory = new IncomingConnectionFactory(_instance, *p, this);
                 factory->initialize(_name);
                _incomingConnectionFactories.push_back(factory);
            }

            if(endpoints.empty())
            {
                TraceLevelsPtr tl = _instance->traceLevels();
                if(tl->network >= 2)
                {
                    Trace out(_instance->initializationData().logger, tl->networkCat);
                    out << "created adapter `" << _name << "' without endpoints";
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
        throw;
    }
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
    ReferencePtr ref = _instance->referenceFactory()->create(ident, facet, _reference, endpoints);
    return _instance->proxyFactory()->referenceToProxy(ref);
}

ObjectPrx
Ice::ObjectAdapterI::newIndirectProxy(const Identity& ident, const string& facet, const string& id) const
{
    //
    // Create an indirect reference with the given adapter id.
    //
    ReferencePtr ref = _instance->referenceFactory()->create(ident, facet, _reference, id);
    
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
Ice::ObjectAdapterI::parseEndpoints(const string& endpts, bool oaEndpoints) const
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

        end = beg;
        while(true)
        {
            end = endpts.find(':', end);
            if(end == string::npos)
            {
                end = endpts.length();
                break;
            }
            else
            {
                bool quoted = false;
                string::size_type quote = beg;
                while(true)
                {
                    quote = endpts.find('\"', quote);
                    if(quote == string::npos || end < quote)
                    {
                        break;
                    }
                    else
                    {
                        quote = endpts.find('\"', ++quote);
                        if(quote == string::npos)
                        {
                            break;
                        }
                        else if(end < quote)
                        {
                            quoted = true;
                            break;
                        }
                        ++quote;
                    }
                }
                if(!quoted)
                {
                    break;
                }
                ++end;
            }
        }
        
        if(end == beg)
        {
            ++end;
            continue;
        }
        
        string s = endpts.substr(beg, end - beg);
        EndpointIPtr endp = _instance->endpointFactoryManager()->create(s, oaEndpoints);
        if(endp == 0)
        {
            EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "invalid object adapter endpoint `" + s + "'";
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
    vector<EndpointIPtr> endpoints = parseEndpoints(endpts, false);
    if(endpoints.empty())
    {
        //
        // If the PublishedEndpoints property isn't set, we compute the published enpdoints
        // from the OA endpoints, expanding any endpoints that may be listening on INADDR_ANY
        // to include actual addresses in the published endpoints.
        //
        for(unsigned int i = 0; i < _incomingConnectionFactories.size(); ++i)
        {
            vector<EndpointIPtr> endps = _incomingConnectionFactories[i]->endpoint()->expand();
            endpoints.insert(endpoints.end(), endps.begin(), endps.end());
        }
    }

    if(_instance->traceLevels()->network >= 1)
    {
        Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
        out << "published endpoints for object adapter `" << getName() << "':\n";
        for(unsigned int i = 0; i < endpoints.size(); ++i)
        {
            if(i > 0)
            {
                out << ":";
            }
            out << endpoints[i]->toString();
        }
    }

    return endpoints;
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
    // Call on the locator registry outside the synchronization to 
    // blocking other threads that need to lock this OA.
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
            if(_instance->traceLevels()->location >= 1)
            {
                Trace out(_instance->initializationData().logger, _instance->traceLevels()->locationCat);
                out << "couldn't update object adapter `" + _id + "' endpoints with the locator registry:\n";
                out << "the object adapter is not known to the locator registry";
            }

            NotRegisteredException ex(__FILE__, __LINE__);
            ex.kindOfObject = "object adapter";
            ex.id = _id;
            throw ex;
        }
        catch(const InvalidReplicaGroupIdException&)
        {
            if(_instance->traceLevels()->location >= 1)
            {
                Trace out(_instance->initializationData().logger, _instance->traceLevels()->locationCat);
                out << "couldn't update object adapter `" + _id + "' endpoints with the locator registry:\n";
                out << "the replica group `" << _replicaGroupId << "' is not known to the locator registry";
            }

            NotRegisteredException ex(__FILE__, __LINE__);
            ex.kindOfObject = "replica group";
            ex.id = _replicaGroupId;
            throw ex;
        }
        catch(const AdapterAlreadyActiveException&)
        {
            if(_instance->traceLevels()->location >= 1)
            {
                Trace out(_instance->initializationData().logger, _instance->traceLevels()->locationCat);
                out << "couldn't update object adapter `" + _id + "' endpoints with the locator registry:\n";
                out << "the object adapter endpoints are already set";
            }

            ObjectAdapterIdInUseException ex(__FILE__, __LINE__);
            ex.id = _id;
            throw ex;
        }
        catch(const LocalException& ex)
        {
            if(_instance->traceLevels()->location >= 1)
            {
                Trace out(_instance->initializationData().logger, _instance->traceLevels()->locationCat);
                out << "couldn't update object adapter `" + _id + "' endpoints with the locator registry:\n" << ex;
            }
            throw; // TODO: Shall we raise a special exception instead of a non obvious local exception?
        }

        if(_instance->traceLevels()->location >= 1)
        {
            Trace out(_instance->initializationData().logger, _instance->traceLevels()->locationCat);
            out << "updated object adapter `" + _id + "' endpoints with the locator registry\n";
            out << "endpoints = ";
            if(proxy)
            {
                EndpointSeq endpts = proxy ? proxy->ice_getEndpoints() : EndpointSeq();
                ostringstream o;
                transform(endpts.begin(), endpts.end(), ostream_iterator<string>(o, endpts.size() > 1 ? ":" : ""), 
                          Ice::constMemFun(&Endpoint::toString));
                out << o.str();
            }
        }
    }

    if(registerProcess && !serverId.empty())
    {
        {
            IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

            if(_processId.name == "")
            {
                ProcessPtr servant = new ProcessI(_communicator);
                _processId = addWithUUID(servant)->ice_getIdentity();
            }
        }

        try
        {
            locatorRegistry->setServerProcessProxy(serverId, ProcessPrx::uncheckedCast(createDirectProxy(_processId)));
        }
        catch(const ServerNotFoundException&)
        {
            if(_instance->traceLevels()->location >= 1)
            {
                Trace out(_instance->initializationData().logger, _instance->traceLevels()->locationCat);
                out << "couldn't register server `" + serverId + "' with the locator registry:\n";
                out << "the server is not known to the locator registry";
            }

            NotRegisteredException ex(__FILE__, __LINE__);
            ex.kindOfObject = "server";
            ex.id = serverId;
            throw ex;
        }
        catch(const Ice::LocalException& ex)
        {
            if(_instance->traceLevels()->location >= 1)
            {
                Trace out(_instance->initializationData().logger, _instance->traceLevels()->locationCat);
                out << "couldn't register server `" + serverId + "' with the locator registry:\n" << ex;
            }
            throw; // TODO: Shall we raise a special exception instead of a non obvious local exception?
        }

        if(_instance->traceLevels()->location >= 1)
        {
            Trace out(_instance->initializationData().logger, _instance->traceLevels()->locationCat);
            out << "registered server `" + serverId + "' with the locator registry";
        }
   }
}

bool
Ice::ObjectAdapterI::filterProperties(StringSeq& unknownProps)
{
    static const string suffixes[] = 
    { 
        "ACM",
        "AdapterId",
        "Endpoints",
        "Locator",
        "Locator.EncodingVersion",
        "Locator.EndpointSelection",
        "Locator.ConnectionCached",
        "Locator.PreferSecure",
        "Locator.CollocationOptimized",
        "Locator.Router",
        "PublishedEndpoints",
        "RegisterProcess",
        "ReplicaGroupId",
        "Router",
        "Router.EncodingVersion",
        "Router.EndpointSelection",
        "Router.ConnectionCached",
        "Router.PreferSecure",
        "Router.CollocationOptimized",
        "Router.Locator",
        "Router.Locator.EndpointSelection",
        "Router.Locator.ConnectionCached",
        "Router.Locator.PreferSecure",
        "Router.Locator.CollocationOptimized",
        "Router.Locator.LocatorCacheTimeout",
        "Router.LocatorCacheTimeout",
        "ProxyOptions",
        "ThreadPool.Size",
        "ThreadPool.SizeMax",
        "ThreadPool.SizeWarn",
        "ThreadPool.StackSize",
        "ThreadPool.Serialize",
        "ThreadPool.ThreadPriority"
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
    for(PropertyDict::const_iterator p = props.begin(); p != props.end(); ++p)
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
            else
            {
            }
        }

        if(!valid && addUnknown)
        {
            unknownProps.push_back(p->first);
        }
    }

    return noProps;
}
