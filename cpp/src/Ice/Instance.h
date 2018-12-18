// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
#include <Ice/StringConverter.h>
#include <Ice/InstanceF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/InstrumentationF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/DefaultsAndOverridesF.h>
#include <Ice/RouterInfoF.h>
#include <Ice/LocatorInfoF.h>
#include <Ice/ReferenceFactoryF.h>
#include <Ice/ProxyFactoryF.h>
#include <Ice/ThreadPoolF.h>
#include <Ice/ConnectionFactoryF.h>
#include <Ice/ACM.h>
#include <Ice/ObjectFactory.h>
#include <Ice/ObjectAdapterFactoryF.h>
#include <Ice/EndpointFactoryManagerF.h>
#include <Ice/IPEndpointIF.h>
#include <Ice/RetryQueueF.h>
#include <Ice/DynamicLibraryF.h>
#include <Ice/PluginF.h>
#include <Ice/NetworkF.h>
#include <Ice/NetworkProxyF.h>
#include <Ice/Initialize.h>
#include <Ice/ImplicitContextI.h>
#include <Ice/FacetMap.h>
#include <Ice/Process.h>
#include <list>

namespace Ice
{

class CommunicatorI;

}

namespace IceInternal
{

class Timer;
typedef IceUtil::Handle<Timer> TimerPtr;

class MetricsAdminI;
ICE_DEFINE_PTR(MetricsAdminIPtr, MetricsAdminI);

class RequestHandlerFactory;
typedef IceUtil::Handle<RequestHandlerFactory> RequestHandlerFactoryPtr;

//
// Structure to track warnings for attempts to set socket buffer sizes
//
struct BufSizeWarnInfo
{
    // Whether send size warning has been emitted
    bool sndWarn;

    // The send size for which the warning wwas emitted
    int sndSize;

    // Whether receive size warning has been emitted
    bool rcvWarn;

    // The receive size for which the warning wwas emitted
    int rcvSize;
};

class Instance : public IceUtil::Shared, public IceUtil::Monitor<IceUtil::RecMutex>
{
public:

    bool destroyed() const;
    const Ice::InitializationData& initializationData() const { return _initData; }
    TraceLevelsPtr traceLevels() const;
    DefaultsAndOverridesPtr defaultsAndOverrides() const;
    RouterManagerPtr routerManager() const;
    LocatorManagerPtr locatorManager() const;
    ReferenceFactoryPtr referenceFactory() const;
    RequestHandlerFactoryPtr requestHandlerFactory() const;
    ProxyFactoryPtr proxyFactory() const;
    OutgoingConnectionFactoryPtr outgoingConnectionFactory() const;
    ObjectAdapterFactoryPtr objectAdapterFactory() const;
    ProtocolSupport protocolSupport() const;
    bool preferIPv6() const;
    NetworkProxyPtr networkProxy() const;
    ThreadPoolPtr clientThreadPool();
    ThreadPoolPtr serverThreadPool();
    EndpointHostResolverPtr endpointHostResolver();
    RetryQueuePtr retryQueue();
    IceUtil::TimerPtr timer();
    EndpointFactoryManagerPtr endpointFactoryManager() const;
    DynamicLibraryListPtr dynamicLibraryList() const;
    Ice::PluginManagerPtr pluginManager() const;
    size_t messageSizeMax() const { return _messageSizeMax; }
    size_t batchAutoFlushSize() const { return _batchAutoFlushSize; }
    size_t classGraphDepthMax() const { return _classGraphDepthMax; }
    bool collectObjects() const { return _collectObjects; }
    Ice::ToStringMode toStringMode() const { return _toStringMode; }
    const ACMConfig& clientACM() const;
    const ACMConfig& serverACM() const;

    Ice::ObjectPrxPtr createAdmin(const Ice::ObjectAdapterPtr&, const Ice::Identity&);
    Ice::ObjectPrxPtr getAdmin();
    void addAdminFacet(const Ice::ObjectPtr&, const std::string&);
    Ice::ObjectPtr removeAdminFacet(const std::string&);
    Ice::ObjectPtr findAdminFacet(const std::string&);
    Ice::FacetMap findAllAdminFacets();

    const Ice::ImplicitContextIPtr& getImplicitContext() const
    {
        return _implicitContext;
    }

