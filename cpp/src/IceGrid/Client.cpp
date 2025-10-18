// Copyright (c) ZeroC, Inc.

#include "../Ice/ConsoleUtil.h"
#include "../Ice/Options.h"
#include "../IceLocatorDiscovery/Plugin.h"
#include "FileParserI.h"
#include "Glacier2/Router.h"
#include "Ice/Ice.h"
#include "Ice/StringUtil.h"
#include "Ice/UUID.h"
#include "IceGrid/Registry.h"
#include "Parser.h"

#include <fstream>
#include <iostream>

//
// For getPassword()
//
#ifndef _WIN32
#    include <termios.h>
#else
#    include <conio.h>
#    include <fcntl.h>
#    include <io.h>
#endif

using namespace std;
using namespace IceInternal;

class Client;

namespace
{
    mutex staticMutex;
    shared_ptr<Ice::Communicator> communicator;
    shared_ptr<IceGrid::Parser> parser;

};

class ReuseConnectionRouter final : public Ice::Router
{
public:
    ReuseConnectionRouter(Ice::ObjectPrx proxy) : _clientProxy(std::move(proxy)) {}

    optional<Ice::ObjectPrx> getClientProxy(optional<bool>& hasRoutingTable, const Ice::Current&) const override
    {
        hasRoutingTable = false;
        return _clientProxy;
    }

    [[nodiscard]] optional<Ice::ObjectPrx> getServerProxy(const Ice::Current&) const override { return nullopt; }

    Ice::ObjectProxySeq addProxies(Ice::ObjectProxySeq, const Ice::Current&) override { return {}; }

private:
    const Ice::ObjectPrx _clientProxy;
};

int run(const Ice::StringSeq&);

