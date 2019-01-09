// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/UUID.h>
#include <Ice/ObjectAdapterI.h>
#include <Ice/CommunicatorI.h>
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
#include <Ice/ConsoleUtil.h>

#ifdef _WIN32
#   include <sys/timeb.h>
#else
#   include <sys/time.h>
#endif

#include <iterator>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{
inline void checkIdentity(const Identity& ident)
{
    if(ident.name.empty())
    {
        throw IllegalIdentityException(__FILE__, __LINE__, ident);
    }
}

inline void checkServant(const ObjectPtr& servant)
{
    if(!servant)
    {
        throw IllegalServantException(__FILE__, __LINE__, "cannot add null servant to Object Adapter");
    }
}

inline EndpointIPtr toEndpointI(const EndpointPtr& endp)
{
    return ICE_DYNAMIC_CAST(EndpointI, endp);
}

}

string
Ice::ObjectAdapterI::getName() const ICE_NOEXCEPT
{
    //
    // No mutex lock necessary, _name is immutable.
    //
    return _noConfig ? string("") : _name;
}

CommunicatorPtr
Ice::ObjectAdapterI::getCommunicator() const ICE_NOEXCEPT
{
    return _communicator;
}

void
Ice::ObjectAdapterI::activate()
{
    LocatorInfoPtr locatorInfo;
    bool printAdapterReady = false;

    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

        checkForDeactivation();

        //
        // If we've previously been initialized we just need to activate the
        // incoming connection factories and we're done.
        //
        if(_state != StateUninitialized)
        {
#ifdef ICE_CPP11_MAPPING
            for_each(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(),
                [](const IncomingConnectionFactoryPtr& factory)
                {
                    factory->activate();
                });
#else
            for_each(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(),
                     Ice::voidMemFun(&IncomingConnectionFactory::activate));
#endif
            return;
        }

        //
        // One off initializations of the adapter: update the
        // locator registry and print the "adapter ready"
        // message. We set set state to StateActivating to prevent
        // deactivation from other threads while these one off
        // initializations are done.
        //
        _state = StateActivating;

        locatorInfo = _locatorInfo;
        if(!_noConfig)
        {
            PropertiesPtr properties = _instance->initializationData().properties;
            printAdapterReady = properties->getPropertyAsInt("Ice.PrintAdapterReady") > 0;
        }
    }

    try
    {
        Ice::Identity dummy;
        dummy.name = "dummy";
        updateLocatorRegistry(locatorInfo, createDirectProxy(dummy));
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
            _state = StateUninitialized;
            notifyAll();
        }
        throw;
    }

    if(printAdapterReady)
    {
        consoleOut << _name << " ready" << endl;
    }

    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
        assert(_state == StateActivating);

#ifdef ICE_CPP11_MAPPING
            for_each(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(),
                [](const IncomingConnectionFactoryPtr& factory)
                {
                    factory->activate();
                });
#else
        for_each(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(),
                 Ice::voidMemFun(&IncomingConnectionFactory::activate));
#endif
        _state = StateActive;
        notifyAll();
    }
}

void
Ice::ObjectAdapterI::hold()
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    checkForDeactivation();
    _state = StateHeld;

#ifdef ICE_CPP11_MAPPING
    for_each(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(),
        [](const IncomingConnectionFactoryPtr& factory)
        {
            factory->hold();
        });
#else
    for_each(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(),
             Ice::voidMemFun(&IncomingConnectionFactory::hold));
#endif
}

void
Ice::ObjectAdapterI::waitForHold()
{
    vector<IncomingConnectionFactoryPtr> incomingConnectionFactories;
    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

        checkForDeactivation();

        incomingConnectionFactories = _incomingConnectionFactories;
    }

#ifdef ICE_CPP11_MAPPING
    for_each(incomingConnectionFactories.begin(), incomingConnectionFactories.end(),
        [](const IncomingConnectionFactoryPtr& factory)
        {
            factory->waitUntilHolding();
        });
#else
    for_each(incomingConnectionFactories.begin(), incomingConnectionFactories.end(),
             Ice::constVoidMemFun(&IncomingConnectionFactory::waitUntilHolding));
