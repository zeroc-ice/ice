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
#include <Freeze/Freeze.h>
#include <IcePack/LocatorI.h>
#include <IcePack/LocatorRegistryI.h>
#include <IcePack/ServerManagerI.h>
#include <IcePack/AdapterManagerI.h>
#include <IcePack/AdminI.h>
#include <IcePack/TraceLevels.h>
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

class Server : public Ice::Application
{
public:

    Server()
    {
    }

    void usage();
    virtual int run(int argc, char* argv[]);

private:

    void initInternal(const CommunicatorPtr&);
    void initLocator(const CommunicatorPtr&);
    void initLocatorRegistry(const CommunicatorPtr&);
    void initAdmin(const CommunicatorPtr&, const CommunicatorPtr&);

    AdapterManagerPrx _adapterManager;
    ServerManagerPrx _serverManager;
    ActivatorPtr _activator;
    AdminPrx _admin;
    LocatorPrx _locator;
    LocatorRegistryPrx _locatorRegistry;
    DBEnvironmentPtr _dbEnv;
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

    ::Server app;
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
	"--deploy descriptor [target1 [target2 ...]]\n"
        "                     Deploy application describe by descriptor\n"
        "                     with optional targets.\n"
	;
}

int
::Server::run(int argc, char* argv[])
{
    PropertiesPtr properties = communicator()->getProperties();

    communicator()->setDefaultLocator(0);

    StringSeq args = argsToStringSeq(argc, argv);
    args = properties->parseCommandLineOptions("IcePack", args);
    args = properties->parseCommandLineOptions("Freeze", args);
    stringSeqToArgs(args, argc, argv);

    bool nowarn = false;
    string descriptor;
    vector<string> targets;
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
	else if(strcmp(argv[i], "--deploy") == 0)
	{
	    if(i + 1 >= argc)
	    {
		cerr << appName() << ": missing descriptor argument for option `" << argv[i] << "'" << endl;
		usage();
		return EXIT_FAILURE;
	    }

	    descriptor = argv[++i];
	    
	    while(argv[++i][0] != '-' && i < argc)
	    {
		targets.push_back(argv[i]);
	    }
	}
	else
	{
	    cerr << appName() << ": unknown option `" << argv[i] << "'" << endl;
	    usage();
	    return EXIT_FAILURE;
	}
    }

    //
    // Check that required properties are set and valid.
    //
    if(properties->getProperty("IcePack.Locator.Endpoints").empty())
    {
	cerr << appName() << ": property `IcePack.Locator.Endpoints' is not set" << endl;
	return EXIT_FAILURE;
    }
    if(properties->getProperty("IcePack.LocatorRegistry.Endpoints").empty())
    {
	cerr << appName() << ": property `IcePack.LocatorRegistry.Endpoints' is not set" << endl;
	return EXIT_FAILURE;
    }
    if(properties->getProperty("IcePack.Data").empty())
    {
	cerr << argv[0] << ": property `IcePack.Data' is not set" << endl;
	return EXIT_FAILURE;
    }
    else
    {
	struct stat filestat;
	if(stat(properties->getProperty("IcePack.Data").c_str(), &filestat) != 0)
	{
	    cerr << argv[0] << ": failed to check if property `IcePack.Data' is set to a directory path: "
		 << strerror(getSystemErrno()) << endl;
	    return EXIT_FAILURE;	    
	}
	if(!S_ISDIR(filestat.st_mode))
	{
	    cerr << argv[0] << ": property `IcePack.Data' is not set to a directory path" << endl;
	    return EXIT_FAILURE;
	}
    }
    if(!properties->getProperty("IcePack.Admin.Endpoints").empty())
    {
	if(!nowarn)
	{
	    cerr << appName() << ": warning: administrative endpoints `IcePack.Admin.Endpoints' enabled" << endl;
	}
    }

    //
    // We need another communicator for two reasons:
    //
    // We need a separate thread pool to dispatch the locator registry
    // incoming calls to avoid deadlocks (where a client makes a
    // request on the locator which in turn causes the activation of a
    // server which invoke on the locator registry interface).
    //
    // We need to make collocated calls on the internal objects after
    // shutdown.
    //
    int dummyArgc = 0;
    char **dummyArgv = 0;
    Ice::CommunicatorPtr backendCommunicator = Ice::initialize(dummyArgc, dummyArgv, communicator()->getProperties());

    int status = EXIT_SUCCESS;

    try
    {
	//
	// Initialize internal objects and external interfaces.
	//
	initInternal(backendCommunicator);
	initLocatorRegistry(backendCommunicator);
	initLocator(communicator());
	initAdmin(communicator(), backendCommunicator);

	
	//
	// Deploy application descriptor if a descriptor is passed as
	// a command line option.
	//
	if(!descriptor.empty())
	{
	    try
	    {
		_admin->addApplication(descriptor, targets);
	    }
	    catch(const ServerDeploymentException& ex)
	    {
		cerr << appName() << ": warning: failed to deploy application " << descriptor << ":" << endl;
		cerr << ex << ": " << ex.server << ": " << ex.reason << endl;
	    }
	    catch(const DeploymentException& ex)
	    {
		cerr << appName() << ": warning: failed to deploy application " << descriptor << ":" << endl;
		cerr << ex << ": " << ex.component << ": " << ex.reason << endl;
	    }
	}

	string bundleName = properties->getProperty("IcePack.PrintServersReady");
	if(!bundleName.empty())
	{
	    cout << bundleName << " ready" << endl;
	}

	shutdownOnInterrupt();
	communicator()->waitForShutdown();
	ignoreInterrupt();
    }
    catch(const DBException& ex)
    {
	cerr << appName() << ": " << ex << ": " << ex.message << endl;
	status = EXIT_FAILURE;
    }
    catch(const Exception& ex)
    {
	cerr << appName() << ": " << ex << endl;
	status = EXIT_FAILURE;
    }
    catch(...)
    {
	cerr << appName() << ": unknown exception" << endl;
	status = EXIT_FAILURE;
    }

    try
    {
	//
	// Destroy and join with activator, must be done before shutting
	// down the backend communicator since invocation on collocated
	// objects are done while deactivating the servers.
	//
	if(_activator)
	{
	    _activator->destroy();
	    _activator = 0;
	}

	_adapterManager = 0;
	_serverManager = 0;
	_admin = 0;
	_locator = 0;
	_locatorRegistry = 0;

	//
	// Shutdown the backend communicator. This cause the internal
	// adapter to evict all its objects and save their state to
	// the database. This needs to be done before destroying the
	// database environment.
	//
	backendCommunicator->shutdown();
	backendCommunicator->waitForShutdown();	

	if(_dbEnv)
	{
	    _dbEnv->close();
	    _dbEnv = 0;
	}

	backendCommunicator->destroy();
	backendCommunicator = 0;
    }
    catch(const DBException& ex)
    {
	cerr << appName() << ": " << ex << ": " << ex.message << endl;
	status = EXIT_FAILURE;
    }

    return status;
}

