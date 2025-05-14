// Copyright (c) ZeroC, Inc.

#include "../Glacier2/CryptPermissionsVerifier.h"
#include "../Ice/ConsoleUtil.h"
#include "../Ice/FileUtil.h"
#include "../Ice/Timer.h"
#include "Activator.h"
#include "DescriptorParser.h"
#include "FileUserAccountMapperI.h"
#include "Ice/Ice.h"
#include "Ice/StringUtil.h"
#include "NodeAdminRouter.h"
#include "NodeI.h"
#include "NodeSessionManager.h"
#include "RegistryI.h"
#include "TraceLevels.h"
#include "Util.h"

#include <iostream>

#ifdef _WIN32
#    include <direct.h>
#    include <sys/types.h>
#    include <winsock2.h>
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace IceGrid;

namespace
{
    class ProcessI final : public Process
    {
    public:
        ProcessI(const shared_ptr<Activator>&, const shared_ptr<Process>&);

        void shutdown(const Current&) override;
        void writeMessage(std::string, int, const Current&) override;

    private:
        shared_ptr<Activator> _activator;
        shared_ptr<Process> _origProcess;
    };

    class NodeService final : public Service
    {
    public:
        bool shutdown() override;

    protected:
        bool start(int, char*[], int&) override;
        bool startImpl(int, char*[], int&);
        void waitForShutdown() override;
        bool stop() override;
        CommunicatorPtr initializeCommunicator(int&, char*[], InitializationData) override;

    private:
        void usage(const std::string&);

        shared_ptr<Activator> _activator;
        IceInternal::TimerPtr _timer;
        shared_ptr<RegistryI> _registry;
        shared_ptr<NodeI> _node;
        unique_ptr<NodeSessionManager> _sessions;
        ObjectAdapterPtr _adapter;
    };

    class CollocatedRegistry final : public RegistryI
    {
    public:
        CollocatedRegistry(
            const CommunicatorPtr&,
            const shared_ptr<Activator>&,
            bool,
            const std::string&,
            const std::string&);
        void shutdown() final;

    private:
        shared_ptr<Activator> _activator;
    };

#ifdef _WIN32
    void setNoIndexingAttribute(const string& path)
    {
        wstring wpath = Ice::stringToWstring(path);
        DWORD attrs = GetFileAttributesW(wpath.c_str());
        if (attrs == INVALID_FILE_ATTRIBUTES)
        {
            throw FileException(__FILE__, __LINE__, path);
        }
        if (!SetFileAttributesW(wpath.c_str(), attrs | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED))
        {
            throw FileException(__FILE__, __LINE__, path);
        }
    }
#endif
}

CollocatedRegistry::CollocatedRegistry(
    const CommunicatorPtr& com,
    const shared_ptr<Activator>& activator,
    bool readonly,
    const string& initFromReplica,
    const string& nodeName)
    : RegistryI(com, make_shared<TraceLevels>(com, "IceGrid.Registry"), readonly, initFromReplica, nodeName),
      _activator(activator)
{
}

void
CollocatedRegistry::shutdown()
{
    _activator->shutdown();
}

ProcessI::ProcessI(const shared_ptr<Activator>& activator, const shared_ptr<Process>& origProcess)
    : _activator(activator),
      _origProcess(origProcess)
{
}

void
ProcessI::shutdown(const Current&)
{
    _activator->shutdown();
}

void
ProcessI::writeMessage(string message, int fd, const Current& current)
{
    _origProcess->writeMessage(std::move(message), fd, current);
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
    if (_sessions->isWaitingForCreate())
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
        if (!startImpl(argc, argv, status))
        {
            stop();
            return false;
        }
    }
    catch (...)
    {
        stop();
        throw;
    }
    return true;
}

