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

#include <Ice/ObjectAdapter.h>
#include <Ice/InstanceF.h>
#include <Ice/ObjectAdapterFactoryF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/CollectorF.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/LocalException.h>
#include <Ice/Shared.h>
#include <map>

namespace Ice
{

class ICE_API ObjectAdapterI : public ObjectAdapter, public JTCMutex
{
public:

    virtual std::string getName();
    virtual CommunicatorPtr getCommunicator();

    virtual void activate();
    virtual void hold();
    virtual void deactivate();

    virtual void add(const ObjectPtr&, const std::string&);
    virtual void addTemporary(const ObjectPtr&);
    virtual void remove(const std::string&);

    virtual void setObjectLocator(const ObjectLocatorPtr&);
    virtual ObjectLocatorPtr getObjectLocator();

    virtual ObjectPtr identityToObject(const std::string&);
    virtual std::string objectToIdentity(const ObjectPtr&);

    virtual ObjectPtr proxyToObject(const ObjectPrx&);
    virtual ObjectPrx objectToProxy(const ObjectPtr&);

    virtual ObjectPrx identityToProxy(const std::string&);
    virtual std::string proxyToIdentity(const ObjectPrx&);

private:

    ObjectAdapterI(const ::IceInternal::InstancePtr&, const std::string&,
		   const std::string&);
    virtual ~ObjectAdapterI();
    friend ::IceInternal::ObjectAdapterFactory;
    
    ::IceInternal::InstancePtr _instance;
    std::string _name;
    std::vector< IceInternal::CollectorFactoryPtr> _collectorFactories;
    std::map<std::string, ObjectPtr> _objects;
    ::Ice::ObjectLocatorPtr _locator;
};

}

#endif
