// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/UUID.h>
#include <Ice/Ice.h>
#include <Ice/ProtocolPluginFacade.h> // Just to get the hostname
#include <Ice/Locator.h>
#include <Ice/Service.h>
#include <IceGrid/Activator.h>
#include <IceGrid/WaitQueue.h>
#include <IceGrid/RegistryI.h>
#include <IceGrid/FileUserAccountMapperI.h>
#include <IceGrid/NodeI.h>
#include <IceGrid/NodeSessionManager.h>
#include <IceGrid/TraceLevels.h>
#include <IceGrid/DescriptorParser.h>
#include <IcePatch2/Util.h>

#ifdef _WIN32
#   include <direct.h>
#   include <sys/types.h>
#   include <sys/stat.h>
#   define S_ISDIR(mode) ((mode) & _S_IFDIR)
#   define S_ISREG(mode) ((mode) & _S_IFREG)
#else
#   include <sys/stat.h>
#endif

using namespace std;
using namespace Ice;
using namespace IceGrid;

namespace IceGrid
{

class ProcessI : public Process
{
public:
    
    ProcessI(const ActivatorPtr&);

    virtual void shutdown(const Current&);
    virtual void writeMessage(const std::string&, Int, const Current&);
    
private:
    
    ActivatorPtr _activator;
};


class NodeService : public Service
{
public:

    NodeService();

    virtual bool shutdown();

protected:

    virtual bool start(int, char*[]);
    virtual void waitForShutdown();
    virtual bool stop();
    virtual CommunicatorPtr initializeCommunicator(int&, char*[], const InitializationData&);

private:

    void usage(const std::string&);

    ActivatorPtr _activator;
    WaitQueuePtr _waitQueue;
    RegistryIPtr _registry;
    NodeIPtr _node;
    NodeSessionManager _sessions;
    Ice::ObjectAdapterPtr _adapter;
};

class CollocatedRegistry : public RegistryI
{
public:

    CollocatedRegistry(const CommunicatorPtr&, const ActivatorPtr&);
    virtual void shutdown();

private:

    ActivatorPtr _activator;
};

} // End of namespace IceGrid

CollocatedRegistry::CollocatedRegistry(const CommunicatorPtr& communicator, const ActivatorPtr& activator) :
    RegistryI(communicator), 
    _activator(activator)
{
}

void
CollocatedRegistry::shutdown()
{
    _activator->shutdown();
}

ProcessI::ProcessI(const ActivatorPtr& activator) : _activator(activator)
{
}

void
ProcessI::shutdown(const Current&)
{
    _activator->shutdown();
}

void
ProcessI::writeMessage(const string& message, Int fd, const Current&)
{
    switch(fd)
    {
	case 1:
	{
	    cout << message << endl;
	    break;
	}
	case 2:
	{
	    cerr << message << endl;
	    break;
	}
    }
}

NodeService::NodeService()
{
}

bool
NodeService::shutdown()
{
    assert(_activator);
    _activator->shutdown();
    return true;
}

