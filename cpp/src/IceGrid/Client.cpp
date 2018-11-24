// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Options.h>
#include <IceUtil/CtrlCHandler.h>
#include <IceUtil/Thread.h>
#include <IceUtil/StringUtil.h>
#include <Ice/ConsoleUtil.h>
#include <Ice/UUID.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>
#include <Ice/Ice.h>
#include <Ice/SliceChecksums.h>
#include <IceGrid/Parser.h>
#include <IceGrid/FileParserI.h>
#include <IceGrid/Registry.h>
#include <IceLocatorDiscovery/Plugin.h>
#include <Glacier2/Router.h>
#include <fstream>

//
// For getPassword()
//
#ifndef _WIN32
#   include <termios.h>
#else
#   include <conio.h>
#   include <fcntl.h>
#   include <io.h>
#endif

using namespace std;
using namespace IceInternal;

class Client;

namespace
{

IceUtil::Mutex* _staticMutex = 0;
Ice::CommunicatorPtr communicator;
IceGrid::ParserPtr parser;

class Init
{
public:

    Init()
    {
        _staticMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete _staticMutex;
        _staticMutex = 0;
    }
};

Init init;

}

class ReuseConnectionRouter : public Ice::Router
{
public:

    ReuseConnectionRouter(const Ice::ObjectPrx& proxy) : _clientProxy(proxy)
    {
    }

    virtual Ice::ObjectPrx
    getClientProxy(IceUtil::Optional<bool>& hasRoutingTable, const Ice::Current&) const
    {
        hasRoutingTable = false;
        return _clientProxy;
    }

    virtual Ice::ObjectPrx
    getServerProxy(const Ice::Current&) const
    {
        return 0;
    }

    virtual void
    addProxy(const Ice::ObjectPrx&, const Ice::Current&)
    {
    }

    virtual Ice::ObjectProxySeq
    addProxies(const Ice::ObjectProxySeq&, const Ice::Current&)
    {
        return Ice::ObjectProxySeq();
    }

private:

    const Ice::ObjectPrx _clientProxy;
};

int run(const Ice::StringSeq&);

//
// Callback for CtrlCHandler
//
static void
interruptCallback(int /*signal*/)
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(_staticMutex);
    if(parser) // If there's an interactive parser, notify the parser.
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
}

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
        Ice::PropertiesPtr defaultProps = Ice::createProperties();
        defaultProps->setProperty("IceGridAdmin.Server.Endpoints", "tcp -h localhost");
        Ice::InitializationData id;
        id.properties = createProperties(args, defaultProps);
        id.properties->setProperty("Ice.Warn.Endpoints", "0");
        Ice::CommunicatorHolder ich(id);
        communicator = ich.communicator();

        ctrlCHandler.setCallback(interruptCallback);

        try
        {
            status = run(args);
        }
        catch(const Ice::CommunicatorDestroyedException&)
        {
            // Expected if the client is interrupted during the initialization.
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        consoleErr << args[0] << ": " << ex << endl;
        status = 1;
    }
    catch(const std::exception& ex)
    {
        consoleErr << args[0] << ": std::exception: " << ex.what() << endl;
        status = 1;
    }
    catch(...)
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
    consoleErr <<
        "Options:\n"
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
        "-r, --replica NAME   Connect to the replica NAME.\n"
        ;
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
    newConf.c_lflag &= (~ECHO);
    tcsetattr(0, TCSANOW, &newConf);
    getline(cin, password);
    tcsetattr(0, TCSANOW, &oldConf);
#else
    char c;
    while((c = static_cast<char>(_getch())) != '\r')
    {
        password += c;
    }
#endif
    consoleOut << endl;
    return IceUtilInternal::trim(password);
}

extern "C" ICE_LOCATOR_DISCOVERY_API Ice::Plugin*
createIceLocatorDiscovery(const Ice::CommunicatorPtr&, const string&, const Ice::StringSeq&);