#endif
}

void
Ice::ObjectAdapterI::deactivate() ICE_NOEXCEPT
{
    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

        //
        // Wait for activation to complete. This is necessary to not
        // get out of order locator updates.
        //
        while(_state == StateActivating || _state == StateDeactivating)
        {
            wait();
        }
        if(_state >= StateDeactivated)
        {
            return;
        }
        _state = StateDeactivating;
    }

    //
    // NOTE: the router/locator infos and incoming connection
    // facatory list are immutable at this point.
    //

    try
    {
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

        updateLocatorRegistry(_locatorInfo, 0);
    }
    catch(const Ice::LocalException&)
    {
        //
        // We can't throw exceptions in deactivate so we ignore
        // failures to update the locator registry.
        //
    }

#ifdef ICE_CPP11_MAPPING
    for_each(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(),
        [](const IncomingConnectionFactoryPtr& factory)
        {
            factory->destroy();
        });
#else
    for_each(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(),
             Ice::voidMemFun(&IncomingConnectionFactory::destroy));
#endif

    _instance->outgoingConnectionFactory()->removeAdapter(ICE_SHARED_FROM_THIS);

    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
        assert(_state == StateDeactivating);
        _state = StateDeactivated;
        notifyAll();
    }
}

void
Ice::ObjectAdapterI::waitForDeactivate() ICE_NOEXCEPT
{
    vector<IceInternal::IncomingConnectionFactoryPtr> incomingConnectionFactories;

    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

        //
        // Wait for deactivation of the adapter itself, and for
        // the return of all direct method calls using this adapter.
        //
        while((_state < StateDeactivated) || _directCount > 0)
        {
            wait();
        }
        if(_state > StateDeactivated)
        {
            return;
        }
        incomingConnectionFactories = _incomingConnectionFactories;
    }

    //
    // Now we wait until all incoming connection factories are
    // finished.
    //
#ifdef ICE_CPP11_MAPPING
    for_each(incomingConnectionFactories.begin(), incomingConnectionFactories.end(),
        [](const IncomingConnectionFactoryPtr& factory)
        {
            factory->waitUntilFinished();
        });
#else
    for_each(incomingConnectionFactories.begin(), incomingConnectionFactories.end(),
             Ice::voidMemFun(&IncomingConnectionFactory::waitUntilFinished));
#endif
}

bool
Ice::ObjectAdapterI::isDeactivated() const ICE_NOEXCEPT
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    return _state >= StateDeactivated;
}

void
Ice::ObjectAdapterI::destroy() ICE_NOEXCEPT
{
    //
    // Deactivate and wait for completion.
    //
    deactivate();
    waitForDeactivate();

    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
        assert(_state >= StateDeactivated);

        //
        // Only a single thread is allowed to destroy the object
        // adapter. Other threads wait for the destruction to be
        // completed.
        //
        while(_state == StateDestroying)
        {
            wait();
        }
        if(_state == StateDestroyed)
        {
            return;
        }
        _state = StateDestroying;
    }

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

    if(_objectAdapterFactory)
    {
        _objectAdapterFactory->removeObjectAdapter(ICE_SHARED_FROM_THIS);
    }

    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

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
        _routerInfo = 0;
        _publishedEndpoints.clear();
        _locatorInfo = 0;
        _reference = 0;
        _objectAdapterFactory = 0;

        _state = StateDestroyed;
        notifyAll();
    }
}

ObjectPrxPtr
Ice::ObjectAdapterI::add(const ObjectPtr& object, const Identity& ident)
{
    return addFacet(object, ident, "");
}

ObjectPrxPtr
Ice::ObjectAdapterI::addFacet(const ObjectPtr& object, const Identity& ident, const string& facet)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    checkForDeactivation();
    checkServant(object);
    checkIdentity(ident);

    _servantManager->addServant(object, ident, facet);

    return newProxy(ident, facet);
}

ObjectPrxPtr
Ice::ObjectAdapterI::addWithUUID(const ObjectPtr& object)
{
    return addFacetWithUUID(object, "");
}

