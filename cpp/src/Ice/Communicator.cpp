// Copyright (c) ZeroC, Inc.

#include "Ice/Communicator.h"
#include "CommunicatorFlushBatchAsync.h"
#include "ConnectionFactory.h"
#include "Instance.h"
#include "ObjectAdapterFactory.h"
#include "ReferenceFactory.h"
#include "ThreadPool.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

ostream&
Ice::operator<<(ostream& os, const Identity& ident)
{
    os << identityToString(ident);
    return os;
}

void
Ice::Communicator::flushBatchRequests(CompressBatch compress)
{
    flushBatchRequestsAsync(compress).get();
}

std::future<void>
Ice::Communicator::flushBatchRequestsAsync(CompressBatch compress)
{
    auto promise = std::make_shared<std::promise<void>>();
    flushBatchRequestsAsync(
        compress,
        [promise](std::exception_ptr ex) { promise->set_exception(ex); },
        [promise](bool) { promise->set_value(); });
    return promise->get_future();
}

void
Ice::Communicator::destroy() noexcept
{
    if (_instance) // see create implementation
    {
        _instance->destroy();
    }
}

void
Ice::Communicator::destroyAsync(function<void()> completed) noexcept
{
    if (completed)
    {
        _instance->destroyAsync(std::move(completed));
    }
    // we tolerate null callbacks, they do nothing
}

void
Ice::Communicator::shutdown() noexcept
{
    try
    {
        _instance->objectAdapterFactory()->shutdown();
    }
    catch (const Ice::CommunicatorDestroyedException&)
    {
        // Ignore
    }
}

void
Ice::Communicator::waitForShutdown() noexcept
{
    try
    {
        _instance->objectAdapterFactory()->waitForShutdown();
    }
    catch (const Ice::CommunicatorDestroyedException&)
    {
        // Ignore
    }
}

void
Ice::Communicator::waitForShutdownAsync(function<void()> completed) noexcept
{
    if (completed)
    {
        ObjectAdapterFactoryPtr factory;
        try
        {
            factory = _instance->objectAdapterFactory();
        }
        catch (const Ice::CommunicatorDestroyedException&)
        {
            completed();
            return;
        }

        factory->waitForShutdownAsync(std::move(completed));
    }
    // we tolerate null callbacks, they do nothing
}

bool
Ice::Communicator::isShutdown() const noexcept
{
    try
    {
        return _instance->objectAdapterFactory()->isShutdown();
    }
    catch (const Ice::CommunicatorDestroyedException&)
    {
        return true;
    }
}

ReferencePtr
Ice::Communicator::_stringToProxy(string_view s) const
{
    return _instance->referenceFactory()->create(s, "");
}

string
Ice::Communicator::proxyToString(const std::optional<ObjectPrx>& proxy) const
{
    return proxy ? proxy->_getReference()->toString() : "";
}

ReferencePtr
Ice::Communicator::_propertyToProxy(string_view p) const
{
    string proxy = _instance->initializationData().properties->getProperty(p);
    return _instance->referenceFactory()->create(proxy, string{p});
}

PropertyDict
Ice::Communicator::proxyToProperty(const std::optional<ObjectPrx>& proxy, string property) const
{
    return proxy ? proxy->_getReference()->toProperty(std::move(property)) : PropertyDict();
}

string
Ice::Communicator::identityToString(const Identity& ident) const
{
    return Ice::identityToString(ident, _instance->toStringMode());
}

ObjectAdapterPtr
Ice::Communicator::createObjectAdapter(
    string name,
    optional<SSL::ServerAuthenticationOptions> serverAuthenticationOptions)
{
    return _instance->objectAdapterFactory()->createObjectAdapter(
        std::move(name),
        nullopt,
        std::move(serverAuthenticationOptions));
}

ObjectAdapterPtr
Ice::Communicator::createObjectAdapterWithEndpoints(
    string name,
    string_view endpoints,
    optional<SSL::ServerAuthenticationOptions> serverAuthenticationOptions)
{
    if (name.empty())
    {
        name = Ice::generateUUID();
    }

    getProperties()->setProperty(name + ".Endpoints", endpoints);
    return _instance->objectAdapterFactory()->createObjectAdapter(
        std::move(name),
        nullopt,
        std::move(serverAuthenticationOptions));
}

ObjectAdapterPtr
Ice::Communicator::createObjectAdapterWithRouter(string name, RouterPrx router)
{
    if (name.empty())
    {
        name = Ice::generateUUID();
    }

    PropertyDict properties = proxyToProperty(router, name + ".Router");
    for (const auto& prop : properties)
    {
        getProperties()->setProperty(prop.first, prop.second);
    }

    return _instance->objectAdapterFactory()->createObjectAdapter(std::move(name), std::move(router), nullopt);
}

ObjectAdapterPtr
Ice::Communicator::getDefaultObjectAdapter() const
{
    return _instance->outgoingConnectionFactory()->getDefaultObjectAdapter();
}

