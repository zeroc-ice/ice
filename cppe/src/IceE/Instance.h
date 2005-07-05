// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_INSTANCE_H
#define ICEE_INSTANCE_H

#include <IceE/Shared.h>
#include <IceE/Mutex.h>
#include <IceE/RecMutex.h>
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
#    include <IceE/ObjectAdapterFactoryF.h>
#endif

namespace IceE
{

class CommunicatorI;

}

namespace IceEInternal
{

class Instance : public IceE::Shared, public IceE::RecMutex
{
public:

    IceE::PropertiesPtr properties() const;
    IceE::LoggerPtr logger() const;
    void logger(const IceE::LoggerPtr&);
    TraceLevelsPtr traceLevels() const;
    DefaultsAndOverridesPtr defaultsAndOverrides() const;
#ifndef ICEE_NO_ROUTER
    RouterManagerPtr routerManager() const;
#endif
#ifndef ICEE_NO_LOCATOR
    LocatorManagerPtr locatorManager() const;
#endif
    ReferenceFactoryPtr referenceFactory() const;
    ProxyFactoryPtr proxyFactory() const;
    OutgoingConnectionFactoryPtr outgoingConnectionFactory() const;
    EndpointFactoryPtr endpointFactory() const;
    size_t messageSizeMax() const;
    IceE::Int connectionIdleTime() const;
#ifndef ICEE_NO_BATCH
    void flushBatchRequests();
#endif
    void setDefaultContext(const ::IceE::Context&);
    const ::IceE::Context& getDefaultContext() const;
    size_t threadPerConnectionStackSize() const;

#ifndef ICEE_PURE_CLIENT
    ObjectAdapterFactoryPtr objectAdapterFactory() const;
#endif
    
private:

    Instance(const IceE::CommunicatorPtr&, const IceE::PropertiesPtr&);
    virtual ~Instance();
    void finishSetup(int&, char*[]);
    void destroy();
    friend class IceE::Communicator;

    bool _destroyed;
    const IceE::PropertiesPtr _properties; // Immutable, not reset by destroy().
    IceE::LoggerPtr _logger; // Not reset by destroy().
    const TraceLevelsPtr _traceLevels; // Immutable, not reset by destroy().
    const DefaultsAndOverridesPtr _defaultsAndOverrides; // Immutable, not reset by destroy().
    const size_t _messageSizeMax; // Immutable, not reset by destroy().
    const size_t _threadPerConnectionStackSize;
#ifndef ICEE_NO_ROUTER
    RouterManagerPtr _routerManager;
#endif
#ifndef ICEE_NO_LOCATOR
    LocatorManagerPtr _locatorManager;
#endif
    ReferenceFactoryPtr _referenceFactory;
    ProxyFactoryPtr _proxyFactory;
    OutgoingConnectionFactoryPtr _outgoingConnectionFactory;
    EndpointFactoryPtr _endpointFactory;
    IceE::Context _defaultContext;

#ifndef ICEE_PURE_CLIENT
    ObjectAdapterFactoryPtr _objectAdapterFactory;
#endif
};

}

#endif
