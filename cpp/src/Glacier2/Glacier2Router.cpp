// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/UUID.h>
#include <IceUtil/Options.h>
#include <IceUtil/FileUtil.h>
#include <Ice/Service.h>
#include <Glacier2/Instance.h>
#include <Glacier2/RouterI.h>
#include <Glacier2/Session.h>
#include <Glacier2/SessionRouterI.h>
#include <Glacier2/NullPermissionsVerifier.h>

using namespace std;
using namespace Ice;
using namespace Glacier2;

namespace
{

class RouterService : public Service
{
public:

    RouterService();

protected:

    virtual bool start(int, char*[], int&);
    virtual bool stop();
    virtual CommunicatorPtr initializeCommunicator(int&, char*[], const InitializationData&);

private:

    void usage(const std::string&);

    InstancePtr _instance;
    SessionRouterIPtr _sessionRouter;
};


class FinderI : public Ice::RouterFinder
{
public:
    
    FinderI(const Glacier2::RouterPrx& router) : _router(router)
    {
    }
    
    virtual Ice::RouterPrx
    getRouter(const Ice::Current&)
    {
        return _router;
    }

private:

    const Glacier2::RouterPrx _router;
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
        args = opts.parse(argc, const_cast<const char**>(argv));
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
        cerr << argv[0] << ": too many arguments" << endl;
        usage(argv[0]);
        return false;
    }

    PropertiesPtr properties = communicator()->getProperties();

    //
    // Initialize the client object adapter.
    //
    const string clientEndpointsProperty = "Glacier2.Client.Endpoints";
    if(properties->getProperty(clientEndpointsProperty).empty())
    {
        error("property `" + clientEndpointsProperty + "' is not set");
        return false;
    }

    if(properties->getPropertyAsInt("Glacier2.SessionTimeout") > 0 &&
       properties->getProperty("Glacier2.Client.ACM.Timeout").empty())
    {
        ostringstream os;
        os << properties->getPropertyAsInt("Glacier2.SessionTimeout");
        properties->setProperty("Glacier2.Client.ACM.Timeout", os.str());
    }

    if(properties->getProperty("Glacier2.Client.ACM.Close").empty())
    {
        properties->setProperty("Glacier2.Client.ACM.Close", "4"); // Forcefull close on invocation and idle.
    }

    ObjectAdapterPtr clientAdapter = communicator()->createObjectAdapter("Glacier2.Client");

    //
    // Initialize the server object adapter only if server endpoints
    // are defined.
    //
    const string serverEndpointsProperty = "Glacier2.Server.Endpoints";
    ObjectAdapterPtr serverAdapter;
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
    PermissionsVerifierPrx verifier;
    ObjectPrx obj;
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
            verifier = PermissionsVerifierPrx::checkedCast(obj);
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
            verifier = PermissionsVerifierPrx::uncheckedCast(obj);
        }
    }
    
    //
    // Get the session manager if specified.
    //
    string sessionManagerProperty = "Glacier2.SessionManager";
    string sessionManagerPropertyValue = properties->getProperty(sessionManagerProperty);
    SessionManagerPrx sessionManager;
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
            sessionManager = SessionManagerPrx::checkedCast(obj);
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
            sessionManager = SessionManagerPrx::uncheckedCast(obj);
        }
        sessionManager = 
            SessionManagerPrx::uncheckedCast(sessionManager->ice_connectionCached(false)->ice_locatorCacheTimeout(
                properties->getPropertyAsIntWithDefault("Glacier2.SessionManager.LocatorCacheTimeout", 600)));
    }

    //
    // Check for an SSL permissions verifier.
    //
    string sslVerifierProperty = verifierProperties[1];
    SSLPermissionsVerifierPrx sslVerifier;
  
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
            sslVerifier = SSLPermissionsVerifierPrx::checkedCast(obj);
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
            sslVerifier = SSLPermissionsVerifierPrx::uncheckedCast(obj);
        }
    }

    //
    // Get the SSL session manager if specified.
    //
    string sslSessionManagerProperty = "Glacier2.SSLSessionManager";
    string sslSessionManagerPropertyValue = properties->getProperty(sslSessionManagerProperty);
    SSLSessionManagerPrx sslSessionManager;
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
            sslSessionManager = SSLSessionManagerPrx::checkedCast(obj);
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
            sslSessionManager = SSLSessionManagerPrx::uncheckedCast(obj);
        }
        sslSessionManager = 
            SSLSessionManagerPrx::uncheckedCast(sslSessionManager->ice_connectionCached(false)->ice_locatorCacheTimeout(
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
        _instance = new Instance(communicator(), clientAdapter, serverAdapter);
    }
    catch(const Ice::InitializationException& ex)
    {
        error("Glacier2 initialization failed:\n" + ex.reason);
        return false;
    }

    //
    // Create the session router. The session router registers itself
    // and all required servant locators, so no registration has to be
    // done here.
    //
    _sessionRouter = new SessionRouterI(_instance, verifier, sessionManager, sslVerifier, sslSessionManager);

    //
    // Th session router is used directly as servant for the main
    // Glacier2 router Ice object.
    //
    Identity routerId;
    routerId.category = _instance->properties()->getPropertyWithDefault("Glacier2.InstanceName", "Glacier2");
    routerId.name = "router";
    Glacier2::RouterPrx routerPrx = Glacier2::RouterPrx::uncheckedCast(clientAdapter->add(_sessionRouter, routerId));

    //
    // Add the Ice router finder object to allow retrieving the router
    // proxy with just the endpoint information of the router.
    //
    Identity finderId;
    finderId.category = "Ice";
    finderId.name = "RouterFinder";
    clientAdapter->add(new FinderI(routerPrx), finderId);

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
        _sessionRouter = 0;
    }

    if(_instance)
    {
        if(_instance->getObserver())
        {
            _instance->getObserver()->setObserverUpdater(0);
        }
        _instance->destroy();
        _instance = 0;
    }
    return true;
}

CommunicatorPtr
RouterService::initializeCommunicator(int& argc, char* argv[], 
                                      const InitializationData& initializationData)
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
   
    return Service::initializeCommunicator(argc, argv, initData);
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
