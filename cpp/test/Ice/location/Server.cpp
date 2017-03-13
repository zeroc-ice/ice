// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <ServerLocator.h>
#include <TestCommon.h>
#include <TestI.h>

DEFINE_TEST("server")

using namespace std;

int
run(int, char**, const Ice::CommunicatorPtr& communicator, const Ice::InitializationData& initData)
{
    //
    // Register the server manager. The server manager creates a new
    // 'server' (a server isn't a different process, it's just a new
    // communicator and object adapter).
    //
    Ice::PropertiesPtr properties = communicator->getProperties();
    properties->setProperty("Ice.ThreadPool.Server.Size", "2");
    properties->setProperty("ServerManager.Endpoints", getTestEndpoint(communicator, 0) + ":udp");

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
    TEST_READY
    communicator->waitForShutdown();

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL();
#endif
    try
    {
        Ice::InitializationData initData = getTestInitData(argc, argv);
        Ice::CommunicatorHolder ich(argc, argv, initData);
        assert(initData.properties != ich->getProperties());
        return run(argc, argv, ich.communicator(), initData);
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        return EXIT_FAILURE;
    }
}
