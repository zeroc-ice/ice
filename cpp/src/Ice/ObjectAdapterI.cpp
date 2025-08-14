// Copyright (c) ZeroC, Inc.

#include "ObjectAdapterI.h"
#include "CheckIdentity.h"
#include "ConnectionFactory.h"
#include "ConsoleUtil.h"
#include "DefaultsAndOverrides.h"
#include "EndpointFactoryManager.h"
#include "EndpointI.h"
#include "Ice/Communicator.h"
#include "Ice/LocalExceptions.h"
#include "Ice/Locator.h"
#include "Ice/LoggerUtil.h"
#include "Ice/Properties.h"
#include "Ice/Proxy.h"
#include "Ice/ProxyFunctions.h"
#include "Ice/Router.h"
#include "Ice/UUID.h"
#include "Instance.h"
#include "LocatorInfo.h"
#include "LoggerMiddleware.h"
#include "ObjectAdapterFactory.h"
#include "ObserverMiddleware.h"
#include "PropertyNames.h"
#include "PropertyUtil.h"
#include "ReferenceFactory.h"
#include "RouterInfo.h"
#include "ServantManager.h"
#include "ThreadPool.h"
#include "TraceLevels.h"

#ifdef _WIN32
#    include <sys/timeb.h>
#else
#    include <sys/time.h>
#endif

#include <algorithm>
#include <iterator>
#include <stdexcept>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{
    inline EndpointIPtr toEndpointI(const EndpointPtr& endp) { return dynamic_pointer_cast<EndpointI>(endp); }

    string emptyName{};
}

Ice::ObjectAdapter::~ObjectAdapter() = default; // avoid weak vtable

const string&
Ice::ObjectAdapterI::getName() const noexcept
{
    return _noConfig ? emptyName : _name;
}

CommunicatorPtr
Ice::ObjectAdapterI::getCommunicator() const noexcept
{
    return _communicator;
}

