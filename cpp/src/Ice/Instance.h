// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
#include <IceUtil/StringConverter.h>
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
#include <Ice/ObjectFactoryManagerF.h>
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
#include <IceUtil/UniquePtr.h>

namespace Ice
{

class CommunicatorI;

}

namespace IceInternal
{

class Timer;
typedef IceUtil::Handle<Timer> TimerPtr;

class MetricsAdminI;
typedef IceUtil::Handle<MetricsAdminI> MetricsAdminIPtr;

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
    ObjectFactoryManagerPtr servantFactoryManager() const;
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
    bool collectObjects() const { return _collectObjects; }
    const ACMConfig& clientACM() const;
    const ACMConfig& serverACM() const;
    Ice::Identity stringToIdentity(const std::string&) const;
    std::string identityToString(const Ice::Identity&) const;

    Ice::ObjectPrx createAdmin(const Ice::ObjectAdapterPtr&, const Ice::Identity&);
    Ice::ObjectPrx getAdmin();
    void addAdminFacet(const Ice::ObjectPtr&, const std::string&);
    Ice::ObjectPtr removeAdminFacet(const std::string&);
    Ice::ObjectPtr findAdminFacet(const std::string&);
    Ice::FacetMap findAllAdminFacets();

    const Ice::ImplicitContextIPtr& getImplicitContext() const
    {
        return _implicitContext;
    }

    void setDefaultLocator(const Ice::LocatorPrx&);
    void setDefaultRouter(const Ice::RouterPrx&);

    void setLogger(const Ice::LoggerPtr&);
    void setThreadHook(const Ice::ThreadNotificationPtr&);

    IceUtil::StringConverterPtr getStringConverter() const { return _stringConverter; }
    IceUtil::WstringConverterPtr getWstringConverter() const { return _wstringConverter; }

    BufSizeWarnInfo getBufSizeWarn(Ice::Short type);
    void setSndBufSizeWarn(Ice::Short type, int size);
    void setRcvBufSizeWarn(Ice::Short type, int size);

private:

    Instance(const Ice::CommunicatorPtr&, const Ice::InitializationData&);
    virtual ~Instance();
    void finishSetup(int&, char*[], const Ice::CommunicatorPtr&);
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
    const bool _collectObjects; // Immutable, not reset by destroy().
    ACMConfig _clientACM;
    ACMConfig _serverACM;
    RouterManagerPtr _routerManager;
    LocatorManagerPtr _locatorManager;
    ReferenceFactoryPtr _referenceFactory;
    RequestHandlerFactoryPtr _requestHandlerFactory;
    ProxyFactoryPtr _proxyFactory;
    OutgoingConnectionFactoryPtr _outgoingConnectionFactory;
    ObjectFactoryManagerPtr _servantFactoryManager;
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
    IceUtil::StringConverterPtr _stringConverter;
    IceUtil::WstringConverterPtr _wstringConverter;
    bool _adminEnabled;
    Ice::ObjectAdapterPtr _adminAdapter;
    Ice::FacetMap _adminFacets;
    Ice::Identity _adminIdentity;
    std::set<std::string> _adminFacetFilter;
    IceInternal::MetricsAdminIPtr _metricsAdmin;
    std::map<Ice::Short, BufSizeWarnInfo> _setBufSizeWarn;
    IceUtil::Mutex _setBufSizeWarnMutex;
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
