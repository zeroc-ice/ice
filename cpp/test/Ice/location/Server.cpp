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
#include <TestCommon.h>
#include <TestI.h>

DEFINE_TEST("server")

using namespace std;

int
run(int, char**, const Ice::CommunicatorPtr& communicator,
    const Ice::InitializationData& initData)
{
    //
    // Register the server manager. The server manager creates a new
    // 'server' (a server isn't a different process, it's just a new
    // communicator and object adapter).
    //
    Ice::PropertiesPtr properties = communicator->getProperties();
    properties->setProperty("Ice.ThreadPool.Server.Size", "2");
    properties->setProperty("ServerManager.Endpoints", "default -p 12010:udp");

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("ServerManager");

    //
    // We also register a sample server locator which implements the
    // locator interface, this locator is used by the clients and the
    // 'servers' created with the server manager interface.
    //
    ServerLocatorRegistryPtr registry = new ServerLocatorRegistry();
    registry->addObject(adapter->createProxy(communicator->stringToIdentity("ServerManager")));
    Ice::ObjectPtr object = new ServerManagerI(registry, initData);
    adapter->add(object, communicator->stringToIdentity("ServerManager"));

    Ice::LocatorRegistryPrx registryPrx =
        Ice::LocatorRegistryPrx::uncheckedCast(adapter->add(registry, communicator->stringToIdentity("registry")));

    Ice::LocatorPtr locator = new ServerLocator(registry, registryPrx);
    adapter->add(locator, communicator->stringToIdentity("locator"));

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

    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties(argc, argv);
        communicator = Ice::initialize(argc, argv, initData);
        assert(initData.properties != communicator->getProperties());

        status = run(argc, argv, communicator, initData);
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        status = EXIT_FAILURE;
    }

    if(communicator)
    {
        try
        {
            communicator->destroy();
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            status = EXIT_FAILURE;
        }
    }

    return status;
}
