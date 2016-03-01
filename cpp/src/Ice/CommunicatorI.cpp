// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/CommunicatorI.h>
#include <Ice/Instance.h>
#include <Ice/Properties.h>
#include <Ice/ConnectionFactory.h>
#include <Ice/ReferenceFactory.h>
#include <Ice/ProxyFactory.h>
#include <Ice/ObjectFactoryManager.h>
#include <Ice/ObjectAdapterFactory.h>
#include <Ice/LoggerUtil.h>
#include <Ice/LocalException.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/TraceLevels.h>
#include <Ice/Router.h>
#include <Ice/OutgoingAsync.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>
#include <IceUtil/UUID.h>

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

ObjectPrx
Ice::CommunicatorI::stringToProxy(const string& s) const
{
    return _instance->proxyFactory()->stringToProxy(s);
}

string
Ice::CommunicatorI::proxyToString(const ObjectPrx& proxy) const
{
    return _instance->proxyFactory()->proxyToString(proxy);
}

ObjectPrx
Ice::CommunicatorI::propertyToProxy(const string& p) const
{
    return _instance->proxyFactory()->propertyToProxy(p);
}

PropertyDict
Ice::CommunicatorI::proxyToProperty(const ObjectPrx& proxy, const string& property) const
{
    return _instance->proxyFactory()->proxyToProperty(proxy, property);
}

Identity
Ice::CommunicatorI::stringToIdentity(const string& s) const
{
    return _instance->stringToIdentity(s);
}

string
Ice::CommunicatorI::identityToString(const Identity& ident) const
{
    return _instance->identityToString(ident);
}

ObjectAdapterPtr
Ice::CommunicatorI::createObjectAdapter(const string& name)
{
    return _instance->objectAdapterFactory()->createObjectAdapter(name, 0);
}

ObjectAdapterPtr
Ice::CommunicatorI::createObjectAdapterWithEndpoints(const string& name, const string& endpoints)
{
    string oaName = name;
    if(oaName.empty())
    {
        oaName = IceUtil::generateUUID();
    }

    getProperties()->setProperty(oaName + ".Endpoints", endpoints);
    return _instance->objectAdapterFactory()->createObjectAdapter(oaName, 0);
}

ObjectAdapterPtr
Ice::CommunicatorI::createObjectAdapterWithRouter(const string& name, const RouterPrx& router)
{
    string oaName = name;
    if(oaName.empty())
    {
        oaName = IceUtil::generateUUID();
    }

    PropertyDict properties = proxyToProperty(router, oaName + ".Router");
    for(PropertyDict::const_iterator p = properties.begin(); p != properties.end(); ++p)
    {
        getProperties()->setProperty(p->first, p->second);
    }

    return _instance->objectAdapterFactory()->createObjectAdapter(oaName, router);
}

void
Ice::CommunicatorI::addObjectFactory(const ObjectFactoryPtr& factory, const string& id)
{
    _instance->servantFactoryManager()->add(factory, id);
}