bool
NodeService::start(int argc, char* argv[])
{
    bool nowarn = false;
    string desc;
    vector<string> targets;
    for(int i = 1; i < argc; ++i)
    {
        if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            usage(argv[0]);
            return false;
        }
        else if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
        {
            print(ICE_STRING_VERSION);
            return false;
        }
        else if(strcmp(argv[i], "--nowarn") == 0)
        {
            nowarn = true;
        }
        else if(strcmp(argv[i], "--deploy") == 0)
        {
            if(i + 1 >= argc)
            {
                error("missing descriptor argument for option `" + string(argv[i]) + "'");
                usage(argv[0]);
                return false;
            }

            desc = argv[++i];

            while(i + 1 < argc && argv[++i][0] != '-')
            {
                targets.push_back(argv[i]);
            }
        }
	else
	{
	    error("invalid option: `" + string(argv[i]) + "'");
	    usage(argv[0]);
	    return false;
	}
    }

    PropertiesPtr properties = communicator()->getProperties();

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

    //
    // Warn the user that setting Ice.ThreadPool.Server isn't useful.
    //
    if(!nowarn && properties->getPropertyAsIntWithDefault("Ice.ThreadPool.Server.Size", 0) > 0)
    {
	Warning out(communicator()->getLogger());
	out << "setting `Ice.ThreadPool.Server.Size' is not useful,\n";
	out << "you should set individual adapter thread pools instead.";
    }
    
    int size = properties->getPropertyAsIntWithDefault("IceGrid.Node.ThreadPool.Size", 0);
    if(size <= 0)
    {
	properties->setProperty("IceGrid.Node.ThreadPool.Size", "1");
	size = 1;
    }
    int sizeMax = properties->getPropertyAsIntWithDefault("IceGrid.Node.ThreadPool.SizeMax", 0);
    if(sizeMax <= 0)
    {
	if(size >= sizeMax)
	{
	    sizeMax = size * 10;
	}
	
	ostringstream os;
	os << sizeMax;
	properties->setProperty("IceGrid.Node.ThreadPool.SizeMax", os.str());
    }

    //
    // Create the activator.
    //
    TraceLevelsPtr traceLevels = new TraceLevels(properties, communicator()->getLogger(), true);
    _activator = new Activator(traceLevels, properties);

    //
    // Collocate the IceGrid registry if we need to.
    //
    if(properties->getPropertyAsInt("IceGrid.Node.CollocateRegistry") > 0)
    {
        _registry = new CollocatedRegistry(communicator(), _activator);
        if(!_registry->start(nowarn))
        {
            return false;
        }

	//
	// Set the default locator property to point to the collocated
	// locator (this property is passed by the activator to each
	// activated server). The default locator is also needed by
	// the node session manager.
        //
	if(properties->getProperty("Ice.Default.Locator").empty())
	{
	    const string instanceNameProperty = "IceGrid.InstanceName";
	    const string locatorId = properties->getPropertyWithDefault(instanceNameProperty, "IceGrid") + "/Locator";
	    string locatorPrx = locatorId + ":" + properties->getProperty("IceGrid.Registry.Client.Endpoints");
	    communicator()->setDefaultLocator(LocatorPrx::uncheckedCast(communicator()->stringToProxy(locatorPrx)));
	    properties->setProperty("Ice.Default.Locator", locatorPrx);
	}
    }
    else if(properties->getProperty("Ice.Default.Locator").empty())
    {
        error("property `Ice.Default.Locator' is not set");
        return false;
    }

    //
    // Initialize the database environment (first setup the directory structure if needed).
    //
    string dataPath = properties->getProperty("IceGrid.Node.Data");
    string dbPath;
    if(dataPath.empty())
    {
        error("property `IceGrid.Node.Data' is not set");
        return false;
    }
    else
    {
#ifdef _WIN32
        struct _stat filestat;
        if(::_stat(dataPath.c_str(), &filestat) != 0 || !S_ISDIR(filestat.st_mode))
        {
	    ostringstream os;
	    FileException ex(__FILE__, __LINE__);
	    ex.path = dataPath;
	    ex.error = getSystemErrno();
	    os << ex;
            error("property `IceGrid.Node.Data' is set to an invalid path:\n" + os.str());
	    return false;
        }            
#else
        struct stat filestat;
        if(::stat(dataPath.c_str(), &filestat) != 0 || !S_ISDIR(filestat.st_mode))
        {
	    ostringstream os;
	    FileException ex(__FILE__, __LINE__);
	    ex.path = dataPath;
	    ex.error = getSystemErrno();
	    os << ex;
            error("property `IceGrid.Node.Data' is set to an invalid path:\n" + os.str());
	    return false;
        }            
#endif

        //
        // Creates subdirectories.
        //
        if(dataPath[dataPath.length() - 1] != '/')
        {
            dataPath += "/"; 
        }

	IcePatch2::createDirectory(dataPath + "servers");
	IcePatch2::createDirectory(dataPath + "tmp");
	IcePatch2::createDirectory(dataPath + "distrib");
    }

    //
    // Check that required properties are set and valid.
    //
    if(properties->getProperty("IceGrid.Node.Endpoints").empty())
    {
        error("property `IceGrid.Node.Endpoints' is not set");
        return false;
    }

    string name = properties->getProperty("IceGrid.Node.Name");
    if(name.empty())
    {
        string hostname = IceInternal::getProtocolPluginFacade(communicator())->getDefaultHost();
	if(!nowarn)
        {
            warning("property `IceGrid.Node.Name' is not set, using hostname: " + hostname);
        }
	properties->setProperty("IceGrid.Node.Name", hostname);
    }

    //
    // Setup the Freeze database environment home directory. The name of the database
    // environment for the IceGrid node is the name of the node.
    //
    properties->setProperty("Freeze.DbEnv." + name + ".DbHome", dbPath);

    //
    // Create the node object adapter.
    //
    properties->setProperty("IceGrid.Node.RegisterProcess", "0");
    properties->setProperty("IceGrid.Node.AdapterId", "");
    _adapter = communicator()->createObjectAdapter("IceGrid.Node");

    //
    // Setup the user account mapper if configured.
    //
    string mapperProperty = properties->getProperty("IceGrid.Node.UserAccountMapper");
    UserAccountMapperPrx mapper;
    if(!mapperProperty.empty())
    {
	try
	{
	    mapper = UserAccountMapperPrx::uncheckedCast(communicator()->stringToProxy(mapperProperty));
	}
	catch(const Ice::LocalException& ex)
	{
	    ostringstream os;
	    os << "user account mapper `" << mapperProperty << "' is invalid:\n" << ex;
	    error(os.str());
	    return false;
	}
    }
    else
    {
	string userAccountFileProperty = properties->getProperty("IceGrid.Node.UserAccounts");
	if(!userAccountFileProperty.empty())
	{
	    try
	    {
		Ice::ObjectPrx object = _adapter->addWithUUID(new FileUserAccountMapperI(userAccountFileProperty));
		object = object->ice_collocationOptimized(true);
		mapper = UserAccountMapperPrx::uncheckedCast(object);
	    }
	    catch(const std::string& msg)
	    {
		error(msg);
		return false;
	    }
	}
    }

    //
    // Create the wait queue.
    //
    _waitQueue = new WaitQueue();
    _waitQueue->start();

    //
    // Create the server factory. The server factory creates persistent objects
    // for the server and server adapter. It also takes care of installing the
    // evictors and object factories necessary to store these objects.
    //
    Identity id = communicator()->stringToIdentity(IceUtil::generateUUID());
    NodePrx nodeProxy = NodePrx::uncheckedCast(_adapter->createProxy(id));
    _node = new NodeI(_adapter, _sessions, _activator, _waitQueue, traceLevels, nodeProxy, name, mapper);
    _adapter->add(_node, nodeProxy->ice_getIdentity());

    //
    // Create the node sessions with the registries.
    //
    _sessions.create(_node);

    //
    // Add a process servant to allow shutdown through the process
    // interface if a server id is set on the node.
    //
    if(!properties->getProperty("Ice.ServerId").empty() && communicator()->getDefaultLocator())
    {
	try
	{
	    ProcessPrx proxy = ProcessPrx::uncheckedCast(_adapter->addWithUUID(new ProcessI(_activator)));
	    LocatorRegistryPrx locatorRegistry = communicator()->getDefaultLocator()->getRegistry();
	    locatorRegistry->setServerProcessProxy(properties->getProperty("Ice.ServerId"), proxy);
	}
	catch(const ServerNotFoundException&)
	{
	}
	catch(const LocalException&)
	{
	}
    }

    //
    // Start the activator.
    //
    _activator->start();

    //
    // Activate the adapter.
    //
    _adapter->activate();

    //
    // Deploy application if a descriptor is passed as a command-line option.
    //
    if(!desc.empty())
    {
        AdminPrx admin;
        try
        {
	    const string instanceName = communicator()->getDefaultLocator()->ice_getIdentity().category;
	    const string adminId = instanceName + "/Admin";
            admin = AdminPrx::checkedCast(communicator()->stringToProxy(adminId));
        }
        catch(const LocalException& ex)
        {
            ostringstream ostr;
            ostr << "couldn't contact IceGrid admin interface to deploy application `" << desc << "':\n" << ex;
            warning(ostr.str());
        }

        if(admin)
        {
            try
            {
		map<string, string> vars;
		ApplicationDescriptor app;
		app = DescriptorParser::parseDescriptor(desc, targets, vars, communicator(), admin);
		try
		{
		    admin->syncApplication(app);
		}
		catch(const ApplicationNotExistException&)
		{
		    admin->addApplication(app);
		}
	    }
            catch(const DeploymentException& ex)
            {
                ostringstream ostr;
                ostr << "failed to deploy application `" << desc << "':\n" << ex << ": " << ex.reason;
                warning(ostr.str());
            }
            catch(const LocalException& ex)
            {
                ostringstream ostr;
                ostr << "failed to deploy application `" << desc << "':\n" << ex;
                warning(ostr.str());
            }
        }
    }

    string bundleName = properties->getProperty("IceGrid.Node.PrintServersReady");
    if(!bundleName.empty())
    {
	//
	// We wait for the node to be registered with the registry
	// before to claim it's ready.
	//
	_sessions.waitForCreate();
	print(bundleName + " ready");
    }

    return true;
}

