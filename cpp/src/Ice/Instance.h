// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
#include <Ice/UserExceptionFactoryManagerF.h>
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

class Instance : public ::IceUtil::Shared, public ::IceUtil::RecMutex
{
public:

    ::Ice::PropertiesPtr properties();
    ::Ice::LoggerPtr logger();
    void logger(const ::Ice::LoggerPtr&);
    TraceLevelsPtr traceLevels();
    DefaultsAndOverridesPtr defaultsAndOverrides();
    RouterManagerPtr routerManager();
    LocatorManagerPtr locatorManager();
    ReferenceFactoryPtr referenceFactory();
    ProxyFactoryPtr proxyFactory();
    OutgoingConnectionFactoryPtr outgoingConnectionFactory();
    ConnectionMonitorPtr connectionMonitor();
    ObjectFactoryManagerPtr servantFactoryManager();
    UserExceptionFactoryManagerPtr userExceptionFactoryManager();
    ObjectAdapterFactoryPtr objectAdapterFactory();
    ThreadPoolPtr clientThreadPool();
    ThreadPoolPtr serverThreadPool();
    EndpointFactoryManagerPtr endpointFactoryManager();
    DynamicLibraryListPtr dynamicLibraryList();
    ::Ice::PluginManagerPtr pluginManager();
    
private:

    Instance(const ::Ice::CommunicatorPtr&, int&, char*[], const ::Ice::PropertiesPtr&);
    virtual ~Instance();
    void finishSetup(int&, char*[]);
    void destroy();
    friend class ::Ice::CommunicatorI;

    bool _destroyed;
    ::Ice::PropertiesPtr _properties; // Immutable, not reset by destroy().
    ::Ice::LoggerPtr _logger; // Not reset by destroy().
    TraceLevelsPtr _traceLevels; // Immutable, not reset by destroy().
    DefaultsAndOverridesPtr _defaultsAndOverrides; // Immutable, not reset by destroy().
    RouterManagerPtr _routerManager;
    LocatorManagerPtr _locatorManager;
    ReferenceFactoryPtr _referenceFactory;
    ProxyFactoryPtr _proxyFactory;
    OutgoingConnectionFactoryPtr _outgoingConnectionFactory;
    ConnectionMonitorPtr _connectionMonitor;
    ObjectFactoryManagerPtr _servantFactoryManager;
    UserExceptionFactoryManagerPtr _userExceptionFactoryManager;
    ObjectAdapterFactoryPtr _objectAdapterFactory;
    ThreadPoolPtr _clientThreadPool;
    ThreadPoolPtr _serverThreadPool;
    EndpointFactoryManagerPtr _endpointFactoryManager;
    DynamicLibraryListPtr _dynamicLibraryList;
    ::Ice::PluginManagerPtr _pluginManager;

    //
    // Global state management
    //
    friend class GlobalStateMutexDestroyer;
    static int _globalStateCounter;
    static ::IceUtil::Mutex* _globalStateMutex;
#ifndef _WIN32
    static std::string _identForOpenlog;
#endif
};

}

#endif
