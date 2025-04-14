// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "ServerLocator.h"
#include "TestHelper.h"
#include "TestI.h"

using namespace std;

class Server : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Server::run(int argc, char** argv)
{
    Ice::InitializationData initData;
    initData.properties = createTestProperties(argc, argv);
    installTransport(initData);

    Ice::CommunicatorHolder communicator = initialize(argc, argv, initData);
    //
    // Register the server manager. The server manager creates a new
    // 'server' (a server isn't a different process, it's just a new
    // communicator and object adapter).
    //
    Ice::PropertiesPtr properties = communicator->getProperties();
    properties->setProperty("Ice.ThreadPool.Server.Size", "2");
    properties->setProperty("ServerManager.Endpoints", getTestEndpoint());

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("ServerManager");

    //
    // We also register a sample server locator which implements the
    // locator interface, this locator is used by the clients and the
    // 'servers' created with the server manager interface.
    //
    ServerLocatorRegistryPtr registry = make_shared<ServerLocatorRegistry>();
    registry->addObject(adapter->createProxy(Ice::stringToIdentity("ServerManager")));

    Ice::ObjectPtr object = make_shared<ServerManagerI>(registry, initData);
    adapter->add(object, Ice::stringToIdentity("ServerManager"));

    auto registryPrx = adapter->add<Ice::LocatorRegistryPrx>(registry, Ice::stringToIdentity("registry"));

    auto locator = make_shared<ServerLocator>(registry, registryPrx);
    adapter->add(locator, Ice::stringToIdentity("locator"));

    adapter->activate();
    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