ObjectPrxPtr
Ice::ObjectAdapterI::addFacetWithUUID(const ObjectPtr& object, const string& facet)
{
    Identity ident;
    ident.name = Ice::generateUUID();
    return addFacet(object, ident, facet);
}

void
Ice::ObjectAdapterI::addDefaultServant(const ObjectPtr& servant, const string& category)
{
    checkServant(servant);

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
Ice::ObjectAdapterI::findByProxy(const ObjectPrxPtr& proxy) const
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    checkForDeactivation();

    ReferencePtr ref = proxy->_getReference();
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

ObjectPrxPtr
Ice::ObjectAdapterI::createProxy(const Identity& ident) const
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    checkForDeactivation();
    checkIdentity(ident);

    return newProxy(ident, "");
}

ObjectPrxPtr
Ice::ObjectAdapterI::createDirectProxy(const Identity& ident) const
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    checkForDeactivation();
    checkIdentity(ident);

    return newDirectProxy(ident, "");
}

ObjectPrxPtr
Ice::ObjectAdapterI::createIndirectProxy(const Identity& ident) const
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    checkForDeactivation();
    checkIdentity(ident);

    return newIndirectProxy(ident, "", _id);
}

void
Ice::ObjectAdapterI::setLocator(const LocatorPrxPtr& locator)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    checkForDeactivation();

    _locatorInfo = _instance->locatorManager()->get(locator);
}

LocatorPrxPtr
Ice::ObjectAdapterI::getLocator() const ICE_NOEXCEPT
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    if(!_locatorInfo)
    {
        return 0;
    }
    else
    {
        return _locatorInfo->getLocator();
    }
}

EndpointSeq
Ice::ObjectAdapterI::getEndpoints() const ICE_NOEXCEPT
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    EndpointSeq endpoints;
    transform(_incomingConnectionFactories.begin(), _incomingConnectionFactories.end(),
            back_inserter(endpoints),
#ifdef ICE_CPP11_MAPPING
            [](const IncomingConnectionFactoryPtr& factory)
            {
                return factory->endpoint();
            });
#else
            Ice::constMemFun(&IncomingConnectionFactory::endpoint));
#endif
    return endpoints;
}

void
Ice::ObjectAdapterI::refreshPublishedEndpoints()
{
    LocatorInfoPtr locatorInfo;
    vector<EndpointIPtr> oldPublishedEndpoints;

    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
        checkForDeactivation();

        oldPublishedEndpoints = _publishedEndpoints;
        _publishedEndpoints = computePublishedEndpoints();

        locatorInfo = _locatorInfo;
    }

    try
    {
        Ice::Identity dummy;
        dummy.name = "dummy";
        updateLocatorRegistry(locatorInfo, createDirectProxy(dummy));
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
Ice::ObjectAdapterI::getPublishedEndpoints() const ICE_NOEXCEPT
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
    return EndpointSeq(_publishedEndpoints.begin(), _publishedEndpoints.end());
}

void
Ice::ObjectAdapterI::setPublishedEndpoints(const EndpointSeq& newEndpoints)
{
    LocatorInfoPtr locatorInfo;
    vector<EndpointIPtr> oldPublishedEndpoints;
    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
        checkForDeactivation();

        if(_routerInfo)
        {
            const string s("can't set published endpoints on object adapter associated with a router");
    #ifdef ICE_CPP11_MAPPING
            throw invalid_argument(s);
    #else
            throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, s);
    #endif
        }

        oldPublishedEndpoints = _publishedEndpoints;
        _publishedEndpoints.clear();
        transform(newEndpoints.begin(), newEndpoints.end(), back_inserter(_publishedEndpoints), toEndpointI);

        locatorInfo = _locatorInfo;
    }

    try
    {
        Ice::Identity dummy;
        dummy.name = "dummy";
        updateLocatorRegistry(locatorInfo, createDirectProxy(dummy));
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

bool
Ice::ObjectAdapterI::isLocal(const ObjectPrxPtr& proxy) const
{
    //
    // NOTE: it's important that isLocal() doesn't perform any blocking operations as
    // it can be called for AMI invocations if the proxy has no delegate set yet.
    //

    ReferencePtr ref = proxy->_getReference();
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
                if((*q)->isLocal(*p))
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
    }

    return false;
}

