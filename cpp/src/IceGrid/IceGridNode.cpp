//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/UUID.h>
#include <IceUtil/Timer.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/FileUtil.h>
#include <Ice/Ice.h>
#include <Ice/ConsoleUtil.h>
#include <Ice/Locator.h>
#include <Ice/Service.h>
#include <IceGrid/Activator.h>
#include <IceGrid/NodeAdminRouter.h>
#include <IceGrid/RegistryI.h>
#include <IceGrid/FileUserAccountMapperI.h>
#include <IceGrid/NodeI.h>
#include <IceGrid/NodeSessionManager.h>
#include <IceGrid/TraceLevels.h>
#include <IceGrid/DescriptorParser.h>
#include <IceGrid/Util.h>

#ifdef _WIN32
#   include <direct.h>
#   include <sys/types.h>
#   include <winsock2.h>
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace IceGrid;

namespace
{

class ProcessI : public Process
{
public:

    ProcessI(const ActivatorPtr&, const ProcessPtr&);

    virtual void shutdown(const Current&);
    virtual void writeMessage(const std::string&, Int, const Current&);

private:

    ActivatorPtr _activator;
    ProcessPtr _origProcess;
};

class NodeService : public Service
{
public:

    NodeService();
    ~NodeService();

    virtual bool shutdown();

protected:

    virtual bool start(int, char*[], int&);
    bool startImpl(int, char*[], int&);
    virtual void waitForShutdown();
    virtual bool stop();
    virtual CommunicatorPtr initializeCommunicator(int&, char*[], const InitializationData&, int);

private:

    void usage(const std::string&);

    ActivatorPtr _activator;
    IceUtil::TimerPtr _timer;
    RegistryIPtr _registry;
    NodeIPtr _node;
    IceInternal::UniquePtr<NodeSessionManager> _sessions;
    Ice::ObjectAdapterPtr _adapter;
};

class CollocatedRegistry : public RegistryI
{
public:

    CollocatedRegistry(const CommunicatorPtr&, const ActivatorPtr&, bool, bool, const std::string&, const std::string&);
    virtual void shutdown();

private:

    ActivatorPtr _activator;
};

#ifdef _WIN32
void
setNoIndexingAttribute(const string& path)
{
    wstring wpath = Ice::stringToWstring(path);
    DWORD attrs = GetFileAttributesW(wpath.c_str());
    if(attrs == INVALID_FILE_ATTRIBUTES)
    {
        throw FileException(__FILE__, __LINE__, IceInternal::getSystemErrno(), path);
    }
    if(!SetFileAttributesW(wpath.c_str(), attrs | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED))
    {
        throw FileException(__FILE__, __LINE__, IceInternal::getSystemErrno(), path);
    }
}
#endif

}

CollocatedRegistry::CollocatedRegistry(const CommunicatorPtr& com,
                                       const ActivatorPtr& activator,
                                       bool nowarn,
                                       bool readonly,
                                       const string& initFromReplica,
                                       const string& nodeName) :
    RegistryI(com, new TraceLevels(com, "IceGrid.Registry"), nowarn, readonly, initFromReplica, nodeName),
    _activator(activator)
{
}

void
CollocatedRegistry::shutdown()
{
    _activator->shutdown();
}

ProcessI::ProcessI(const ActivatorPtr& activator, const ProcessPtr& origProcess) :
    _activator(activator),
    _origProcess(origProcess)
{
}

void
ProcessI::shutdown(const Current&)
{
    _activator->shutdown();
}

void
ProcessI::writeMessage(const string& message, Int fd, const Current& current)
{
    _origProcess->writeMessage(message, fd, current);
}

NodeService::NodeService()
{
}

NodeService::~NodeService()
{
}

bool
NodeService::shutdown()
{
    assert(_activator && _sessions.get());
    _activator->shutdown();

    //
    // If the session manager waits for session creation with the master, we interrupt
    // the session creation. This is necessary to unblock the main thread which might
    // be waiting for waitForCreate to return.
    //
    if(_sessions->isWaitingForCreate())
    {
        _sessions->terminate();
    }

    return true;
}

bool
NodeService::start(int argc, char* argv[], int& status)
{
    try
    {
        if(!startImpl(argc, argv, status))
        {
            stop();
            return false;
        }
    }
    catch(...)
    {
        stop();
        throw;
    }
    return true;
}

