// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <ServerLocator.h>
#include <TestI.h>

using namespace std;

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    //
    // Register the server manager. The server manager creates a new
    // 'server' (a server isn't a different process, it's just a new
    // communicator and object adapter).
    //
    Ice::PropertiesPtr properties = communicator->getProperties();
    properties->setProperty("ServerManager.Endpoints", "default -p 12345");

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("ServerManager");

    Ice::ObjectPtr object = new ServerManagerI(adapter);
    adapter->add(object, Ice::stringToIdentity("ServerManager"));

    //
    // We also register a sample server locator which implements the
    // locator interface, this locator is used by the clients and the
    // 'servers' created with the server manager interface.
    //
    ServerLocatorRegistryPtr registry = new ServerLocatorRegistry();
    Ice::LocatorRegistryPrx registryPrx = 
	Ice::LocatorRegistryPrx::uncheckedCast(adapter->add(registry, Ice::stringToIdentity("registry")));

    Ice::LocatorPtr locator = new ServerLocator(registry, registryPrx);
    adapter->add(locator, Ice::stringToIdentity("locator"));

    adapter->activate();
    communicator->waitForShutdown();

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
	communicator = Ice::initialize(argc, argv);
	status = run(argc, argv, communicator);
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
