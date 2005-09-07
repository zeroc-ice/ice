// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

    bool destroyed() const;
    Ice::PropertiesPtr properties() const;
    Ice::LoggerPtr logger() const;
    void logger(const Ice::LoggerPtr&);
    Ice::StatsPtr stats() const;
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
    ThreadPoolPtr clientThreadPool();
    ThreadPoolPtr serverThreadPool();
    bool threadPerConnection() const;
    size_t threadPerConnectionStackSize() const;
    EndpointFactoryManagerPtr endpointFactoryManager() const;
    DynamicLibraryListPtr dynamicLibraryList() const;
    Ice::PluginManagerPtr pluginManager() const;
    size_t messageSizeMax() const;
    Ice::Int clientACM() const;
    Ice::Int serverACM() const;
    void flushBatchRequests();
    void setDefaultContext(const ::Ice::Context&);
    const ::Ice::Context& getDefaultContext() const;
    
private:

    Instance(const Ice::CommunicatorPtr&, const Ice::PropertiesPtr&);
    virtual ~Instance();
    void finishSetup(int&, char*[]);
    bool destroy();
    friend class Ice::CommunicatorI;

    enum State
    {
	StateActive,
	StateDestroyInProgress,
	StateDestroyed
    };
    State _state;
    const Ice::PropertiesPtr _properties; // Immutable, not reset by destroy().
    Ice::LoggerPtr _logger; // Not reset by destroy().
    Ice::StatsPtr _stats; // Not reset by destroy().
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
    ThreadPoolPtr _clientThreadPool;
    ThreadPoolPtr _serverThreadPool;
    const bool _threadPerConnection;
    const size_t _threadPerConnectionStackSize;
    EndpointFactoryManagerPtr _endpointFactoryManager;
    DynamicLibraryListPtr _dynamicLibraryList;
    Ice::PluginManagerPtr _pluginManager;
    Ice::Context _defaultContext;
};

}

#endif