bool
NodeService::startImpl(int argc, char* argv[], int& status)
{
    bool nowarn = false;
    bool readonly = false;
    string initFromReplica;
    string desc;
    vector<string> targets;

    for(int i = 1; i < argc; ++i)
    {
        if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            usage(argv[0]);
            status = EXIT_SUCCESS;
            return false;
        }
        else if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
        {
            print(ICE_STRING_VERSION);
            status = EXIT_SUCCESS;
            return false;
        }
        else if(strcmp(argv[i], "--nowarn") == 0)
        {
            nowarn = true;
        }
        else if(strcmp(argv[i], "--readonly") == 0)
        {
            readonly = true;
        }
        else if(strcmp(argv[i], "--initdb-from-replica") == 0)
        {
            if(i + 1 >= argc)
            {
                error("missing replica argument for option `" + string(argv[i]) + "'");
                usage(argv[0]);
                return false;
            }

            initFromReplica = argv[++i];
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

    string name = properties->getProperty("IceGrid.Node.Name");
    if(name.empty())
    {
        error("property `IceGrid.Node.Name' is not set");
        return false;
    }

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
        out << "setting `Ice.ThreadPool.Server.Size' is not useful, ";
        out << "you should set individual adapter thread pools instead.";
    }

    setupThreadPool(properties, "IceGrid.Node.ThreadPool", 1, 100);

    //
    // Create the activator.
    //
    TraceLevelsPtr traceLevels = new TraceLevels(communicator(), "IceGrid.Node");
    _activator = new Activator(traceLevels);

    //
    // Collocate the IceGrid registry if we need to.
    //
    if(properties->getPropertyAsInt("IceGrid.Node.CollocateRegistry") > 0)
    {
        _registry = new CollocatedRegistry(communicator(), _activator, nowarn, readonly, initFromReplica, name);
        if(!_registry->start())
        {
            return false;
        }

        communicator()->setDefaultLocator(_registry->getLocator());

        //
        // Set the default locator property to point to the collocated
        // locator (this property is passed by the activator to each
        // activated server). The default locator is also needed by
        // the node session manager.
        //
        if(properties->getProperty("Ice.Default.Locator").empty())
        {
            properties->setProperty("Ice.Default.Locator", communicator()->getDefaultLocator()->ice_toString());
        }
    }
    else if(!communicator()->getDefaultLocator())
    {
        error("property `Ice.Default.Locator' is not set");
        return false;
    }

    //
    // Initialize the database environment (first setup the directory structure if needed).
    //
    string dataPath = properties->getProperty("IceGrid.Node.Data");
    if(dataPath.empty())
    {
        error("property `IceGrid.Node.Data' is not set");
        return false;
    }
    else
    {
        if(!IceUtilInternal::directoryExists(dataPath))
        {
            FileException ex(__FILE__, __LINE__, IceInternal::getSystemErrno(), dataPath);
            ServiceError err(this);
            err << "property `IceGrid.Node.Data' is set to an invalid path:\n" << ex;
            return false;
        }

        //
        // Creates subdirectories.
        //
        if(dataPath[dataPath.length() - 1] != '/')
        {
            dataPath += "/";
        }

        createDirectory(dataPath + "servers");
        createDirectory(dataPath + "tmp");

#ifdef _WIN32
        //
        // Make sure these directories are not indexed by the Windows
        // indexing service (which can cause random "Access Denied"
        // errors if indexing runs at the same time as the node is
        // creating/deleting files).
        //
        try
        {
            setNoIndexingAttribute(dataPath + "servers");
            setNoIndexingAttribute(dataPath + "tmp");
        }
        catch(const FileException& ex)
        {
            if(!nowarn)
            {
                Warning out(communicator()->getLogger());
                out << "couldn't disable file indexing:\n" << ex;
            }
        }
#endif
    }

    //
    // Check that required properties are set and valid.
    //
    if(properties->getProperty("IceGrid.Node.Endpoints").empty())
    {
        error("property `IceGrid.Node.Endpoints' is not set");
        return false;
    }

    //
    // Create the node object adapter.
    //
    _adapter = communicator()->createObjectAdapter("IceGrid.Node");

    //
    // Setup the user account mapper if configured.
    //
    string mapperProperty = "IceGrid.Node.UserAccountMapper";
    string mapperPropertyValue = properties->getProperty(mapperProperty);
    UserAccountMapperPrx mapper;
    if(!mapperPropertyValue.empty())
    {
        try
        {
            mapper = UserAccountMapperPrx::uncheckedCast(communicator()->propertyToProxy(mapperProperty));
        }
        catch(const std::exception& ex)
        {
            ServiceError err(this);
            err << "user account mapper `" << mapperProperty << "' is invalid:\n" << ex;
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
                mapper = UserAccountMapperPrx::uncheckedCast(object);
            }
            catch(const exception& ex)
            {
                error(ex.what());
                return false;
            }
        }
    }

    //
    // Create a new timer to handle server activation/deactivation timeouts.
    //
    _timer = new IceUtil::Timer();

    //
    // The IceGrid instance name.
    //
    string instanceName = properties->getProperty("IceGrid.InstanceName");
    if(instanceName.empty())
    {
        instanceName = properties->getProperty("IceLocatorDiscovery.InstanceName");
    }
    if(instanceName.empty())
    {
        instanceName = communicator()->getDefaultLocator()->ice_getIdentity().category;
    }
    if(instanceName.empty())
    {
        instanceName = "IceGrid";
    }

    _sessions.reset(new NodeSessionManager(communicator(), instanceName));

    //
    // Create the server factory. The server factory creates persistent objects
    // for the server and server adapter. It also takes care of installing the
    // evictors and object factories necessary to store these objects.
    //
    Identity id = stringToIdentity(instanceName + "/Node-" + name);
    NodePrx nodeProxy = NodePrx::uncheckedCast(_adapter->createProxy(id));
    _node = new NodeI(_adapter, *_sessions, _activator, _timer, traceLevels, nodeProxy, name, mapper, instanceName);
    _adapter->add(_node, nodeProxy->ice_getIdentity());

    _adapter->addDefaultServant(new NodeServerAdminRouter(_node), _node->getServerAdminCategory());

    //
    // Keep the old default servant for backward compatibility with IceGrid registries 3.5 that
    // still forward requests to this category. This can be removed when we decide to break
    // backward compatibility with 3.5 registries.
    //
    _adapter->addDefaultServant(new NodeServerAdminRouter(_node), instanceName + "-NodeRouter");

    //
    // Start the platform info thread if needed.
    //
    _node->getPlatformInfo().start();

    //
    // Ensures that the locator is reachable.
    //
    if(!nowarn)
    {
        try
        {
            communicator()->getDefaultLocator()->ice_timeout(1000)->ice_ping();
        }
        catch(const Ice::Exception& ex)
        {
            Warning out(communicator()->getLogger());
            out << "couldn't reach the IceGrid registry (this is expected ";
            out << "if it's down, otherwise please check the value of the ";
            out << "Ice.Default.Locator property):\n" << ex;
        }
    }

    //
    // Create the node sessions with the registries.
    //
    _sessions->create(_node);

    //
    // Create Admin unless there is a collocated registry with its own Admin
    //
    if(!_registry && properties->getPropertyAsInt("Ice.Admin.Enabled") > 0)
    {
        // Replace Admin facet
        ProcessPtr origProcess = ProcessPtr::dynamicCast(communicator()->removeAdminFacet("Process"));
        communicator()->addAdminFacet(new ProcessI(_activator, origProcess), "Process");

        Identity adminId;
        adminId.name = "NodeAdmin-" + name;
        adminId.category = instanceName;
        communicator()->createAdmin(_adapter, adminId);
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
    // Notify the node session manager that the node can start
    // accepting incoming connections.
    //
    _sessions->activate();

    string bundleName = properties->getProperty("IceGrid.Node.PrintServersReady");
    if(!bundleName.empty() || !desc.empty())
    {
        enableInterrupt();
        if(!_sessions->waitForCreate())
        {
            //
            // Create was interrupted, return true as if the service was
            // correctly initiliazed to make sure it's properly stopped.
            //
            return true;
        }
        disableInterrupt();
    }

    //
    // Deploy application if a descriptor is passed as a command-line option.
    //
    if(!desc.empty())
    {
        try
        {
            Ice::Identity regId;
            regId.category = instanceName;
            regId.name = "Registry";

            RegistryPrx registry = RegistryPrx::checkedCast(communicator()->getDefaultLocator()->findObjectById(regId));
            if(!registry)
            {
                throw runtime_error("invalid registry");
            }

            registry = registry->ice_preferSecure(true); // Use SSL if available.

            IceGrid::AdminSessionPrx session;
            if(communicator()->getProperties()->getPropertyAsInt("IceGridAdmin.AuthenticateUsingSSL"))
            {
                session = registry->createAdminSessionFromSecureConnection();
            }
            else
            {
                string username = communicator()->getProperties()->getProperty("IceGridAdmin.Username");
                string password = communicator()->getProperties()->getProperty("IceGridAdmin.Password");
                while(username.empty())
                {
                    consoleOut << "user id: " << flush;
                    getline(cin, username);
                    username = IceUtilInternal::trim(username);
                }

                if(password.empty())
                {
                    consoleOut << "password: " << flush;
                    getline(cin, password);
                    password = IceUtilInternal::trim(password);
                }

                session = registry->createAdminSession(username, password);
            }
            assert(session);

            AdminPrx admin = session->getAdmin();
            map<string, string> vars;
            ApplicationDescriptor app = DescriptorParser::parseDescriptor(desc, targets, vars, communicator(), admin);

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
            ServiceWarning warn(this);
            warn << "failed to deploy application `" << desc << "':\n" << ex;
        }
        catch(const AccessDeniedException& ex)
        {
            ServiceWarning warn(this);
            warn << "failed to deploy application `" << desc << "':\n"
                 << "registry database is locked by `" << ex.lockUserId << "'";
        }
        catch(const std::exception& ex)
        {
            ServiceWarning warn(this);
            warn << "failed to deploy application `" << desc << "':\n" << ex.what();
        }
    }

    if(!bundleName.empty())
    {
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
    if(_activator)
    {
        try
        {
            _activator->shutdown();
            _activator->destroy();
        }
        catch(...)
        {
            assert(false);
        }
    }

    if(_timer)
    {
        //
        // The timer must be destroyed after the activator and before the
        // communicator is shutdown.
        //
        try
        {
            _timer->destroy();
        }
        catch(...)
        {
            assert(false);
        }
        _timer = 0;
    }

    //
    // Deactivate the node object adapter.
    //
    if(_adapter)
    {
        try
        {
            _adapter->deactivate();
            _adapter = 0;
        }
        catch(const std::exception& ex)
        {
            ServiceWarning warn(this);
            warn << "unexpected exception while shutting down node:\n" << ex;
        }
    }

    //
    // Terminate the node sessions with the registries.
    //
    if(_sessions.get())
    {
        _sessions->destroy();
    }

    //
    // Stop the platform info thread.
    //
    if(_node)
    {
        _node->getPlatformInfo().stop();
    }

    //
    // We can now safely shutdown the communicator.
    //
    try
    {
        communicator()->shutdown();
        communicator()->waitForShutdown();
    }
    catch(const std::exception& ex)
    {
        ServiceWarning warn(this);
        warn << "unexpected exception while shutting down node:\n" << ex;
    }

    //
    // Break cylic reference counts.
    //
    if(_node)
    {
        _node->shutdown();
        _node = 0;
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
                                    const InitializationData& initializationData,
                                    int version)
{
    InitializationData initData = initializationData;
    initData.properties = createProperties(argc, argv, initData.properties);

    // If IceGrid.Registry.[Admin]PermissionsVerifier is not set and
    // IceGrid.Registry.[Admin]CryptPasswords is set, load the
    // Glacier2CryptPermissionsVerifier plug-in
    //
    vector<string> vTypes;
    vTypes.push_back("");
    vTypes.push_back("Admin");

    for(vector<string>::const_iterator p = vTypes.begin(); p != vTypes.end(); ++p)
    {
        string verifier = "IceGrid.Registry." + *p + "PermissionsVerifier";

        if(initData.properties->getProperty(verifier).empty())
        {
            string cryptPasswords = initData.properties->getProperty("IceGrid.Registry." + *p + "CryptPasswords");

            if(!cryptPasswords.empty())
            {
                initData.properties->setProperty("Ice.Plugin.Glacier2CryptPermissionsVerifier",
                                                 "Glacier2CryptPermissionsVerifier:createCryptPermissionsVerifier");

                initData.properties->setProperty("Glacier2CryptPermissionsVerifier.IceGrid.Registry." + *p +
                                                 "PermissionsVerifier", cryptPasswords);
            }
        }
    }

    //
    // Never create Admin object in Ice.Admin adapter
    //
    initData.properties->setProperty("Ice.Admin.Endpoints", "");

    //
    // Enable Admin unless explicitely disabled (or enabled) in configuration
    //
    if(initData.properties->getProperty("Ice.Admin.Enabled").empty())
    {
        initData.properties->setProperty("Ice.Admin.Enabled", "1");
    }

    //
    // Setup the client thread pool size.
    //
    setupThreadPool(initData.properties, "Ice.ThreadPool.Client", 1, 100);

    //
    // Close idle connections
    //
    initData.properties->setProperty("Ice.ACM.Close", "3");

    return Service::initializeCommunicator(argc, argv, initData, version);
}

void
NodeService::usage(const string& appName)
{
    string options =
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        "--nowarn             Don't print any security warnings.\n"
        "--readonly           Start the collocated master registry in read-only mode.\n"
        "--initdb-from-replica <replica>\n"
        "                     Initialize the collocated registry database from the\n"
        "                     given replica.\n"
        "--deploy DESCRIPTOR [TARGET1 [TARGET2 ...]]\n"
        "                     Add or update descriptor in file DESCRIPTOR, with\n"
        "                     optional targets.\n";
#ifndef _WIN32
    options.append(
        "\n"
        "\n"
        "--daemon             Run as a daemon.\n"
        "--noclose            Do not close open file descriptors.\n"
        "--nochdir            Do not change the current working directory.\n"
        "--pidfile FILE       Write process ID into FILE."
    );
#endif
    print("Usage: " + appName + " [options]\n" + options);
}

#ifdef _WIN32

int
wmain(int argc, wchar_t* argv[])

#else

int
main(int argc, char* argv[])

#endif
{
    NodeService svc;
    return svc.main(argc, argv);
}
