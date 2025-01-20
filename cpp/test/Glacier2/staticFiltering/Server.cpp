// Copyright (c) ZeroC, Inc.

#include "BackendI.h"
#include "Ice/Ice.h"
#include "TestHelper.h"

using namespace std;
using namespace Ice;
using namespace Test;

//
// Dummy ServerLocatorRegistry, ServerLocator and ServantLocator. For
// simplicity, we essentially 'alias' all possible requests to a single
// object adapter and a single servant.
//
class ServerLocatorRegistry final : public LocatorRegistry
{
public:
    void setAdapterDirectProxyAsync(
        string,
        optional<ObjectPrx>,
        function<void()> response,
        function<void(exception_ptr)>,
        const Current&) override
    {
        response();
    }

    void setReplicatedAdapterDirectProxyAsync(
        string,
        string,
        optional<ObjectPrx>,
        function<void()> response,
        function<void(exception_ptr)>,
        const Current&) override
    {
        response();
    }

    void setServerProcessProxyAsync(
        string,
        optional<ProcessPrx>,
        function<void()> response,
        function<void(exception_ptr)>,
        const Current&) override
    {
        response();
    }
};

class ServerLocatorI final : public Locator
{
public:
    ServerLocatorI(shared_ptr<Backend> backend, ObjectAdapterPtr adapter)
        : _backend(std::move(backend)),
          _adapter(std::move(adapter)),
          _registryPrx(_adapter->add<LocatorRegistryPrx>(
              make_shared<ServerLocatorRegistry>(),
              Ice::stringToIdentity("registry")))
    {
    }

    void findObjectByIdAsync(
        Identity id,
        function<void(const optional<ObjectPrx>&)> response,
        function<void(exception_ptr)>,
        const Current&) const override
    {
        response(_adapter->createProxy(id));
    }

    void findAdapterByIdAsync(
        string,
        function<void(const optional<ObjectPrx>&)> response,
        function<void(exception_ptr)>,
        const Current&) const override
    {
        response(_adapter->createDirectProxy(stringToIdentity("dummy")));
    }

    [[nodiscard]] optional<LocatorRegistryPrx> getRegistry(const Current&) const override { return _registryPrx; }

private:
    const shared_ptr<Backend> _backend;
    const ObjectAdapterPtr _adapter;
    const LocatorRegistryPrx _registryPrx;
};

class BackendServer final : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
BackendServer::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    string endpoints =
        communicator->getProperties()->getPropertyWithDefault("BackendAdapter.Endpoints", "tcp -p 12010:ssl -p 12011");

    communicator->getProperties()->setProperty("BackendAdapter.Endpoints", endpoints);
    auto adapter = communicator->createObjectAdapter("BackendAdapter");
    auto backend = make_shared<BackendI>();
    auto locator = make_shared<ServerLocatorI>(backend, adapter);
    adapter->add(locator, Ice::stringToIdentity("locator"));
    adapter->addDefaultServant(backend, "");
    adapter->activate();
    communicator->waitForShutdown();
}

DEFINE_TEST(BackendServer)