void
Ice::ObjectAdapterI::activate()
{
    LocatorInfoPtr locatorInfo;
    bool printAdapterReady = false;

    {
        lock_guard lock(_mutex);

        checkForDeactivation();

        //
        // If we've previously been initialized we just need to activate the
        // incoming connection factories and we're done.
        //
        if (_state != StateUninitialized)
        {
            for_each(
                _incomingConnectionFactories.begin(),
                _incomingConnectionFactories.end(),
                [](const IncomingConnectionFactoryPtr& factory) { factory->activate(); });

            _state = StateActive;
            _conditionVariable.notify_all();
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
        if (!_noConfig)
        {
            PropertiesPtr properties = _instance->initializationData().properties;
            printAdapterReady = properties->getIcePropertyAsInt("Ice.PrintAdapterReady") > 0;
        }
    }

    try
    {
        Ice::Identity dummy;
        dummy.name = "dummy";
        updateLocatorRegistry(locatorInfo, createDirectProxy(dummy));
    }
    catch (const Ice::LocalException&)
    {
        //
        // If we couldn't update the locator registry, we let the
        // exception go through and don't activate the adapter to
        // allow to user code to retry activating the adapter
        // later.
        //
        {
            lock_guard lock(_mutex);
            _state = StateUninitialized;
            _conditionVariable.notify_all();
        }
        throw;
    }

    if (printAdapterReady)
    {
        consoleOut << _name << " ready" << endl;
    }

    {
        lock_guard lock(_mutex);
        assert(_state == StateActivating);
        for_each(
            _incomingConnectionFactories.begin(),
            _incomingConnectionFactories.end(),
            [](const IncomingConnectionFactoryPtr& factory) { factory->activate(); });
        _state = StateActive;
        _conditionVariable.notify_all();
    }
}

void
Ice::ObjectAdapterI::hold()
{
    lock_guard lock(_mutex);

    checkForDeactivation();
    _state = StateHeld;
    for_each(
        _incomingConnectionFactories.begin(),
        _incomingConnectionFactories.end(),
        [](const IncomingConnectionFactoryPtr& factory) { factory->hold(); });
}

void
Ice::ObjectAdapterI::waitForHold()
{
    vector<IncomingConnectionFactoryPtr> incomingConnectionFactories;
    {
        lock_guard lock(_mutex);

        checkForDeactivation();

        incomingConnectionFactories = _incomingConnectionFactories;
    }

    for_each(
        incomingConnectionFactories.begin(),
        incomingConnectionFactories.end(),
        [](const IncomingConnectionFactoryPtr& factory) { factory->waitUntilHolding(); });
}

void
Ice::ObjectAdapterI::deactivate() noexcept
{
    {
        unique_lock lock(_mutex);

        // Wait for activation or a previous deactivation to complete.
        // This is necessary to avoid out of order locator updates.
        _conditionVariable.wait(lock, [this] { return _state != StateActivating && _state != StateDeactivating; });

        if (_state >= StateDeactivated)
        {
            return;
        }
        _state = StateDeactivating;
    }

    //
    // NOTE: the router/locator infos and incoming connection
    // factory list are immutable at this point.
    //

    try
    {
        updateLocatorRegistry(_locatorInfo, nullopt);
    }
    catch (const Ice::LocalException&)
    {
        //
        // We can't throw exceptions in deactivate so we ignore
        // failures to update the locator registry.
        //
    }

    for_each(
        _incomingConnectionFactories.begin(),
        _incomingConnectionFactories.end(),
        [](const IncomingConnectionFactoryPtr& factory) { factory->destroy(); });

    {
        lock_guard lock(_mutex);
        assert(_state == StateDeactivating);
        _state = StateDeactivated;
        _conditionVariable.notify_all();
    }
}

void
Ice::ObjectAdapterI::waitForDeactivate() noexcept
{
    vector<IceInternal::IncomingConnectionFactoryPtr> incomingConnectionFactories;

    {
        unique_lock lock(_mutex);

        //
        // Wait for deactivation of the adapter itself, and for
        // the return of all direct method calls using this adapter.
        //
        _conditionVariable.wait(lock, [this] { return _state >= StateDeactivated && _directCount == 0; });
        if (_state > StateDeactivated)
        {
            return;
        }
        incomingConnectionFactories = _incomingConnectionFactories;
    }

    // Now we wait until all incoming connection factories are finished.
    for_each(
        incomingConnectionFactories.begin(),
        incomingConnectionFactories.end(),
        [](const IncomingConnectionFactoryPtr& factory) { factory->waitUntilFinished(); });
}

bool
Ice::ObjectAdapterI::isDeactivated() const noexcept
{
    lock_guard lock(_mutex);

    return _state >= StateDeactivated;
}

void
Ice::ObjectAdapterI::destroy() noexcept
{
    // Deactivate and wait for completion.
    deactivate();
    waitForDeactivate();

    {
        unique_lock lock(_mutex);
        assert(_state >= StateDeactivated);

        //
        // Only a single thread is allowed to destroy the object
        // adapter. Other threads wait for the destruction to be
        // completed.
        //
        _conditionVariable.wait(lock, [this] { return _state != StateDestroying; });
        if (_state == StateDestroyed)
        {
            return;
        }
        _state = StateDestroying;
    }

    if (_routerInfo)
    {
        // Remove entry from the router manager.
        _instance->routerManager()->erase(_routerInfo->getRouter());

        // Clear this object adapter with the router.
        _routerInfo->setAdapter(nullptr);
    }

    _instance->outgoingConnectionFactory()->removeAdapter(shared_from_this());

    //
    // Now it's also time to clean up our servants and servant
    // locators.
    //
    _servantManager->destroy();

    //
    // Destroy the thread pool.
    //
    if (_threadPool)
    {
        _threadPool->destroy();
        _threadPool->joinWithAllThreads();
    }

    if (_objectAdapterFactory)
    {
        _objectAdapterFactory->removeObjectAdapter(shared_from_this());
    }

    {
        lock_guard lock(_mutex);

        //
        // We're done, now we can throw away all incoming connection
        // factories.
        //
        _incomingConnectionFactories.clear();

        //
        // Remove object references (some of them cyclic).
        //
        _instance = nullptr;
        _threadPool = nullptr;
        _routerInfo = nullptr;
        _publishedEndpoints.clear();
        _locatorInfo = nullptr;
        _reference = nullptr;
        _objectAdapterFactory = nullptr;

        _state = StateDestroyed;
        _conditionVariable.notify_all();
    }
}

ObjectAdapterPtr
Ice::ObjectAdapterI::use(function<ObjectPtr(ObjectPtr)> middlewareFactory)
{
    // This code is not thread-safe and is not supposed to be.
    if (_dispatchPipeline)
    {
        throw InitializationException{__FILE__, __LINE__, "all middleware must be installed before the first dispatch"};
    }

    _middlewareFactoryStack.push(std::move(middlewareFactory));
    return shared_from_this();
}

ObjectPrx
Ice::ObjectAdapterI::_add(ObjectPtr object, Identity ident)
{
    return _addFacet(std::move(object), std::move(ident), "");
}

ObjectPrx
Ice::ObjectAdapterI::_addFacet(ObjectPtr object, Identity ident, string facet)
{
    lock_guard lock(_mutex);

    checkForDestruction();
    if (!object)
    {
        throw std::invalid_argument{"cannot add null servant to Ice object adapter"};
    }
    checkIdentity(ident, __FILE__, __LINE__);

    _servantManager->addServant(std::move(object), ident, facet);

    return newProxy(std::move(ident), std::move(facet));
}

ObjectPrx
Ice::ObjectAdapterI::_addWithUUID(ObjectPtr object)
{
    return _addFacetWithUUID(std::move(object), "");
}

ObjectPrx
Ice::ObjectAdapterI::_addFacetWithUUID(ObjectPtr object, string facet)
{
    Identity ident;
    ident.name = Ice::generateUUID();
    return _addFacet(std::move(object), std::move(ident), std::move(facet));
}

void
Ice::ObjectAdapterI::addDefaultServant(ObjectPtr servant, string category)
{
    if (!servant)
    {
        throw std::invalid_argument{"cannot add null servant to Ice object adapter"};
    }

    lock_guard lock(_mutex);

    checkForDestruction();
    _servantManager->addDefaultServant(std::move(servant), std::move(category));
}

ObjectPtr
Ice::ObjectAdapterI::remove(const Identity& ident)
{
    return removeFacet(ident, "");
}

ObjectPtr
Ice::ObjectAdapterI::removeFacet(const Identity& ident, string_view facet)
{
    lock_guard lock(_mutex);

    checkForDestruction();
    checkIdentity(ident, __FILE__, __LINE__);

    return _servantManager->removeServant(ident, facet);
}

FacetMap
Ice::ObjectAdapterI::removeAllFacets(const Identity& ident)
{
    lock_guard lock(_mutex);

    checkForDestruction();
    checkIdentity(ident, __FILE__, __LINE__);

    return _servantManager->removeAllFacets(ident);
}

ObjectPtr
Ice::ObjectAdapterI::removeDefaultServant(string_view category)
{
    lock_guard lock(_mutex);

    checkForDestruction();

    return _servantManager->removeDefaultServant(category);
}

ObjectPtr
Ice::ObjectAdapterI::find(const Identity& ident) const
{
    return findFacet(ident, "");
}

ObjectPtr
Ice::ObjectAdapterI::findFacet(const Identity& ident, string_view facet) const
{
    lock_guard lock(_mutex);

    checkForDestruction();
    checkIdentity(ident, __FILE__, __LINE__);

    return _servantManager->findServant(ident, facet);
}

FacetMap
Ice::ObjectAdapterI::findAllFacets(const Identity& ident) const
{
    lock_guard lock(_mutex);

    checkForDestruction();
    checkIdentity(ident, __FILE__, __LINE__);

    return _servantManager->findAllFacets(ident);
}

ObjectPtr
Ice::ObjectAdapterI::findByProxy(const ObjectPrx& proxy) const
{
    lock_guard lock(_mutex);

    checkForDestruction();

    ReferencePtr ref = proxy->_getReference();
    return findFacet(ref->getIdentity(), ref->getFacet());
}

ObjectPtr
Ice::ObjectAdapterI::findDefaultServant(string_view category) const
{
    lock_guard lock(_mutex);

    checkForDestruction();

    return _servantManager->findDefaultServant(category);
}

void
Ice::ObjectAdapterI::addServantLocator(ServantLocatorPtr locator, string prefix)
{
    lock_guard lock(_mutex);

    checkForDestruction();

    _servantManager->addServantLocator(std::move(locator), std::move(prefix));
}

ServantLocatorPtr
Ice::ObjectAdapterI::removeServantLocator(string_view prefix)
{
    lock_guard lock(_mutex);

    checkForDestruction();

    return _servantManager->removeServantLocator(prefix);
}

ServantLocatorPtr
Ice::ObjectAdapterI::findServantLocator(string_view prefix) const
{
    lock_guard lock(_mutex);

    checkForDestruction();

    return _servantManager->findServantLocator(prefix);
}

const ObjectPtr&
Ice::ObjectAdapterI::dispatchPipeline() const noexcept
{
    lock_guard lock(_mutex);
    if (!_dispatchPipeline)
    {
        _dispatchPipeline = _servantManager;
        while (!_middlewareFactoryStack.empty())
        {
            _dispatchPipeline = _middlewareFactoryStack.top()(std::move(_dispatchPipeline));
            _middlewareFactoryStack.pop();
        }
    }
    return _dispatchPipeline;
}

ObjectPrx
Ice::ObjectAdapterI::_createProxy(Identity ident) const
{
    lock_guard lock(_mutex);

    checkForDestruction();
    checkIdentity(ident, __FILE__, __LINE__);

    return newProxy(std::move(ident), "");
}

ObjectPrx
Ice::ObjectAdapterI::_createDirectProxy(Identity ident) const
{
    lock_guard lock(_mutex);

    checkForDestruction();
    checkIdentity(ident, __FILE__, __LINE__);

    return newDirectProxy(std::move(ident), "");
}

ObjectPrx
Ice::ObjectAdapterI::_createIndirectProxy(Identity ident) const
{
    lock_guard lock(_mutex);

    checkForDestruction();
    checkIdentity(ident, __FILE__, __LINE__);

    return newIndirectProxy(std::move(ident), "", _id);
}

void
Ice::ObjectAdapterI::setLocator(optional<LocatorPrx> locator)
{
    lock_guard lock(_mutex);
    checkForDeactivation();
    _locatorInfo = locator ? _instance->locatorManager()->get(locator.value()) : nullptr;
}

optional<LocatorPrx>
Ice::ObjectAdapterI::getLocator() const noexcept
{
    lock_guard lock(_mutex);
    return _locatorInfo ? optional<LocatorPrx>(_locatorInfo->getLocator()) : nullopt;
}

EndpointSeq
Ice::ObjectAdapterI::getEndpoints() const
{
    lock_guard lock(_mutex);

    EndpointSeq endpoints;
    transform(
        _incomingConnectionFactories.begin(),
        _incomingConnectionFactories.end(),
        back_inserter(endpoints),
        [](const IncomingConnectionFactoryPtr& factory) { return factory->endpoint(); });
    return endpoints;
}

EndpointSeq
Ice::ObjectAdapterI::getPublishedEndpoints() const
{
    lock_guard lock(_mutex);
    return {_publishedEndpoints.begin(), _publishedEndpoints.end()};
}

void
Ice::ObjectAdapterI::setPublishedEndpoints(EndpointSeq newEndpoints)
{
    LocatorInfoPtr locatorInfo;
    vector<EndpointIPtr> oldPublishedEndpoints;
    {
        lock_guard lock(_mutex);
        checkForDeactivation();

        if (_routerInfo)
        {
            const string s("can't set published endpoints on object adapter associated with a router");
            throw invalid_argument(s);
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
        updateLocatorRegistry(locatorInfo, createDirectProxy(std::move(dummy)));
    }
    catch (const Ice::LocalException&)
    {
        lock_guard lock(_mutex);

        //
        // Restore the old published endpoints.
        //
        _publishedEndpoints = oldPublishedEndpoints;
        throw;
    }
}

bool
Ice::ObjectAdapterI::isLocal(const ReferencePtr& ref) const
{
    //
    // NOTE: it's important that isLocal() doesn't perform any blocking operations as
    // it can be called for AMI invocations if the proxy has no delegate set yet.
    //

    if (ref->isWellKnown())
    {
        //
        // Check the active servant map to see if the well-known
        // proxy is for a local object.
        //
        return _servantManager->hasServant(ref->getIdentity());
    }
    else if (ref->isIndirect())
    {
        //
        // Proxy is local if the reference adapter id matches this
        // adapter id or replica group id.
        //
        return ref->getAdapterId() == _id || ref->getAdapterId() == _replicaGroupId;
    }
    else
    {
        lock_guard lock(_mutex);
        checkForDestruction();

        // Proxies which have at least one endpoint in common with the published endpoints are considered local.
        // This check doesn't take datagram endpoints into account; this effectively disables colloc optimization
        // for UDP.
        for (const auto& endpoint : ref->getEndpoints())
        {
            if (!endpoint->datagram())
            {
                for (const auto& publishedEndpoint : _publishedEndpoints)
                {
                    if (endpoint->equivalent(publishedEndpoint))
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
Ice::ObjectAdapterI::flushAsyncBatchRequests(const CommunicatorFlushBatchAsyncPtr& outAsync, CompressBatch compress)
{
    vector<IncomingConnectionFactoryPtr> f;
    {
        lock_guard lock(_mutex);
        f = _incomingConnectionFactories;
    }

    for (const auto& p : f)
    {
        p->flushAsyncBatchRequests(outAsync, compress);
    }
}

void
Ice::ObjectAdapterI::updateConnectionObservers()
{
    vector<IncomingConnectionFactoryPtr> f;
    {
        lock_guard lock(_mutex);
        f = _incomingConnectionFactories;
    }
    for_each(
        f.begin(),
        f.end(),
        [](const IncomingConnectionFactoryPtr& factory) { factory->updateConnectionObservers(); });
}

void
Ice::ObjectAdapterI::updateThreadObservers()
{
    ThreadPoolPtr threadPool;
    {
        lock_guard lock(_mutex);
        threadPool = _threadPool;
    }
    if (threadPool)
    {
        threadPool->updateObservers();
    }
}

void
Ice::ObjectAdapterI::incDirectCount()
{
    lock_guard lock(_mutex);

    checkForDestruction();

    assert(_directCount >= 0);
    ++_directCount;
}

void
Ice::ObjectAdapterI::decDirectCount()
{
    lock_guard lock(_mutex);

    // Not check for deactivation here!

    assert(_instance); // Must not be called after destroy().

    assert(_directCount > 0);
    if (--_directCount == 0)
    {
        _conditionVariable.notify_all();
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

    if (_threadPool)
    {
        return _threadPool;
    }
    else
    {
        return _instance->serverThreadPool();
    }
}

void
Ice::ObjectAdapterI::setAdapterOnConnection(const Ice::ConnectionIPtr& connection)
{
    lock_guard lock(_mutex);
    checkForDestruction();
    connection->setAdapterFromAdapter(shared_from_this());
}

//
// COMPILERFIX: The ObjectAdapterI setup is broken out into a separate initialize
// function because when it was part of the constructor C++Builder 2010 apps would
// crash if an exception was thrown from any calls within the constructor.
//
Ice::ObjectAdapterI::ObjectAdapterI(
    InstancePtr instance,
    CommunicatorPtr communicator,
    ObjectAdapterFactoryPtr objectAdapterFactory,
    string name,
    bool noConfig,
    optional<SSL::ServerAuthenticationOptions> serverAuthenticationOptions)
    : _instance(std::move(instance)),
      _communicator(std::move(communicator)),
      _objectAdapterFactory(std::move(objectAdapterFactory)),
      _servantManager(make_shared<ServantManager>(_instance, name)),
      _name(std::move(name)),
      _noConfig(noConfig),
      _serverAuthenticationOptions(std::move(serverAuthenticationOptions))
{
#if defined(ICE_USE_SCHANNEL)
    if (_serverAuthenticationOptions && _serverAuthenticationOptions->trustedRootCertificates)
    {
        CertDuplicateStore(_serverAuthenticationOptions->trustedRootCertificates);
    }
#elif defined(ICE_USE_SECURE_TRANSPORT)
    if (_serverAuthenticationOptions && _serverAuthenticationOptions->trustedRootCertificates)
    {
        CFRetain(_serverAuthenticationOptions->trustedRootCertificates);
    }
#endif
}

void
Ice::ObjectAdapterI::initialize(optional<RouterPrx> router)
{
    // shared_from_this() is available now and is called by `use`.

    const LoggerPtr logger = _instance->initializationData().logger;
    if (logger)
    {
        int warningLevel = _instance->initializationData().properties->getIcePropertyAsInt("Ice.Warn.Dispatch");
        if (_instance->traceLevels()->dispatch > 0 || warningLevel > 0)
        {
            use(
                [logger,
                 warningLevel,
                 traceLevel = _instance->traceLevels()->dispatch,
                 traceCat = _instance->traceLevels()->dispatchCat,
                 toStringMode = _instance->toStringMode()](ObjectPtr next)
                {
                    return make_shared<LoggerMiddleware>(
                        std::move(next),
                        logger,
                        traceLevel,
                        traceCat,
                        warningLevel,
                        toStringMode);
                });
        }
    }

    const Instrumentation::CommunicatorObserverPtr observer = _instance->initializationData().observer;
    if (observer)
    {
        use([observer](ObjectPtr next) { return make_shared<ObserverMiddleware>(std::move(next), observer); });
    }

    if (_noConfig)
    {
        _reference = _instance->referenceFactory()->create("dummy -t", "");
        return;
    }

    PropertiesPtr properties = _instance->initializationData().properties;

    try
    {
        validatePropertiesWithPrefix(_name, properties, &PropertyNames::ObjectAdapterProps);

        if (!router && properties->getPropertiesForPrefix(_name + ".").size() == 0)
        {
            throw InitializationException(__FILE__, __LINE__, "object adapter '" + _name + "' requires configuration");
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
        catch (const ParseException&)
        {
            throw InitializationException(
                __FILE__,
                __LINE__,
                "invalid proxy options '" + proxyOptions + "' for object adapter '" + _name + "'");
        }

        {
            const int defaultMessageSizeMax = static_cast<int>(_instance->messageSizeMax() / 1024);
            int32_t messageSizeMax =
                properties->getPropertyAsIntWithDefault(_name + ".MessageSizeMax", defaultMessageSizeMax);
            // The maximum size of an Ice protocol message in bytes. This is limited to 0x7fffffff, which corresponds to
            // the maximum value of a 32-bit signed integer (int32_t).
            const int32_t messageSizeMaxUpperLimit = numeric_limits<int32_t>::max();
            if (messageSizeMax > messageSizeMaxUpperLimit / 1024)
            {
                ostringstream os;
                os << _name << ".MessageSizeMax '" << messageSizeMax
                   << "' is too large, it must be less than or equal to '" << (messageSizeMaxUpperLimit / 1024)
                   << "' KiB";
                throw InitializationException{__FILE__, __LINE__, os.str()};
            }
            else if (messageSizeMax < 1)
            {
                const_cast<int32_t&>(_messageSizeMax) = 0x7fffffff;
            }
            else
            {
                const_cast<int32_t&>(_messageSizeMax) = messageSizeMax * 1024;
            }
        }

        int threadPoolSize = properties->getPropertyAsInt(_name + ".ThreadPool.Size");
        int threadPoolSizeMax = properties->getPropertyAsInt(_name + ".ThreadPool.SizeMax");

        //
        // Create the per-adapter thread pool, if necessary. This is done before the creation of the incoming
        // connection factory as the thread pool is needed during creation for the call to incFdsInUse.
        //
        if (threadPoolSize > 0 || threadPoolSizeMax > 0)
        {
            _threadPool = ThreadPool::create(_instance, _name + ".ThreadPool", 0);
        }

        if (!router)
        {
            router = _communicator->propertyToProxy<RouterPrx>(_name + ".Router");
        }
        if (router)
        {
            _routerInfo = _instance->routerManager()->get(router.value());
            assert(_routerInfo);

            if (!properties->getProperty(_name + ".Endpoints").empty())
            {
                throw InitializationException{
                    __FILE__,
                    __LINE__,
                    "an object adapter with a router cannot accept incoming connections"};
            }

            //
            // Make sure this router is not already registered with another adapter.
            //
            if (_routerInfo->getAdapter())
            {
                throw AlreadyRegisteredException(
                    __FILE__,
                    __LINE__,
                    "object adapter with router",
                    _communicator->identityToString(router->ice_getIdentity()));
            }

            //
            // Associate this object adapter with the router. This way, new outgoing connections
            // to the router's client proxy will use this object adapter for callbacks.
            //
            _routerInfo->setAdapter(shared_from_this());

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
            for (const auto& endpoint : endpoints)
            {
                for (const auto& expanded : endpoint->expandHost())
                {
                    auto factory = make_shared<IncomingConnectionFactory>(_instance, expanded, shared_from_this());
                    factory->initialize();
                    _incomingConnectionFactories.push_back(factory);
                }
            }
            if (endpoints.empty())
            {
                TraceLevelsPtr tl = _instance->traceLevels();
                if (tl->network >= 2)
                {
                    Trace out(_instance->initializationData().logger, tl->networkCat);
                    out << "created adapter '" << _name << "' without endpoints";
                }
            }
        }

        //
        // Compute the published endpoints.
        //
        _publishedEndpoints = computePublishedEndpoints();

        if (!properties->getProperty(_name + ".Locator").empty())
        {
            setLocator(_communicator->propertyToProxy<LocatorPrx>(_name + ".Locator"));
        }
        else
        {
            setLocator(_instance->referenceFactory()->getDefaultLocator());
        }
    }
    catch (...)
    {
        destroy();
        throw;
    }
}

Ice::ObjectAdapterI::~ObjectAdapterI()
{
#if defined(ICE_USE_SCHANNEL)
    if (_serverAuthenticationOptions && _serverAuthenticationOptions->trustedRootCertificates)
    {
        CertCloseStore(_serverAuthenticationOptions->trustedRootCertificates, 0);
    }
#elif defined(ICE_USE_SECURE_TRANSPORT)
    if (_serverAuthenticationOptions && _serverAuthenticationOptions->trustedRootCertificates)
    {
        CFRelease(_serverAuthenticationOptions->trustedRootCertificates);
    }
#endif
    if (_state < StateDeactivated)
    {
        Warning out(_instance->initializationData().logger);
        out << "object adapter '" << getName() << "' has not been deactivated";
    }
    else if (_state != StateDestroyed)
    {
        Warning out(_instance->initializationData().logger);
        out << "object adapter '" << getName() << "' has not been destroyed";
    }
    else
    {
        // assert(!_servantManager); // We don't clear this reference, it needs to be immutable.
        assert(!_threadPool);
        assert(_incomingConnectionFactories.empty());
        assert(_directCount == 0);
    }
}

ObjectPrx
Ice::ObjectAdapterI::newProxy(Identity ident, string facet) const
{
    if (_id.empty())
    {
        return newDirectProxy(std::move(ident), std::move(facet));
    }
    else if (_replicaGroupId.empty())
    {
        return newIndirectProxy(std::move(ident), std::move(facet), _id);
    }
    else
    {
        return newIndirectProxy(std::move(ident), std::move(facet), _replicaGroupId);
    }
}

ObjectPrx
Ice::ObjectAdapterI::newDirectProxy(Identity ident, string facet) const
{
    return ObjectPrx::_fromReference(
        _instance->referenceFactory()->create(std::move(ident), std::move(facet), _reference, _publishedEndpoints));
}

ObjectPrx
Ice::ObjectAdapterI::newIndirectProxy(Identity ident, string facet, string id) const
{
    //
    // Create an indirect reference with the given adapter id.
    //
    return ObjectPrx::_fromReference(
        _instance->referenceFactory()->create(std::move(ident), std::move(facet), _reference, std::move(id)));
}

void
Ice::ObjectAdapterI::checkForDeactivation() const
{
    checkForDestruction();
    if (_state >= StateDeactivating)
    {
        throw ObjectAdapterDeactivatedException{__FILE__, __LINE__, getName()};
    }
}

void
Ice::ObjectAdapterI::checkForDestruction() const
{
    if (_state >= StateDestroying)
    {
        throw ObjectAdapterDestroyedException{__FILE__, __LINE__, getName()};
    }
}

vector<EndpointIPtr>
Ice::ObjectAdapterI::parseEndpoints(string_view endpts, bool oaEndpoints) const
{
    string::size_type beg;
    string::size_type end = 0;

    vector<EndpointIPtr> endpoints;
    while (end < endpts.length())
    {
        const string delim = " \t\n\r";

        beg = endpts.find_first_not_of(delim, end);
        if (beg == string::npos)
        {
            if (!endpoints.empty())
            {
                throw ParseException(__FILE__, __LINE__, "invalid empty object adapter endpoint");
            }
            break;
        }

        end = beg;
        while (true)
        {
            end = endpts.find(':', end);
            if (end == string::npos)
            {
                end = endpts.length();
                break;
            }
            else
            {
                bool quoted = false;
                string::size_type quote = beg;
                while (true)
                {
                    quote = endpts.find('\"', quote);
                    if (quote == string::npos || end < quote)
                    {
                        break;
                    }
                    else
                    {
                        quote = endpts.find('\"', ++quote);
                        if (quote == string::npos)
                        {
                            break;
                        }
                        else if (end < quote)
                        {
                            quoted = true;
                            break;
                        }
                        ++quote;
                    }
                }
                if (!quoted)
                {
                    break;
                }
                ++end;
            }
        }

        if (end == beg)
        {
            throw ParseException(__FILE__, __LINE__, "invalid empty object adapter endpoint");
        }

        string_view s = endpts.substr(beg, end - beg);
        EndpointIPtr endp = _instance->endpointFactoryManager()->create(s, oaEndpoints);
        if (endp == nullptr)
        {
            throw ParseException(__FILE__, __LINE__, "invalid object adapter endpoint '" + string{s} + "'");
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
    if (_routerInfo)
    {
        // Get the router's server proxy endpoints and use them as the published endpoints.
        endpoints = _routerInfo->getServerEndpoints();
    }
    else
    {
        // Parse published endpoints. If set, these are used in proxies instead of the connection factory endpoints.
        endpoints = parseEndpoints(_communicator->getProperties()->getProperty(_name + ".PublishedEndpoints"), false);
        if (endpoints.empty())
        {
            // If the PublishedEndpoints property isn't set, we compute the published endpoints from the factory
            // endpoints.
            for (const auto& factory : _incomingConnectionFactories)
            {
                endpoints.push_back(factory->endpoint());
            }

            // Remove all loopback/multicast endpoints
            vector<EndpointIPtr> endpointsNoLoopback;
            copy_if(
                endpoints.begin(),
                endpoints.end(),
                back_inserter(endpointsNoLoopback),
                [](const EndpointIPtr& endpoint) { return !endpoint->isLoopbackOrMulticast(); });

            // Retrieve published host
            string publishedHost = _communicator->getProperties()->getProperty(_name + ".PublishedHost");

            if (!endpointsNoLoopback.empty())
            {
                endpoints = std::move(endpointsNoLoopback);

                // For non-loopback/multicast endpoints, we use the fully qualified name of the local host as default
                // for publishedHost.
                if (publishedHost.empty())
                {
                    publishedHost = getHostName(); // fully qualified name of local host
                }
            }
            // else keep endpoints as-is; they are all loopback or multicast

            vector<EndpointIPtr> newEndpoints;

            // Replace the host in all endpoints by publishedHost (when applicable), clear local options and remove
            // duplicates.
            for (const auto& endpoint : endpoints)
            {
                EndpointIPtr newEndpoint = endpoint->toPublishedEndpoint(publishedHost);
                if (none_of(
                        newEndpoints.begin(),
                        newEndpoints.end(),
                        [&newEndpoint](const EndpointIPtr& p) { return *newEndpoint == *p; }))
                {
                    newEndpoints.push_back(newEndpoint);
                }
            }
            endpoints = std::move(newEndpoints);
        }
    }

    if (_instance->traceLevels()->network >= 1 && !endpoints.empty())
    {
        Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
        out << "published endpoints for object adapter '" << getName() << "':\n";
        for (size_t i = 0; i < endpoints.size(); ++i)
        {
            if (i > 0)
            {
                out << ":";
            }
            out << endpoints[i]->toString();
        }
    }

    return endpoints;
}

void
ObjectAdapterI::updateLocatorRegistry(const IceInternal::LocatorInfoPtr& locatorInfo, const optional<ObjectPrx>& proxy)
{
    if (_id.empty() || !locatorInfo)
    {
        return; // Nothing to update.
    }

    optional<LocatorRegistryPrx> locatorRegistry = locatorInfo->getLocatorRegistry();
    if (!locatorRegistry)
    {
        return;
    }

    try
    {
        if (_replicaGroupId.empty())
        {
            locatorRegistry->setAdapterDirectProxy(_id, proxy);
        }
        else
        {
            locatorRegistry->setReplicatedAdapterDirectProxy(_id, _replicaGroupId, proxy);
        }
    }
    catch (const AdapterNotFoundException&)
    {
        if (_instance->traceLevels()->location >= 1)
        {
            Trace out(_instance->initializationData().logger, _instance->traceLevels()->locationCat);
            out << "couldn't update object adapter '" + _id + "' endpoints with the locator registry:\n";
            out << "the object adapter is not known to the locator registry";
        }

        throw NotRegisteredException(__FILE__, __LINE__, "object adapter", _id);
    }
    catch (const InvalidReplicaGroupIdException&)
    {
        if (_instance->traceLevels()->location >= 1)
        {
            Trace out(_instance->initializationData().logger, _instance->traceLevels()->locationCat);
            out << "couldn't update object adapter '" + _id + "' endpoints with the locator registry:\n";
            out << "the replica group '" << _replicaGroupId << "' is not known to the locator registry";
        }

        throw NotRegisteredException(__FILE__, __LINE__, "replica group", _replicaGroupId);
    }
    catch (const AdapterAlreadyActiveException&)
    {
        if (_instance->traceLevels()->location >= 1)
        {
            Trace out(_instance->initializationData().logger, _instance->traceLevels()->locationCat);
            out << "couldn't update object adapter '" + _id + "' endpoints with the locator registry:\n";
            out << "the object adapter endpoints are already set";
        }

        throw ObjectAdapterIdInUseException(__FILE__, __LINE__, _id);
    }
    catch (const ObjectAdapterDeactivatedException&)
    {
        // Expected if collocated call and OA is deactivated, ignore.
    }
    catch (const CommunicatorDestroyedException&)
    {
        // Ignore.
    }
    catch (const LocalException& ex)
    {
        if (_instance->traceLevels()->location >= 1)
        {
            Trace out(_instance->initializationData().logger, _instance->traceLevels()->locationCat);
            out << "couldn't update object adapter '" + _id + "' endpoints with the locator registry:\n" << ex;
        }
        throw; // TODO: Shall we raise a special exception instead of a non obvious local exception?
    }

    if (_instance->traceLevels()->location >= 1)
    {
        Trace out(_instance->initializationData().logger, _instance->traceLevels()->locationCat);
        out << "updated object adapter '" + _id + "' endpoints with the locator registry\n";
        out << "endpoints = ";
        if (proxy)
        {
            EndpointSeq endpts = proxy ? proxy->ice_getEndpoints() : EndpointSeq();
            ostringstream o;
            transform(
                endpts.begin(),
                endpts.end(),
                ostream_iterator<string>(o, endpts.size() > 1 ? ":" : ""),
                [](const EndpointPtr& endpoint) { return endpoint->toString(); });
            out << o.str();
        }
    }
}
