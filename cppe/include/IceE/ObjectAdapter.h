// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_OBJECT_ADAPTER_H
#define ICEE_OBJECT_ADAPTER_H

#include <IceE/ObjectAdapterF.h>
#include <IceE/InstanceF.h>
#include <IceE/ObjectAdapterFactoryF.h>
#include <IceE/CommunicatorF.h>
#include <IceE/IncomingConnectionFactoryF.h>
#include <IceE/ServantManagerF.h>
#include <IceE/ProxyF.h>
#include <IceE/ObjectF.h>
#include <IceE/EndpointF.h>

#ifdef ICEE_HAS_ROUTER
#   include <IceE/RouterF.h>
#endif

#ifdef ICEE_HAS_LOCATOR
#   include <IceE/LocatorF.h>
#   include <IceE/LocatorInfoF.h>
#endif

#include <IceE/Exception.h>
#include <IceE/Shared.h>
#include <IceE/RecMutex.h>
#include <IceE/Monitor.h>
#include <IceE/FacetMap.h>
#include <IceE/Proxy.h>
#include <list>

namespace Ice
{

class ICE_API ObjectAdapter : public IceUtil::Monitor<IceUtil::RecMutex>, public ::IceUtil::Shared
{
public:

    std::string getName() const;

    CommunicatorPtr getCommunicator() const;

    void activate();
    void hold();
    void waitForHold();
    void deactivate();
    void waitForDeactivate();

    ObjectPrx add(const ObjectPtr&, const Identity&);
    ObjectPrx addFacet(const ObjectPtr&, const Identity&, const std::string&);
    ObjectPrx addWithUUID(const ObjectPtr&);
    ObjectPrx addFacetWithUUID(const ObjectPtr&, const std::string&);
    ObjectPtr remove(const Identity&);
    ObjectPtr removeFacet(const Identity&, const std::string&);
    FacetMap removeAllFacets(const Identity&);
    ObjectPtr find(const Identity&) const;
    ObjectPtr findFacet(const Identity&, const std::string&) const;
    FacetMap findAllFacets(const Identity&) const;
    ObjectPtr findByProxy(const ObjectPrx&) const;

    ObjectPrx createProxy(const Identity&) const;
    ObjectPrx createDirectProxy(const Identity&) const;
    ObjectPrx createReverseProxy(const Identity&) const;

#ifdef ICEE_HAS_ROUTER
    void addRouter(const RouterPrx&);
#endif
#ifdef ICEE_HAS_LOCATOR
    void setLocator(const LocatorPrx&);
#endif
    
    bool isLocal(const ObjectPrx&) const;

    void flushBatchRequests();

    void incDirectCount();
    void decDirectCount();

    IceInternal::ServantManagerPtr getServantManager() const;

private:

    ObjectAdapter(const IceInternal::InstancePtr&, const CommunicatorPtr&, const std::string&, const std::string&);
    ~ObjectAdapter();
    friend class IceInternal::ObjectAdapterFactory;
    
    ObjectPrx newProxy(const Identity&, const std::string&) const;
    ObjectPrx newDirectProxy(const Identity&, const std::string&) const;
    void checkForDeactivation() const;
    static void checkIdentity(const Identity&);
    std::vector<IceInternal::EndpointPtr> parseEndpoints(const std::string&) const;

    bool _deactivated;
    IceInternal::InstancePtr _instance;
    CommunicatorPtr _communicator;
    IceInternal::ServantManagerPtr _servantManager;
    bool _printAdapterReadyDone;
    const std::string _name;
    const std::string _id;
    std::vector<IceInternal::IncomingConnectionFactoryPtr> _incomingConnectionFactories;
#ifdef ICEE_HAS_ROUTER
    std::vector<IceInternal::EndpointPtr> _routerEndpoints;
#endif
    std::vector<IceInternal::EndpointPtr> _publishedEndpoints;
#ifdef ICEE_HAS_LOCATOR
    IceInternal::LocatorInfoPtr _locatorInfo;
#endif
    int _directCount; // The number of direct proxies dispatching on this object adapter.
    bool _waitForDeactivate;
};

}

#endif