void
Ice::ObjectAdapterI::flushAsyncBatchRequests(const CommunicatorFlushBatchAsyncPtr& outAsync, CompressBatch compress)
{
    vector<IncomingConnectionFactoryPtr> f;
    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
        f = _incomingConnectionFactories;
    }

    for(vector<IncomingConnectionFactoryPtr>::const_iterator p = f.begin(); p != f.end(); ++p)
    {
        (*p)->flushAsyncBatchRequests(outAsync, compress);
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
#ifdef ICE_CPP11_MAPPING
    for_each(f.begin(), f.end(),
        [](const IncomingConnectionFactoryPtr& factory)
        {
            factory->updateConnectionObservers();
        });
#else
    for_each(f.begin(), f.end(), Ice::voidMemFun(&IncomingConnectionFactory::updateConnectionObservers));
#endif
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

IceInternal::ACMConfig
Ice::ObjectAdapterI::getACM() const
{
    // Not check for deactivation here!

    assert(_instance); // Must not be called after destroy().
    return _acm;
}

void
Ice::ObjectAdapterI::setAdapterOnConnection(const Ice::ConnectionIPtr& connection)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
    checkForDeactivation();
    connection->setAdapterAndServantManager(ICE_SHARED_FROM_THIS, _servantManager);
}

//
// COMPILERFIX: The ObjectAdapterI setup is broken out into a separate initialize
// function because when it was part of the constructor C++Builder 2010 apps would
// crash if an execption was thrown from any calls within the constructor.
//
Ice::ObjectAdapterI::ObjectAdapterI(const InstancePtr& instance, const CommunicatorPtr& communicator,
                                    const ObjectAdapterFactoryPtr& objectAdapterFactory, const string& name,
                                    /*const RouterPrxPtr& router,*/ bool noConfig) :
    _state(StateUninitialized),
    _instance(instance),
    _communicator(communicator),
    _objectAdapterFactory(objectAdapterFactory),
    _servantManager(new ServantManager(instance, name)),
    _name(name),
    _directCount(0),
    _noConfig(noConfig),
    _messageSizeMax(0)
{
}

