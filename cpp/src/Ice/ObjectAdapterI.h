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
#include <Ice/LocalException.h>

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

    virtual ObjectPrx add(const ObjectPtr&, const std::string&);
    virtual ObjectPrx addTemporary(const ObjectPtr&);
    virtual void remove(const std::string&);

    virtual void setServantLocator(const ServantLocatorPtr&);
    virtual ServantLocatorPtr getServantLocator();

    virtual ObjectPtr identityToServant(const ::std::string&);
    virtual ObjectPtr proxyToServant(const ObjectPrx&);

    virtual ObjectPrx createProxy(const ::std::string&);

private:

    ObjectAdapterI(const ::IceInternal::InstancePtr&, const std::string&, const std::string&);
    virtual ~ObjectAdapterI();
    friend ::IceInternal::ObjectAdapterFactory;
    
    ObjectPrx newProxy(const ::std::string&);

    ::IceInternal::InstancePtr _instance;
    std::string _name;
    std::vector< IceInternal::CollectorFactoryPtr> _collectorFactories;
    std::map<std::string, ObjectPtr> _asm;
    std::map<std::string, ObjectPtr>::iterator _asmHint;
    ServantLocatorPtr _locator;
};

}

#endif
