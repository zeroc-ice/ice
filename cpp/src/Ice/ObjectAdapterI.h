// **********************************************************************
//
// Copyright (c) 2003
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

#ifndef ICE_OBJECT_ADAPTER_I_H
#define ICE_OBJECT_ADAPTER_I_H

#include <IceUtil/Shared.h>
#include <IceUtil/RecMutex.h>
#include <IceUtil/Monitor.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/InstanceF.h>
#include <Ice/ObjectAdapterFactoryF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/ConnectionFactoryF.h>
#include <Ice/ServantManagerF.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/Exception.h>
#include <Ice/EndpointF.h>
#include <Ice/LocatorInfoF.h>
#include <Ice/ThreadPoolF.h>
#include <list>

namespace Ice
{

class ObjectAdapterI;
typedef IceUtil::Handle<ObjectAdapterI> ObjectAdapterIPtr;

class ObjectAdapterI : public ObjectAdapter, public IceUtil::Monitor<IceUtil::RecMutex>
{
public:

    virtual std::string getName();

    virtual CommunicatorPtr getCommunicator();

    virtual void activate();
    virtual void hold();
    virtual void waitForHold();
    virtual void deactivate();
    virtual void waitForDeactivate();

    virtual ObjectPrx add(const ObjectPtr&, const Identity&);
    virtual ObjectPrx addWithUUID(const ObjectPtr&);
    virtual void remove(const Identity&);

    virtual void addServantLocator(const ServantLocatorPtr&, const std::string&);
    virtual ServantLocatorPtr findServantLocator(const std::string&);

    virtual ObjectPtr identityToServant(const Identity&);
    virtual ObjectPtr proxyToServant(const ObjectPrx&);

    virtual ObjectPrx createProxy(const Identity&);
    virtual ObjectPrx createDirectProxy(const Identity&);
    virtual ObjectPrx createReverseProxy(const Identity&);

    virtual void addRouter(const RouterPrx&);

    virtual void setLocator(const LocatorPrx&);
    
    bool isLocal(const ObjectPrx&) const;

    std::list<IceInternal::ConnectionPtr> getIncomingConnections() const;

    void incDirectCount();
    void decDirectCount();

    IceInternal::ThreadPoolPtr getThreadPool() const;
    IceInternal::ServantManagerPtr getServantManager() const;

private:

    ObjectAdapterI(const IceInternal::InstancePtr&, const CommunicatorPtr&, const std::string&);
    virtual ~ObjectAdapterI();
    friend class IceInternal::ObjectAdapterFactory;
    
    ObjectPrx newProxy(const Identity&) const;
    ObjectPrx newDirectProxy(const Identity&) const;
    void checkForDeactivation() const;
    static void checkIdentity(const Identity&);

    bool _deactivated;
    IceInternal::InstancePtr _instance;
    CommunicatorPtr _communicator;
    IceInternal::ThreadPoolPtr _threadPool;
    IceInternal::ServantManagerPtr _servantManager;
    bool _printAdapterReadyDone;
    const std::string _name;
    const std::string _id;
    std::vector<IceInternal::IncomingConnectionFactoryPtr> _incomingConnectionFactories;
    std::vector<IceInternal::EndpointPtr> _routerEndpoints;
    IceInternal::LocatorInfoPtr _locatorInfo;
    int _directCount; // The number of direct proxies dispatching on this object adapter.
    bool _waitForDeactivate;
};

}

#endif