void
Ice::ObjectAdapterI::initialize(const RouterPrxPtr& router)
{
    if(_noConfig)
    {
        _reference = _instance->referenceFactory()->create("dummy -t", "");
        const_cast<ACMConfig&>(_acm) = _instance->serverACM();
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
            throw InitializationException(__FILE__, __LINE__, "object adapter `" + _name + "' requires configuration");
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
            throw InitializationException(__FILE__, __LINE__, "invalid proxy options `" + proxyOptions +
                                          "' for object adapter `" + _name + "'");
        }

        const_cast<ACMConfig&>(_acm) =
            ACMConfig(properties, _communicator->getLogger(), _name + ".ACM", _instance->serverACM());

        {
            const int defaultMessageSizeMax = static_cast<int>(_instance->messageSizeMax() / 1024);
            Int num = properties->getPropertyAsIntWithDefault(_name + ".MessageSizeMax", defaultMessageSizeMax);
            if(num < 1 || static_cast<size_t>(num) > static_cast<size_t>(0x7fffffff / 1024))
            {
                const_cast<size_t&>(_messageSizeMax) = static_cast<size_t>(0x7fffffff);
            }
            else
            {
                const_cast<size_t&>(_messageSizeMax) = static_cast<size_t>(num) * 1024;
            }
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

        if(!router)
        {
            const_cast<RouterPrxPtr&>(router) = ICE_UNCHECKED_CAST(RouterPrx,
                                _instance->proxyFactory()->propertyToProxy(_name + ".Router"));
        }
        if(router)
        {
            _routerInfo = _instance->routerManager()->get(router);
            assert(_routerInfo);

            //
            // Make sure this router is not already registered with another adapter.
            //
            if(_routerInfo->getAdapter())
            {
                throw AlreadyRegisteredException(__FILE__, __LINE__,
                                                 "object adapter with router",
                                                 _communicator->identityToString(router->ice_getIdentity()));
            }

            //
            // Associate this object adapter with the router. This way, new outgoing connections
            // to the router's client proxy will use this object adapter for callbacks.
            //
            _routerInfo->setAdapter(ICE_SHARED_FROM_THIS);

            //
            // Also modify all existing outgoing connections to the router's client proxy to use
            // this object adapter for callbacks.
            //
            _instance->outgoingConnectionFactory()->setRouterInfo(_routerInfo);
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
                EndpointIPtr publishedEndpoint;
                vector<EndpointIPtr> expanded = (*p)->expandHost(publishedEndpoint);
                for(vector<EndpointIPtr>::iterator q = expanded.begin(); q != expanded.end(); ++q)
                {
                    IncomingConnectionFactoryPtr factory = ICE_MAKE_SHARED(IncomingConnectionFactory,
                                                                           _instance,
                                                                           *q,
                                                                           publishedEndpoint,
                                                                           ICE_SHARED_FROM_THIS);
                    factory->initialize();
                    _incomingConnectionFactories.push_back(factory);
                }
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
        }

        //
        // Compute the published endpoints.
        //
        _publishedEndpoints = computePublishedEndpoints();

        if(!properties->getProperty(_name + ".Locator").empty())
        {
            setLocator(ICE_UNCHECKED_CAST(LocatorPrx, _instance->proxyFactory()->propertyToProxy(_name + ".Locator")));
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
    if(_state < StateDeactivated)
    {
        Warning out(_instance->initializationData().logger);
        out << "object adapter `" << getName() << "' has not been deactivated";
    }
    else if(_state != StateDestroyed)
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
    }
}

ObjectPrxPtr
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

ObjectPrxPtr
Ice::ObjectAdapterI::newDirectProxy(const Identity& ident, const string& facet) const
{
    //
    // Create a reference and return a proxy for this reference.
    //
    ReferencePtr ref = _instance->referenceFactory()->create(ident, facet, _reference, _publishedEndpoints);
    return _instance->proxyFactory()->referenceToProxy(ref);
}

ObjectPrxPtr
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
    if(_state >= StateDeactivating)
    {
        throw ObjectAdapterDeactivatedException(__FILE__, __LINE__, getName());
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
            if(!endpoints.empty())
            {
                throw EndpointParseException(__FILE__, __LINE__, "invalid empty object adapter endpoint");
            }
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
            throw EndpointParseException(__FILE__, __LINE__, "invalid empty object adapter endpoint");
        }

        string s = endpts.substr(beg, end - beg);
        EndpointIPtr endp = _instance->endpointFactoryManager()->create(s, oaEndpoints);
        if(endp == 0)
        {
            throw EndpointParseException(__FILE__, __LINE__, "invalid object adapter endpoint `" + s + "'");
        }
        endpoints.push_back(endp);

        ++end;
    }

    return endpoints;
}