ObjectFactoryPtr
Ice::CommunicatorI::findObjectFactory(const string& id) const
{
    return _instance->servantFactoryManager()->find(id);
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

RouterPrx
Ice::CommunicatorI::getDefaultRouter() const
{
    return _instance->referenceFactory()->getDefaultRouter();
}

void
Ice::CommunicatorI::setDefaultRouter(const RouterPrx& router)
{
    _instance->setDefaultRouter(router);
}

LocatorPrx
Ice::CommunicatorI::getDefaultLocator() const
{
    return _instance->referenceFactory()->getDefaultLocator();
}

void
Ice::CommunicatorI::setDefaultLocator(const LocatorPrx& locator)
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

void
Ice::CommunicatorI::flushBatchRequests()
{
    end_flushBatchRequests(begin_flushBatchRequests());
}

AsyncResultPtr
Ice::CommunicatorI::begin_flushBatchRequests()
{
    return __begin_flushBatchRequests(::IceInternal::__dummyCallback, 0);
}

AsyncResultPtr
Ice::CommunicatorI::begin_flushBatchRequests(const CallbackPtr& cb, const LocalObjectPtr& cookie)
{
    return __begin_flushBatchRequests(cb, cookie);
}

AsyncResultPtr
Ice::CommunicatorI::begin_flushBatchRequests(const Callback_Communicator_flushBatchRequestsPtr& cb,
                                             const LocalObjectPtr& cookie)
{
    return __begin_flushBatchRequests(cb, cookie);
}

AsyncResultPtr
Ice::CommunicatorI::begin_flushBatchRequests(const IceInternal::Function<void (const Exception&)>& exception,
                                             const IceInternal::Function<void (bool)>& sent)
{
#ifdef ICE_CPP11
    class Cpp11CB : public IceInternal::Cpp11FnCallbackNC
    {

    public:

        Cpp11CB(const IceInternal::Function<void (const Exception&)>& excb,
                const IceInternal::Function<void (bool)>& sentcb) :
            IceInternal::Cpp11FnCallbackNC(excb, sentcb)
        {
            CallbackBase::checkCallback(true, excb != nullptr);
        }

        virtual void
        completed(const AsyncResultPtr& __result) const
        {
            CommunicatorPtr __com = __result->getCommunicator();
            assert(__com);
            try
            {
                __com->end_flushBatchRequests(__result);
                assert(false);
            }
            catch(const Exception& ex)
            {
                IceInternal::Cpp11FnCallbackNC::exception(__result, ex);
            }
        }
    };

    return __begin_flushBatchRequests(new Cpp11CB(exception, sent), 0);
#else
    assert(false); // Ice not built with C++11 support.
    return 0;
#endif
}

namespace
{

const ::std::string __flushBatchRequests_name = "flushBatchRequests";

}

AsyncResultPtr
Ice::CommunicatorI::__begin_flushBatchRequests(const IceInternal::CallbackBasePtr& cb, const LocalObjectPtr& cookie)
{
    OutgoingConnectionFactoryPtr connectionFactory = _instance->outgoingConnectionFactory();
    ObjectAdapterFactoryPtr adapterFactory = _instance->objectAdapterFactory();

    //
    // This callback object receives the results of all invocations
    // of Connection::begin_flushBatchRequests.
    //
    CommunicatorFlushBatchAsyncPtr result = new CommunicatorFlushBatchAsync(this,
                                                                            _instance,
                                                                            __flushBatchRequests_name,
                                                                            cb,
                                                                            cookie);

    connectionFactory->flushAsyncBatchRequests(result);
    adapterFactory->flushAsyncBatchRequests(result);

    //
    // Inform the callback that we have finished initiating all of the
    // flush requests.
    //
    result->ready();

    return result;
}

void
Ice::CommunicatorI::end_flushBatchRequests(const AsyncResultPtr& r)
{
    AsyncResult::__check(r, this, __flushBatchRequests_name);
    r->__wait();
}

ObjectPrx
Ice::CommunicatorI::createAdmin(const ObjectAdapterPtr& adminAdapter, const Identity& adminId)
{
    return _instance->createAdmin(adminAdapter, adminId);
}
ObjectPrx
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

Ice::CommunicatorI::CommunicatorI(const InitializationData& initData)
{
    __setNoDelete(true);
    try
    {
        const_cast<InstancePtr&>(_instance) = new Instance(this, initData);

        //
        // Keep a reference to the dynamic library list to ensure
        // the libraries are not unloaded until this Communicator's
        // destructor is invoked.
        //
        const_cast<DynamicLibraryListPtr&>(_dynamicLibraryList) = _instance->dynamicLibraryList();
    }
    catch(...)
    {
        destroy();
        __setNoDelete(false);
        throw;
    }
    __setNoDelete(false);
}

Ice::CommunicatorI::~CommunicatorI()
{
    if(!_instance->destroyed())
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
        _instance->finishSetup(argc, argv, this);
    }
    catch(...)
    {
        destroy();
        throw;
    }
}
