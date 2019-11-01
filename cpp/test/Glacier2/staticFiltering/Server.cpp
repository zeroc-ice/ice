//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <BackendI.h>
#include <TestHelper.h>

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

    void
    setAdapterDirectProxyAsync(string, shared_ptr<ObjectPrx>, function<void()> response, function<void(exception_ptr)>,
                               const Current&) override
    {
        response();
    }

    void
    setReplicatedAdapterDirectProxyAsync(string, string, shared_ptr<ObjectPrx>,
                                         function<void()> response, function<void(exception_ptr)>,
                                         const Current&) override
    {
        response();
    }

    void
    setServerProcessProxyAsync(string, shared_ptr<ProcessPrx>,
                               function<void()> response, function<void(exception_ptr)>,
                               const Current&) override
    {
        response();
    }
};

class ServerLocatorI final : public Locator
{
public:

    ServerLocatorI(shared_ptr<Backend> backend, const shared_ptr<ObjectAdapter>& adapter) :
        _backend(move(backend)),
        _adapter(adapter),
        _registryPrx(uncheckedCast<LocatorRegistryPrx>(
                         adapter->add(make_shared<ServerLocatorRegistry>(), Ice::stringToIdentity("registry"))))
    {
    }

    void
    findObjectByIdAsync(Identity id,
                        function<void(const shared_ptr<ObjectPrx>&)> response, function<void(exception_ptr)>,
                        const Current&) const override
    {
        response(_adapter->createProxy(id));
    }

    void
    findAdapterByIdAsync(string,
                         function<void(const shared_ptr<ObjectPrx>&)> response, function<void(exception_ptr)>,
                         const Current&) const override
    {
        response(_adapter->createDirectProxy(stringToIdentity("dummy")));
    }

    shared_ptr<LocatorRegistryPrx>
    getRegistry(const Current&) const override
    {
        return _registryPrx;
    }

private:
    const shared_ptr<Backend> _backend;
    const shared_ptr<ObjectAdapter> _adapter;
    const shared_ptr<LocatorRegistryPrx> _registryPrx;
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
    string endpoints = communicator->getProperties()->getPropertyWithDefault("BackendAdapter.Endpoints",
                                                                             "tcp -p 12010:ssl -p 12011");

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
