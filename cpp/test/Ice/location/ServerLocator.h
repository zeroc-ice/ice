// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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

#ifdef ICE_CPP11_MAPPING
    virtual void setAdapterDirectProxyAsync(std::string, std::shared_ptr<::Ice::ObjectPrx>,
                                            std::function<void()>,
                                            std::function<void(std::exception_ptr)>,
                                            const ::Ice::Current&);
    virtual void setReplicatedAdapterDirectProxyAsync(std::string, std::string, std::shared_ptr<Ice::ObjectPrx>,
                                                      std::function<void()>,
                                                      std::function<void(std::exception_ptr)>,
                                                      const ::Ice::Current&);

    virtual void setServerProcessProxyAsync(std::string, std::shared_ptr<Ice::ProcessPrx>,
                                            std::function<void()>,
                                            std::function<void(std::exception_ptr)>,
                                            const ::Ice::Current&);
    void addObject(std::shared_ptr<::Ice::ObjectPrx>, const ::Ice::Current&);
#else
    virtual void setAdapterDirectProxy_async(const Ice::AMD_LocatorRegistry_setAdapterDirectProxyPtr&,
                                             const ::std::string&, const ::Ice::ObjectPrx&, const ::Ice::Current&);
    virtual void setReplicatedAdapterDirectProxy_async(
        const Ice::AMD_LocatorRegistry_setReplicatedAdapterDirectProxyPtr&,
        const std::string&, const ::std::string&, const ::Ice::ObjectPrx&, const ::Ice::Current&);
    virtual void setServerProcessProxy_async(const Ice::AMD_LocatorRegistry_setServerProcessProxyPtr&,
                                             const ::std::string&, const ::Ice::ProcessPrx&, const ::Ice::Current&);
    void addObject(const ::Ice::ObjectPrx&, const ::Ice::Current&);
#endif

    //
    // Internal method
    //
    ::Ice::ObjectPrxPtr getAdapter(const ::std::string&) const;
    ::Ice::ObjectPrxPtr getObject(const ::Ice::Identity&) const;
    void addObject(const ::Ice::ObjectPrxPtr&);

private:

    ::std::map< ::std::string, ::Ice::ObjectPrxPtr> _adapters;
    ::std::map< ::Ice::Identity, ::Ice::ObjectPrxPtr> _objects;
};
ICE_DEFINE_PTR(ServerLocatorRegistryPtr, ServerLocatorRegistry);

class ServerLocator : public Test::TestLocator
{
public:

    ServerLocator(const ::ServerLocatorRegistryPtr&, const ::Ice::LocatorRegistryPrxPtr&);

#ifdef ICE_CPP11_MAPPING
    virtual void findObjectByIdAsync(::Ice::Identity,
                                      std::function<void(const std::shared_ptr<Ice::ObjectPrx>&)>,
                                      std::function<void(std::exception_ptr)>,
                                      const ::Ice::Current&) const;

    virtual void findAdapterByIdAsync(::std::string,
                                       std::function<void(const std::shared_ptr<Ice::ObjectPrx>&)>,
                                       std::function<void(std::exception_ptr)>,
                                       const ::Ice::Current&) const;
#else
    virtual void findObjectById_async(const ::Ice::AMD_Locator_findObjectByIdPtr&, const ::Ice::Identity&,
                                      const ::Ice::Current&) const;

    virtual void findAdapterById_async(const ::Ice::AMD_Locator_findAdapterByIdPtr&, const ::std::string&,
                                       const ::Ice::Current&) const;
#endif
    virtual ::Ice::LocatorRegistryPrxPtr getRegistry(const ::Ice::Current&) const;

    virtual int getRequestCount(const Ice::Current&) const;

private:

    ServerLocatorRegistryPtr _registry;
    ::Ice::LocatorRegistryPrxPtr _registryPrx;
    int _requestCount;
};

#endif
