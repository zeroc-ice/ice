// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>

#ifdef ICEE_HAS_LOCATOR

#include <ServerLocator.h>
#include <TestApplication.h>
#include <TestI.h>

using namespace std;

class LocationTestApplication : public TestApplication
{
public:

    LocationTestApplication() :
        TestApplication("location server")
    {
    }

    virtual int
    run(int argc, char* argv[])
    {
	Ice::PropertiesPtr properties = Ice::createProperties();
        properties->setProperty("ServerManager.Endpoints", "default -p 12345");

        loadConfig(properties);

	//
	// These properties cannot be overridden. The OAs started by
	// the ServerManager must be local.
	//
	properties->setProperty("TestAdapter.Endpoints", "default");
	properties->setProperty("TestAdapter.AdapterId", "TestAdapter");
	properties->setProperty("TestAdapter2.Endpoints", "default");
	properties->setProperty("TestAdapter2.AdapterId", "TestAdapter2");

        setCommunicator(Ice::initializeWithProperties(argc, argv, properties));

        //
        // Register the server manager. The server manager creates a
        // new 'server' (a server isn't a different process, it's just
        // a new communicator and object adapter).
        //
        Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("ServerManager");

        //
        // We also register a sample server locator which implements
        // the locator interface, this locator is used by the clients
        // and the 'servers' created with the server manager
        // interface.
        //
        ServerLocatorRegistryPtr registry = new ServerLocatorRegistry();
        registry->addObject(adapter->createProxy(Ice::stringToIdentity("ServerManager")));
        Ice::ObjectPtr object = new ServerManagerI(adapter, registry, properties);
        adapter->add(object, Ice::stringToIdentity("ServerManager"));

        Ice::LocatorRegistryPrx registryPrx = 
	    Ice::LocatorRegistryPrx::uncheckedCast(adapter->add(registry, Ice::stringToIdentity("registry")));

        Ice::LocatorPtr locator = new ServerLocator(registry, registryPrx);
        adapter->add(locator, Ice::stringToIdentity("locator"));

        adapter->activate();

#ifndef _WIN32_WCE
	communicator()->waitForShutdown();
#endif

        return EXIT_SUCCESS;
    }
};

#ifdef _WIN32_WCE

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    LocationTestApplication app;
    return app.main(hInstance);
}

#else

int
main(int argc, char** argv)
{
    LocationTestApplication app;
    return app.main(argc, argv);
}

#endif

#else

#include <TestCommon.h>

int
main(int argc, char* argv[])
{
#ifdef _WIN32
    tprintf("%d\n", _getpid());
#else
    tprintf("%d\n", getpid());
#endif
    tprintf("Adapter Ready\n");

    return 0;
}

#endif