void
NodeService::waitForShutdown()
{
    //
    // Wait for the activator shutdown. Once the run method returns
    // all the servers have been deactivated.
    //
    enableInterrupt();
    _activator->waitForShutdown();
    disableInterrupt();
}

bool
NodeService::stop()
{
    try
    {
        _activator->destroy();
    }
    catch(...)
    {
	assert(false);
    }

    //
    // The wait queue must be destroyed after the activator and before
    // the communicator is shutdown.
    //
    try
    {
        _waitQueue->destroy();
	_waitQueue = 0;
    }
    catch(...)
    {
	assert(false);
    }

    _activator = 0;

    //
    // Deactivate the node object adapter.
    //
    try
    {
	_adapter->deactivate();
	_adapter = 0;
    }
    catch(const Ice::LocalException& ex)
    {
	ostringstream ostr;
	ostr << "unexpected exception while shutting down node:\n" << ex;
	warning(ostr.str());
    }

    //
    // Terminate the node sessions with the registries.
    //
    _sessions.destroy();
    _node = 0;

    //
    // We can now safely shutdown the communicator.
    //
    try
    {
        communicator()->shutdown();
        communicator()->waitForShutdown();
    }
    catch(const Ice::LocalException& ex)
    {
	ostringstream ostr;
	ostr << "unexpected exception while shutting down node:\n" << ex;
	warning(ostr.str());
    }

    //
    // And shutdown the collocated registry.
    //
    if(_registry)
    {
	_registry->stop();
	_registry = 0;
    }

    return true;
}

