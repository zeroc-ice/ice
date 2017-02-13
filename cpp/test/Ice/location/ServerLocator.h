// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SERVER_LOCATOR_H
#define SERVER_LOCATOR_H

#include <Ice/Locator.h>
#include <Test.h>
#include <Ice/ProxyF.h>

class ServerLocatorRegistry : public Test::TestLocatorRegistry
{
public:

    ServerLocatorRegistry();
    
    virtual void setAdapterDirectProxy_async(const Ice::AMD_LocatorRegistry_setAdapterDirectProxyPtr&,
                                             const ::std::string&, const ::Ice::ObjectPrx&, const ::Ice::Current&);
    virtual void setReplicatedAdapterDirectProxy_async(
        const Ice::AMD_LocatorRegistry_setReplicatedAdapterDirectProxyPtr&,
        const std::string&, const ::std::string&, const ::Ice::ObjectPrx&, const ::Ice::Current&);
    virtual void setServerProcessProxy_async(const Ice::AMD_LocatorRegistry_setServerProcessProxyPtr&,
                                             const ::std::string&, const ::Ice::ProcessPrx&, const ::Ice::Current&);
    void addObject(const ::Ice::ObjectPrx&, const ::Ice::Current&);
    
    //
    // Internal method
    //
    ::Ice::ObjectPrx getAdapter(const ::std::string&) const;
    ::Ice::ObjectPrx getObject(const ::Ice::Identity&) const;
    void addObject(const ::Ice::ObjectPrx&);

private:
    
    ::std::map< ::std::string, ::Ice::ObjectPrx> _adapters;
    ::std::map< ::Ice::Identity, ::Ice::ObjectPrx> _objects;
};

typedef ::IceInternal::Handle< ServerLocatorRegistry> ServerLocatorRegistryPtr;

class ServerLocator : public Test::TestLocator
{
public:

    ServerLocator(const ::ServerLocatorRegistryPtr&, const ::Ice::LocatorRegistryPrx&);

    virtual void findObjectById_async(const ::Ice::AMD_Locator_findObjectByIdPtr&, const ::Ice::Identity&, 
                                      const ::Ice::Current&) const;

    virtual void findAdapterById_async(const ::Ice::AMD_Locator_findAdapterByIdPtr&, const ::std::string&, 
                                       const ::Ice::Current&) const;

    virtual ::Ice::LocatorRegistryPrx getRegistry(const ::Ice::Current&) const;

    virtual int getRequestCount(const Ice::Current&) const;

private:
    
    ServerLocatorRegistryPtr _registry;
    ::Ice::LocatorRegistryPrx _registryPrx;
    int _requestCount;
};

#endif
