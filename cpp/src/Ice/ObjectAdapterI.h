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

#ifndef ICE_OBJECT_ADAPTER_I_H
#define ICE_OBJECT_ADAPTER_I_H

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/InstanceF.h>
#include <Ice/ObjectAdapterFactoryF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/LoggerF.h>
#include <Ice/ConnectionFactoryF.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/Exception.h>
#include <Ice/EndpointF.h>
#include <Ice/LocatorInfoF.h>
#include <list>

namespace Ice
{

class ObjectAdapterI;
typedef IceUtil::Handle<ObjectAdapterI> ObjectAdapterIPtr;

class ObjectAdapterI : public ObjectAdapter, public ::IceUtil::Monitor< ::IceUtil::Mutex>
{
public:

    virtual CommunicatorPtr getCommunicator();

    virtual void activate();
    virtual void hold();
    virtual void deactivate();
    virtual void waitForDeactivate();

    virtual ObjectPrx add(const ObjectPtr&, const Identity&);
    virtual ObjectPrx addWithUUID(const ObjectPtr&);
    virtual void remove(const Identity&);

    virtual void addServantLocator(const ServantLocatorPtr&, const std::string&);
    virtual void removeServantLocator(const std::string&);
    virtual ServantLocatorPtr findServantLocator(const std::string&);

    virtual ObjectPtr identityToServant(const Identity&);
    virtual ObjectPtr proxyToServant(const ObjectPrx&);

    virtual ObjectPrx createProxy(const Identity&);
    virtual ObjectPrx createDirectProxy(const Identity&);
    virtual ObjectPrx createReverseProxy(const Identity&);

    virtual void addRouter(const RouterPrx&);

    virtual void setLocator(const LocatorPrx&);
    
    std::list< ::IceInternal::ConnectionPtr> getIncomingConnections() const;
    void incUsageCount();
    void decUsageCount();

private:

    ObjectAdapterI(const ::IceInternal::InstancePtr&, const CommunicatorPtr&, const std::string&, const std::string&,
		   const std::string&);
    virtual ~ObjectAdapterI();
    friend class ::IceInternal::ObjectAdapterFactory;
    
    ObjectPrx newProxy(const Identity&) const;
    ObjectPrx newDirectProxy(const Identity&) const;
    static void checkIdentity(const Identity&);
    bool isLocal(const ObjectPrx&) const;

    ::IceInternal::InstancePtr _instance;
    CommunicatorPtr _communicator;
    bool _printAdapterReadyDone;
    std::string _name;
    std::string _id;
    LoggerPtr _logger;
    ObjectDict _activeServantMap;
    ObjectDict::iterator _activeServantMapHint;
    std::map<std::string, ServantLocatorPtr> _locatorMap;
    std::map<std::string, ServantLocatorPtr>::iterator _locatorMapHint;
    std::vector< ::IceInternal::IncomingConnectionFactoryPtr> _incomingConnectionFactories;
    std::vector< ::IceInternal::EndpointPtr> _routerEndpoints;
    IceUtil::Mutex _routerEndpointsMutex;
    ::IceInternal::LocatorInfoPtr _locatorInfo;
    int _usageCount;
    static const char * const _kindOfObject;
};

}

#endif
