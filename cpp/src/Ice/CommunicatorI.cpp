// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

#ifndef ICE_CPP11_MAPPING
IceUtil::Shared* IceInternal::upCast(CommunicatorFlushBatchAsync* p) { return p; }
#endif

CommunicatorFlushBatchAsync::~CommunicatorFlushBatchAsync()
{
    // Out of line to avoid weak vtable
}

CommunicatorFlushBatchAsync::CommunicatorFlushBatchAsync(const InstancePtr& instance) :
    OutgoingAsyncBase(instance)
{
    //
    // _useCount is initialized to 1 to prevent premature callbacks.
    // The caller must invoke ready() after all flush requests have
    // been initiated.
    //
    _useCount = 1;
}

void
CommunicatorFlushBatchAsync::flushConnection(const ConnectionIPtr& con, Ice::CompressBatch compressBatch)
{
    class FlushBatch : public OutgoingAsyncBase
    {
    public:

        FlushBatch(const CommunicatorFlushBatchAsyncPtr& outAsync,
                   const InstancePtr& instance,
                   InvocationObserver& observer) :
            OutgoingAsyncBase(instance), _outAsync(outAsync), _observer(observer)
        {
        }

        virtual bool
        sent()
        {
            _childObserver.detach();
            _outAsync->check(false);
            return false;
        }

        virtual bool
        exception(const Exception& ex)
        {
            _childObserver.failed(ex.ice_id());
            _childObserver.detach();
            _outAsync->check(false);
            return false;
        }

        virtual InvocationObserver&
        getObserver()
        {
            return _observer;
        }

        virtual bool handleSent(bool, bool)
        {
            return false;
        }

        virtual bool handleException(const Ice::Exception&)
        {
            return false;
        }

        virtual bool handleResponse(bool)
        {
            return false;
        }

        virtual void handleInvokeSent(bool, OutgoingAsyncBase*) const
        {
            assert(false);
        }

        virtual void handleInvokeException(const Ice::Exception&, OutgoingAsyncBase*) const
        {
            assert(false);
        }

        virtual void handleInvokeResponse(bool, OutgoingAsyncBase*) const
        {
            assert(false);
        }

    private:

        const CommunicatorFlushBatchAsyncPtr _outAsync;
        InvocationObserver& _observer;
    };

    {
        Lock sync(_m);
        ++_useCount;
    }

    try
    {
        OutgoingAsyncBasePtr flushBatch = ICE_MAKE_SHARED(FlushBatch, ICE_SHARED_FROM_THIS, _instance, _observer);
        bool compress;
        int batchRequestNum = con->getBatchRequestQueue()->swap(flushBatch->getOs(), compress);
        if(batchRequestNum == 0)
        {
            flushBatch->sent();
        }
        else
        {
            if(compressBatch == ICE_SCOPED_ENUM(CompressBatch, Yes))
            {
                compress = true;
            }
            else if(compressBatch == ICE_SCOPED_ENUM(CompressBatch, No))
            {
                compress = false;
            }
            con->sendAsyncRequest(flushBatch, compress, false, batchRequestNum);
        }
    }
    catch(const LocalException&)
    {
        check(false);
        throw;
    }
}

void
CommunicatorFlushBatchAsync::invoke(const string& operation, CompressBatch compressBatch)
{
    _observer.attach(_instance.get(), operation);
    _instance->outgoingConnectionFactory()->flushAsyncBatchRequests(ICE_SHARED_FROM_THIS, compressBatch);
    _instance->objectAdapterFactory()->flushAsyncBatchRequests(ICE_SHARED_FROM_THIS, compressBatch);
    check(true);
}

void
CommunicatorFlushBatchAsync::check(bool userThread)
{
    {
        Lock sync(_m);
        assert(_useCount > 0);
        if(--_useCount > 0)
        {
            return;
        }
    }

    if(sentImpl(true))
    {
        if(userThread)
        {
            _sentSynchronously = true;
            invokeSent();
        }
        else
        {
            invokeSentAsync();
        }
    }
}

