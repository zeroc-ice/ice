// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <IceUtil/UUID.h>
#include <IceUtil/CtrlCHandler.h>
#include <Ice/Application.h>
#include <Freeze/Freeze.h>
#include <IcePack/ActivatorI.h>
#include <IcePack/ServerFactory.h>
#include <IcePack/AdapterFactory.h>
#include <IcePack/ServerDeployerI.h>
#include <IcePack/AdapterI.h>
#include <IcePack/NodeI.h>
#include <IcePack/NodeInfo.h>
#include <IcePack/TraceLevels.h>
#include <IcePack/Registry.h>

#ifdef _WIN32
#   include <direct.h>
#   include <sys/types.h>
#   include <sys/stat.h>
#   define S_ISDIR(mode) ((mode) & _S_IFDIR)
#   define S_ISREG(mode) ((mode) & _S_IFREG)
#else
#   include <csignal>
#   include <signal.h>
#   include <sys/wait.h>
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <unistd.h>
#endif

using namespace std;
using namespace IcePack;

//
// The activator needs to be global since it's used by the interupt
// signal handler to shut it down. We can't use the mechanism provided
// by Ice or Freeze Application because we need to perform some
// shutdown tasks that require to do collocated invocations.
//
ActivatorPtr activator;

void
usage(const char* appName)
{
    cerr << "Usage: " << appName << " [options]\n";
    cerr <<	
	"Options:\n"
	"-h, --help           Show this message.\n"
	"-v, --version        Display the Ice version.\n"
	"--deploy descriptor [target1 [target2 ...]]\n"
	"--nowarn             Don't print any security warnings.\n"
	;
}

#ifndef _WIN32
extern "C"
{

static void
childHandler(int)
{
    //
    // Call wait to de-allocate any resources allocated for the child
    // process and avoid zombie processes. See man wait or waitpid for
    // more information.
    //
    wait(0);
}

}
#endif

static IceUtil::CtrlCHandler* _ctrlCHandler = 0;

static void
shutdownCallback(int)
{
    assert(activator);
    activator->shutdown();
}


