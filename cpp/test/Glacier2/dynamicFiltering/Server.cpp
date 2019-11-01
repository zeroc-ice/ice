//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <Ice/Locator.h>
#include <Glacier2/PermissionsVerifier.h>
#include <TestHelper.h>
#include <TestControllerI.h>
#include <SessionI.h>
#include <BackendI.h>

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
    setAdapterDirectProxyAsync(string, shared_ptr<ObjectPrx>, function<void()> response,
                               function<void(exception_ptr)>, const Current&) override
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
    ServerLocatorI(shared_ptr<Backend> backend, shared_ptr<ObjectAdapter> adapter) :
        _backend(move(backend)),
        _adapter(move(adapter))
    {
        _registryPrx = uncheckedCast<LocatorRegistryPrx>(_adapter->add(make_shared<ServerLocatorRegistry>(),
                                                                       Ice::stringToIdentity("registry")));
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
    shared_ptr<Backend> _backend;
    shared_ptr<ObjectAdapter> _adapter;
    shared_ptr<LocatorRegistryPrx> _registryPrx;
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
