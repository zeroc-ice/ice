// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
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
#include <IceE/RouterF.h>
#include <IceE/LocatorF.h>
#include <IceE/LocatorInfoF.h>

#include <IceE/Exception.h>
#include <IceE/Shared.h>
#include <IceE/RecMutex.h>
#include <IceE/Monitor.h>
#include <IceE/FacetMap.h>
#include <list>

namespace IceE
{

class ICEE_API ObjectAdapter : public IceE::Monitor<IceE::RecMutex>, public ::IceE::Shared
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

    void addRouter(const RouterPrx&);
    void setLocator(const LocatorPrx&);
    
    bool isLocal(const ObjectPrx&) const;

    void flushBatchRequests();

    void incDirectCount();
    void decDirectCount();

    IceEInternal::ServantManagerPtr getServantManager() const;

private:

    ObjectAdapter(const IceEInternal::InstancePtr&, const CommunicatorPtr&, const std::string&);
    ~ObjectAdapter();
    friend class IceEInternal::ObjectAdapterFactory;
    
    ObjectPrx newProxy(const Identity&, const std::string&) const;
    ObjectPrx newDirectProxy(const Identity&, const std::string&) const;
    void checkForDeactivation() const;
    static void checkIdentity(const Identity&);
    std::vector<IceEInternal::EndpointPtr> parseEndpoints(const std::string&) const;

    bool _deactivated;
    IceEInternal::InstancePtr _instance;
    CommunicatorPtr _communicator;
    IceEInternal::ServantManagerPtr _servantManager;
    bool _printAdapterReadyDone;
    const std::string _name;
    const std::string _id;
    std::vector<IceEInternal::IncomingConnectionFactoryPtr> _incomingConnectionFactories;
#ifndef ICEE_NO_ROUTER
    std::vector<IceEInternal::EndpointPtr> _routerEndpoints;
#endif
    std::vector<IceEInternal::EndpointPtr> _publishedEndpoints;
#ifndef ICEE_NO_LOCATOR
    IceEInternal::LocatorInfoPtr _locatorInfo;
#endif
    int _directCount; // The number of direct proxies dispatching on this object adapter.
    bool _waitForDeactivate;
};

}

#endif
