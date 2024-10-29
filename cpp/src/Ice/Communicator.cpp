//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
    if (_instance)
    {
        _instance->destroy();
    }
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
    const string& name,
    const optional<SSL::ServerAuthenticationOptions>& serverAuthenticationOptions)
{
    return _instance->objectAdapterFactory()->createObjectAdapter(name, nullopt, serverAuthenticationOptions);
}

ObjectAdapterPtr
Ice::Communicator::createObjectAdapterWithEndpoints(
    const string& name,
    const string& endpoints,
    const optional<SSL::ServerAuthenticationOptions>& serverAuthenticationOptions)
{
    string oaName = name;
    if (oaName.empty())
    {
        oaName = Ice::generateUUID();
    }

    getProperties()->setProperty(oaName + ".Endpoints", endpoints);
    return _instance->objectAdapterFactory()->createObjectAdapter(oaName, nullopt, serverAuthenticationOptions);
}

ObjectAdapterPtr
Ice::Communicator::createObjectAdapterWithRouter(const string& name, const RouterPrx& router)
{
    string oaName = name;
    if (oaName.empty())
    {
        oaName = Ice::generateUUID();
    }

    PropertyDict properties = proxyToProperty(router, oaName + ".Router");
    for (PropertyDict::const_iterator p = properties.begin(); p != properties.end(); ++p)
    {
        getProperties()->setProperty(p->first, p->second);
    }

    return _instance->objectAdapterFactory()->createObjectAdapter(oaName, router, nullopt);
}

ObjectAdapterPtr
Ice::Communicator::getDefaultObjectAdapter() const noexcept
{
    return _instance->outgoingConnectionFactory()->getDefaultObjectAdapter();
}

void
Ice::Communicator::setDefaultObjectAdapter(ObjectAdapterPtr adapter) noexcept
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

ValueFactoryManagerPtr
Ice::Communicator::getValueFactoryManager() const noexcept
{
    return _instance->initializationData().valueFactoryManager;
}

void
Ice::Communicator::postToClientThreadPool(function<void()> call)
{
    _instance->clientThreadPool()->execute(call, nullptr);
}

::std::function<void()>
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
    auto outAsync = make_shared<CommunicatorFlushBatchLambda>(_instance, ex, sent);
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
Ice::Communicator::addAdminFacet(const ObjectPtr& servant, const string& facet)
{
    _instance->addAdminFacet(servant, facet);
}

ObjectPtr
Ice::Communicator::removeAdminFacet(string_view facet)
{
    return _instance->removeAdminFacet(facet);
}

ObjectPtr
Ice::Communicator::findAdminFacet(string_view facet)
{
    return _instance->findAdminFacet(facet);
}

Ice::FacetMap
Ice::Communicator::findAllAdminFacets()
{
    return _instance->findAllAdminFacets();
}

CommunicatorPtr
Ice::Communicator::create(const InitializationData& initData)
{
    Ice::CommunicatorPtr communicator = make_shared<Communicator>();
    try
    {
        const_cast<InstancePtr&>(communicator->_instance) = Instance::create(communicator, initData);
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
Ice::Communicator::finishSetup(int& argc, const char* argv[])
{
    try
    {
        _instance->finishSetup(argc, argv, shared_from_this());
    }
    catch (...)
    {
        destroy();
        throw;
    }
}