void
Ice::Communicator::setDefaultObjectAdapter(ObjectAdapterPtr adapter)
{
    return _instance->outgoingConnectionFactory()->setDefaultObjectAdapter(std::move(adapter));
}

PropertiesPtr
Ice::Communicator::getProperties() const noexcept
{
    return _instance->initializationData().properties;
}

LoggerPtr
Ice::Communicator::getLogger() const noexcept
{
    return _instance->initializationData().logger;
}

void
Ice::Communicator::addSliceLoader(SliceLoaderPtr sliceLoader) noexcept
{
    _instance->addSliceLoader(std::move(sliceLoader));
}

Ice::Instrumentation::CommunicatorObserverPtr
Ice::Communicator::getObserver() const noexcept
{
    return _instance->initializationData().observer;
}

std::optional<RouterPrx>
Ice::Communicator::getDefaultRouter() const
{
    return _instance->referenceFactory()->getDefaultRouter();
}

void
Ice::Communicator::setDefaultRouter(const std::optional<RouterPrx>& router)
{
    _instance->setDefaultRouter(router);
}

optional<LocatorPrx>
Ice::Communicator::getDefaultLocator() const
{
    return _instance->referenceFactory()->getDefaultLocator();
}

void
Ice::Communicator::setDefaultLocator(const optional<LocatorPrx>& locator)
{
    _instance->setDefaultLocator(locator);
}

Ice::ImplicitContextPtr
Ice::Communicator::getImplicitContext() const noexcept
{
    return _instance->getImplicitContext();
}

PluginManagerPtr
Ice::Communicator::getPluginManager() const
{
    return _instance->pluginManager();
}

std::function<void()>
Ice::Communicator::flushBatchRequestsAsync(
    CompressBatch compress,
    function<void(exception_ptr)> ex,
    function<void(bool)> sent)
{
    class CommunicatorFlushBatchLambda : public CommunicatorFlushBatchAsync, public LambdaInvoke
    {
    public:
        CommunicatorFlushBatchLambda(
            const InstancePtr& instance,
            std::function<void(std::exception_ptr)> ex,
            std::function<void(bool)> sent)
            : CommunicatorFlushBatchAsync(instance),
              LambdaInvoke(std::move(ex), std::move(sent))
        {
        }
    };
    auto outAsync = make_shared<CommunicatorFlushBatchLambda>(_instance, std::move(ex), std::move(sent));
    static constexpr string_view operationName = "flushBatchRequests";
    outAsync->invoke(operationName, compress);
    return [outAsync]() { outAsync->cancel(); };
}

ObjectPrx
Ice::Communicator::createAdmin(const ObjectAdapterPtr& adminAdapter, const Identity& adminId)
{
    return _instance->createAdmin(adminAdapter, adminId);
}

std::optional<ObjectPrx>
Ice::Communicator::getAdmin() const
{
    return _instance->getAdmin();
}

void
Ice::Communicator::addAdminFacet(ObjectPtr servant, string facet)
{
    _instance->addAdminFacet(std::move(servant), std::move(facet));
}

ObjectPtr
Ice::Communicator::removeAdminFacet(string_view facet)
{
    return _instance->removeAdminFacet(facet);
}

ObjectPtr
Ice::Communicator::_findAdminFacet(string_view facet)
{
    return _instance->findAdminFacet(facet);
}

Ice::FacetMap
Ice::Communicator::findAllAdminFacets()
{
    return _instance->findAllAdminFacets();
}

CommunicatorPtr
Ice::Communicator::create(InitializationData initData)
{
    CommunicatorPtr communicator{new Communicator()};
    try
    {
        const_cast<InstancePtr&>(communicator->_instance) = Instance::create(communicator, std::move(initData));
    }
    catch (...)
    {
        communicator->destroy();
        throw;
    }
    return communicator;
}

Ice::Communicator::~Communicator()
{
    if (_instance && !_instance->destroyed())
    {
        Warning out(_instance->initializationData().logger);
        out << "Ice::Communicator::destroy() has not been called";
    }
}

void
Ice::Communicator::finishSetup()
{
    try
    {
        _instance->finishSetup(shared_from_this());
    }
    catch (...)
    {
        destroy();
        throw;
    }
}

Ice::CommunicatorHolder::CommunicatorHolder(CommunicatorPtr communicator) noexcept
    : _communicator(std::move(communicator))
{
}

Ice::CommunicatorHolder&
Ice::CommunicatorHolder::operator=(CommunicatorPtr communicator) noexcept
{
    if (_communicator)
    {
        _communicator->destroy();
    }
    _communicator = std::move(communicator);
    return *this;
}

Ice::CommunicatorHolder&
Ice::CommunicatorHolder::operator=(CommunicatorHolder&& other) noexcept
{
    if (_communicator)
    {
        _communicator->destroy();
    }
    _communicator = std::move(other._communicator);
    return *this;
}

Ice::CommunicatorHolder::~CommunicatorHolder()
{
    if (_communicator)
    {
        _communicator->destroy();
    }
}
