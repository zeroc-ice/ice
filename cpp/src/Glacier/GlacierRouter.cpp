// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Application.h>
#include <Ice/RoutingTable.h>
#include <Glacier/RouterI.h>
#include <Glacier/ClientBlobject.h>
#include <Glacier/ServerBlobject.h>
#include <IceUtil/Base64.h>
#include <Ice/CertificateVerifierF.h>
#include <Ice/System.h>
#include <Ice/SslExtension.h>

using namespace std;
using namespace Ice;
using namespace Glacier;

namespace Glacier
{

class ServantLocator : public Ice::ServantLocator
{
public:

    ServantLocator(const ::Ice::ObjectPtr&);
    
    virtual Ice::ObjectPtr locate(const Ice::ObjectAdapterPtr&, const Ice::Current&, Ice::LocalObjectPtr&);
    virtual void finished(const Ice::ObjectAdapterPtr&, const Ice::Current&,
			  const Ice::ObjectPtr&, const Ice::LocalObjectPtr&);
    virtual void deactivate();

private:

    ::Ice::ObjectPtr _blobject;
};

class Router : public Application
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
Glacier::ServantLocator::locate(const ObjectAdapterPtr&, const Current&, LocalObjectPtr&)
{
    return _blobject;
}

void
Glacier::ServantLocator::finished(const ObjectAdapterPtr&, const Current&, const ObjectPtr&, const LocalObjectPtr&)
{
    // Nothing to do
}

void
Glacier::ServantLocator::deactivate()
{
    ClientBlobject* clientBlobject = dynamic_cast<ClientBlobject*>(_blobject.get());
    if (clientBlobject)
    {
	clientBlobject->destroy();
    }
    
    ServerBlobject* serverBlobject = dynamic_cast<ServerBlobject*>(_blobject.get());
    if (serverBlobject)
    {
	serverBlobject->destroy();
    }
    
    _blobject = 0;
}

void
Glacier::Router::usage()
{
    cerr << "Usage: " << appName() << " [options]\n";
    cerr <<     
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        ;
}

int
Glacier::Router::run(int argc, char* argv[])
{
    for (int i = 1; i < argc; ++i)
    {
	if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
	{
	    usage();
	    return EXIT_SUCCESS;
	}
	else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
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

    string clientConfig = properties->getProperty("Ice.SSL.Client.Config");
    string serverConfig = properties->getProperty("Ice.SSL.Server.Config");

    // Only do this if we've been configured for SSL
    if (!clientConfig.empty() && !serverConfig.empty())
    {
        IceSSL::ContextType contextType = IceSSL::ClientServer;

        // Get our SSL System
        IceSSL::SystemPtr sslSystem = communicator()->getSslSystem();

        // The system must configure itself (using config files as specified)
        sslSystem->configure(contextType);

        // If we have been told only to only accept a single certificate.
        string clientCertBase64 = properties->getProperty("Glacier.Router.AcceptCert");
        if (!clientCertBase64.empty())
        {
            // Get an instance of the SSL Extension itself
            IceSSL::SslExtensionPtr sslExtension = communicator()->getSslExtension();
            // Install a Certificate Verifier that only accepts indicated certificate.
            Ice::ByteSeq clientCert = IceUtil::Base64::decode(clientCertBase64);
            sslSystem->setCertificateVerifier(contextType, sslExtension->getSingleCertVerifier(clientCert));
        
            // Add the Client's certificate as a trusted certificate.
            sslSystem->addTrustedCertificateBase64(contextType, clientCertBase64);
        }
    }

    //
    // Create the routing table.
    //
    IceInternal::RoutingTablePtr routingTable = new IceInternal::RoutingTable;

    //
    // Initialize the client object adapter.
    //
    const char* clientEndpointsProperty = "Glacier.Client.Endpoints";
    string clientEndpoints = properties->getProperty(clientEndpointsProperty);
    if (clientEndpoints.empty())
    {
	cerr << appName() << ": property `" << clientEndpointsProperty << "' is not set" << endl;
	return EXIT_FAILURE;
    }
    ObjectAdapterPtr clientAdapter = communicator()->createObjectAdapterFromProperty("Client",
										     clientEndpointsProperty);

    //
    // Initialize the server object adapter.
    //
    const char* serverEndpointsProperty = "Glacier.Server.Endpoints";
    string serverEndpoints = properties->getProperty(serverEndpointsProperty);
    ObjectAdapterPtr serverAdapter;
    if (!serverEndpoints.empty())
    {
	serverAdapter = communicator()->createObjectAdapterFromProperty("Server", serverEndpointsProperty);
    }

    //
    // Create the client and server blobjects and the associated
    // servant locators.
    //
    ObjectPtr clientBlobject = new ClientBlobject(communicator(), routingTable);
    Ice::ServantLocatorPtr clientServantLocator = new Glacier::ServantLocator(clientBlobject);
    clientAdapter->addServantLocator(clientServantLocator, "");
    if (serverAdapter)
    {
	ObjectPtr serverBlobject = new ServerBlobject(clientAdapter);
	Ice::ServantLocatorPtr serverServantLocator = new Glacier::ServantLocator(serverBlobject);
	serverAdapter->addServantLocator(serverServantLocator, "");
    }

    //
    // Initialize the router object adapter and the router object.
    //
    const char* routerEndpointsProperty = "Glacier.Router.Endpoints";
    string routerEndpoints = properties->getProperty(routerEndpointsProperty);
    if (routerEndpoints.empty())
    {
	cerr << appName() << ": property `" << routerEndpointsProperty << "' is not set" << endl;
	return EXIT_FAILURE;
    }
    const char* routerIdentityProperty = "Glacier.Router.Identity";
    string routerIdentity = properties->getProperty(routerIdentityProperty);
    if (routerIdentity.empty())
    {
	routerIdentity = "Glacier/router";
    }
    ObjectAdapterPtr routerAdapter =
	communicator()->createObjectAdapterFromProperty("Router", routerEndpointsProperty);
    RouterPtr router = new RouterI(clientAdapter, serverAdapter, routingTable);
    routerAdapter->add(router, stringToIdentity(routerIdentity));

#ifndef _WIN32
    //
    // Print the stringified router proxy on a filedescriptor
    // specified in the properties, if so requested.
    //
    string outputFd = properties->getProperty("Glacier.Router.PrintProxyOnFd");
    if (!outputFd.empty())
    {
	int fd = atoi(outputFd.c_str());
	string ref = communicator()->proxyToString(routerAdapter->createProxy(stringToIdentity(routerIdentity)));
	ref += "\n";
	string::size_type sz = static_cast<string::size_type>(write(fd, ref.c_str(), ref.length()));
	if (sz != ref.length())
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
    if (serverAdapter)
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
    RouterI* rtr = dynamic_cast<RouterI*>(router.get());
    assert(rtr);
    rtr->destroy();

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    addArgumentPrefix("Glacier");

    //
    // Make sure that this process doesn't use a router.
    //
    PropertiesPtr defaultProperties;
    try
    {
	defaultProperties = getDefaultProperties(argc, argv);
    }
    catch(const Exception& ex)
    {
	cerr << argv[0] << ": " << ex << endl;
	return EXIT_FAILURE;
    }
    defaultProperties->setProperty("Ice.DefaultRouter", "");

    Glacier::Router app;
    return app.main(argc, argv);
}
