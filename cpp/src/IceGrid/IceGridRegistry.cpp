// Copyright (c) ZeroC, Inc.

#include "../Glacier2/CryptPermissionsVerifier.h"
#include "../Ice/ConsoleUtil.h"
#include "../Ice/Options.h"
#include "Ice/Ice.h"
#include "RegistryI.h"
#include "TraceLevels.h"
#include "Util.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace IceGrid;

namespace IceGrid
{
    class RegistryService : public Service
    {
    public:
        bool shutdown() override;

    protected:
        bool start(int, char*[], int&) override;
        void waitForShutdown() override;
        bool stop() override;
        CommunicatorPtr initializeCommunicator(int&, char*[], InitializationData) override;

    private:
        void usage(const std::string&);

        shared_ptr<RegistryI> _registry;
    };

} // End of namespace IceGrid

bool
RegistryService::shutdown()
{
    assert(_registry);
    _registry->shutdown();
    return true;
}

bool
RegistryService::start(int argc, char* argv[], int& status)
{
    bool readonly;
    std::string initFromReplica;

    IceInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("", "readonly");
    opts.addOpt("", "initdb-from-replica", IceInternal::Options::NeedArg);

    vector<string> args;
    try
    {
        args = opts.parse(argc, const_cast<const char**>(argv));
    }
    catch (const IceInternal::BadOptException& e)
    {
        error(e.what());
        usage(argv[0]);
        return false;
    }

    if (opts.isSet("help"))
    {
        usage(argv[0]);
        status = EXIT_SUCCESS;
        return false;
    }
    if (opts.isSet("version"))
    {
        print(ICE_STRING_VERSION);
        status = EXIT_SUCCESS;
        return false;
    }

    readonly = opts.isSet("readonly");
    if (opts.isSet("initdb-from-replica"))
    {
        initFromReplica = opts.optArg("initdb-from-replica");
    }

    if (!args.empty())
    {
        consoleErr << argv[0] << ": too many arguments" << endl;
        usage(argv[0]);
        return false;
    }

    auto properties = communicator()->getProperties();

    //
    // Warn the user that setting Ice.ThreadPool.Server isn't useful.
    //
    if (!properties->getProperty("Ice.ThreadPool.Server.Size").empty())
    {
        Warning out(communicator()->getLogger());
        out << "setting 'Ice.ThreadPool.Server.Size' is not useful, ";
        out << "you should set individual adapter thread pools instead.";
    }

    auto traceLevels = make_shared<TraceLevels>(communicator(), "IceGrid.Registry");

    _registry = make_shared<RegistryI>(communicator(), traceLevels, readonly, initFromReplica, "");
    if (!_registry->start())
    {
        return false;
    }

    return true;
}

void
RegistryService::waitForShutdown()
{
    //
    // Wait for the activator shutdown. Once the run method returns
    // all the servers have been deactivated.
    //
    enableInterrupt();
    _registry->waitForShutdown();
    disableInterrupt();
}

bool
RegistryService::stop()
{
    _registry->stop();
    return true;
}

CommunicatorPtr
RegistryService::initializeCommunicator(int& argc, char* argv[], InitializationData initData)
{
    initData.properties = createProperties(argc, argv, initData.properties);

    // If IceGrid.Registry.[Admin]PermissionsVerifier is not set and
    // IceGrid.Registry.[Admin]CryptPasswords is set, load the
    // Glacier2CryptPermissionsVerifier plug-in
    //
    vector<string> vTypes;
    vTypes.emplace_back("");
    vTypes.emplace_back("Admin");

    for (const auto& vType : vTypes)
    {
        string verifier = "IceGrid.Registry." + vType + "PermissionsVerifier";

        if (initData.properties->getProperty(verifier).empty())
        {
            string cryptPasswords = initData.properties->getIceProperty("IceGrid.Registry." + vType + "CryptPasswords");

            if (!cryptPasswords.empty())
            {
                initData.pluginFactories.push_back(Glacier2::cryptPermissionsVerifierPluginFactory());

                initData.properties->setProperty(
                    "Glacier2CryptPermissionsVerifier.IceGrid.Registry." + vType + "PermissionsVerifier",
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

    // Turn-off the inactivity timeout for the IceGrid.Registry.Client object adapter unless the application sets this
    // property. That's because the IceGrid.Registry.Client object adapter hosts connection-bound sessions
    // (admin sessions and resource allocation sessions).
    if (initData.properties->getIceProperty("IceGrid.Registry.Client.Connection.InactivityTimeout").empty())
    {
        initData.properties->setProperty("IceGrid.Registry.Client.Connection.InactivityTimeout", "0");
    }

    //
    // Setup the client thread pool size.
    //
    setupThreadPool(initData.properties, "Ice.ThreadPool.Client", 1, 100);

    return Service::initializeCommunicator(argc, argv, std::move(initData));
}

void
RegistryService::usage(const string& appName)
{
    string options = "Options:\n"
                     "-h, --help           Show this message.\n"
                     "-v, --version        Display the Ice version.\n"
                     "--readonly           Start the master registry in read-only mode.\n"
                     "--initdb-from-replica <replica>\n"
                     "                     Initialize the database from the given replica.";
#ifndef _WIN32
    options.append("\n"
                   "\n"
                   "--daemon             Run as a daemon.\n"
                   "--pidfile FILE       Write process ID into FILE.\n"
                   "--noclose            Do not close open file descriptors.\n"
                   "--nochdir            Do not change the current working directory.\n");
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
    RegistryService svc;
    // Initialize the service with a Properties object with the correct property prefix enabled.
    Ice::InitializationData initData;
    initData.properties = make_shared<Properties>(vector<string>{"IceGrid", "IceStorm"});
    return svc.main(argc, argv, std::move(initData));
}
