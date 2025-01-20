// Copyright (c) ZeroC, Inc.

#include "BackendI.h"
#include "Ice/Ice.h"
#include "Ice/Locator.h"
#include "SessionI.h"
#include "TestControllerI.h"
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
    shared_ptr<Backend> _backend;
    ObjectAdapterPtr _adapter;
    optional<LocatorRegistryPrx> _registryPrx;
};

class SessionControlServer final : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
SessionControlServer::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    //
    // The server requires 3 separate server endpoints. One for the test
    // controller that will coordinate the tests and the required
    // configurations across the client and the router, an adapter for
    // the session manager for the router to communicate with and
    // finally, an adapter for the dummy backend server that the client
    // will ultimately attempt to make calls on. The backend uses a
    // servant locator that responds to each lookup with the same
    // servant, allowing us to use any reference as long as the client
    // expects to use a proxy for the correct type of object.
    //
    communicator->getProperties()->setProperty("TestControllerAdapter.Endpoints", getTestEndpoint(2, "tcp"));
    auto controllerAdapter = communicator->createObjectAdapter("TestControllerAdapter");
    auto controller = make_shared<TestControllerI>(getTestEndpoint(1));
    controllerAdapter->add(controller, Ice::stringToIdentity("testController"));
    controllerAdapter->activate();

    communicator->getProperties()->setProperty("SessionControlAdapter.Endpoints", getTestEndpoint());
    auto adapter = communicator->createObjectAdapter("SessionControlAdapter");
    adapter->add(make_shared<SessionManagerI>(controller), Ice::stringToIdentity("SessionManager"));
    adapter->activate();

    auto backend = make_shared<BackendI>();
    communicator->getProperties()->setProperty("BackendAdapter.Endpoints", getTestEndpoint(1));
    auto backendAdapter = communicator->createObjectAdapter("BackendAdapter");
    backendAdapter->addDefaultServant(backend, "");
    backendAdapter->activate();

    auto locator = make_shared<ServerLocatorI>(backend, backendAdapter);
    backendAdapter->add(locator, Ice::stringToIdentity("locator"));

    communicator->waitForShutdown();
}

DEFINE_TEST(SessionControlServer)