//
// Initialize internal objects: the adapter manager, the server
// manager and the activator.
//
void
::Server::initInternal(const CommunicatorPtr& communicator)
{
    PropertiesPtr properties = communicator->getProperties();

    TraceLevelsPtr traceLevels = new TraceLevels(properties, communicator->getLogger());

    //
    // Creates subdirectories db and servers if they don't already
    // exist.
    //
    string dataPath = properties->getProperty("IcePack.Data");
    if(dataPath[dataPath.length() - 1] != '/')
    {
	dataPath += "/"; 
    }

    string dbPath = dataPath + "db";
    string serversPath = dataPath + "servers";
    
    struct stat filestat;
    if(stat(dbPath.c_str(), &filestat) != 0)
    {
	mkdir(dbPath.c_str(), 0755);
    }
    
    if(stat(serversPath.c_str(), &filestat) != 0)
    {
	mkdir(serversPath.c_str(), 0755);
    }

    _dbEnv = Freeze::initialize(communicator, dbPath);

    ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("IcePackInternalAdapter", "");
    adapter->setLocator(0);

    Ice::ObjectPrx object;

    ActivatorIPtr activator = new ActivatorI(communicator, traceLevels);
    activator->start();
    _activator = activator;

    AdapterManagerPtr adapterManager = new AdapterManagerI(adapter, traceLevels, _dbEnv);
    object = adapter->add(adapterManager, stringToIdentity("IcePack/AdapterManager"));
    _adapterManager = AdapterManagerPrx::uncheckedCast(object);

    ServerManagerPtr serverManager = new ServerManagerI(adapter, traceLevels, _dbEnv, _adapterManager, _activator);
    object = adapter->add(serverManager, stringToIdentity("IcePack/ServerManager"));
    _serverManager = ServerManagerPrx::uncheckedCast(object);

    adapter->activate();
}

