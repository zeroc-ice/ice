// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <ServerLocator.h>
#include <TestHelper.h>
#include <TestI.h>

using namespace std;

class Server : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Server::run(int argc, char** argv)
{
    Ice::InitializationData initData;
    initData.properties = createTestProperties(argc, argv);

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
    ServerLocatorRegistryPtr registry = ICE_MAKE_SHARED(ServerLocatorRegistry);
    registry->addObject(adapter->createProxy(Ice::stringToIdentity("ServerManager")));
    Ice::ObjectPtr object = ICE_MAKE_SHARED(ServerManagerI, registry, initData);
    adapter->add(object, Ice::stringToIdentity("ServerManager"));

    Ice::LocatorRegistryPrxPtr registryPrx =
        ICE_UNCHECKED_CAST(Ice::LocatorRegistryPrx,
                           adapter->add(registry, Ice::stringToIdentity("registry")));

    Ice::LocatorPtr locator = ICE_MAKE_SHARED(ServerLocator, registry, registryPrx);
    adapter->add(locator, Ice::stringToIdentity("locator"));

    adapter->activate();
    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