int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator, const string& envName)
{
    Ice::PropertiesPtr properties = communicator->getProperties();

    bool nowarn = false;
    string descriptor;
    vector<string> targets;
    for(int i = 1; i < argc; ++i)
    {
	if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
	{
	    usage(argv[0]);
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
		Ice::Error out(communicator->getLogger());
		out << "missing descriptor argument for option `" << argv[i] << "'";
		usage(argv[0]);
		return EXIT_FAILURE;
	    }

	    descriptor = argv[++i];
	    
	    while(i + 1 < argc && argv[++i][0] != '-')
	    {
		targets.push_back(argv[i]);
	    }
	}
    }

    //
    // Check that required properties are set and valid.
    //
    if(properties->getProperty("IcePack.Node.Endpoints").empty())
    {
	Ice::Error out(communicator->getLogger());
	out << "property `IcePack.Node.Endpoints' is not set";
	return EXIT_FAILURE;
    }

    string name = properties->getProperty("IcePack.Node.Name");
    if(name.empty())
    {
	char host[1024 + 1];
	if(gethostname(host, 1024) != 0)
	{
	    Ice::Error out(communicator->getLogger());
	    out << "property `IcePack.Node.Name' is not set and couldn't get the hostname: "
		<< strerror(getSystemErrno());
	    return EXIT_FAILURE;
	}
	else if(!nowarn)
	{
	    Ice::Warning out(communicator->getLogger());
	    out << "property `IcePack.Node.Name' is not set, using hostname: " << host ;
	}
    }

    //
    // Set the adapter id for this node and create the node object
    // adapter.
    //
    properties->setProperty("IcePack.Node.AdapterId", "IcePack.Node-" + name);

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("IcePack.Node");
	
    TraceLevelsPtr traceLevels = new TraceLevels(properties, communicator->getLogger());
	
    //
    // Create the activator.
    //
    activator = new ActivatorI(traceLevels, properties);

    //
    // Create the wait queue.
    //
    WaitQueuePtr waitQueue = new WaitQueue();
    waitQueue->start();

    //
    // Creates the server factory. The server factory creates server
    // and server adapter persistent objects. It also takes care of
    // installing the evictors and object factory necessary to persist
    // and create these objects.
    //
    ServerFactoryPtr serverFactory = new ServerFactory(adapter, traceLevels, envName, activator, waitQueue);
    
    //
    // Create the node object and the node info. Because of circular
    // dependencies on the node info we need to create the proxy for
    // the server registry and deployer now.
    //
    Ice::Identity deployerId;
    deployerId.name = IceUtil::generateUUID();

    NodePtr node = new NodeI(activator, name, ServerDeployerPrx::uncheckedCast(adapter->createProxy(deployerId)));
    NodePrx nodeProxy = NodePrx::uncheckedCast(adapter->addWithUUID(node));
	
    NodeInfoPtr nodeInfo = new NodeInfo(communicator, serverFactory, node, traceLevels);

    //
    // Create the server deployer.
    //
    ServerDeployerPtr deployer = new ServerDeployerI(nodeInfo);
    adapter->add(deployer, deployerId);

    //
    // Register this node with the node registry.
    //
    try
    {
	NodeRegistryPrx nodeRegistry = NodeRegistryPrx::checkedCast(
	    communicator->stringToProxy("IcePack/NodeRegistry@IcePack.Registry.Internal"));
	nodeRegistry->add(name, nodeProxy);
    }
    catch(const NodeActiveException&)
    {
	Ice::Error out(communicator->getLogger());
	out << "a node with the same name is already registered and active";
	throw;
    }
    catch(const Ice::LocalException&)
    {
	Ice::Error out(communicator->getLogger());
	out << "couldn't contact the IcePack registry:";
	throw;
    }

    //
    // Start the activator.
    //
    activator->start();
    
    //
    // We are ready to go! Activate the object adapter.
    //
    adapter->activate();

    //
    // Deploy application descriptor if a descriptor is passed as
    // a command line option.
    //
    if(!descriptor.empty())
    {
	AdminPrx admin;
	try
	{
	    admin = AdminPrx::checkedCast(communicator->stringToProxy("IcePack/Admin"));
	}
	catch(const Ice::LocalException& ex)
	{
	    Ice::Warning out(communicator->getLogger());
	    out << "couldn't contact IcePack admin interface to deploy application `" << descriptor << "':";
	    out << ex;
	}

	if(admin)
	{
	    try
	    {
		admin->addApplication(descriptor, targets);
	    }
	    catch(const ServerDeploymentException& ex)
	    {
		Ice::Warning out(communicator->getLogger());
		out << "failed to deploy application `" << descriptor << "':\n";
		out << ex << ": " << ex.server << ": " << ex.reason;
	    }
	    catch(const DeploymentException& ex)
	    {
		Ice::Warning out(communicator->getLogger());
		out << "failed to deploy application `" << descriptor << "':\n";
		out << ex << ": " << ex.component << ": " << ex.reason;
	    }
	    catch(const Ice::LocalException& ex)
	    {
		Ice::Warning out(communicator->getLogger());
		out << "failed to deploy application `" << descriptor << "':\n";
		out << ex;
	    }
	}
    }

    string bundleName = properties->getProperty("IcePack.Node.PrintServersReady");
    if(!bundleName.empty())
    {
	cout << bundleName << " ready" << endl;
    }
    
    //
    // Wait for the activator shutdown. Once the run method returns
    // all the servers have been deactivated.
    //
    _ctrlCHandler->setCallback(shutdownCallback);
    activator->waitForShutdown();
    _ctrlCHandler->setCallback(0);

    activator->destroy();

    //
    // The wait queue must be destroyed after the activator and before
    // the communicator is shutdown.
    //
    waitQueue->destroy();

    //
    // We can now safelly shutdown the communicator.
    //
    communicator->shutdown();
    communicator->waitForShutdown();

    activator = 0;

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
#ifndef _WIN32
    //
    // This application forks, so we need a signal handler for child termination.
    //
    struct sigaction action;
    action.sa_handler = childHandler;
    sigemptyset(&action.sa_mask);
    sigaddset(&action.sa_mask, SIGCHLD);
    action.sa_flags = 0;
    sigaction(SIGCHLD, &action, 0);
