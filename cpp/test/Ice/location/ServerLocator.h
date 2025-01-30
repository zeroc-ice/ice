// Copyright (c) ZeroC, Inc.

#ifndef SERVER_LOCATOR_H
#define SERVER_LOCATOR_H

#include "Ice/Locator.h"
#include "Test.h"

class ServerLocatorRegistry final : public Test::TestLocatorRegistry
{
public:
    ServerLocatorRegistry();

    void setAdapterDirectProxyAsync(
        std::string,
        std::optional<Ice::ObjectPrx>,
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void setReplicatedAdapterDirectProxyAsync(
        std::string,
        std::string,
        std::optional<Ice::ObjectPrx>,
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void setServerProcessProxyAsync(
        std::string,
        std::optional<Ice::ProcessPrx>,
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void addObject(std::optional<Ice::ObjectPrx>, const Ice::Current&) final;

    //
    // Internal method
    //
    [[nodiscard]] std::optional<Ice::ObjectPrx> getAdapter(const std::string&) const;
    [[nodiscard]] std::optional<Ice::ObjectPrx> getObject(const Ice::Identity&) const;
    void addObject(const std::optional<Ice::ObjectPrx>&);

private:
    std::map<std::string, std::optional<Ice::ObjectPrx>> _adapters;
    std::map<Ice::Identity, std::optional<Ice::ObjectPrx>> _objects;
};
using ServerLocatorRegistryPtr = std::shared_ptr<ServerLocatorRegistry>;

class ServerLocator final : public Test::TestLocator
{
public:
    ServerLocator(::ServerLocatorRegistryPtr, const std::optional<Ice::LocatorRegistryPrx>&);

    void findObjectByIdAsync(
        Ice::Identity,
        std::function<void(const std::optional<Ice::ObjectPrx>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) const final;

    void findAdapterByIdAsync(
        std::string,
        std::function<void(const std::optional<Ice::ObjectPrx>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) const final;

    [[nodiscard]] std::optional<Ice::LocatorRegistryPrx> getRegistry(const Ice::Current&) const final;

    [[nodiscard]] int getRequestCount(const Ice::Current&) const final;

private:
    ServerLocatorRegistryPtr _registry;
    std::optional<Ice::LocatorRegistryPrx> _registryPrx;
    int _requestCount{0};
};

#endif
