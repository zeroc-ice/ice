//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/UUID.h>
#include <IceUtil/Options.h>
#include <Ice/ConsoleUtil.h>
#include <Ice/Service.h>
#include <Glacier2/Instance.h>
#include <Glacier2/RouterI.h>
#include <Glacier2/Session.h>
#include <Glacier2/SessionRouterI.h>
#include <Glacier2/NullPermissionsVerifier.h>

using namespace std;
using namespace Ice;
using namespace Glacier2;
using namespace IceInternal;
namespace
{

class ClientLocator final : public ServantLocator
{
public:

    ClientLocator(shared_ptr<SessionRouterI> sessionRouter) :
        _sessionRouter(move(sessionRouter))
    {
    }

    shared_ptr<Object>
    locate(const Current& current, shared_ptr<void>&) override
    {
        return _sessionRouter->getClientBlobject(current.con, current.id);
    }

    void
    finished(const Current&, const shared_ptr<Object>&, const shared_ptr<void>&) override
    {
    }

    void
    deactivate(const string&) override
    {
    }

private:

    const shared_ptr<SessionRouterI> _sessionRouter;
};

class ServerLocator final : public ServantLocator
{
public:

    ServerLocator(shared_ptr<SessionRouterI> sessionRouter) :
        _sessionRouter(move(sessionRouter))
    {
    }

    shared_ptr<Object>
    locate(const Current& current, shared_ptr<void>&) override
    {
        return _sessionRouter->getServerBlobject(current.id.category);
    }

    void
    finished(const Current&, const shared_ptr<Object>&, const shared_ptr<void>&) override
    {
    }

    void
    deactivate(const string&) override
    {
    }

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
    shared_ptr<Communicator> initializeCommunicator(int&, char*[], const InitializationData&, int) override;

private:

    void usage(const std::string&);

    shared_ptr<Glacier2::Instance> _instance;
    shared_ptr<SessionRouterI> _sessionRouter;
};

class FinderI : public Ice::RouterFinder
{
public:

    FinderI(shared_ptr<Glacier2::RouterPrx> router) : _router(move(router))
    {
    }

    shared_ptr<Ice::RouterPrx>
    getRouter(const Ice::Current&) override
    {
        return _router;
    }

private:

    const shared_ptr<Glacier2::RouterPrx> _router;
};

};

RouterService::RouterService()
{
}

