// **********************************************************************
//
// Copyright (c) 2001
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

#include <Ice/Application.h>
#include <Ice/RoutingTable.h>
#include <Glacier/RouterI.h>
#include <Glacier/ClientBlobject.h>
#include <Glacier/ServerBlobject.h>
#include <Glacier/SessionManager.h>
#include <IceUtil/Base64.h>
#include <IceSSL/CertificateVerifierF.h>
#include <IceSSL/Plugin.h>

using namespace std;
using namespace Ice;
using namespace Glacier;

namespace Glacier
{

class ServantLocator : public Ice::ServantLocator
{
public:

    ServantLocator(const ::Ice::ObjectPtr&);
    
    virtual Ice::ObjectPtr locate(const Ice::Current&, Ice::LocalObjectPtr&);
    virtual void finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&);
    virtual void deactivate();

private:

    ::Ice::ObjectPtr _blobject;
};

class RouterApp : public Application
{
public:

    void usage();
    virtual int run(int, char*[]);
};

};

Glacier::ServantLocator::ServantLocator(const ObjectPtr& blobject) :
    _blobject(blobject)
{
}

ObjectPtr
Glacier::ServantLocator::locate(const Current&, LocalObjectPtr&)
{
    return _blobject;
}

void
Glacier::ServantLocator::finished(const Current&, const ObjectPtr&, const LocalObjectPtr&)
{
    // Nothing to do
}

void
Glacier::ServantLocator::deactivate()
{
    ClientBlobject* clientBlobject = dynamic_cast<ClientBlobject*>(_blobject.get());
    if(clientBlobject)
    {
	clientBlobject->destroy();
    }
    
    ServerBlobject* serverBlobject = dynamic_cast<ServerBlobject*>(_blobject.get());
    if(serverBlobject)
    {
	serverBlobject->destroy();
    }
    
    _blobject = 0;
}

void
Glacier::RouterApp::usage()
{
    cerr << "Usage: " << appName() << " [options]\n";
    cerr <<     
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        ;
}

