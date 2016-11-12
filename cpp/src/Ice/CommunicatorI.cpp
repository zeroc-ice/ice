// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Ice/CommunicatorI.h>
#include <Ice/Instance.h>
#include <Ice/Properties.h>
#include <Ice/ConnectionFactory.h>
#include <Ice/ReferenceFactory.h>
#include <Ice/ProxyFactory.h>
#include <Ice/ObjectAdapterFactory.h>
#include <Ice/LoggerUtil.h>
#include <Ice/LocalException.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/TraceLevels.h>
#include <Ice/Router.h>
#include <Ice/OutgoingAsync.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>
#include <Ice/UUID.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void
Ice::CommunicatorI::destroy()
{
    if(_instance)
    {
        _instance->destroy();
    }
}

void
Ice::CommunicatorI::shutdown()
{
    _instance->objectAdapterFactory()->shutdown();
}

void
Ice::CommunicatorI::waitForShutdown()
{
    _instance->objectAdapterFactory()->waitForShutdown();
}

bool
Ice::CommunicatorI::isShutdown() const
{
    return _instance->objectAdapterFactory()->isShutdown();
}

ObjectPrxPtr
Ice::CommunicatorI::stringToProxy(const string& s) const
{
    return _instance->proxyFactory()->stringToProxy(s);
}

string
Ice::CommunicatorI::proxyToString(const ObjectPrxPtr& proxy) const
{
    return _instance->proxyFactory()->proxyToString(proxy);
}

ObjectPrxPtr
Ice::CommunicatorI::propertyToProxy(const string& p) const
{
    return _instance->proxyFactory()->propertyToProxy(p);
}

PropertyDict
Ice::CommunicatorI::proxyToProperty(const ObjectPrxPtr& proxy, const string& property) const
{
    return _instance->proxyFactory()->proxyToProperty(proxy, property);
}

Identity
Ice::CommunicatorI::stringToIdentity(const string& s) const
{
    return Ice::stringToIdentity(s);
}

string
Ice::CommunicatorI::identityToString(const Identity& ident) const
{
    return Ice::identityToString(ident, _instance->toStringMode());
}

ObjectAdapterPtr
Ice::CommunicatorI::createObjectAdapter(const string& name)
{
    return _instance->objectAdapterFactory()->createObjectAdapter(name, ICE_NULLPTR);
}

ObjectAdapterPtr
Ice::CommunicatorI::createObjectAdapterWithEndpoints(const string& name, const string& endpoints)
{
    string oaName = name;
    if(oaName.empty())
    {
        oaName = Ice::generateUUID();
    }

    getProperties()->setProperty(oaName + ".Endpoints", endpoints);
    return _instance->objectAdapterFactory()->createObjectAdapter(oaName, ICE_NULLPTR);
}

ObjectAdapterPtr
Ice::CommunicatorI::createObjectAdapterWithRouter(const string& name, const RouterPrxPtr& router)
{
    string oaName = name;
    if(oaName.empty())
    {
        oaName = Ice::generateUUID();
    }

    PropertyDict properties = proxyToProperty(router, oaName + ".Router");
    for(PropertyDict::const_iterator p = properties.begin(); p != properties.end(); ++p)
    {
        getProperties()->setProperty(p->first, p->second);
    }

    return _instance->objectAdapterFactory()->createObjectAdapter(oaName, router);
}

void
Ice::CommunicatorI::addObjectFactory(const ::Ice::ObjectFactoryPtr& factory, const string& id)
{
    _instance->addObjectFactory(factory, id);
}

::Ice::ObjectFactoryPtr
Ice::CommunicatorI::findObjectFactory(const string& id) const
{
    return _instance->findObjectFactory(id);
}

PropertiesPtr
Ice::CommunicatorI::getProperties() const
{
    return _instance->initializationData().properties;
}

LoggerPtr
Ice::CommunicatorI::getLogger() const
{
    return _instance->initializationData().logger;
}

Ice::Instrumentation::CommunicatorObserverPtr
Ice::CommunicatorI::getObserver() const
{
    return _instance->initializationData().observer;
}

RouterPrxPtr
Ice::CommunicatorI::getDefaultRouter() const
{
    return _instance->referenceFactory()->getDefaultRouter();
}

void
Ice::CommunicatorI::setDefaultRouter(const RouterPrxPtr& router)
{
    _instance->setDefaultRouter(router);
}

LocatorPrxPtr
Ice::CommunicatorI::getDefaultLocator() const
{
    return _instance->referenceFactory()->getDefaultLocator();
}

void
Ice::CommunicatorI::setDefaultLocator(const LocatorPrxPtr& locator)
{
    _instance->setDefaultLocator(locator);
}

Ice::ImplicitContextPtr
Ice::CommunicatorI::getImplicitContext() const
{
    return _instance->getImplicitContext();
}

PluginManagerPtr
Ice::CommunicatorI::getPluginManager() const
{
    return _instance->pluginManager();
}

ValueFactoryManagerPtr
Ice::CommunicatorI::getValueFactoryManager() const
{
    return _instance->initializationData().valueFactoryManager;
}

namespace
{

const ::std::string flushBatchRequests_name = "flushBatchRequests";

}

#ifdef ICE_CPP11_MAPPING
void
Ice::CommunicatorI::flushBatchRequests()
{
    Communicator::flushBatchRequestsAsync().get();
}

