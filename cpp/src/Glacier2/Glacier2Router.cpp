// Copyright (c) ZeroC, Inc.

#include "../Glacier2Lib/NullPermissionsVerifier.h"
#include "../Ice/ConsoleUtil.h"
#include "../Ice/Options.h"
#include "CryptPermissionsVerifier.h"
#include "Glacier2/Session.h"
#include "Ice/Ice.h"
#include "Instance.h"
#include "RouterI.h"
#include "SessionRouterI.h"

using namespace std;
using namespace Ice;
using namespace Glacier2;
using namespace IceInternal;

namespace
{
    class ClientLocator final : public ServantLocator
    {
    public:
        ClientLocator(shared_ptr<SessionRouterI> sessionRouter) : _sessionRouter(std::move(sessionRouter)) {}

        ObjectPtr locate(const Current& current, shared_ptr<void>&) final
        {
            return _sessionRouter->getClientBlobject(current.con, current.id);
        }

        void finished(const Current&, const ObjectPtr&, const shared_ptr<void>&) final {}

        void deactivate(string_view) final {}

    private:
        const shared_ptr<SessionRouterI> _sessionRouter;
    };

    class ServerLocator final : public ServantLocator
    {
    public:
        ServerLocator(shared_ptr<SessionRouterI> sessionRouter) : _sessionRouter(std::move(sessionRouter)) {}

        ObjectPtr locate(const Current& current, shared_ptr<void>&) final
        {
            return _sessionRouter->getServerBlobject(current.id.category);
        }

        void finished(const Current&, const ObjectPtr&, const shared_ptr<void>&) final {}

        void deactivate(string_view) final {}

    private:
        const std::shared_ptr<SessionRouterI> _sessionRouter;
    };

    class RouterService : public Service
    {
    public:
        RouterService();

    protected:
        bool start(int, char*[], int&) override;
        bool stop() override;
        CommunicatorPtr initializeCommunicator(int&, char*[], InitializationData) override;

    private:
        void usage(const std::string&);

        shared_ptr<Glacier2::Instance> _instance;
        shared_ptr<SessionRouterI> _sessionRouter;
    };

    class FinderI : public Ice::RouterFinder
    {
    public:
        FinderI(Glacier2::RouterPrx router) : _router(std::move(router)) {}

        optional<Ice::RouterPrx> getRouter(const Ice::Current&) override { return _router; }

    private:
        const Glacier2::RouterPrx _router;
    };

};

RouterService::RouterService() = default;