void
Ice::CommunicatorI::destroy() ICE_NOEXCEPT
{
    if(_instance)
    {
        _instance->destroy();
    }
}

void
Ice::CommunicatorI::shutdown() ICE_NOEXCEPT
{
    try
    {
        _instance->objectAdapterFactory()->shutdown();
    }
    catch(const Ice::CommunicatorDestroyedException&)
    {
        // Ignore
    }
}

void
Ice::CommunicatorI::waitForShutdown() ICE_NOEXCEPT
{
    try
    {
        _instance->objectAdapterFactory()->waitForShutdown();
    }
    catch(const Ice::CommunicatorDestroyedException&)
    {
        // Ignore
    }
}

bool
Ice::CommunicatorI::isShutdown() const ICE_NOEXCEPT
{
    try
    {
        return _instance->objectAdapterFactory()->isShutdown();
    }
    catch(const Ice::CommunicatorDestroyedException&)
    {
        return true;
    }
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
Ice::CommunicatorI::findObjectFactory(const string& id) const ICE_NOEXCEPT
{
    return _instance->findObjectFactory(id);
}

PropertiesPtr
Ice::CommunicatorI::getProperties() const ICE_NOEXCEPT
{
    return _instance->initializationData().properties;
}

LoggerPtr
Ice::CommunicatorI::getLogger() const ICE_NOEXCEPT
{
    return _instance->initializationData().logger;
}

Ice::Instrumentation::CommunicatorObserverPtr
Ice::CommunicatorI::getObserver() const ICE_NOEXCEPT
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
Ice::CommunicatorI::getImplicitContext() const ICE_NOEXCEPT
{
    return _instance->getImplicitContext();
}

PluginManagerPtr
Ice::CommunicatorI::getPluginManager() const
{
    return _instance->pluginManager();
}

ValueFactoryManagerPtr
Ice::CommunicatorI::getValueFactoryManager() const ICE_NOEXCEPT
{
    return _instance->initializationData().valueFactoryManager;
}

namespace
{

const ::std::string flushBatchRequests_name = "flushBatchRequests";

}

#ifdef ICE_CPP11_MAPPING

::std::function<void()>
Ice::CommunicatorI::flushBatchRequestsAsync(CompressBatch compress,
                                            function<void(exception_ptr)> ex,
                                            function<void(bool)> sent)
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
    outAsync->invoke(flushBatchRequests_name, compress);
    return [outAsync]() { outAsync->cancel(); };
}

#else

void
Ice::CommunicatorI::flushBatchRequests(CompressBatch compress)
{
    end_flushBatchRequests(begin_flushBatchRequests(compress));
}

AsyncResultPtr
Ice::CommunicatorI::begin_flushBatchRequests(CompressBatch compress)
{
    return _iceI_begin_flushBatchRequests(compress, ::IceInternal::dummyCallback, 0);
}

AsyncResultPtr
Ice::CommunicatorI::begin_flushBatchRequests(CompressBatch compress,
                                             const CallbackPtr& cb,
                                             const LocalObjectPtr& cookie)
{
    return _iceI_begin_flushBatchRequests(compress, cb, cookie);
}

AsyncResultPtr
Ice::CommunicatorI::begin_flushBatchRequests(CompressBatch compress,
                                             const Callback_Communicator_flushBatchRequestsPtr& cb,
                                             const LocalObjectPtr& cookie)
{
    return _iceI_begin_flushBatchRequests(compress, cb, cookie);
}

AsyncResultPtr
Ice::CommunicatorI::_iceI_begin_flushBatchRequests(CompressBatch compress,
                                                   const IceInternal::CallbackBasePtr& cb,
                                                   const LocalObjectPtr& cookie)
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
    result->invoke(flushBatchRequests_name, compress);
    return result;
}

void
Ice::CommunicatorI::end_flushBatchRequests(const AsyncResultPtr& r)
{
    AsyncResult::_check(r, this, flushBatchRequests_name);
    r->_waitForResponse();
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
Ice::CommunicatorI::finishSetup(int& argc, const char* argv[])
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
