// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>

#ifdef ICE_NO_LOCATOR

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

#else

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
	IceE::PropertiesPtr properties = IceE::getDefaultProperties(argc, argv);
        properties->setProperty("ServerManager.Endpoints", "default -p 12345");
        setCommunicator(IceE::initialize(argc, argv));

        //
        // Register the server manager. The server manager creates a
        // new 'server' (a server isn't a different process, it's just
        // a new communicator and object adapter).
        //
        IceE::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("ServerManager");

        //
        // We also register a sample server locator which implements
        // the locator interface, this locator is used by the clients
        // and the 'servers' created with the server manager
        // interface.
        //
        ServerLocatorRegistryPtr registry = new ServerLocatorRegistry();
        registry->addObject(adapter->createProxy(IceE::stringToIdentity("ServerManager")));
        IceE::ObjectPtr object = new ServerManagerI(adapter, registry);
        adapter->add(object, IceE::stringToIdentity("ServerManager"));

        IceE::LocatorRegistryPrx registryPrx = 
	    IceE::LocatorRegistryPrx::uncheckedCast(adapter->add(registry, IceE::stringToIdentity("registry")));

        IceE::LocatorPtr locator = new ServerLocator(registry, registryPrx);
        adapter->add(locator, IceE::stringToIdentity("locator"));

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

#endif