::std::function<void()>
Ice::CommunicatorI::flushBatchRequestsAsync(function<void(exception_ptr)> ex, function<void(bool)> sent)
{
    class CommunicatorFlushBatchLambda : public CommunicatorFlushBatchAsync, public LambdaInvoke
    {
    public:

        CommunicatorFlushBatchLambda(const InstancePtr& instance,
                                     std::function<void(std::exception_ptr)> ex,
                                     std::function<void(bool)> sent) :
            CommunicatorFlushBatchAsync(instance), LambdaInvoke(std::move(ex), std::move(sent))
        {
        }
    };
    auto outAsync = make_shared<CommunicatorFlushBatchLambda>(_instance, ex, sent);
    outAsync->invoke(flushBatchRequests_name);
    return [outAsync]() { outAsync->cancel(); };
}

#else

void
Ice::CommunicatorI::flushBatchRequests()
{
    end_flushBatchRequests(begin_flushBatchRequests());
}

AsyncResultPtr
Ice::CommunicatorI::begin_flushBatchRequests()
{
    return _iceI_begin_flushBatchRequests(::IceInternal::dummyCallback, 0);
}

AsyncResultPtr
Ice::CommunicatorI::begin_flushBatchRequests(const CallbackPtr& cb, const LocalObjectPtr& cookie)
{
    return _iceI_begin_flushBatchRequests(cb, cookie);
}

AsyncResultPtr
Ice::CommunicatorI::begin_flushBatchRequests(const Callback_Communicator_flushBatchRequestsPtr& cb,
                                             const LocalObjectPtr& cookie)
{
    return _iceI_begin_flushBatchRequests(cb, cookie);
}

AsyncResultPtr
Ice::CommunicatorI::_iceI_begin_flushBatchRequests(const IceInternal::CallbackBasePtr& cb, const LocalObjectPtr& cookie)
{
    class CommunicatorFlushBatchAsyncWithCallback : public CommunicatorFlushBatchAsync, public CallbackCompletion
    {
    public:

        CommunicatorFlushBatchAsyncWithCallback(const Ice::CommunicatorPtr& communicator,
                                                const InstancePtr& instance,
                                                const CallbackBasePtr& callback,
                                                const Ice::LocalObjectPtr& cookie) :
            CommunicatorFlushBatchAsync(instance), CallbackCompletion(callback, cookie), _communicator(communicator)
        {
            _cookie = cookie;
        }

        virtual Ice::CommunicatorPtr getCommunicator() const
        {
            return _communicator;
        }

        virtual const std::string&
        getOperation() const
        {
            return flushBatchRequests_name;
        }

    private:

        Ice::CommunicatorPtr _communicator;
    };

    CommunicatorFlushBatchAsyncPtr result = new CommunicatorFlushBatchAsyncWithCallback(this, _instance, cb, cookie);
    result->invoke(flushBatchRequests_name);
    return result;
}

void
Ice::CommunicatorI::end_flushBatchRequests(const AsyncResultPtr& r)
{
    AsyncResult::check(r, this, flushBatchRequests_name);
    r->waitForResponse();
}
#endif

ObjectPrxPtr
Ice::CommunicatorI::createAdmin(const ObjectAdapterPtr& adminAdapter, const Identity& adminId)
{
    return _instance->createAdmin(adminAdapter, adminId);
}
ObjectPrxPtr
Ice::CommunicatorI::getAdmin() const
{
    return _instance->getAdmin();
}

void
Ice::CommunicatorI::addAdminFacet(const Ice::ObjectPtr& servant, const string& facet)
{
    _instance->addAdminFacet(servant, facet);
}

Ice::ObjectPtr
Ice::CommunicatorI::removeAdminFacet(const string& facet)
{
    return _instance->removeAdminFacet(facet);
}

Ice::ObjectPtr
Ice::CommunicatorI::findAdminFacet(const string& facet)
{
    return _instance->findAdminFacet(facet);
}

Ice::FacetMap
Ice::CommunicatorI::findAllAdminFacets()
{
    return _instance->findAllAdminFacets();
}

CommunicatorIPtr
Ice::CommunicatorI::create(const InitializationData& initData)
{
    Ice::CommunicatorIPtr communicator = ICE_MAKE_SHARED(CommunicatorI);
    try
    {
        const_cast<InstancePtr&>(communicator->_instance) = new Instance(communicator, initData);

        //
        // Keep a reference to the dynamic library list to ensure
        // the libraries are not unloaded until this Communicator's
        // destructor is invoked.
        //
        const_cast<DynamicLibraryListPtr&>(communicator->_dynamicLibraryList) = communicator->_instance->dynamicLibraryList();
    }
    catch(...)
    {
        communicator->destroy();
        throw;
    }
    return communicator;
}

Ice::CommunicatorI::~CommunicatorI()
{
    if(_instance && !_instance->destroyed())
    {
        Warning out(_instance->initializationData().logger);
        out << "Ice::Communicator::destroy() has not been called";
    }
}

void
Ice::CommunicatorI::finishSetup(int& argc, char* argv[])
{
    try
    {
        _instance->finishSetup(argc, argv, ICE_SHARED_FROM_THIS);
    }
    catch(...)
    {
        destroy();
        throw;
    }
}
