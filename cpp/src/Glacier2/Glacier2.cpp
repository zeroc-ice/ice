// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Base64.h>
#include <Ice/Application.h>
#include <IceSSL/CertificateVerifierF.h>
#include <IceSSL/Plugin.h>
#include <Glacier/SessionManager.h>
#include <Glacier2/ClientServantLocator.h>
#include <Glacier2/ServerServantLocator.h>

using namespace std;
using namespace Ice;
using namespace Glacier;


namespace Glacier
{

class RouterApp : public Application
{
public:

    void usage();
    virtual int run(int, char*[]);
};

};

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
            ByteSeq clientCert = IceUtil::Base64::decode(clientCertBase64);
            sslPlugin->setCertificateVerifier(contextType, sslPlugin->getSingleCertVerifier(clientCert));
        
            // Add the Client's certificate as a trusted certificate.
            sslPlugin->addTrustedCertificateBase64(contextType, clientCertBase64);
        }
    }

    //
    // Get the session manager.
    //
    const char* sessionManagerProperty = "Glacier.Router.SessionManager";
    string sessionManager = properties->getProperty(sessionManagerProperty);
    SessionManagerPrx sessionManagerPrx;
    if(!sessionManager.empty())
    {
	sessionManagerPrx = SessionManagerPrx::checkedCast(communicator()->stringToProxy(sessionManager));
    }

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
    // Create and add the servant locators.
    //
    Ice::ServantLocatorPtr clientServantLocator = new ClientServantLocator(clientAdapter,
									   serverAdapter,
									   sessionManagerPrx);
    clientAdapter->addServantLocator(clientServantLocator, "");

    if(serverAdapter)
    {
	Ice::ServantLocatorPtr serverServantLocator = new ServerServantLocator(clientAdapter);
	serverAdapter->addServantLocator(serverServantLocator, "");
    }

    //
    // Everything ok, let's go.
    //
    shutdownOnInterrupt();
    clientAdapter->activate();
    if(serverAdapter)
    {
	serverAdapter->activate();
    }
    communicator()->waitForShutdown();
    ignoreInterrupt();

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    //
    // Make sure that this process doesn't use a router.
    //
    try
    {
	PropertiesPtr defaultProperties = getDefaultProperties(argc, argv);
	defaultProperties->setProperty("Ice.Default.Router", "");
    }
    catch(const Exception& e)
    {
	cerr << e << endl;
	exit(EXIT_FAILURE);
    }

    RouterApp app;
    return app.main(argc, argv);
}
