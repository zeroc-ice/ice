// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Application.h>
#include <IcePack/LocatorI.h>
#include <IcePack/LocatorRegistryI.h>
#include <IcePack/ServerManagerI.h>
#include <IcePack/AdapterManagerI.h>
#include <IcePack/AdminI.h>
#ifndef _WIN32
#   include <IcePack/ActivatorI.h>
#   include <signal.h>
#   include <sys/wait.h>
#endif

using namespace std;
using namespace Ice;
using namespace IcePack;

class Server : public Application
{
public:

    void usage();
    virtual int run(int, char*[]);
};

#ifndef _WIN32
static void
childHandler(int)
{
    wait(0);
}
#endif

int
main(int argc, char* argv[])
{
#ifndef _WIN32
    //
    // This application forks, so we need a signal handler for child
    // termination.
    //
    struct sigaction action;
    action.sa_handler = childHandler;
    sigemptyset(&action.sa_mask);
    sigaddset(&action.sa_mask, SIGCHLD);
    action.sa_flags = 0;
    sigaction(SIGCHLD, &action, 0);
#endif

    ::Server app;    
    return app.main(argc, argv);
}

void
::Server::usage()
{
    cerr << "Usage: " << appName() << " [options]\n";
    cerr <<	
	"Options:\n"
	"-h, --help           Show this message.\n"
	"-v, --version        Display the Ice version.\n"
	"--nowarn             Don't print any security warnings.\n"
	;
}