bool
RouterService::start(int argc, char* argv[], int& status)
{
    IceInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");

    vector<string> args;
    try
    {
        args = opts.parse(argc, argv);
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

    if (!args.empty())
    {
        consoleErr << argv[0] << ": too many arguments" << endl;
        usage(argv[0]);
        return false;
    }

    auto properties = communicator()->getProperties();

    //
    // Initialize the client object adapter.
    //
    const string clientEndpointsProperty = "Glacier2.Client.Endpoints";
    if (properties->getIceProperty(clientEndpointsProperty).empty())
    {
        error("property '" + clientEndpointsProperty + "' is not set");
        return false;
    }

    auto clientAdapter = communicator()->createObjectAdapter("Glacier2.Client");

    //
    // Initialize the server object adapter only if server endpoints
    // are defined.
    //
    const string serverEndpointsProperty = "Glacier2.Server.Endpoints";
    ObjectAdapterPtr serverAdapter;
    if (!properties->getIceProperty(serverEndpointsProperty).empty())
    {
        serverAdapter = communicator()->createObjectAdapter("Glacier2.Server");
    }

    string instanceName = communicator()->getProperties()->getIceProperty("Glacier2.InstanceName");

    vector<string> verifierProperties;
    verifierProperties.emplace_back("Glacier2.PermissionsVerifier");
    verifierProperties.emplace_back("Glacier2.SSLPermissionsVerifier");

    try
    {
        Glacier2Internal::setupNullPermissionsVerifier(communicator(), instanceName, verifierProperties);
    }
    catch (const std::exception& ex)
    {
        ServiceError err(this);
        err << "unable to setup null permissions verifier:\n" << ex;
        return false;
    }

    const string verifierProperty = verifierProperties[0];
    optional<PermissionsVerifierPrx> verifier;
    try
    {
        verifier = communicator()->propertyToProxy<PermissionsVerifierPrx>(verifierProperty);
        if (verifier)
        {
            verifier->ice_ping();
        }
    }
    catch (const Ice::Exception& ex)
    {
        ServiceWarning warn(this);
        warn << "unable to contact permissions verifier '" << properties->getIceProperty(verifierProperty) << "'\n"
             << ex;
    }
    catch (const std::exception& ex)
    {
        ServiceError err(this);
        err << "permissions verifier '" << properties->getIceProperty(verifierProperty) << "' is invalid:\n" << ex;
        return false;
    }

    //
    // Get the session manager if specified.
    //
    const string sessionManagerProperty = "Glacier2.SessionManager";
    optional<SessionManagerPrx> sessionManager;
    try
    {
        sessionManager = communicator()->propertyToProxy<SessionManagerPrx>(sessionManagerProperty);
        if (sessionManager)
        {
            sessionManager->ice_ping();
            sessionManager = sessionManager->ice_connectionCached(false)->ice_locatorCacheTimeout(
                properties->getPropertyAsIntWithDefault("Glacier2.SessionManager.LocatorCacheTimeout", 600));
        }
    }
    catch (const Ice::Exception& ex)
    {
        ServiceWarning warn(this);
        warn << "unable to contact session manager '" << properties->getIceProperty(sessionManagerProperty) << "'\n"
             << ex;
    }
    catch (const std::exception& ex)
    {
        ServiceError err(this);
        err << "session manager '" << properties->getIceProperty(sessionManagerProperty) << "' is invalid:\n" << ex;
        return false;
    }

    //
    // Check for an SSL permissions verifier.
    //
    const string sslVerifierProperty = verifierProperties[1];
    optional<SSLPermissionsVerifierPrx> sslVerifier;
    try
    {
        sslVerifier = communicator()->propertyToProxy<SSLPermissionsVerifierPrx>(sslVerifierProperty);
        if (sslVerifier)
        {
            sslVerifier->ice_ping();
        }
    }
    catch (const Ice::Exception& ex)
    {
        ServiceWarning warn(this);
        warn << "unable to contact ssl permissions verifier '" << properties->getIceProperty(sslVerifierProperty)
             << "'\n"
             << ex;
    }
    catch (const std::exception& ex)
    {
        ServiceError err(this);
        err << "ssl permissions verifier '" << communicator()->getProperties()->getIceProperty(sslVerifierProperty)
            << "' is invalid:\n"
            << ex;
        return false;
    }

    if (!verifier && !sslVerifier)
    {
        error("Glacier2 requires a permissions verifier or password file");
        return false;
    }

    //
    // Get the SSL session manager if specified.
    //
    const string sslSessionManagerProperty = "Glacier2.SSLSessionManager";
    optional<SSLSessionManagerPrx> sslSessionManager;
    try
    {
        sslSessionManager = communicator()->propertyToProxy<SSLSessionManagerPrx>(sslSessionManagerProperty);
        if (sslSessionManager)
        {
            sslSessionManager->ice_ping();
            sslSessionManager = sslSessionManager->ice_connectionCached(false)->ice_locatorCacheTimeout(
                properties->getPropertyAsIntWithDefault("Glacier2.SSLSessionManager.LocatorCacheTimeout", 600));
        }
    }
    catch (const Ice::Exception& ex)
    {
        ServiceWarning warn(this);
        warn << "unable to contact ssl session manager '" << properties->getIceProperty(sslSessionManagerProperty)
             << "'\n"
             << ex;
    }
    catch (const std::exception& ex)
    {
        ServiceError err(this);
        err << "ssl session manager '" << properties->getIceProperty(sslSessionManagerProperty) << "' is invalid:\n"
            << ex;
        return false;
    }

    //
    // Create the instance object.
    //
    try
    {
        _instance = make_shared<Glacier2::Instance>(communicator(), clientAdapter, serverAdapter);
    }
    catch (const Ice::InitializationException& ex)
    {
        error("Glacier2 initialization failed:\n" + string{ex.what()});
        return false;
    }

    _sessionRouter = make_shared<SessionRouterI>(
        _instance,
        std::move(verifier),
        std::move(sessionManager),
        std::move(sslVerifier),
        std::move(sslSessionManager));

    //
    // Registers session router and all required servant locators
    //
    try
    {
        //
        // All other calls on the client object adapter are dispatched to
        // a router servant based on connection information.
        //
        _instance->clientObjectAdapter()->addServantLocator(make_shared<ClientLocator>(_sessionRouter), "");

        //
        // If there is a server object adapter, all calls on this adapter
        // are dispatched to a router servant based on the category field
        // of the identity.
        //
        if (_instance->serverObjectAdapter())
        {
            _instance->serverObjectAdapter()->addServantLocator(make_shared<ServerLocator>(_sessionRouter), "");
        }
    }
    catch (const Ice::ObjectAdapterDestroyedException&)
    {
        // Ignore.
    }

    _instance->setSessionRouter(_sessionRouter);

    //
    // The session router is used directly as a servant for the main
    // Glacier2 router Ice object.
    //
    auto routerPrx = clientAdapter->add<Glacier2::RouterPrx>(_sessionRouter, {"router", instanceName});

    //
    // Add the Ice router finder object to allow retrieving the router
    // proxy with just the endpoint information of the router.
    //
    clientAdapter->add(make_shared<FinderI>(routerPrx), {"RouterFinder", "Ice"});

    if (_instance->getObserver())
    {
        _instance->getObserver()->setObserverUpdater(_sessionRouter);
    }

    //
    // Everything ok, let's go.
    //
    try
    {
        clientAdapter->activate();
        if (serverAdapter)
        {
            serverAdapter->activate();
        }
    }
    catch (const std::exception& ex)
    {
        {
            ServiceError err(this);
            err << "caught exception activating object adapters\n" << ex;
        }

        stop();
        return false;
    }

    return true;
}

bool
RouterService::stop()
{
    if (_sessionRouter)
    {
        _sessionRouter->destroy();
        _sessionRouter = nullptr;
    }

    if (_instance)
    {
        if (_instance->getObserver())
        {
            _instance->getObserver()->setObserverUpdater(nullptr);
        }
        _instance->destroy();
        _instance = nullptr;
    }
    return true;
}

CommunicatorPtr
RouterService::initializeCommunicator(int& argc, char* argv[], InitializationData initData)
{
    initData.properties = createProperties(argc, argv, initData.properties);

    //
    // Make sure that Glacier2 doesn't use a router.
    //
    initData.properties->setProperty("Ice.Default.Router", "");

    // Turn-off the inactivity timeout for the Glacier2.Client object adapter unless the application sets this property.
    if (initData.properties->getProperty("Glacier2.Client.Connection.InactivityTimeout").empty())
    {
        initData.properties->setProperty("Glacier2.Client.Connection.InactivityTimeout", "0");
    }

    //
    // If Glacier2.PermissionsVerifier is not set and Glacier2.CryptPasswords is set,
    // load the Glacier2CryptPermissionsVerifier plug-in
    //
    if (initData.properties->getIceProperty("Glacier2.PermissionsVerifier").empty())
    {
        string cryptPasswords = initData.properties->getIceProperty("Glacier2.CryptPasswords");

        if (!cryptPasswords.empty())
        {
            initData.pluginFactories.push_back(Glacier2::cryptPermissionsVerifierPluginFactory());

            initData.properties->setProperty(
                "Glacier2CryptPermissionsVerifier.Glacier2.PermissionsVerifier",
                cryptPasswords);
        }
    }

    // We do not need to set Ice.RetryIntervals to -1, i.e., we do
    // not have to disable connection retry. It is safe for
    // Glacier2 to retry outgoing connections to servers. Retry
    // for incoming connections from clients must be disabled in
    // the clients.

    return Service::initializeCommunicator(argc, argv, std::move(initData));
}

void
RouterService::usage(const string& appName)
{
    string options = "Options:\n"
                     "-h, --help           Show this message.\n"
                     "-v, --version        Display the Ice version.\n";
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
    RouterService svc;
    // Initialize the service with a Properties object with the correct property prefix enabled.
    Ice::InitializationData initData;
    initData.properties = make_shared<Properties>(vector<string>{"Glacier2"});
    return svc.main(argc, argv, std::move(initData));
}