int
run(const Ice::StringSeq& args)
{
    string commands;
    bool debug;

    IceUtilInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("e", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("i", "instanceName", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::NoRepeat);
    opts.addOpt("H", "host", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::NoRepeat);
    opts.addOpt("P", "port", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::NoRepeat);
    opts.addOpt("u", "username", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::NoRepeat);
    opts.addOpt("p", "password", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::NoRepeat);
    opts.addOpt("S", "ssl");
    opts.addOpt("d", "debug");
    opts.addOpt("s", "server");
    opts.addOpt("r", "replica", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::NoRepeat);

    try
    {
        if(!opts.parse(args).empty())
        {
            consoleErr << args[0] << ": too many arguments" << endl;
            usage(args[0]);
            return 1;
        }
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        consoleErr << e.reason << endl;
        usage(args[0]);
        return 1;
    }

    if(opts.isSet("help"))
    {
        usage(args[0]);
        return 0;
    }
    if(opts.isSet("version"))
    {
        consoleErr << ICE_STRING_VERSION << endl;
        return 0;
    }

    if(opts.isSet("server"))
    {
        Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("IceGridAdmin.Server");
        adapter->activate();
        Ice::ObjectPrx proxy = adapter->add(new FileParserI, Ice::stringToIdentity("FileParser"));
        consoleOut << proxy << endl;

        communicator->waitForShutdown();
        return 0;
    }

    if(opts.isSet("e"))
    {
        vector<string> optargs = opts.argVec("e");
        for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
        {
            commands += *i + ";";
        }
    }
    debug = opts.isSet("debug");

    bool ssl = communicator->getProperties()->getPropertyAsInt("IceGridAdmin.AuthenticateUsingSSL") > 0;
    if(opts.isSet("ssl"))
    {
        ssl = true;
    }

    string id = communicator->getProperties()->getProperty("IceGridAdmin.Username");
    if(!opts.optArg("username").empty())
    {
        id = opts.optArg("username");
    }
    string password = communicator->getProperties()->getProperty("IceGridAdmin.Password");
    if(!opts.optArg("password").empty())
    {
        password = opts.optArg("password");
    }

    string host = communicator->getProperties()->getProperty("IceGridAdmin.Host");
    if(!opts.optArg("host").empty())
    {
        host = opts.optArg("host");
    }

    string instanceName = communicator->getProperties()->getProperty("IceGridAdmin.InstanceName");
    if(!opts.optArg("instanceName").empty())
    {
        instanceName = opts.optArg("instanceName");
    }

    int port = communicator->getProperties()->getPropertyAsInt("IceGridAdmin.Port");
    if(!opts.optArg("port").empty())
    {
        istringstream is(opts.optArg("port"));
        if(!(is >> port))
        {
            consoleErr << args[0] << ": given port number is not a numeric value" << endl;
            return 1;
        }
    }

    Ice::PropertiesPtr properties = communicator->getProperties();
    string replica = properties->getProperty("IceGridAdmin.Replica");
    if(!opts.optArg("replica").empty())
    {
        replica = opts.optArg("replica");
    }

    Glacier2::RouterPrx router;
    IceGrid::AdminSessionPrx session;
    int status = 0;
    try
    {
        int acmTimeout = 0;
        if(!communicator->getDefaultLocator() && !communicator->getDefaultRouter())
        {
            if(!host.empty())
            {
                const int timeout = 3000; // 3s connection timeout.
                ostringstream os;
                os << "Ice/LocatorFinder" << (ssl ? " -s" : "");
                os << ":tcp -h \"" << host << "\" -p " << (port == 0 ? 4061 : port) << " -t " << timeout;
                os << ":ssl -h \"" << host << "\" -p " << (port == 0 ? 4062 : port) << " -t " << timeout;
                Ice::LocatorFinderPrx finder = Ice::LocatorFinderPrx::uncheckedCast(communicator->stringToProxy(os.str()));
                try
                {
                    communicator->setDefaultLocator(finder->getLocator());
                }
                catch(const Ice::LocalException&)
                {
                    // Ignore.
                }
                if(!instanceName.empty() &&
                   communicator->getDefaultLocator()->ice_getIdentity().category != instanceName)
                {
                    consoleErr << args[0] << ": registry running on `" << host << "' uses a different instance name:\n";
                    consoleErr << communicator->getDefaultLocator()->ice_getIdentity().category << endl;
                    return 1;
                }
            }
            else
            {
                //
                // NOTE: we don't configure the plugin with the Ice communicator on initialization
                // because it would install a default locator. Instead, we create the plugin here
                // to lookup for locator proxies. We destroy the plugin, once we have selected a
                // locator.
                //
                Ice::PluginPtr pluginObj = createIceLocatorDiscovery(communicator, "IceGridAdmin.Discovery", Ice::StringSeq());
                IceLocatorDiscovery::PluginPtr plugin = IceLocatorDiscovery::PluginPtr::dynamicCast(pluginObj);
                plugin->initialize();

                vector<Ice::LocatorPrx> locators = plugin->getLocators(instanceName, IceUtil::Time::milliSeconds(300));
                if(locators.size() > 1)
                {
                    consoleOut << "found " << locators.size() << " Ice locators:" << endl;
                    unsigned int num = 0;
                    for(vector<Ice::LocatorPrx>::const_iterator p = locators.begin(); p != locators.end(); ++p)
                    {
                        consoleOut << ++num << ": proxy = `" << *p << "'" << endl;
                    }

                    num = 0;
                    while(num == 0 && cin.good())
                    {
                        consoleOut << "please enter the locator number to use: " << flush;
                        string line;
                        getline(cin, line);
                        if(!cin.good() || line.empty())
                        {
                            return 1;
                        }
                        line = IceUtilInternal::trim(line);

                        istringstream is(line);
                        is >> num;
                        if(num > locators.size())
                        {
                            num = 0;
                        }
                    }

                    assert(num <= locators.size());
                    communicator->setDefaultLocator(locators[num - 1]);
                }
                else if(locators.size() == 1)
                {
                    consoleOut << "using discovered locator:\nproxy = `" << locators[0] << "'" << endl;
                    communicator->setDefaultLocator(locators[0]);
                }
                else
                {
                    communicator->setDefaultLocator(0);
                }

                //
                // Destroy the plugin, we no longer need it.
                //
                plugin->destroy();
            }
        }

        if(communicator->getDefaultRouter())
        {
            try
            {
                // Use SSL if available.
                router = Glacier2::RouterPrx::checkedCast(communicator->getDefaultRouter()->ice_preferSecure(true));
                if(!router)
                {
                    consoleErr << args[0] << ": configured router is not a Glacier2 router" << endl;
                    return 1;
                }
            }
            catch(const Ice::LocalException& ex)
            {
                consoleErr << args[0] << ": could not contact the default router:" << endl << ex << endl;
                return 1;
            }

            if(ssl)
            {
                session = IceGrid::AdminSessionPrx::uncheckedCast(router->createSessionFromSecureConnection());
                if(!session)
                {
                    consoleErr << args[0]
                         << ": Glacier2 returned a null session, please set the Glacier2.SSLSessionManager property"
                         << endl;
                    return 1;
                }
            }
            else
            {
                while(id.empty() && cin.good())
                {
                    consoleOut << "user id: " << flush;
                    getline(cin, id);
                    if(!cin.good())
                    {
                        return 1;
                    }
                    id = IceUtilInternal::trim(id);
                }

                if(password.empty())
                {
                    password = getPassword("password: ");
#ifndef _WIN32
                    if(!cin.good())
                    {
                        return 1;
                    }
#endif
                }

                session = IceGrid::AdminSessionPrx::uncheckedCast(router->createSession(id, password));
                fill(password.begin(), password.end(), '\0'); // Zero the password string.

                if(!session)
                {
                    consoleErr << args[0]
                         << ": Glacier2 returned a null session, please set the Glacier2.SessionManager property"
                         << endl;
                    return 1;
                }
            }

            try
            {
                acmTimeout = router->getACMTimeout();
            }
            catch(const Ice::OperationNotExistException&)
            {
                consoleErr << args[0] << ": can't talk to old Glacier2 router version" << endl;
                return 1;
            }
        }
        else if(communicator->getDefaultLocator())
        {
            //
            // Create the identity of the registry to connect to.
            //
            Ice::Identity registryId;
            registryId.category = communicator->getDefaultLocator()->ice_getIdentity().category;
            registryId.name = "Registry";
            if(!replica.empty() && replica != "Master")
            {
                registryId.name += "-" + replica;
            }

            //
            // First try to contact the locator. If we can't talk to the locator,
            // no need to go further. Otherwise, we get the proxy of local registry
            // proxy.
            //
            IceGrid::LocatorPrx locator;
            IceGrid::RegistryPrx localRegistry;
            try
            {
                locator = IceGrid::LocatorPrx::checkedCast(communicator->getDefaultLocator());
                if(!locator)
                {
                    consoleErr << args[0] << ": configured locator is not an IceGrid locator" << endl;
                    return 1;
                }
                localRegistry = locator->getLocalRegistry();
            }
            catch(const Ice::LocalException& ex)
            {
                consoleErr << args[0] << ": could not contact the default locator:" << endl << ex << endl;
                return 1;
            }

            IceGrid::RegistryPrx registry;
            if(localRegistry->ice_getIdentity() == registryId)
            {
                registry = localRegistry;
            }
            else
            {
                //
                // The locator local registry isn't the registry we want to connect to.
                //

                try
                {
                    registry = IceGrid::RegistryPrx::checkedCast(locator->findObjectById(registryId));
                    if(!registry)
                    {
                        consoleErr << args[0] << ": could not contact an IceGrid registry" << endl;
                    }
                }
                catch(const Ice::ObjectNotFoundException&)
                {
                    consoleErr << args[0] << ": no active registry replica named `" << replica << "'" << endl;
                    return 1;
                }
                catch(const Ice::LocalException& ex)
                {
                    if(!replica.empty())
                    {
                        consoleErr << args[0] << ": could not contact the registry replica named `" << replica << "':\n";
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
                        if(pos != string::npos)
                        {
                            name = name.substr(prefix.size());
                        }
                        consoleErr << args[0] << ": warning: could not contact master, using slave `" << name << "'" << endl;
                    }
                }
            }

            //
            // If the registry to use is the locator local registry, we install a default router
            // to ensure we'll use a single connection regardless of the endpoints returned in the
            // proxies of the various session/admin methods (useful if used over an ssh tunnel).
            //
            if(registry->ice_getIdentity() == localRegistry->ice_getIdentity())
            {
                Ice::ObjectAdapterPtr colloc = communicator->createObjectAdapter(""); // colloc-only adapter
                communicator->setDefaultRouter(Ice::RouterPrx::uncheckedCast(
                    colloc->addWithUUID(new ReuseConnectionRouter(locator))));
                registry = registry->ice_router(communicator->getDefaultRouter());
            }

            // Prefer SSL.
            registry = registry->ice_preferSecure(true);

            if(ssl)
            {
                session = registry->createAdminSessionFromSecureConnection();
            }
            else
            {
                while(id.empty() && cin.good())
                {
                    consoleOut << "user id: " << flush;
                    getline(cin, id);
                    if(!cin.good())
                    {
                        return 1;
                    }
                    id = IceUtilInternal::trim(id);
                }

                if(password.empty())
                {
                    password = getPassword("password: ");
#ifndef _WIN32
                    if(!cin.good())
                    {
                        return 1;
                    }
#endif
                }

                session = registry->createAdminSession(id, password);
                fill(password.begin(), password.end(), '\0'); // Zero the password string.
            }

            try
            {
                acmTimeout = registry->getACMTimeout();
            }
            catch(const Ice::OperationNotExistException&)
            {
                consoleErr << args[0] << ": can't talk to old IceGrid registry version" << endl;
                return 1;
            }
        }
        else // No default locator or router set.
        {
            consoleErr << args[0] << ": could not contact the registry:" << endl;
            consoleErr << "no default locator or router configured" << endl;
            return 1;
        }

        if(acmTimeout > 0)
        {
            session->ice_getConnection()->setACM(acmTimeout, IceUtil::None, Ice::ICE_ENUM(ACMHeartbeat, HeartbeatAlways));
        }

        {
            IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(_staticMutex);
            parser = IceGrid::Parser::createParser(communicator, session, session->getAdmin(), commands.empty());
        }

        if(!commands.empty()) // Commands were given
        {
            int parseStatus = parser->parse(commands, debug);
            if(parseStatus == 1)
            {
                status = 1;
            }
        }
        else // No commands, let's use standard input
        {
            parser->showBanner();

            int parseStatus = parser->parse(stdin, debug);
            if(parseStatus == 1)
            {
                status = 1;
            }
        }
    }
    catch(const IceGrid::PermissionDeniedException& ex)
    {
        consoleOut << "permission denied:\n" << ex.reason << endl;
        return 1;
    }
    catch(const Glacier2::PermissionDeniedException& ex)
    {
        consoleOut << "permission denied:\n" << ex.reason << endl;
        return 1;
    }
    catch(const Glacier2::CannotCreateSessionException& ex)
    {
        consoleOut << "session creation failed:\n" << ex.reason << endl;
        return 1;
    }
    catch(...)
    {
        try
        {
            if(router)
            {
                router->destroySession();
            }
            else if(session)
            {
                session->destroy();
            }
        }
        catch(const Ice::Exception&)
        {
        }
        throw;
    }

    if(session)
    {
        try
        {
            if(router)
            {
                router->destroySession();
            }
            else
            {
                session->destroy();
            }
        }
        catch(const Ice::Exception&)
        {
            // Ignore. If the registry has been shutdown this will cause
            // an exception.
        }
    }

    return status;
}
