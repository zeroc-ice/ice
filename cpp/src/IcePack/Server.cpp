// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Freeze/Application.h>
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
#include <util/PlatformUtils.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;
using namespace Ice;
using namespace Freeze;
using namespace IcePack;

class Server : public Freeze::Application
{
public:

    Server(const string& dbEnvName) :
	Freeze::Application(dbEnvName)
    {
    }

    void usage();
    virtual int runFreeze(int argc, char* argv[], const DBEnvironmentPtr&);
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

    try
    {
	XMLPlatformUtils::Initialize();
    }
    catch(const XMLException& e)
    {
	cout << e.getMessage() << endl;
	return EXIT_FAILURE;
    }

    PropertiesPtr defaultProperties;
    try
    {
	defaultProperties = getDefaultProperties(argc, argv);
	StringSeq args = argsToStringSeq(argc, argv);
	args = defaultProperties->parseCommandLineOptions("IcePack", args);
	stringSeqToArgs(args, argc, argv);
    }
    catch(const SystemException& ex)
    {
	cerr << argv[0] << ": " << ex << endl;
	return EXIT_FAILURE;
    }

    //
    // Check that IcePack.Data property is set and creates
    // subdirectories db and servers if they don't already exist.
    //
    string dataPath = defaultProperties->getProperty("IcePack.Data");
    if(dataPath.empty())
    {
	cerr << argv[0] << ": property `IcePack.Data' is not set" << endl;
	return EXIT_FAILURE;
    }
    if(dataPath[dataPath.length() - 1] != '/')
    {
	dataPath += "/"; 
    }

    string dbPath = dataPath + "db";
    string serversPath = dataPath + "servers";
    
    struct stat filestat;
    if(stat(dataPath.c_str(), &filestat) == 0 && S_ISDIR(filestat.st_mode))
    {
	if(stat(dbPath.c_str(), &filestat) != 0)
	{
	    mkdir(dbPath.c_str(), 0755);
	}

	if(stat(serversPath.c_str(), &filestat) != 0)
	{
	    mkdir(serversPath.c_str(), 0755);
	}
    }
    else
    {
	cerr << argv[0] << ": IcePack.Data doesn't contain a valid directory path." << endl;
	return EXIT_FAILURE;
    }

    ::Server app(dbPath);
    int rc = app.main(argc, argv);

    XMLPlatformUtils::Terminate();

    return rc;
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
::Server::runFreeze(int argc, char* argv[], const DBEnvironmentPtr& dbEnv)
{
    PropertiesPtr properties = communicator()->getProperties();

    StringSeq args = argsToStringSeq(argc, argv);
    args = properties->parseCommandLineOptions("IcePack", args);
    args = properties->parseCommandLineOptions("Freeze", args);
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

    communicator()->getProperties()->setProperty("Ice.Default.Locator", locatorId + ":" + locatorEndpoints);

    //
    // An internal object adapter for internal objects which are not
    // exposed to the outside world. They might be at one point.
    //
    ObjectAdapterPtr internalAdapter = communicator()->createObjectAdapterWithEndpoints("IcePack.Internal", "");
    internalAdapter->setLocator(0);

    //
    // Creates and register the adapter manager.
    //
    AdapterManagerPtr adapterManager = new AdapterManagerI(internalAdapter, dbEnv);
    AdapterManagerPrx adapterManagerProxy = 
	AdapterManagerPrx::uncheckedCast(internalAdapter->add(adapterManager, 
							      stringToIdentity("IcePack/adaptermanager")));

    //
    // Activator and server manager are not supported on Windows yet.
    //
    ServerManagerPrx serverManagerProxy;

#ifndef _WIN32

    ActivatorIPtr activator = new ActivatorI(communicator());
    activator->start();
    ActivatorPrx  activatorProxy = ActivatorPrx::uncheckedCast(internalAdapter->add(activator, 
								      stringToIdentity("IcePack/activator")));

    ServerManagerPtr serverManager = new ServerManagerI(internalAdapter, dbEnv, adapterManagerProxy, activatorProxy);
    serverManagerProxy = 
	ServerManagerPrx::uncheckedCast(internalAdapter->add(serverManager, 
							     stringToIdentity("IcePack/servermanager")));
    internalAdapter->activate();

#endif

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
	LocatorRegistryPrx::uncheckedCast(locatorRegistryAdapter->add(locatorRegistry, 
								      stringToIdentity(locatorRegistryId)));

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
