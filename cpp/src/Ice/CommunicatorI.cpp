// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Ice/CommunicatorI.h>
#include <Ice/Instance.h>
#include <Ice/Properties.h>
#include <Ice/ReferenceFactory.h>
#include <Ice/ProxyFactory.h>
#include <Ice/ObjectFactoryManager.h>
#include <Ice/ObjectAdapterFactory.h>
#include <Ice/LoggerUtil.h>
#include <Ice/LocalException.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/TraceLevels.h>
#include <Ice/GC.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace IceInternal
{

IceUtil::Handle<IceInternal::GC> theCollector = 0;

}

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

static int communicatorCount = 0;
static IceUtil::StaticMutex gcMutex = ICE_STATIC_MUTEX_INITIALIZER;
static GarbageCollectorStats gcStats;
static int gcTraceLevel;
static string gcTraceCat;
static int gcInterval;

static void
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

void
Ice::CommunicatorI::destroy()
{
    if(_instance->destroy())
    {
        IceUtil::StaticMutex::Lock sync(gcMutex);

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
    return _instance->objectAdapterFactory()->createObjectAdapter(name, "", 0);
}

ObjectAdapterPtr
Ice::CommunicatorI::createObjectAdapterWithEndpoints(const string& name, const string& endpoints)
{
    return _instance->objectAdapterFactory()->createObjectAdapter(name, endpoints, 0);
}

ObjectAdapterPtr
Ice::CommunicatorI::createObjectAdapterWithRouter(const string& name, const RouterPrx& router)
{
    return _instance->objectAdapterFactory()->createObjectAdapter(name, "", router);
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

RouterPrx
Ice::CommunicatorI::getDefaultRouter() const
{
    return _instance->referenceFactory()->getDefaultRouter();
}

void
Ice::CommunicatorI::setDefaultRouter(const RouterPrx& router)
{
    _instance->referenceFactory()->setDefaultRouter(router);
}

LocatorPrx
Ice::CommunicatorI::getDefaultLocator() const
{
    return _instance->referenceFactory()->getDefaultLocator();
}

void
Ice::CommunicatorI::setDefaultLocator(const LocatorPrx& locator)
{
    _instance->referenceFactory()->setDefaultLocator(locator);
}

Ice::Context
Ice::CommunicatorI::getDefaultContext() const
{
    return _instance->getDefaultContext()->getValue();
}

void
Ice::CommunicatorI::setDefaultContext(const Context& ctx)
{
    _instance->setDefaultContext(ctx);
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
    _instance->flushBatchRequests();
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
        __setNoDelete(false);
        throw;
    }
    __setNoDelete(false);

    {
        //
        // If this is the first communicator that is created, use that communicator's
        // property settings to determine whether to start the garbage collector.
        // We remember that communicator's trace and logger settings so the garbage
        // collector can continue to log messages even if the first communicator that
        // is created isn't the last communicator to be destroyed.
        //
        IceUtil::StaticMutex::Lock sync(gcMutex);
        static bool gcOnce = true;
        if(gcOnce)
        {
            gcTraceLevel = _instance->traceLevels()->gc;
            gcTraceCat = _instance->traceLevels()->gcCat;
            gcInterval = _instance->initializationData().properties->getPropertyAsInt("Ice.GC.Interval");
            gcOnce = false;
        }
        if(++communicatorCount == 1)
        {
            theCollector = new IceInternal::GC(gcInterval, printGCStats);
            if(gcInterval > 0)
            {
                theCollector->start();
            }
        }
    }
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
        _instance->destroy();
        throw;
    }
}