int
::Server::run(int argc, char* argv[])
{
    PropertiesPtr properties = communicator()->getProperties();

    StringSeq args = argsToStringSeq(argc, argv);
    args = properties->parseCommandLineOptions("IcePack", args);
    stringSeqToArgs(args, argc, argv);

    bool nowarn = false;
    for(int i = 1; i < argc; ++i)
    {
	if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
	{
	    usage();
	    return EXIT_SUCCESS;
	}
	else if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
	{
	    cout << ICE_STRING_VERSION << endl;
	    return EXIT_SUCCESS;
	}
	else if(strcmp(argv[i], "--nowarn") == 0)
	{
	    nowarn = true;
	}
	else
	{
	    cerr << appName() << ": unknown option `" << argv[i] << "'" << endl;
	    usage();
	    return EXIT_FAILURE;
	}
    }

    string locatorEndpoints = properties->getProperty("IcePack.Locator.Endpoints");
    if(locatorEndpoints.empty())
    {
	cerr << appName() << ": property `IcePack.Locator.Endpoints' is not set" << endl;
	return EXIT_FAILURE;
    }

    string locatorRegistryEndpoints = properties->getProperty("IcePack.LocatorRegistry.Endpoints");
    if(locatorRegistryEndpoints.empty())
    {
	cerr << appName() << ": property `IcePack.LocatorRegistry.Endpoints' is not set" << endl;
	return EXIT_FAILURE;
    }

    string adminEndpoints = properties->getProperty("IcePack.Admin.Endpoints");
    if(!adminEndpoints.empty())
    {
	if(!nowarn)
	{
	    cerr << appName() << ": warning: administrative endpoints `IcePack.Admin.Endpoints' enabled" << endl;
	}
    }

    string locatorId = properties->getPropertyWithDefault("IcePack.Locator.Identity", "IcePack/locator");
    string locatorRegistryId = properties->getPropertyWithDefault("IcePack.LocatorRegistry.Identity", 
								  "IcePack/locatorregistry");
    string adminId = properties->getPropertyWithDefault("IcePack.Admin.Identity", "IcePack/admin");
    
    //
    // Register the server manager and adapter manager with an
    // internal object adapter. We ensure that the internal object
    // adapter doesn't have any endpoints, all the objects registered
    // with this adapter are *only* accessed internally through
    // collocation.
    //
    ObjectAdapterPtr internalAdapter = communicator()->createObjectAdapterWithEndpoints("IcePack.Internal", "");
    internalAdapter->setLocator(0);

    //
    // Activator isn't supported on Windows yet, just pass an empty
    // acticator proxy.
    //
    ActivatorPrx activatorProxy;

#ifndef _WIN32
    //
    // Setup default arguments which will be passed to each activated
    // process. Then, create and start the activator.
    //
    Args defaultArgs;
    defaultArgs.push_back("--Ice.Default.Locator=" + locatorId + ":" + locatorEndpoints);

    ActivatorIPtr activator = new ActivatorI(communicator(), defaultArgs);
    activator->start();
    activatorProxy = ActivatorPrx::uncheckedCast(internalAdapter->add(activator, 
								      stringToIdentity("IcePack/activator")));
#endif

    AdapterManagerPtr adapterManager = new AdapterManagerI(internalAdapter);
    AdapterManagerPrx adapterManagerProxy = 
	AdapterManagerPrx::uncheckedCast(internalAdapter->add(adapterManager, 
							      stringToIdentity("IcePack/adaptermanager")));

    ServerManagerPtr serverManager = new ServerManagerI(internalAdapter, adapterManagerProxy, activatorProxy);
    ServerManagerPrx serverManagerProxy = 
	ServerManagerPrx::uncheckedCast(internalAdapter->add(serverManager, 
							     stringToIdentity("IcePack/servermanager")));
    internalAdapter->activate();

    //
    // Create the "IcePack.Admin" object adapter and register the
    // admin object. The admin object is used by icepackadmin to
    // administrate IcePack.
    //
    ObjectAdapterPtr adminAdapter = communicator()->createObjectAdapterWithEndpoints("IcePack.Admin", adminEndpoints);
    AdminPtr admin = new AdminI(communicator(), serverManagerProxy, adapterManagerProxy);
    adminAdapter->add(admin, stringToIdentity(adminId));

    //
    // Create the "IcePack.LocatorRegistry" object adapter and
    // registry the locator registry object.
    //
    // The locator registry object provides an implementation of the
    // Ice::LocatorRegistry interface. This interface is used by Ice
    // servers to register their object adapters.
    //
    ObjectAdapterPtr locatorRegistryAdapter = 
	communicator()->createObjectAdapterWithEndpoints("IcePack.LocatorRegistry", locatorRegistryEndpoints);
    locatorRegistryAdapter->setLocator(0);
    LocatorRegistryPtr locatorRegistry = new LocatorRegistryI(adapterManagerProxy);
    LocatorRegistryPrx locatorRegistryProxy =
	LocatorRegistryPrx::uncheckedCast(locatorRegistryAdapter->add(locatorRegistry, stringToIdentity(adminId)));

    //
    // Create the "IcePack.Locator" object adapter and register the
    // locator object.
    //
    // The locator locator object provides an implementation of the
    // Ice::Locator interface. This interface is used by Ice clients
    // to locate object adapters and their associated endpoints.
    //
    LocatorPtr locator = new LocatorI(adapterManagerProxy, locatorRegistryProxy);
    ObjectAdapterPtr locatorAdapter = communicator()->createObjectAdapterWithEndpoints("IcePack.Locator", 
										       locatorEndpoints);
    locatorAdapter->setLocator(0);
    LocatorPrx locatorProxy = LocatorPrx::uncheckedCast(locatorAdapter->add(locator, stringToIdentity(locatorId)));

    //
    // Set the locator for the admin object adapter.
    //
    adminAdapter->setLocator(locatorProxy);
    
    //
    // Activate the adapters.
    //
    shutdownOnInterrupt();
    adminAdapter->activate();
    locatorAdapter->activate();
    locatorRegistryAdapter->activate();
    communicator()->waitForShutdown();
    ignoreInterrupt();

#ifndef _WIN32
    //
    // Destroy and join with activator.
    //
    activator->destroy();
    activator->getThreadControl().join();
#endif

    return EXIT_SUCCESS;
}
