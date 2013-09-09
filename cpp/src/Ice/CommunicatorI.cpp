// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
#include <Ice/ObjectFactoryManager.h>
#include <Ice/ObjectAdapterFactory.h>
#include <Ice/LoggerUtil.h>
#include <Ice/LocalException.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/TraceLevels.h>
#include <Ice/GC.h>
#include <Ice/Router.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>
#include <IceUtil/UUID.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace IceInternal
{

IceUtil::Handle<IceInternal::GC> theCollector = 0;

}

namespace
{

struct GarbageCollectorStats
{
    GarbageCollectorStats() :
        runs(0), examined(0), collected(0)
    {
    }
    int runs;
    int examined;
    int collected;
    IceUtil::Time time;
};

int communicatorCount = 0;
IceUtil::Mutex* gcMutex = 0;
GarbageCollectorStats gcStats;
int gcTraceLevel;
string gcTraceCat;
int gcInterval;
bool gcHasPriority;
int gcThreadPriority;

class Init
{
public:

    Init()
    {
        gcMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete gcMutex;
        gcMutex = 0;
    }
};

Init init;

void
printGCStats(const IceInternal::GCStats& stats)
{
    if(gcTraceLevel)
    {
        if(gcTraceLevel > 1)
        {
            Trace out(getProcessLogger(), gcTraceCat);
            out << stats.collected << "/" << stats.examined << ", " << stats.time * 1000 << "ms";
        }
        ++gcStats.runs;
        gcStats.examined += stats.examined;
        gcStats.collected += stats.collected;
        gcStats.time += stats.time;
    }
}

}

void
Ice::CommunicatorI::destroy()
{
    if(_instance && _instance->destroy())
    {
        IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(gcMutex);

        //
        // Wait for the collector thread to stop if this is the last communicator
        // to be destroyed.
        //
        bool last = (--communicatorCount == 0);
        if(last && gcInterval > 0 && theCollector)
        {
            theCollector->stop();
        }

        if(theCollector)
        {
            theCollector->collectGarbage(); // Collect whenever a communicator is destroyed.
        }

        if(last)
        {
            if(gcTraceLevel)
            {
                Trace out(getProcessLogger(), gcTraceCat);
                out << "totals: " << gcStats.collected << "/" << gcStats.examined << ", "
                    << gcStats.time * 1000 << "ms" << ", " << gcStats.runs << " run";
                if(gcStats.runs != 1)
                {
                    out << "s";
                }
            }
            theCollector = 0; // Force destruction of the collector.
        }
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

StatsPtr
Ice::CommunicatorI::getStats() const
{
    return _instance->initializationData().stats;
}

Ice::Instrumentation::CommunicatorObserverPtr
Ice::CommunicatorI::getObserver() const
{
    return _instance->getObserver();
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
    AsyncResultPtr r = begin_flushBatchRequests();
    end_flushBatchRequests(r);
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

namespace
{

const ::std::string __flushBatchRequests_name = "flushBatchRequests";

}

AsyncResultPtr
Ice::CommunicatorI::__begin_flushBatchRequests(const IceInternal::CallbackBasePtr& cb,
                                                     const LocalObjectPtr& cookie)
{
    OutgoingConnectionFactoryPtr connectionFactory = _instance->outgoingConnectionFactory();
    ObjectAdapterFactoryPtr adapterFactory = _instance->objectAdapterFactory();

    //
    // This callback object receives the results of all invocations
    // of Connection::begin_flushBatchRequests.
    //
    CommunicatorBatchOutgoingAsyncPtr result =
        new CommunicatorBatchOutgoingAsync(this, _instance, __flushBatchRequests_name, cb, cookie);

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

        //
        // If this is the first communicator that is created, use that communicator's
        // property settings to determine whether to start the garbage collector.
        // We remember that communicator's trace and logger settings so the garbage
        // collector can continue to log messages even if the first communicator that
        // is created isn't the last communicator to be destroyed.
        //
        IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(gcMutex);
        static bool gcOnce = true;
        if(gcOnce)
        {
            gcTraceLevel = _instance->traceLevels()->gc;
            gcTraceCat = _instance->traceLevels()->gcCat;
            gcInterval = _instance->initializationData().properties->getPropertyAsInt("Ice.GC.Interval");
            gcHasPriority = _instance->initializationData().properties->getProperty("Ice.ThreadPriority") != "";
            gcThreadPriority = _instance->initializationData().properties->getPropertyAsInt("Ice.ThreadPriority");
            gcOnce = false;
        }
        if(++communicatorCount == 1)
        {
            IceUtil::Handle<IceInternal::GC> collector  = new IceInternal::GC(gcInterval, printGCStats);
            if(gcInterval > 0)
            {
                if(gcHasPriority)
                {
                    collector->start(0, gcThreadPriority);
                }
                else
                {
                    collector->start();
                }
            }

            //
            // Assign only if start() succeeds, if it fails this makes sure stop isn't called in destroy().
            //
            theCollector = collector; 
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
        _instance->finishSetup(argc, argv);
    }
    catch(...)
    {
        destroy();
        throw;
    }
}
