// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_OBJECT_ADAPTER_I_H
#define ICE_OBJECT_ADAPTER_I_H

#include <IceUtil/Shared.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/InstanceF.h>
#include <Ice/ObjectAdapterFactoryF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/CollectorF.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/Exception.h>
#include <Ice/EndpointF.h>

namespace Ice
{

class ObjectAdapterI;
typedef IceUtil::Handle<ObjectAdapterI> ObjectAdapterIPtr;

class ObjectAdapterI : public ObjectAdapter, public JTCMutex
{
public:

    virtual std::string getName();
    virtual CommunicatorPtr getCommunicator();

    virtual void activate();
    virtual void hold();
    virtual void deactivate();

    virtual ObjectPrx add(const ObjectPtr&, const Identity&);
    virtual ObjectPrx addTemporary(const ObjectPtr&);
    virtual void remove(const Identity&);

    virtual void addServantLocator(const ServantLocatorPtr&, const std::string&);
    virtual void removeServantLocator(const std::string&);
    virtual ServantLocatorPtr findServantLocator(const std::string&);

    virtual ObjectPtr identityToServant(const Identity&);
    virtual ObjectPtr proxyToServant(const ObjectPrx&);

    virtual ObjectPrx createProxy(const Identity&);

private:

    ObjectAdapterI(const ::IceInternal::InstancePtr&, const std::string&, const std::string&);
    virtual ~ObjectAdapterI();
    friend ::IceInternal::ObjectAdapterFactory;
    
    ObjectPrx newProxy(const Identity&);
    bool isLocal(const ObjectPrx&);

    ::IceInternal::InstancePtr _instance;
    std::string _name;
    std::vector< IceInternal::CollectorFactoryPtr> _collectorFactories;
    ObjectDict _activeServantMap;
    ObjectDict::iterator _activeServantMapHint;
    std::map<std::string, ServantLocatorPtr> _locatorMap;
    std::map<std::string, ServantLocatorPtr>::iterator _locatorMapHint;
};

}

#endif