    void setDefaultLocator(const Ice::LocatorPrxPtr&);
    void setDefaultRouter(const Ice::RouterPrxPtr&);

    void setLogger(const Ice::LoggerPtr&);
#ifdef ICE_CPP11_MAPPING
    void setThreadHook(std::function<void()>, std::function<void()>);
#else
    void setThreadHook(const Ice::ThreadNotificationPtr&);
#endif

    const Ice::StringConverterPtr& getStringConverter() const { return _stringConverter; }
    const Ice::WstringConverterPtr& getWstringConverter() const { return _wstringConverter; }

    BufSizeWarnInfo getBufSizeWarn(Ice::Short type);
    void setSndBufSizeWarn(Ice::Short type, int size);
    void setRcvBufSizeWarn(Ice::Short type, int size);

    void addObjectFactory(const Ice::ObjectFactoryPtr&, const std::string&);
    Ice::ObjectFactoryPtr findObjectFactory(const std::string&) const;

    typedef std::map<std::string, Ice::ObjectFactoryPtr> ObjectFactoryMap;

private:

    Instance(const Ice::CommunicatorPtr&, const Ice::InitializationData&);
    virtual ~Instance();
    void finishSetup(int&, const char*[], const Ice::CommunicatorPtr&);
    void destroy();
    friend class Ice::CommunicatorI;

    void updateConnectionObservers();
    void updateThreadObservers();
    friend class ObserverUpdaterI;

    void addAllAdminFacets();
    void setServerProcessProxy(const Ice::ObjectAdapterPtr&, const Ice::Identity&);

    BufSizeWarnInfo getBufSizeWarnInternal(Ice::Short type);

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
    const size_t _batchAutoFlushSize; // Immutable, not reset by destroy().
    const size_t _classGraphDepthMax; // Immutable, not reset by destroy().
    const bool _collectObjects; // Immutable, not reset by destroy().
    const Ice::ToStringMode _toStringMode; // Immutable, not reset by destroy()
    ACMConfig _clientACM;
    ACMConfig _serverACM;
    RouterManagerPtr _routerManager;
    LocatorManagerPtr _locatorManager;
    ReferenceFactoryPtr _referenceFactory;
    RequestHandlerFactoryPtr _requestHandlerFactory;
    ProxyFactoryPtr _proxyFactory;
    OutgoingConnectionFactoryPtr _outgoingConnectionFactory;
    ObjectAdapterFactoryPtr _objectAdapterFactory;
    ProtocolSupport _protocolSupport;
    bool _preferIPv6;
    NetworkProxyPtr _networkProxy;
    ThreadPoolPtr _clientThreadPool;
    ThreadPoolPtr _serverThreadPool;
    EndpointHostResolverPtr _endpointHostResolver;
    RetryQueuePtr _retryQueue;
    TimerPtr _timer;
    EndpointFactoryManagerPtr _endpointFactoryManager;
    DynamicLibraryListPtr _dynamicLibraryList;
    Ice::PluginManagerPtr _pluginManager;
    const Ice::ImplicitContextIPtr _implicitContext;
    Ice::StringConverterPtr _stringConverter;
    Ice::WstringConverterPtr _wstringConverter;
    bool _adminEnabled;
    Ice::ObjectAdapterPtr _adminAdapter;
    Ice::FacetMap _adminFacets;
    Ice::Identity _adminIdentity;
    std::set<std::string> _adminFacetFilter;
    IceInternal::MetricsAdminIPtr _metricsAdmin;
    std::map<Ice::Short, BufSizeWarnInfo> _setBufSizeWarn;
    IceUtil::Mutex _setBufSizeWarnMutex;
    ObjectFactoryMap _objectFactoryMap;
    mutable ObjectFactoryMap::iterator _objectFactoryMapHint;
};

class ProcessI : public Ice::Process
{
public:

    ProcessI(const Ice::CommunicatorPtr&);

    virtual void shutdown(const Ice::Current&);
#ifdef ICE_CPP11_MAPPING
    virtual void writeMessage(std::string, Ice::Int, const Ice::Current&);
#else
    virtual void writeMessage(const std::string&, Ice::Int, const Ice::Current&);
#endif

private:

    const Ice::CommunicatorPtr _communicator;
};

}

#endif