#endif

    Ice::CommunicatorPtr communicator;

    int status;
    try
    {
	//
	// Initialize the communicator.
	//
	communicator = Ice::initialize(argc, argv);

	//
	// Handles CTRL+C like signals. Initially, just ignore them
	//
	IceUtil::CtrlCHandler ctrlCHandler;
	_ctrlCHandler = &ctrlCHandler;
	
	Ice::PropertiesPtr properties = communicator->getProperties();

	//
	// Disable server idle time. Otherwise, the adapter would be
	// shutdown prematurely and the deactivation would fail.
        // Deactivation of the node relies on the object adapter
	// to be active since it needs to terminate servers.
	//
	// TODO: implement Ice.ServerIdleTime in the activator
	// termination listener instead?
	//
	properties->setProperty("Ice.ServerIdleTime", "0");

	bool nowarn = false;
	for(int i = 1; i < argc; ++i)
	{
	    if(strcmp(argv[i], "--nowarn") == 0)
	    {
		nowarn = true;
		break;
	    }
	}

	//
	// Collocate the IcePack registry if we need to.
	//
	auto_ptr<Registry> registry;
	if(properties->getPropertyAsInt("IcePack.Node.CollocateRegistry") > 0)
	{
	    registry = auto_ptr<Registry>(new Registry(communicator));
	    if(!registry->start(nowarn, true))
	    {
		return EXIT_FAILURE;
	    }
	    
	    //
	    // The node needs a different thread pool to avoid
	    // deadlocks in connection validation.
	    //
	    if(properties->getPropertyAsInt("IcePack.Node.ThreadPool.Size") == 0)
	    {
		int size = properties->getPropertyAsIntWithDefault("Ice.ThreadPool.Server.Size", 10);

		ostringstream os1;
		os1 << static_cast<int>(size / 3);
		properties->setProperty("IcePack.Node.ThreadPool.Size", os1.str());

		ostringstream os2;
		os2 << size - static_cast<int>(size / 3);
		properties->setProperty("Ice.ThreadPool.Server.Size", os2.str());
	    }

	    //
	    // Set the Ice.Default.Locator property to point to the
	    // collocated locator (this property is passed by the
	    // activator to each activated server).
	    //
	    string locatorPrx = "IcePack/Locator:" + properties->getProperty("IcePack.Registry.Client.Endpoints");
	    properties->setProperty("Ice.Default.Locator", locatorPrx);
	}
	else if(properties->getProperty("Ice.Default.Locator").empty())
	{
	    cerr << argv[0] << ": property `Ice.Default.Locator' is not set" << endl;
	    return EXIT_FAILURE;
	}

	//
	// Initialize the database environment (first setup the
	// directory structure if needed).
	//
	string envName;
	string dataPath = properties->getProperty("IcePack.Node.Data");
	if(dataPath.empty())
	{
	    cerr << argv[0] << ": property `IcePack.Node.Data' is not set" << endl;
	    return EXIT_FAILURE;
	}
	else
	{
#ifdef _WIN32
	    struct _stat filestat;
	    if(::_stat(dataPath.c_str(), &filestat) != 0 || !S_ISDIR(filestat.st_mode))
	    {
		cerr << argv[0] << ": property `IcePack.Node.Data' is not set to a valid directory path" << endl;
		return EXIT_FAILURE;
	    }	    
#else
	    struct stat filestat;
	    if(::stat(dataPath.c_str(), &filestat) != 0 || !S_ISDIR(filestat.st_mode))
	    {
		cerr << argv[0] << ": property `IcePack.Node.Data' is not set to a valid directory path" << endl;
		return EXIT_FAILURE;
	    }	    
#endif

	    //
	    // Creates subdirectories db and servers in the IcePack.Node.Data
	    // directory if they don't already exist.
	    //
	    if(dataPath[dataPath.length() - 1] != '/')
	    {
		dataPath += "/"; 
	    }
	    
	    envName = dataPath + "db";
	    string serversPath = dataPath + "servers";
    
#ifdef _WIN32
	    if(::_stat(envName.c_str(), &filestat) != 0)
	    {
		_mkdir(envName.c_str());
	    }
	    if(::_stat(serversPath.c_str(), &filestat) != 0)
	    {
		_mkdir(serversPath.c_str());
	    }
#else
	    if(::stat(envName.c_str(), &filestat) != 0)
	    {
		mkdir(envName.c_str(), 0755);
	    }
	    if(::stat(serversPath.c_str(), &filestat) != 0)
	    {
		mkdir(serversPath.c_str(), 0755);
	    }
#endif
	}

	status = run(argc, argv, communicator, envName);
    }
    catch(const Ice::Exception& ex)
    {
	cerr << argv[0] << ": " << ex << endl;
	status = EXIT_FAILURE;
    }
    catch(...)
    {
	cerr << argv[0] << ": unknown exception" << endl;
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
	    cerr << argv[0] << ": " << ex << endl;
	    status = EXIT_FAILURE;
	}
	catch(...)
	{
	    cerr << argv[0] << ": unknown exception" << endl;
	    status = EXIT_FAILURE;
	}
	communicator = 0;
    }

    return status;
}
