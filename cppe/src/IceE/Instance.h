// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_INSTANCE_H
#define ICEE_INSTANCE_H

#include <IceE/InstanceF.h>
#include <IceE/CommunicatorF.h>
#include <IceE/PropertiesF.h>
#include <IceE/LoggerF.h>
#include <IceE/TraceLevelsF.h>
#include <IceE/DefaultsAndOverridesF.h>
#include <IceE/RouterInfoF.h>
#include <IceE/LocatorInfoF.h>
#include <IceE/ReferenceFactoryF.h>
#include <IceE/ProxyFactoryF.h>
#include <IceE/OutgoingConnectionFactoryF.h>
#include <IceE/EndpointFactoryF.h>
#ifndef ICEE_PURE_CLIENT
#   include <IceE/ObjectAdapterFactoryF.h>
#endif
#include <IceE/Shared.h>
#include <IceE/RecMutex.h>
#include <IceE/Initialize.h>

namespace IceInternal
{

class Instance : public IceUtil::Shared, public IceUtil::RecMutex
{
public:

    bool destroyed() const;
    const Ice::InitializationData& initializationData() const { return _initData; }
    TraceLevelsPtr traceLevels() const;
    DefaultsAndOverridesPtr defaultsAndOverrides() const;
#ifdef ICEE_HAS_ROUTER
    RouterManagerPtr routerManager() const;
#endif
#ifdef ICEE_HAS_LOCATOR
    LocatorManagerPtr locatorManager() const;
#endif
    ReferenceFactoryPtr referenceFactory() const;
    ProxyFactoryPtr proxyFactory() const;
    OutgoingConnectionFactoryPtr outgoingConnectionFactory() const;
    EndpointFactoryPtr endpointFactory() const;
    size_t messageSizeMax() const { return _messageSizeMax; /* Immutable */ }  // Inlined for performance reasons.
    Ice::Int connectionIdleTime() const;
#ifdef ICEE_HAS_BATCH
    void flushBatchRequests();
#endif
#ifndef ICEE_PURE_BLOCKING_CLIENT
    size_t threadPerConnectionStackSize() const;
#endif

#ifndef ICEE_PURE_CLIENT
    ObjectAdapterFactoryPtr objectAdapterFactory() const;
#endif

private:

    Instance(const Ice::CommunicatorPtr&, const Ice::InitializationData&);
    virtual ~Instance();

    void finishSetup(int&, char*[]);
    void destroy();
    friend class Ice::Communicator;

    enum State
    {
	StateActive,
	StateDestroyInProgress,
	StateDestroyed
    };
    State _state;
    Ice::InitializationData _initData; // Immutable, not reset by destroy().
    const TraceLevelsPtr _traceLevels; // Immutable, not reset by destroy().
    const DefaultsAndOverridesPtr _defaultsAndOverrides; // Immutable, not reset by destroy().
    const size_t _messageSizeMax; // Immutable, not reset by destroy().
#ifndef ICEE_PURE_BLOCKING_CLIENT
    const size_t _threadPerConnectionStackSize;
#endif
#ifdef ICEE_HAS_ROUTER
    RouterManagerPtr _routerManager;
#endif
#ifdef ICEE_HAS_LOCATOR
    LocatorManagerPtr _locatorManager;
#endif
    ReferenceFactoryPtr _referenceFactory;
    ProxyFactoryPtr _proxyFactory;
    OutgoingConnectionFactoryPtr _outgoingConnectionFactory;
    EndpointFactoryPtr _endpointFactory;

#ifndef ICEE_PURE_CLIENT
    ObjectAdapterFactoryPtr _objectAdapterFactory;
#endif
};

}

#endif