std::vector<EndpointIPtr>
ObjectAdapterI::computePublishedEndpoints()
{
    vector<EndpointIPtr> endpoints;
    if(_routerInfo)
    {
        //
        // Get the router's server proxy endpoints and use them as the published endpoints.
        //
        vector<EndpointIPtr> endps = _routerInfo->getServerEndpoints();
        for(vector<EndpointIPtr>::const_iterator p = endps.begin(); p != endps.end(); ++p)
        {
            if(::find(endpoints.begin(), endpoints.end(), *p) == endpoints.end())
            {
                endpoints.push_back(*p);
            }
        }
    }
    else
    {
        //
        // Parse published endpoints. If set, these are used in proxies
        // instead of the connection factory endpoints.
        //
        string endpts = _communicator->getProperties()->getProperty(_name + ".PublishedEndpoints");
        endpoints = parseEndpoints(endpts, false);
        if(endpoints.empty())
        {
            //
            // If the PublishedEndpoints property isn't set, we compute the published enpdoints
            // from the OA endpoints, expanding any endpoints that may be listening on INADDR_ANY
            // to include actual addresses in the published endpoints.
            //
            for(unsigned int i = 0; i < _incomingConnectionFactories.size(); ++i)
            {
                vector<EndpointIPtr> endps = _incomingConnectionFactories[i]->endpoint()->expandIfWildcard();
                for(vector<EndpointIPtr>::const_iterator p = endps.begin(); p != endps.end(); ++p)
                {
                    //
                    // Check for duplicate endpoints, this might occur if an endpoint with a DNS name
                    // expands to multiple addresses. In this case, multiple incoming connection
                    // factories can point to the same published endpoint.
                    //
                    if(::find(endpoints.begin(), endpoints.end(), *p) == endpoints.end())
                    {
                        endpoints.push_back(*p);
                    }
                }
            }
        }
    }

    if(_instance->traceLevels()->network >= 1 && !endpoints.empty())
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
ObjectAdapterI::updateLocatorRegistry(const IceInternal::LocatorInfoPtr& locatorInfo, const Ice::ObjectPrxPtr& proxy)
{
    if(_id.empty() || !locatorInfo)
    {
        return; // Nothing to update.
    }

    LocatorRegistryPrxPtr locatorRegistry = locatorInfo->getLocatorRegistry();
    if(!locatorRegistry)
    {
        return;
    }

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

        throw NotRegisteredException(__FILE__, __LINE__, "object adapter", _id);
    }
    catch(const InvalidReplicaGroupIdException&)
    {
        if(_instance->traceLevels()->location >= 1)
        {
            Trace out(_instance->initializationData().logger, _instance->traceLevels()->locationCat);
            out << "couldn't update object adapter `" + _id + "' endpoints with the locator registry:\n";
            out << "the replica group `" << _replicaGroupId << "' is not known to the locator registry";
        }

        throw NotRegisteredException(__FILE__, __LINE__, "replica group", _replicaGroupId);
    }
    catch(const AdapterAlreadyActiveException&)
    {
        if(_instance->traceLevels()->location >= 1)
        {
            Trace out(_instance->initializationData().logger, _instance->traceLevels()->locationCat);
            out << "couldn't update object adapter `" + _id + "' endpoints with the locator registry:\n";
            out << "the object adapter endpoints are already set";
        }

        throw ObjectAdapterIdInUseException(__FILE__, __LINE__, _id);
    }
    catch(const ObjectAdapterDeactivatedException&)
    {
        // Expected if collocated call and OA is deactivated, ignore.
    }
    catch(const CommunicatorDestroyedException&)
    {
        // Ignore.
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
#ifdef ICE_CPP11_COMPILER
            transform(endpts.begin(), endpts.end(), ostream_iterator<string>(o, endpts.size() > 1 ? ":" : ""),
                      [](const EndpointPtr& endpoint)
                      {
                          return endpoint->toString();
                      });
#else
            transform(endpts.begin(), endpts.end(), ostream_iterator<string>(o, endpts.size() > 1 ? ":" : ""),
                      Ice::constMemFun(&Endpoint::toString));
#endif
            out << o.str();
        }
    }
}

bool
Ice::ObjectAdapterI::filterProperties(StringSeq& unknownProps)
{
    static const string suffixes[] =
    {
        "ACM",
        "ACM.Close",
        "ACM.Heartbeat",
        "ACM.Timeout",
        "AdapterId",
        "Endpoints",
        "Locator",
        "Locator.EncodingVersion",
        "Locator.EndpointSelection",
        "Locator.ConnectionCached",
        "Locator.PreferSecure",
        "Locator.CollocationOptimized",
        "Locator.Router",
        "MessageSizeMax",
        "PublishedEndpoints",
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
        "Router.Locator.InvocationTimeout",
        "Router.LocatorCacheTimeout",
        "Router.InvocationTimeout",
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
        }

        if(!valid && addUnknown)
        {
            unknownProps.push_back(p->first);
        }
    }

    return noProps;
}