bool
RouterService::start(int argc, char* argv[], int& status)
{
    bool nowarn;

    IceUtilInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("", "nowarn");

    vector<string> args;
    try
    {
        args = opts.parse(argc, argv);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        error(e.reason);
        usage(argv[0]);
        return false;
    }

    if(opts.isSet("help"))
    {
        usage(argv[0]);
        status = EXIT_SUCCESS;
        return false;
    }
    if(opts.isSet("version"))
    {
        print(ICE_STRING_VERSION);
        status = EXIT_SUCCESS;
        return false;
    }
    nowarn = opts.isSet("nowarn");

    if(!args.empty())
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
    if(properties->getProperty(clientEndpointsProperty).empty())
    {
        error("property `" + clientEndpointsProperty + "' is not set");
        return false;
    }

    if(properties->getProperty("Glacier2.Client.ACM.Close").empty())
    {
        properties->setProperty("Glacier2.Client.ACM.Close", "4"); // Forcefull close on invocation and idle.
    }

    auto clientAdapter = communicator()->createObjectAdapter("Glacier2.Client");

    //
    // Initialize the server object adapter only if server endpoints
    // are defined.
    //
    const string serverEndpointsProperty = "Glacier2.Server.Endpoints";
    shared_ptr<ObjectAdapter> serverAdapter;
    if(!properties->getProperty(serverEndpointsProperty).empty())
    {
        serverAdapter = communicator()->createObjectAdapter("Glacier2.Server");
    }

    string instanceName = communicator()->getProperties()->getPropertyWithDefault("Glacier2.InstanceName", "Glacier2");

    vector<string> verifierProperties;
    verifierProperties.push_back("Glacier2.PermissionsVerifier");
    verifierProperties.push_back("Glacier2.SSLPermissionsVerifier");

    Glacier2Internal::setupNullPermissionsVerifier(communicator(), instanceName, verifierProperties);

    string verifierProperty = verifierProperties[0];
    shared_ptr<PermissionsVerifierPrx> verifier;
    shared_ptr<ObjectPrx> obj;
    try
    {
        //
        // We use propertyToProxy instead of stringToProxy because the property
        // can provide proxy attributes
        //
        obj = communicator()->propertyToProxy(verifierProperty);
    }
    catch(const std::exception& ex)
    {
        ServiceError err(this);
        err << "permissions verifier `" << communicator()->getProperties()->getProperty(verifierProperty)
            << "' is invalid:\n" << ex;
        return false;
    }

    if(obj)
    {
        try
        {
            verifier = Ice::checkedCast<PermissionsVerifierPrx>(obj);
            if(!verifier)
            {
                ServiceError err(this);
                err << "permissions verifier `" << communicator()->getProperties()->getProperty(verifierProperty)
                    << "' is invalid";
                return false;
            }
        }
        catch(const Ice::Exception& ex)
        {
            if(!nowarn)
            {
                ServiceWarning warn(this);
                warn << "unable to contact permissions verifier `"
                     << communicator()->getProperties()->getProperty(verifierProperty) << "'\n" << ex;
            }
            verifier = Ice::uncheckedCast<PermissionsVerifierPrx>(obj);
        }
    }

    //
    // Get the session manager if specified.
    //
    string sessionManagerProperty = "Glacier2.SessionManager";
    string sessionManagerPropertyValue = properties->getProperty(sessionManagerProperty);
    shared_ptr<SessionManagerPrx> sessionManager;
    if(!sessionManagerPropertyValue.empty())
    {
        try
        {
            obj = communicator()->propertyToProxy(sessionManagerProperty);
        }
        catch(const std::exception& ex)
        {
            ServiceError err(this);
            err << "session manager `" << sessionManagerPropertyValue << "' is invalid\n:" << ex;
            return false;
        }
        try
        {
            sessionManager = Ice::checkedCast<SessionManagerPrx>(obj);
            if(!sessionManager)
            {
                error("session manager `" + sessionManagerPropertyValue + "' is invalid");
                return false;
            }
        }
        catch(const std::exception& ex)
        {
            if(!nowarn)
            {
                ServiceWarning warn(this);
                warn << "unable to contact session manager `" << sessionManagerPropertyValue << "'\n" << ex;
            }
            sessionManager = Ice::uncheckedCast<SessionManagerPrx>(obj);
        }
        sessionManager =
            Ice::uncheckedCast<SessionManagerPrx>(sessionManager->ice_connectionCached(false)->ice_locatorCacheTimeout(
                properties->getPropertyAsIntWithDefault("Glacier2.SessionManager.LocatorCacheTimeout", 600)));
    }

    //
    // Check for an SSL permissions verifier.
    //
    string sslVerifierProperty = verifierProperties[1];
    shared_ptr<SSLPermissionsVerifierPrx> sslVerifier;

    try
    {
        obj = communicator()->propertyToProxy(sslVerifierProperty);
    }
    catch(const std::exception& ex)
    {
        ServiceError err(this);
        err << "ssl permissions verifier `" << communicator()->getProperties()->getProperty(sslVerifierProperty)
            << "' is invalid:\n" << ex;
        return false;
    }

    if(obj)
    {
        try
        {
            sslVerifier = Ice::checkedCast<SSLPermissionsVerifierPrx>(obj);
            if(!sslVerifier)
            {
                ServiceError err(this);
                err << "ssl permissions verifier `"
                    << communicator()->getProperties()->getProperty(sslVerifierProperty)
                    << "' is invalid";
            }
        }
        catch(const Ice::Exception& ex)
        {
            if(!nowarn)
            {
                ServiceWarning warn(this);
                warn << "unable to contact ssl permissions verifier `"
                     <<  communicator()->getProperties()->getProperty(sslVerifierProperty) << "'\n"
                     << ex;
            }
            sslVerifier = Ice::uncheckedCast<SSLPermissionsVerifierPrx>(obj);
        }
    }

    //
    // Get the SSL session manager if specified.
    //
    string sslSessionManagerProperty = "Glacier2.SSLSessionManager";
    string sslSessionManagerPropertyValue = properties->getProperty(sslSessionManagerProperty);
    shared_ptr<SSLSessionManagerPrx> sslSessionManager;
    if(!sslSessionManagerPropertyValue.empty())
    {
        try
        {
            obj = communicator()->propertyToProxy(sslSessionManagerProperty);
        }
        catch(const std::exception& ex)
        {
            ServiceError err(this);
            err << "ssl session manager `" << sslSessionManagerPropertyValue << "' is invalid:\n" << ex;
            return false;
        }
        try
        {
            sslSessionManager = Ice::checkedCast<SSLSessionManagerPrx>(obj);
            if(!sslSessionManager)
            {
                error("ssl session manager `" + sslSessionManagerPropertyValue + "' is invalid");
                return false;
            }
        }
        catch(const Ice::Exception& ex)
        {
            if(!nowarn)
            {
                ServiceWarning warn(this);
                warn << "unable to contact ssl session manager `" << sslSessionManagerPropertyValue
                     << "'\n" << ex;
            }
            sslSessionManager = Ice::uncheckedCast<SSLSessionManagerPrx>(obj);
        }
        sslSessionManager =
            Ice::uncheckedCast<SSLSessionManagerPrx>(sslSessionManager->ice_connectionCached(false)->ice_locatorCacheTimeout(
                properties->getPropertyAsIntWithDefault("Glacier2.SSLSessionManager.LocatorCacheTimeout", 600)));
    }

    if(!verifier && !sslVerifier)
    {
        error("Glacier2 requires a permissions verifier or password file");
        return false;
    }

    //
    // Create the instance object.
    //
    try
    {
        _instance = make_shared<Glacier2::Instance>(communicator(), clientAdapter, serverAdapter);
    }
    catch(const Ice::InitializationException& ex)
    {
        error("Glacier2 initialization failed:\n" + ex.reason);
        return false;
    }

    _sessionRouter = make_shared<SessionRouterI>(_instance, move(verifier), move(sessionManager), move(sslVerifier),
                                                 move(sslSessionManager));

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
        if(_instance->serverObjectAdapter())
        {
            _instance->serverObjectAdapter()->addServantLocator(make_shared<ServerLocator>(_sessionRouter), "");
        }
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
        // Ignore.
    }

    _instance->setSessionRouter(_sessionRouter);

    //
    // The session router is used directly as a servant for the main
    // Glacier2 router Ice object.
    //
    auto routerPrx = Ice::uncheckedCast<Glacier2::RouterPrx>(clientAdapter->add(_sessionRouter, {"router", instanceName}));

    //
    // Add the Ice router finder object to allow retrieving the router
    // proxy with just the endpoint information of the router.
    //
    clientAdapter->add(make_shared<FinderI>(routerPrx), {"RouterFinder", "Ice"});

    if(_instance->getObserver())
    {
        _instance->getObserver()->setObserverUpdater(_sessionRouter);
    }

    //
    // Everything ok, let's go.
    //
    try
    {
        clientAdapter->activate();
        if(serverAdapter)
        {
            serverAdapter->activate();
        }
    }
    catch(const std::exception& ex)
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
    if(_sessionRouter)
    {
        _sessionRouter->destroy();
        _sessionRouter = nullptr;
    }

    if(_instance)
    {
        if(_instance->getObserver())
        {
            _instance->getObserver()->setObserverUpdater(nullptr);
        }
        _instance->destroy();
        _instance = nullptr;
    }
    return true;
}

