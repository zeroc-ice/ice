// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
    	Ice::InitializationData initData;
	initData.properties = Ice::createProperties(argc, argv);
        initData.properties->setProperty("ServerManager.Endpoints", "default -p 12010");

        loadConfig(initData.properties);

	//
	// For blocking client test, set timeout so CloseConnection send will
	// return quickly. Otherwise server will hang since client is not 
	// listening for these messages.
	//
	if(initData.properties->getPropertyAsInt("Ice.Blocking") > 0)
	{
	    initData.properties->setProperty("Ice.Override.Timeout", "100");
	    initData.properties->setProperty("Ice.Warn.Connections", "0");
	}

	//
	// These properties cannot be overridden. The OAs started by
	// the ServerManager must be local.
	//
	initData.properties->setProperty("TestAdapter.Endpoints", "default");
	initData.properties->setProperty("TestAdapter.AdapterId", "TestAdapter");
	initData.properties->setProperty("TestAdapter.ReplicaGroupId", "ReplicatedAdapter");
	initData.properties->setProperty("TestAdapter2.Endpoints", "default");
	initData.properties->setProperty("TestAdapter2.AdapterId", "TestAdapter2");

	initData.logger = getLogger();
        setCommunicator(Ice::initialize(argc, argv, initData));

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
        registry->addObject(adapter->createProxy(communicator()->stringToIdentity("ServerManager")));
        Ice::ObjectPtr object = new ServerManagerI(adapter, registry, initData);
        adapter->add(object, communicator()->stringToIdentity("ServerManager"));

        Ice::LocatorRegistryPrx registryPrx = 
	    Ice::LocatorRegistryPrx::uncheckedCast(adapter->add(registry, communicator()->stringToIdentity("registry")));

        Ice::LocatorPtr locator = new ServerLocator(registry, registryPrx);
        adapter->add(locator, communicator()->stringToIdentity("locator"));

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

#ifdef _WIN32_WCE

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    tprintf("Adapter Ready\n");

    return 0;
}

#else

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

#endif