CommunicatorPtr
NodeService::initializeCommunicator(int& argc, char* argv[], 
				    const InitializationData& initializationData)
{
    InitializationData initData = initializationData;
    initData.properties = createProperties(argc, argv, initData.properties);

    //
    // Make sure that IceGridNode doesn't use thread-per-connection or
    // collocation optimization
    //
    initData.properties->setProperty("Ice.ThreadPerConnection", "");
    initData.properties->setProperty("Ice.Default.CollocationOptimization", "0");

    return Service::initializeCommunicator(argc, argv, initData);
}

void
NodeService::usage(const string& appName)
{
    string options =
	"Options:\n"
	"-h, --help           Show this message.\n"
	"-v, --version        Display the Ice version.\n"
	"--nowarn             Don't print any security warnings.\n"
	"\n"
	"--deploy DESCRIPTOR [TARGET1 [TARGET2 ...]]\n"
	"                     Add or update descriptor in file DESCRIPTOR, with\n"
	"                     optional targets.\n";
#ifdef _WIN32
    if(checkSystem())
    {
        options.append(
	"\n"
	"\n"
	"--service NAME       Run as the Windows service NAME.\n"
	"\n"
	"--install NAME [--display DISP] [--executable EXEC] [args]\n"
	"                     Install as Windows service NAME. If DISP is\n"
	"                     provided, use it as the display name,\n"
	"                     otherwise NAME is used. If EXEC is provided,\n"
	"                     use it as the service executable, otherwise\n"
	"                     this executable is used. Any additional\n"
	"                     arguments are passed unchanged to the\n"
	"                     service at startup.\n"
	"--uninstall NAME     Uninstall Windows service NAME.\n"
	"--start NAME [args]  Start Windows service NAME. Any additional\n"
	"                     arguments are passed unchanged to the\n"
	"                     service.\n"
	"--stop NAME          Stop Windows service NAME."
        );
    }
#else
    options.append(
        "\n"
        "\n"
        "--daemon             Run as a daemon.\n"
        "--noclose            Do not close open file descriptors.\n"
        "--nochdir            Do not change the current working directory."
    );
#endif
    print("Usage: " + appName + " [options]\n" + options);
}

int
main(int argc, char* argv[])
{
    NodeService svc;
    return svc.main(argc, argv);
}