bool
NodeService::startImpl(int argc, char* argv[], int& status)
{
    bool readonly = false;
    string initFromReplica;
    string desc;
    vector<string> targets;

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            usage(argv[0]);
            status = EXIT_SUCCESS;
            return false;
        }
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
        {
            print(ICE_STRING_VERSION);
            status = EXIT_SUCCESS;
            return false;
        }
        else if (strcmp(argv[i], "--readonly") == 0)
        {
            readonly = true;
        }
        else if (strcmp(argv[i], "--initdb-from-replica") == 0)
        {
            if (i + 1 >= argc)
            {
                error("missing replica argument for option '" + string(argv[i]) + "'");
                usage(argv[0]);
                return false;
            }

            initFromReplica = argv[++i];
        }
        else if (strcmp(argv[i], "--deploy") == 0)
        {
            if (i + 1 >= argc)
            {
                error("missing descriptor argument for option '" + string(argv[i]) + "'");
                usage(argv[0]);
                return false;
            }

            desc = argv[++i];

            while (i + 1 < argc && argv[++i][0] != '-')
            {
                targets.emplace_back(argv[i]);
            }
        }
        else
        {
            error("invalid option: '" + string(argv[i]) + "'");
            usage(argv[0]);
            return false;
        }
    }

    auto properties = communicator()->getProperties();

    string name = properties->getIceProperty("IceGrid.Node.Name");
    if (name.empty())
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
    if (!properties->getProperty("Ice.ThreadPool.Server.Size").empty())
    {
        Warning out(communicator()->getLogger());
        out << "setting 'Ice.ThreadPool.Server.Size' is not useful, ";
        out << "you should set individual adapter thread pools instead.";
    }

    setupThreadPool(properties, "IceGrid.Node.ThreadPool", 1, 100);

    //
    // Create the activator.
    //
    auto traceLevels = make_shared<TraceLevels>(communicator(), "IceGrid.Node");
    _activator = make_shared<Activator>(traceLevels);

    //
    // Collocate the IceGrid registry if we need to.
    //
    if (properties->getIcePropertyAsInt("IceGrid.Node.CollocateRegistry") > 0)
    {
        _registry = make_shared<CollocatedRegistry>(communicator(), _activator, readonly, initFromReplica, name);
        if (!_registry->start())
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
        if (properties->getIceProperty("Ice.Default.Locator").empty())
        {
            properties->setProperty("Ice.Default.Locator", communicator()->getDefaultLocator()->ice_toString());
        }
    }
    else if (!communicator()->getDefaultLocator())
    {
        error("property `Ice.Default.Locator' is not set");
        return false;
    }

    //
    // Initialize the database environment (first setup the directory structure if needed).
    //
    string dataPath = properties->getIceProperty("IceGrid.Node.Data");
    if (dataPath.empty())
    {
        error("property `IceGrid.Node.Data' is not set");
        return false;
    }
    else
    {
        if (!IceInternal::directoryExists(dataPath))
        {
            FileException ex(__FILE__, __LINE__, dataPath);
            ServiceError err(this);
            err << "property `IceGrid.Node.Data' is set to an invalid path:\n" << ex;
            return false;
        }

        //
        // Creates subdirectories.
        //
        if (dataPath[dataPath.length() - 1] != '/')
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
        catch (const FileException& ex)
        {
            Warning out(communicator()->getLogger());
            out << "couldn't disable file indexing:\n" << ex;
        }
#endif
    }

    //
    // Check that required properties are set and valid.
    //
    if (properties->getIceProperty("IceGrid.Node.Endpoints").empty())
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
    string mapperPropertyValue = properties->getIceProperty(mapperProperty);
    optional<UserAccountMapperPrx> mapper;
    if (!mapperPropertyValue.empty())
    {
        try
        {
            mapper = communicator()->propertyToProxy<UserAccountMapperPrx>(mapperProperty);
        }
        catch (const std::exception& ex)
        {
            ServiceError err(this);
            err << "user account mapper '" << mapperProperty << "' is invalid:\n" << ex;
            return false;
        }
    }
    else
    {
        string userAccountFileProperty = properties->getIceProperty("IceGrid.Node.UserAccounts");
        if (!userAccountFileProperty.empty())
        {
            try
            {
                mapper = _adapter->addWithUUID<UserAccountMapperPrx>(
                    make_shared<FileUserAccountMapperI>(userAccountFileProperty));
            }
            catch (const exception& ex)
            {
                error(ex.what());
                return false;
            }
        }
    }

    //
    // Create a new timer to handle server activation/deactivation timeouts.
    //
    _timer = make_shared<IceInternal::Timer>();

    string instanceName = properties->getIceProperty("IceGrid.InstanceName");
    if (instanceName.empty()) // not set explicitly
    {
        instanceName = properties->getProperty("IceLocatorDiscovery.InstanceName");
    }
    if (instanceName.empty())
    {
        instanceName = communicator()->getDefaultLocator()->ice_getIdentity().category;
    }
    if (instanceName.empty())
    {
        instanceName = "IceGrid";
    }

    _sessions = make_unique<NodeSessionManager>(communicator(), instanceName);

    // Create the server factory. The server factory creates persistent objects for the server and server adapter. It
    // also takes care of installing the evictors and object factories necessary to store these objects.
    auto nodeProxy = _adapter->createProxy<NodePrx>(stringToIdentity(instanceName + "/Node-" + name));
    _node = make_shared<
        NodeI>(_adapter, *_sessions, _activator, _timer, traceLevels, nodeProxy, name, mapper, instanceName);
    _adapter->add(_node, nodeProxy->ice_getIdentity());

    _adapter->addDefaultServant(make_shared<NodeServerAdminRouter>(_node), _node->getServerAdminCategory());

    // Keep the old default servant for backward compatibility with IceGrid registries 3.5 that
    // still forward requests to this category. This can be removed when we decide to break
    // backward compatibility with 3.5 registries.
    _adapter->addDefaultServant(make_shared<NodeServerAdminRouter>(_node), instanceName + "-NodeRouter");

    //
    // Start the platform info thread if needed.
    //
    _node->getPlatformInfo().start();

    // Ensures that the IceGrid registry is reachable.
    auto locator = communicator()->getDefaultLocator();

    try
    {
        locator->ice_invocationTimeout(1s)->ice_ping();
    }
    catch (const std::exception& ex)
    {
        Warning out(communicator()->getLogger());
        out << "could not reach IceGrid registry '" << locator;
        out << "': " << ex.what()
            << ". This warning is expected if the IceGrid registry is not running yet; otherwise, please check the "
               "value of the Ice.Default.Locator property in the config file of this IceGrid node.";
    }

    //
    // Create the node sessions with the registries.
    //
    _sessions->create(_node);

    //
    // Create Admin unless there is a collocated registry with its own Admin
    //
    if (!_registry && properties->getIcePropertyAsInt("Ice.Admin.Enabled") > 0)
    {
        // Replace Admin facet
        auto origProcess = dynamic_pointer_cast<Process>(communicator()->removeAdminFacet("Process"));
        communicator()->addAdminFacet(make_shared<ProcessI>(_activator, origProcess), "Process");
        communicator()->createAdmin(_adapter, {"NodeAdmin-" + name, instanceName});
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

    string bundleName = properties->getIceProperty("IceGrid.Node.PrintServersReady");
    if (!bundleName.empty() || !desc.empty())
    {
        enableInterrupt();
        if (!_sessions->waitForCreate())
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
    if (!desc.empty())
    {
        try
        {
            Ice::Identity regId = {"Registry", instanceName};

            auto registry = checkedCast<RegistryPrx>(communicator()->getDefaultLocator()->findObjectById(regId));
            if (!registry)
            {
                throw runtime_error("invalid registry");
            }

            registry = registry->ice_preferSecure(true); // Use SSL if available.

            optional<AdminSessionPrx> session;
            if (communicator()->getProperties()->getIcePropertyAsInt("IceGridAdmin.AuthenticateUsingSSL"))
            {
                session = registry->createAdminSessionFromSecureConnection();
            }
            else
            {
                string username = communicator()->getProperties()->getIceProperty("IceGridAdmin.Username");
                string password = communicator()->getProperties()->getIceProperty("IceGridAdmin.Password");
                while (username.empty())
                {
                    consoleOut << "user id: " << flush;
                    getline(cin, username);
                    username = IceInternal::trim(username);
                }

                if (password.empty())
                {
                    consoleOut << "password: " << flush;
                    getline(cin, password);
                    password = IceInternal::trim(password);
                }

                session = registry->createAdminSession(username, password);
            }
            assert(session);

            auto admin = session->getAdmin();
            assert(admin);
            map<string, string> vars;
            auto app = DescriptorParser::parseDescriptor(desc, targets, vars, communicator(), *admin);

            try
            {
                admin->syncApplication(app);
            }
            catch (const ApplicationNotExistException&)
            {
                admin->addApplication(app);
            }
        }
        catch (const DeploymentException& ex)
        {
            ServiceWarning warn(this);
            warn << "failed to deploy application '" << desc << "':\n" << ex;
        }
        catch (const AccessDeniedException& ex)
        {
            ServiceWarning warn(this);
            warn << "failed to deploy application '" << desc << "':\n"
                 << "registry database is locked by '" << ex.lockUserId << "'";
        }
        catch (const std::exception& ex)
        {
            ServiceWarning warn(this);
            warn << "failed to deploy application '" << desc << "':\n" << ex.what();
        }
    }

    if (!bundleName.empty())
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
    if (_activator)
    {
        try
        {
            _activator->shutdown();
            _activator->destroy();
        }
        catch (...)
        {
            assert(false);
        }
    }

    if (_timer)
    {
        //
        // The timer must be destroyed after the activator and before the
        // communicator is shutdown.
        //
        try
        {
            _timer->destroy();
        }
        catch (...)
        {
            assert(false);
        }
        _timer = nullptr;
    }

    //
    // Deactivate the node object adapter.
    //
    if (_adapter)
    {
        try
        {
            _adapter->deactivate();
            _adapter = nullptr;
        }
        catch (const std::exception& ex)
        {
            ServiceWarning warn(this);
            warn << "unexpected exception while shutting down node:\n" << ex;
        }
    }

    //
    // Terminate the node sessions with the registries.
    //
    if (_sessions.get())
    {
        _sessions->destroy();
    }

    //
    // Stop the platform info thread.
    //
    if (_node)
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
    catch (const std::exception& ex)
    {
        ServiceWarning warn(this);
        warn << "unexpected exception while shutting down node:\n" << ex;
    }

    //
    // Break cylic reference counts.
    //
    if (_node)
    {
        _node->shutdown();
        _node = nullptr;
    }

    //
    // And shutdown the collocated registry.
    //
    if (_registry)
    {
        _registry->stop();
        _registry = nullptr;
    }

    return true;
}

CommunicatorPtr
NodeService::initializeCommunicator(int& argc, char* argv[], InitializationData initData)
{
    initData.properties = createProperties(argc, argv, initData.properties);

    // If IceGrid.Registry.[Admin]PermissionsVerifier is not set and
    // IceGrid.Registry.[Admin]CryptPasswords is set, load the
    // Glacier2CryptPermissionsVerifier plug-in
    //
    vector<string> vTypes;
    vTypes.emplace_back("");
    vTypes.emplace_back("Admin");

    for (const auto& type : vTypes)
    {
        string verifier = "IceGrid.Registry." + type + "PermissionsVerifier";

        if (initData.properties->getIceProperty(verifier).empty())
        {
            string cryptPasswords = initData.properties->getIceProperty("IceGrid.Registry." + type + "CryptPasswords");

            if (!cryptPasswords.empty())
            {
                initData.pluginFactories.push_back(Glacier2::cryptPermissionsVerifierPluginFactory());

                initData.properties->setProperty(
                    "Glacier2CryptPermissionsVerifier.IceGrid.Registry." + type + "PermissionsVerifier",
                    cryptPasswords);
            }
        }
    }

    //
    // Never create Admin object in Ice.Admin adapter
    //
    initData.properties->setProperty("Ice.Admin.Endpoints", "");

    //
    // Enable Admin unless explicitly disabled (or enabled) in configuration
    //
    if (initData.properties->getIceProperty("Ice.Admin.Enabled").empty())
    {
        initData.properties->setProperty("Ice.Admin.Enabled", "1");
    }

    //
    // Setup the client thread pool size.
    //
    setupThreadPool(initData.properties, "Ice.ThreadPool.Client", 1, 100);

    return Service::initializeCommunicator(argc, argv, std::move(initData));
}

void
NodeService::usage(const string& appName)
{
    string options = "Options:\n"
                     "-h, --help           Show this message.\n"
                     "-v, --version        Display the Ice version.\n"
                     "--readonly           Start the collocated master registry in read-only mode.\n"
                     "--initdb-from-replica <replica>\n"
                     "                     Initialize the collocated registry database from the\n"
                     "                     given replica.\n"
                     "--deploy DESCRIPTOR [TARGET1 [TARGET2 ...]]\n"
                     "                     Add or update descriptor in file DESCRIPTOR, with\n"
                     "                     optional targets.\n";
#ifndef _WIN32
    options.append("\n"
                   "\n"
                   "--daemon             Run as a daemon.\n"
                   "--noclose            Do not close open file descriptors.\n"
                   "--nochdir            Do not change the current working directory.\n"
                   "--pidfile FILE       Write process ID into FILE.");
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
    // Initialize the service with a Properties object with the correct property prefixes enabled.
    Ice::InitializationData initData;
    initData.properties = make_shared<Properties>(vector<string>{"IceGrid", "IceGridAdmin"});
    return svc.main(argc, argv, std::move(initData));
}