int
#ifdef _WIN32
wmain(int argc, wchar_t* argv[])
{
    //
    // Enable binary input mode for stdin to avoid automatic conversions.
    //
    _setmode(_fileno(stdin), _O_BINARY);
#else
main(int argc, char* argv[])
{
#endif
    int status = 0;
    Ice::StringSeq args = Ice::argsToStringSeq(argc, argv);

    try
    {
        Ice::CtrlCHandler ctrlCHandler;
        // Initialize IceGrid properties with a Properties object with the correct property prefix enabled.
        auto defaultProps = make_shared<Ice::Properties>("IceGridAdmin");
        defaultProps->setProperty("IceGridAdmin.Server.Endpoints", "tcp -h 127.0.0.1");

        // Turn-off inactivity timeout for outgoing connections.
        defaultProps->setProperty("Ice.Connection.Client.InactivityTimeout", "0");

        Ice::InitializationData id;
        id.properties = createProperties(args, defaultProps);
        id.properties->setProperty("Ice.Warn.Endpoints", "0");
        Ice::CommunicatorHolder ich(id);
        communicator = ich.communicator();

        ctrlCHandler.setCallback(
            [](int)
            {
                lock_guard lg(staticMutex);
                if (parser) // If there's an interactive parser, notify the parser.
                {
                    parser->interrupt();
                }
                else
                {
                    //
                    // Otherwise, destroy the communicator.
                    //
                    assert(communicator);
                    communicator->destroy();
                }
            });

        try
        {
            status = run(args);
        }
        catch (const Ice::CommunicatorDestroyedException&)
        {
            // Expected if the client is interrupted during the initialization.
        }
    }
    catch (const Ice::Exception& ex)
    {
        consoleErr << args[0] << ": " << ex << endl;
        status = 1;
    }
    catch (const std::exception& ex)
    {
        consoleErr << args[0] << ": std::exception: " << ex.what() << endl;
        status = 1;
    }
    catch (...)
    {
        consoleErr << args[0] << ": unknown exception" << endl;
        status = 1;
    }

    return status;
}

void
usage(const string& name)
{
    consoleErr << "Usage: " << name << " [options]\n";
    consoleErr << "Options:\n"
                  "-h, --help           Show this message.\n"
                  "-v, --version        Display the Ice version.\n"
                  "-e COMMANDS          Execute COMMANDS.\n"
                  "-d, --debug          Print debug messages.\n"
                  "-s, --server         Start icegridadmin as a server (to parse XML files).\n"
                  "-i, --instanceName   Connect to the registry with the given instance name.\n"
                  "-H, --host           Connect to the registry at the given host.\n"
                  "-P, --port           Connect to the registry running on the given port.\n"
                  "-u, --username       Login with the given username.\n"
                  "-p, --password       Login with the given password.\n"
                  "-S, --ssl            Authenticate through SSL.\n"
                  "-r, --replica NAME   Connect to the replica NAME.\n";
}

string
getPassword(const string& prompt)
{
    consoleOut << prompt << flush;
    string password;
#ifndef _WIN32
    struct termios oldConf;
    struct termios newConf;
    tcgetattr(0, &oldConf);
    newConf = oldConf;
    newConf.c_lflag &= static_cast<tcflag_t>(~(ECHO));
    tcsetattr(0, TCSANOW, &newConf);
    getline(cin, password);
    tcsetattr(0, TCSANOW, &oldConf);
#else
    char c;
    while ((c = static_cast<char>(_getch())) != '\r')
    {
        password += c;
    }
#endif
    consoleOut << endl;
    return IceInternal::trim(password);
}

int
run(const Ice::StringSeq& args)
{
    string commands;
    bool debug;

    IceInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("e", "", IceInternal::Options::NeedArg, "", IceInternal::Options::Repeat);
    opts.addOpt("i", "instanceName", IceInternal::Options::NeedArg, "", IceInternal::Options::NoRepeat);
    opts.addOpt("H", "host", IceInternal::Options::NeedArg, "", IceInternal::Options::NoRepeat);
    opts.addOpt("P", "port", IceInternal::Options::NeedArg, "", IceInternal::Options::NoRepeat);
    opts.addOpt("u", "username", IceInternal::Options::NeedArg, "", IceInternal::Options::NoRepeat);
    opts.addOpt("p", "password", IceInternal::Options::NeedArg, "", IceInternal::Options::NoRepeat);
    opts.addOpt("S", "ssl");
    opts.addOpt("d", "debug");
    opts.addOpt("s", "server");
    opts.addOpt("r", "replica", IceInternal::Options::NeedArg, "", IceInternal::Options::NoRepeat);

    try
    {
        if (!opts.parse(args).empty())
        {
            consoleErr << args[0] << ": too many arguments" << endl;
            usage(args[0]);
            return 1;
        }
    }
    catch (const IceInternal::BadOptException& e)
    {
        consoleErr << e.what() << endl;
        usage(args[0]);
        return 1;
    }

    if (opts.isSet("help"))
    {
        usage(args[0]);
        return 0;
    }
    if (opts.isSet("version"))
    {
        consoleOut << ICE_STRING_VERSION << endl;
        return 0;
    }

    if (opts.isSet("server"))
    {
        auto adapter = communicator->createObjectAdapter("IceGridAdmin.Server");
        adapter->activate();
        auto proxy = adapter->add(make_shared<IceGrid::FileParserI>(), Ice::stringToIdentity("FileParser"));
        consoleOut << proxy << endl;

        communicator->waitForShutdown();
        return 0;
    }

    if (opts.isSet("e"))
    {
        for (const auto& arg : opts.argVec("e"))
        {
            commands += arg + ";";
        }
    }
    debug = opts.isSet("debug");

    bool ssl = communicator->getProperties()->getIcePropertyAsInt("IceGridAdmin.AuthenticateUsingSSL") > 0;
    if (opts.isSet("ssl"))
    {
        ssl = true;
    }

    string id = communicator->getProperties()->getIceProperty("IceGridAdmin.Username");
    if (!opts.optArg("username").empty())
    {
        id = opts.optArg("username");
    }
    string password = communicator->getProperties()->getIceProperty("IceGridAdmin.Password");
    if (!opts.optArg("password").empty())
    {
        password = opts.optArg("password");
    }

    string host = communicator->getProperties()->getIceProperty("IceGridAdmin.Host");
    if (!opts.optArg("host").empty())
    {
        host = opts.optArg("host");
    }

    string instanceName = communicator->getProperties()->getIceProperty("IceGridAdmin.InstanceName");
    if (!opts.optArg("instanceName").empty())
    {
        instanceName = opts.optArg("instanceName");
    }

    int port = communicator->getProperties()->getIcePropertyAsInt("IceGridAdmin.Port");
    if (!opts.optArg("port").empty())
    {
        istringstream is(opts.optArg("port"));
        if (!(is >> port))
        {
            consoleErr << args[0] << ": given port number is not a numeric value" << endl;
            return 1;
        }
    }

    auto properties = communicator->getProperties();
    string replica = properties->getIceProperty("IceGridAdmin.Replica");
    if (!opts.optArg("replica").empty())
    {
        replica = opts.optArg("replica");
    }

    optional<IceGrid::AdminSessionPrx> session;
    optional<Glacier2::RouterPrx> router;
    int status = 0;
    try
    {
        if (!communicator->getDefaultLocator() && !communicator->getDefaultRouter())
        {
            if (!host.empty())
            {
                ostringstream os;
                os << "Ice/LocatorFinder" << (ssl ? " -s" : "");
                os << ":tcp -h \"" << host << "\" -p " << (port == 0 ? 4061 : port);
                os << ":ssl -h \"" << host << "\" -p " << (port == 0 ? 4062 : port);
                Ice::LocatorFinderPrx finder{communicator, os.str()};
                try
                {
                    communicator->setDefaultLocator(finder->getLocator());
                }
                catch (const Ice::LocalException&)
                {
                    // Ignore.
                }
                if (!instanceName.empty() &&
                    communicator->getDefaultLocator()->ice_getIdentity().category != instanceName)
                {
                    consoleErr << args[0] << ": registry running on '" << host << "' uses a different instance name:\n";
                    consoleErr << communicator->getDefaultLocator()->ice_getIdentity().category << endl;
                    return 1;
                }
            }
            else
            {
                // NOTE: we don't configure the plugin with the Ice communicator on initialization because it would
                // install a default locator. Instead, we create the plugin here to lookup for locator proxies. We
                // destroy the plugin, once we have selected a locator.
                Ice::PluginFactory pluginFactory = IceLocatorDiscovery::locatorDiscoveryPluginFactory();

                shared_ptr<Ice::Plugin> pluginObj{
                    pluginFactory.factoryFunc(communicator, pluginFactory.pluginName, Ice::StringSeq())};

                auto plugin = dynamic_pointer_cast<IceLocatorDiscovery::Plugin>(pluginObj);
                plugin->initialize();

                auto locators = plugin->getLocators(instanceName, chrono::milliseconds(300));
                if (locators.size() > 1)
                {
                    consoleOut << "found " << locators.size() << " Ice locators:" << endl;
                    unsigned int num = 0;
                    for (const auto& locator : locators)
                    {
                        consoleOut << ++num << ": proxy = '" << locator << "'" << endl;
                    }

                    num = 0;
                    while (num == 0 && cin.good())
                    {
                        consoleOut << "please enter the locator number to use: " << flush;
                        string line;
                        getline(cin, line);
                        if (!cin.good() || line.empty())
                        {
                            return 1;
                        }
                        line = IceInternal::trim(line);

                        istringstream is(line);
                        is >> num;
                        if (num > locators.size())
                        {
                            num = 0;
                        }
                    }

                    assert(num <= locators.size());
                    communicator->setDefaultLocator(locators[num - 1]);
                }
                else if (locators.size() == 1)
                {
                    consoleOut << "using discovered locator:\nproxy = '" << locators[0] << "'" << endl;
                    communicator->setDefaultLocator(locators[0]);
                }
                else
                {
                    communicator->setDefaultLocator(nullopt);
                }

                //
                // Destroy the plugin, we no longer need it.
                //
                plugin->destroy();
            }
        }

        if (communicator->getDefaultRouter())
        {
            // Use SSL if available.
            router = Ice::uncheckedCast<Glacier2::RouterPrx>(communicator->getDefaultRouter());
            if (ssl)
            {
                session = Ice::uncheckedCast<IceGrid::AdminSessionPrx>(router->createSessionFromSecureConnection());
                if (!session)
                {
                    consoleErr
                        << args[0]
                        << ": Glacier2 returned a null session, please set the Glacier2.SSLSessionManager property"
                        << endl;
                    return 1;
                }
            }
            else
            {
                while (id.empty() && cin.good())
                {
                    consoleOut << "user id: " << flush;
                    getline(cin, id);
                    if (!cin.good())
                    {
                        return 1;
                    }
                    id = IceInternal::trim(id);
                }

                if (password.empty())
                {
                    password = getPassword("password: ");
#ifndef _WIN32
                    if (!cin.good())
                    {
                        return 1;
                    }
#endif
                }

                session = Ice::uncheckedCast<IceGrid::AdminSessionPrx>(router->createSession(id, password));
                fill(password.begin(), password.end(), '\0'); // Zero the password string.

                if (!session)
                {
                    consoleErr << args[0]
                               << ": Glacier2 returned a null session, please set the Glacier2.SessionManager property"
                               << endl;
                    return 1;
                }
            }
        }
        else if (communicator->getDefaultLocator())
        {
            //
            // Create the identity of the registry to connect to.
            //
            Ice::Identity registryId;
            registryId.category = communicator->getDefaultLocator()->ice_getIdentity().category;
            registryId.name = "Registry";
            if (!replica.empty() && replica != "Master")
            {
                registryId.name += "-" + replica;
            }

            //
            // First try to contact the locator. If we can't talk to the locator,
            // no need to go further. Otherwise, we get the proxy of local registry
            // proxy.
            //
            auto locator = Ice::uncheckedCast<IceGrid::LocatorPrx>(*communicator->getDefaultLocator());
            optional<IceGrid::RegistryPrx> localRegistry;
            try
            {
                localRegistry = locator->getLocalRegistry();
            }
            catch (const Ice::LocalException& ex)
            {
                consoleErr << args[0] << ": could not contact the default locator:" << endl << ex << endl;
                return 1;
            }

            optional<IceGrid::RegistryPrx> registry;
            if (localRegistry->ice_getIdentity() == registryId)
            {
                registry = localRegistry;
            }
            else
            {
                // The locator local registry isn't the registry we want to connect to.
                try
                {
                    registry = Ice::uncheckedCast<IceGrid::RegistryPrx>(locator->findObjectById(registryId));
                    if (!registry)
                    {
                        consoleErr << args[0] << ": could not contact an IceGrid registry" << endl;
                        return 1;
                    }
                }
                catch (const Ice::ObjectNotFoundException&)
                {
                    consoleErr << args[0] << ": no active registry replica named '" << replica << "'" << endl;
                    return 1;
                }
                catch (const Ice::LocalException& ex)
                {
                    if (!replica.empty())
                    {
                        consoleErr << args[0] << ": could not contact the registry replica named '" << replica
                                   << "':\n";
                        consoleErr << ex << endl;
                        return 1;
                    }
                    else
                    {
                        //
                        // If we can't contact the master, use the local registry.
                        //
                        registry = localRegistry;
                        string name = registry->ice_getIdentity().name;
                        const string prefix("Registry-");
                        string::size_type pos = name.find(prefix);
                        if (pos != string::npos)
                        {
                            name = name.substr(prefix.size());
                        }
                        consoleErr << args[0] << ": warning: could not contact master, using slave '" << name << "'"
                                   << endl;
                    }
                }
            }
            assert(registry);

            //
            // If the registry to use is the locator local registry, we install a default router
            // to ensure we'll use a single connection regardless of the endpoints returned in the
            // proxies of the various session/admin methods (useful if used over an ssh tunnel).
            //
            if (registry->ice_getIdentity() == localRegistry->ice_getIdentity())
            {
                auto colloc = communicator->createObjectAdapter(""); // colloc-only adapter
                communicator->setDefaultRouter(
                    colloc->addWithUUID<Ice::RouterPrx>(make_shared<ReuseConnectionRouter>(locator)));
                registry = registry->ice_router(communicator->getDefaultRouter());
            }

            if (ssl)
            {
                session = registry->createAdminSessionFromSecureConnection();
                if (!session)
                {
                    consoleErr << args[0] << ": registry returned a null admin session" << endl;
                    return 1;
                }
            }
            else
            {
                while (id.empty() && cin.good())
                {
                    consoleOut << "user id: " << flush;
                    getline(cin, id);
                    if (!cin.good())
                    {
                        return 1;
                    }
                    id = IceInternal::trim(id);
                }

                if (password.empty())
                {
                    password = getPassword("password: ");
#ifndef _WIN32
                    if (!cin.good())
                    {
                        return 1;
                    }
#endif
                }

                session = registry->createAdminSession(id, password);
                fill(password.begin(), password.end(), '\0'); // Zero the password string.
                if (!session)
                {
                    consoleErr << args[0] << ": registry returned a null admin session" << endl;
                    return 1;
                }
            }
        }
        else // No default locator or router set.
        {
            consoleErr << args[0] << ": could not contact the registry:" << endl;
            consoleErr << "no default locator or router configured" << endl;
            return 1;
        }

        {
            lock_guard lock(staticMutex);
            parser = make_shared<IceGrid::Parser>(communicator, *session, session->getAdmin(), commands.empty());
        }

        if (!commands.empty()) // Commands were given
        {
            int parseStatus = parser->parse(commands, debug);
            if (parseStatus == 1)
            {
                status = 1;
            }
        }
        else // No commands, let's use standard input
        {
            parser->showBanner();

            int parseStatus = parser->parse(stdin, debug);
            if (parseStatus == 1)
            {
                status = 1;
            }
        }
    }
    catch (const IceGrid::PermissionDeniedException& ex)
    {
        consoleOut << "permission denied:\n" << ex.reason << endl;
        return 1;
    }
    catch (const Glacier2::PermissionDeniedException& ex)
    {
        consoleOut << "permission denied:\n" << ex.reason << endl;
        return 1;
    }
    catch (const Glacier2::CannotCreateSessionException& ex)
    {
        consoleOut << "session creation failed:\n" << ex.reason << endl;
        return 1;
    }
    catch (...)
    {
        try
        {
            if (router)
            {
                router->destroySession();
            }
            else if (session)
            {
                session->destroy();
            }
        }
        catch (const Ice::Exception&)
        {
        }
        throw;
    }

    if (session)
    {
        try
        {
            if (router)
            {
                router->destroySession();
            }
            else
            {
                session->destroy();
            }
        }
        catch (const Ice::Exception&)
        {
            // Ignore. If the registry has been shutdown this will cause
            // an exception.
        }
    }

    return status;
}