shared_ptr<Communicator>
RouterService::initializeCommunicator(int& argc, char* argv[],
                                      const InitializationData& initializationData,
                                      int version)
{
    InitializationData initData = initializationData;
    initData.properties = createProperties(argc, argv, initializationData.properties);

    //
    // Make sure that Glacier2 doesn't use a router.
    //
    initData.properties->setProperty("Ice.Default.Router", "");

    //
    // If Glacier2.PermissionsVerifier is not set and Glacier2.CryptPasswords is set,
    // load the Glacier2CryptPermissionsVerifier plug-in
    //
    string verifier = "Glacier2.PermissionsVerifier";
    if(initData.properties->getProperty(verifier).empty())
    {
        string cryptPasswords = initData.properties->getProperty("Glacier2.CryptPasswords");

        if(!cryptPasswords.empty())
        {
            initData.properties->setProperty("Ice.Plugin.Glacier2CryptPermissionsVerifier",
                                             "Glacier2CryptPermissionsVerifier:createCryptPermissionsVerifier");

            initData.properties->setProperty("Glacier2CryptPermissionsVerifier.Glacier2.PermissionsVerifier",
                                             cryptPasswords);
        }
    }

    //
    // Active connection management is permitted with Glacier2. For
    // the client object adapter, the ACM timeout is set to the
    // session timeout to ensure client connections are not closed
    // prematurely,
    //
    //initData.properties->setProperty("Ice.ACM.Client", "0");
    //initData.properties->setProperty("Ice.ACM.Server", "0");

    //
    // We do not need to set Ice.RetryIntervals to -1, i.e., we do
    // not have to disable connection retry. It is safe for
    // Glacier2 to retry outgoing connections to servers. Retry
    // for incoming connections from clients must be disabled in
    // the clients.
    //

    return Service::initializeCommunicator(argc, argv, initData, version);
}

void
RouterService::usage(const string& appName)
{
    string options =
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        "--nowarn             Suppress warnings.";
#ifndef _WIN32
    options.append(
        "\n"
        "\n"
        "--daemon             Run as a daemon.\n"
        "--pidfile FILE       Write process ID into FILE.\n"
        "--noclose            Do not close open file descriptors.\n"
        "--nochdir            Do not change the current working directory.\n"
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
    RouterService svc;
    return svc.main(argc, argv);
}