void
::Server::initLocator(const CommunicatorPtr& communicator)
{
    assert(_adapterManager && _locatorRegistry);

    PropertiesPtr properties = communicator->getProperties();

    string endpoints = properties->getProperty("IcePack.Locator.Endpoints");
    string id = properties->getPropertyWithDefault("IcePack.Locator.Identity", "IcePack/Locator");

    //
    // Create the "IcePack.Locator" object adapter and register the
    // locator object.
    //
    // The locator object provides an implementation of the
    // Ice::Locator interface. This interface is used by Ice clients
    // to locate object adapters and their associated endpoints.
    //
    ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("IcePackLocatorAdapter", endpoints);
    adapter->setLocator(0);

    LocatorPtr locator = new LocatorI(_adapterManager, _locatorRegistry);
    _locator = LocatorPrx::uncheckedCast(adapter->add(locator, stringToIdentity(id)));
    
    adapter->activate();
}

void
::Server::initLocatorRegistry(const CommunicatorPtr& communicator)
{
    assert(_adapterManager);

    PropertiesPtr properties = communicator->getProperties();

    string endpoints = properties->getProperty("IcePack.LocatorRegistry.Endpoints");
    string id = properties->getPropertyWithDefault("IcePack.LocatorRegistry.Identity", "IcePack/LocatorRegistry");

    //
    // Create the "IcePack.LocatorRegistry" object adapter and
    // register the locator registry object.
    //
    // The locator registry object provides an implementation of the
    // Ice::LocatorRegistry interface. This interface is used by Ice
    // servers to register their object adapters.
    //
    ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("IcePackLocatorRegistryAdapter",
									      endpoints);
    adapter->setLocator(0);
    LocatorRegistryPtr locatorRegistry = new LocatorRegistryI(_adapterManager);
    _locatorRegistry = LocatorRegistryPrx::uncheckedCast(adapter->add(locatorRegistry, stringToIdentity(id)));

    adapter->activate();
}

void
::Server::initAdmin(const CommunicatorPtr& communicator, const CommunicatorPtr& backendCommunicator)
{
    assert(_serverManager && _adapterManager && _locator);

    PropertiesPtr properties = communicator->getProperties();

    //
    // The deployer get the locator proxy from the communicator
    // properties.
    //
    properties->setProperty("Ice.Default.Locator", communicator->proxyToString(_locator));

    backendCommunicator->setDefaultLocator(_locator);

    string endpoints = properties->getProperty("IcePack.Admin.Endpoints");
    string id = properties->getPropertyWithDefault("IcePack.Admin.Identity", "IcePack/Admin");

    ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("IcePackAdminAdapter", endpoints);
    AdminPtr admin = new AdminI(communicator, backendCommunicator, _serverManager, _adapterManager);
    _admin = AdminPrx::uncheckedCast(adapter->add(admin, stringToIdentity(id)));

    _locatorRegistry->addAdapter("IcePackAdminAdapter", adapter->createProxy(stringToIdentity("dummy")));

    adapter->activate();
}
