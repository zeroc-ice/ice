// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_INSTANCE_H
#define ICE_INSTANCE_H

#include <IceUtil/Config.h>
#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/RecMutex.h>
#include <IceUtil/Timer.h>
#include <Ice/InstanceF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/StatsF.h>
#include <Ice/InstrumentationF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/DefaultsAndOverridesF.h>
#include <Ice/RouterInfoF.h>
#include <Ice/LocatorInfoF.h>
#include <Ice/ReferenceFactoryF.h>
#include <Ice/ProxyFactoryF.h>
#include <Ice/ThreadPoolF.h>
#include <Ice/ConnectionFactoryF.h>
#include <Ice/ConnectionMonitorF.h>
#include <Ice/ObjectFactoryManagerF.h>
#include <Ice/ObjectAdapterFactoryF.h>
#include <Ice/EndpointFactoryManagerF.h>
#include <Ice/IPEndpointIF.h>
#include <Ice/RetryQueueF.h>
#include <Ice/DynamicLibraryF.h>
#include <Ice/PluginF.h>
#include <Ice/NetworkF.h>
#include <Ice/Initialize.h>
#include <Ice/ImplicitContextI.h>
#include <Ice/FacetMap.h>
#include <Ice/Process.h>
#include <list>
#include <IceUtil/UniquePtr.h>

namespace Ice
{

class CommunicatorI;

}

namespace IceInternal
{

class MetricsAdminI;
typedef IceUtil::Handle<MetricsAdminI> MetricsAdminIPtr;

class Instance : public IceUtil::Shared, public IceUtil::RecMutex
{
public:

    bool destroyed() const;
    const Ice::InitializationData& initializationData() const { return _initData; }
    void stats(const Ice::StatsPtr&);
    TraceLevelsPtr traceLevels() const;
    DefaultsAndOverridesPtr defaultsAndOverrides() const;
    RouterManagerPtr routerManager() const;
    LocatorManagerPtr locatorManager() const;
    ReferenceFactoryPtr referenceFactory() const;
    ProxyFactoryPtr proxyFactory() const;
    OutgoingConnectionFactoryPtr outgoingConnectionFactory() const;
    ConnectionMonitorPtr connectionMonitor() const;
    ObjectFactoryManagerPtr servantFactoryManager() const;
    ObjectAdapterFactoryPtr objectAdapterFactory() const;
    ProtocolSupport protocolSupport() const;
    bool preferIPv6() const;
    NetworkProxyPtr networkProxy() const;
    ThreadPoolPtr clientThreadPool();
    ThreadPoolPtr serverThreadPool(bool create = true);
    EndpointHostResolverPtr endpointHostResolver();
    RetryQueuePtr retryQueue();
    IceUtil::TimerPtr timer();
    EndpointFactoryManagerPtr endpointFactoryManager() const;
    DynamicLibraryListPtr dynamicLibraryList() const;
    Ice::PluginManagerPtr pluginManager() const;
    size_t messageSizeMax() const { return _messageSizeMax; }
    Ice::Int clientACM() const;
    Ice::Int serverACM() const;
    Ice::Identity stringToIdentity(const std::string&) const;
    std::string identityToString(const Ice::Identity&) const;

    Ice::ObjectPrx getAdmin();
    void addAdminFacet(const Ice::ObjectPtr&, const std::string&);
    Ice::ObjectPtr removeAdminFacet(const std::string&);
    Ice::ObjectPtr findAdminFacet(const std::string&);

    const Ice::Instrumentation::CommunicatorObserverPtr& getObserver() const
    {
        return _observer;
    }
    
    const Ice::ImplicitContextIPtr& getImplicitContext() const
    {
        return _implicitContext;
    }

    void setDefaultLocator(const Ice::LocatorPrx&);
    void setDefaultRouter(const Ice::RouterPrx&);

    void setStringConverter(const Ice::StringConverterPtr&);
    void setWstringConverter(const Ice::WstringConverterPtr&);
    void setLogger(const Ice::LoggerPtr&);
    void setThreadHook(const Ice::ThreadNotificationPtr&);

private:

    Instance(const Ice::CommunicatorPtr&, const Ice::InitializationData&);
    virtual ~Instance();
    void finishSetup(int&, char*[]);
    bool destroy();
    friend class Ice::CommunicatorI;

    void updateConnectionObservers();
    void updateThreadObservers();
    friend class ObserverUpdaterI;

    enum State
    {
        StateActive,
        StateDestroyInProgress,
        StateDestroyed
    };
    State _state;
    Ice::InitializationData _initData;
    const TraceLevelsPtr _traceLevels; // Immutable, not reset by destroy().
    const DefaultsAndOverridesPtr _defaultsAndOverrides; // Immutable, not reset by destroy().
    const size_t _messageSizeMax; // Immutable, not reset by destroy().
    const Ice::Int _clientACM; // Immutable, not reset by destroy().
    const Ice::Int _serverACM; // Immutable, not reset by destroy().
    RouterManagerPtr _routerManager;
    LocatorManagerPtr _locatorManager;
    ReferenceFactoryPtr _referenceFactory;
    ProxyFactoryPtr _proxyFactory;
    OutgoingConnectionFactoryPtr _outgoingConnectionFactory;
    ConnectionMonitorPtr _connectionMonitor;
    ObjectFactoryManagerPtr _servantFactoryManager;
    ObjectAdapterFactoryPtr _objectAdapterFactory;
    ProtocolSupport _protocolSupport;
    bool _preferIPv6;
    NetworkProxyPtr _networkProxy;
    ThreadPoolPtr _clientThreadPool;
    ThreadPoolPtr _serverThreadPool;
    EndpointHostResolverPtr _endpointHostResolver;
    RetryQueuePtr _retryQueue;
    IceUtil::TimerPtr _timer;
    EndpointFactoryManagerPtr _endpointFactoryManager;
    DynamicLibraryListPtr _dynamicLibraryList;
    Ice::PluginManagerPtr _pluginManager;
    const Ice::ImplicitContextIPtr _implicitContext;
    Ice::ObjectAdapterPtr _adminAdapter;
    Ice::FacetMap _adminFacets;
    Ice::Identity _adminIdentity;
    std::set<std::string> _adminFacetFilter;
    IceInternal::MetricsAdminIPtr _metricsAdmin;
    Ice::Instrumentation::CommunicatorObserverPtr _observer;
};

class ProcessI : public Ice::Process
{
public:
    
    ProcessI(const Ice::CommunicatorPtr&);
    
    virtual void shutdown(const Ice::Current&);
    virtual void writeMessage(const std::string&, Ice::Int, const Ice::Current&);
    
private:
    
    const Ice::CommunicatorPtr _communicator;
};

}

#endif
