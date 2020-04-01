//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef SERVER_LOCATOR_H
#define SERVER_LOCATOR_H

#include <Ice/Locator.h>
#include <Test.h>
#include <Ice/ProxyF.h>

class ServerLocatorRegistry : public Test::TestLocatorRegistry
{
public:

    ServerLocatorRegistry();

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

    virtual void findObjectByIdAsync(::Ice::Identity,
                                      std::function<void(const std::shared_ptr<Ice::ObjectPrx>&)>,
                                      std::function<void(std::exception_ptr)>,
                                      const ::Ice::Current&) const;

    virtual void findAdapterByIdAsync(::std::string,
                                       std::function<void(const std::shared_ptr<Ice::ObjectPrx>&)>,
                                       std::function<void(std::exception_ptr)>,
                                       const ::Ice::Current&) const;
    virtual ::Ice::LocatorRegistryPrxPtr getRegistry(const ::Ice::Current&) const;

    virtual int getRequestCount(const Ice::Current&) const;

private:

    ServerLocatorRegistryPtr _registry;
    ::Ice::LocatorRegistryPrxPtr _registryPrx;
    int _requestCount;
};

#endif
