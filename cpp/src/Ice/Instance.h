// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_INSTANCE_H
#define ICE_INSTANCE_H

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/RecMutex.h>
#include <Ice/InstanceF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/PropertiesF.h>
#include <Ice/LoggerF.h>
#include <Ice/StatsF.h>
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
#include <Ice/DynamicLibraryF.h>
#include <Ice/PluginF.h>
#include <list>

namespace Ice
{

class CommunicatorI;

}

namespace IceInternal
{

class Instance : public IceUtil::Shared, public IceUtil::RecMutex
{
public:

    Ice::PropertiesPtr properties();
    Ice::LoggerPtr logger();
    void logger(const Ice::LoggerPtr&);
    Ice::StatsPtr stats();
    void stats(const Ice::StatsPtr&);
    TraceLevelsPtr traceLevels();
    DefaultsAndOverridesPtr defaultsAndOverrides();
    RouterManagerPtr routerManager();
    LocatorManagerPtr locatorManager();
    ReferenceFactoryPtr referenceFactory();
    ProxyFactoryPtr proxyFactory();
    OutgoingConnectionFactoryPtr outgoingConnectionFactory();
    ConnectionMonitorPtr connectionMonitor();
    ObjectFactoryManagerPtr servantFactoryManager();
    ObjectAdapterFactoryPtr objectAdapterFactory();
    ThreadPoolPtr clientThreadPool();
    ThreadPoolPtr serverThreadPool();
    EndpointFactoryManagerPtr endpointFactoryManager();
    DynamicLibraryListPtr dynamicLibraryList();
    Ice::PluginManagerPtr pluginManager();
    size_t messageSizeMax() const;
    Ice::Int connectionIdleTime() const;
    void flushBatchRequests();
    
private:

    Instance(const Ice::CommunicatorPtr&, const Ice::PropertiesPtr&);
    virtual ~Instance();
    void finishSetup(int&, char*[]);
    void destroy();
    friend class Ice::CommunicatorI;

    bool _destroyed;
    const Ice::PropertiesPtr _properties; // Immutable, not reset by destroy().
    Ice::LoggerPtr _logger; // Not reset by destroy().
    Ice::StatsPtr _stats; // Not reset by destroy().
    const TraceLevelsPtr _traceLevels; // Immutable, not reset by destroy().
    const DefaultsAndOverridesPtr _defaultsAndOverrides; // Immutable, not reset by destroy().
    const size_t _messageSizeMax; // Immutable, not reset by destroy().
    const Ice::Int _connectionIdleTime; // Immutable, not reset by destroy().
    RouterManagerPtr _routerManager;
    LocatorManagerPtr _locatorManager;
    ReferenceFactoryPtr _referenceFactory;
    ProxyFactoryPtr _proxyFactory;
    OutgoingConnectionFactoryPtr _outgoingConnectionFactory;
    ConnectionMonitorPtr _connectionMonitor;
    ObjectFactoryManagerPtr _servantFactoryManager;
    ObjectAdapterFactoryPtr _objectAdapterFactory;
    ThreadPoolPtr _clientThreadPool;
    ThreadPoolPtr _serverThreadPool;
    EndpointFactoryManagerPtr _endpointFactoryManager;
    DynamicLibraryListPtr _dynamicLibraryList;
    Ice::PluginManagerPtr _pluginManager;
    volatile static bool _printProcessIdDone;

    //
    // Global state management
    //

    friend class GlobalStateMutexDestroyer;

    static int _globalStateCounter;

    //
    // This is *not* a StaticMutex. Only the pointer is static, but not
    // the mutex itself.
    //
    static IceUtil::Mutex* _globalStateMutex;

#ifndef _WIN32
    static std::string _identForOpenlog;
#endif
};

}

#endif