int
Glacier::RouterApp::run(int argc, char* argv[])
{
    for(int i = 1; i < argc; ++i)
    {
	if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
	{
	    usage();
	    return EXIT_SUCCESS;
	}
	else if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
	{
	    cout << ICE_STRING_VERSION << endl;
	    return EXIT_SUCCESS;
	}
	else
	{
	    cerr << appName() << ": unknown option `" << argv[i] << "'" << endl;
	    usage();
	    return EXIT_FAILURE;
	}
    }

    PropertiesPtr properties = communicator()->getProperties();

    string clientConfig = properties->getProperty("IceSSL.Client.Config");
    string serverConfig = properties->getProperty("IceSSL.Server.Config");

    //
    // Only do this if we've been configured for SSL.
    //
    if(!clientConfig.empty() && !serverConfig.empty())
    {
        IceSSL::ContextType contextType = IceSSL::ClientServer;

        //
        // Get the SSL plugin.
        //
        PluginManagerPtr pluginManager = communicator()->getPluginManager();
        PluginPtr plugin = pluginManager->getPlugin("IceSSL");
        IceSSL::PluginPtr sslPlugin = IceSSL::PluginPtr::dynamicCast(plugin);
        assert(sslPlugin);

        //
        // The plug-in must configure itself (using config files as specified).
        //
        sslPlugin->configure(contextType);

        // If we have been told only to only accept a single certificate.
        string clientCertBase64 = properties->getProperty("Glacier.Router.AcceptCert");
        if(!clientCertBase64.empty())
        {
            // Install a Certificate Verifier that only accepts indicated certificate.
            Ice::ByteSeq clientCert = IceUtil::Base64::decode(clientCertBase64);
            sslPlugin->setCertificateVerifier(contextType, sslPlugin->getSingleCertVerifier(clientCert));
        
            // Add the Client's certificate as a trusted certificate.
            sslPlugin->addTrustedCertificateBase64(contextType, clientCertBase64);
        }
    }

    //
    // Create the routing table.
    //
    IceInternal::RoutingTablePtr routingTable = new IceInternal::RoutingTable;

    //
    // Initialize the client object adapter.
    //
    const char* clientEndpointsProperty = "Glacier.Router.Client.Endpoints";
    if(properties->getProperty(clientEndpointsProperty).empty())
    {
	cerr << appName() << ": property `" << clientEndpointsProperty << "' is not set" << endl;
	return EXIT_FAILURE;
    }
    ObjectAdapterPtr clientAdapter = communicator()->createObjectAdapter("Glacier.Router.Client");

    //
    // Initialize the server object adapter.
    //
    ObjectAdapterPtr serverAdapter;
    if(!properties->getProperty("Glacier.Router.Server.Endpoints").empty())
    {
	serverAdapter = communicator()->createObjectAdapter("Glacier.Router.Server");
    }

    //
    // Create the client and server blobjects and the associated
    // servant locators.
    //
    const char* allowCategoriesProperty = "Glacier.Router.AllowCategories";
    string allowCategories = properties->getProperty(allowCategoriesProperty);
    ObjectPtr clientBlobject = new ClientBlobject(communicator(), routingTable, allowCategories);
    Ice::ServantLocatorPtr clientServantLocator = new Glacier::ServantLocator(clientBlobject);
    clientAdapter->addServantLocator(clientServantLocator, "");
    if(serverAdapter)
    {
	ObjectPtr serverBlobject = new ServerBlobject(clientAdapter);
	Ice::ServantLocatorPtr serverServantLocator = new Glacier::ServantLocator(serverBlobject);
	serverAdapter->addServantLocator(serverServantLocator, "");
    }

    //
    // Initialize the router object adapter and the router object.
    //
    const char* routerEndpointsProperty = "Glacier.Router.Endpoints";
    if(properties->getProperty(routerEndpointsProperty).empty())
    {
	cerr << appName() << ": property `" << routerEndpointsProperty << "' is not set" << endl;
	return EXIT_FAILURE;
    }

    const char* routerIdentityProperty = "Glacier.Router.Identity";
    string routerIdentity = properties->getPropertyWithDefault(routerIdentityProperty, "Glacier/router");

    const char* sessionManagerProperty = "Glacier.Router.SessionManager";
    string sessionManager = properties->getProperty(sessionManagerProperty);

    SessionManagerPrx sessionManagerPrx;
    if(!sessionManager.empty())
    {
	sessionManagerPrx = SessionManagerPrx::checkedCast(communicator()->stringToProxy(sessionManager));
    }

    const char* userIdProperty = "Glacier.Router.UserId";
    string userId = properties->getProperty(userIdProperty);

    ObjectAdapterPtr routerAdapter = communicator()->createObjectAdapter("Glacier.Router");
    RouterPtr router = new RouterI(clientAdapter, serverAdapter, routingTable, sessionManagerPrx, userId);
    routerAdapter->add(router, stringToIdentity(routerIdentity));

#ifndef _WIN32
    //
    // Print the stringified router proxy on a filedescriptor
    // specified in the properties, if so requested.
    //
    string outputFd = properties->getProperty("Glacier.Router.PrintProxyOnFd");
    if(!outputFd.empty())
    {
	int fd = atoi(outputFd.c_str());
	string ref = communicator()->proxyToString(routerAdapter->createProxy(stringToIdentity(routerIdentity)));
	ref += "\n";
	string::size_type sz = static_cast<string::size_type>(write(fd, ref.c_str(), ref.length()));
	if(sz != ref.length())
	{
	    cerr << appName() << ": cannot write stringified router proxy to filedescriptor " << fd << ": "
		 << strerror(errno) << endl;

	    //
	    // Destroy the router. The client and server blobjects get
	    // destroyed by ServantLocator::deactivate.
	    //
	    RouterI* rtr = dynamic_cast<RouterI*>(router.get());
	    assert(rtr);
	    rtr->destroy();

	    return EXIT_FAILURE;
	}
	close(fd);
    }
#endif

    //
    // Everything ok, let's go.
    //
    shutdownOnInterrupt();
    clientAdapter->activate();
    if(serverAdapter)
    {
	serverAdapter->activate();
    }
    routerAdapter->activate();
    communicator()->waitForShutdown();
    ignoreInterrupt();

    //
    // Destroy the router. The client and server blobjects get
    // destroyed by ServantLocator::deactivate.
    //
    // Destroying the router will also destroy all sessions associated
    // with the router.
    //
    RouterI* rtr = dynamic_cast<RouterI*>(router.get());
    assert(rtr);
    rtr->destroy();

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    //
    // Make sure that this process doesn't use a router.
    //
    PropertiesPtr defaultProperties;
    try
    {
	defaultProperties = getDefaultProperties(argc, argv);
        StringSeq args = argsToStringSeq(argc, argv);
        args = defaultProperties->parseCommandLineOptions("Ice", args);
        args = defaultProperties->parseCommandLineOptions("Glacier.Router", args);
        stringSeqToArgs(args, argc, argv);
    }
    catch(const Exception& ex)
    {
	cerr << argv[0] << ": " << ex << endl;
	return EXIT_FAILURE;
    }
    defaultProperties->setProperty("Ice.Default.Router", "");

    Glacier::RouterApp app;
    return app.main(argc, argv);
}
