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

    //
    // Create routing table
    //
    IceInternal::RoutingTablePtr routingTable = new IceInternal::RoutingTable;

    //
    // Initialize Client Object Adapter
    //
    const char* clientEndpointsProperty = "Glacier.Client.Endpoints";
    string clientEndpoints = properties->getProperty(clientEndpointsProperty);
    if (clientEndpoints.empty())
    {
	cerr << appName() << ": property `" << clientEndpointsProperty << "' is not set" << endl;
	return EXIT_FAILURE;
    }

    ObjectAdapterPtr clientAdapter =
	communicator()->createObjectAdapterFromProperty("Client", clientEndpointsProperty);
    clientAdapter->activate();

    //
    // Initialize Server Object Adapter
    //
    const char* serverEndpointsProperty = "Glacier.Server.Endpoints";
    string serverEndpoints = properties->getProperty(serverEndpointsProperty);
    ObjectAdapterPtr serverAdapter;
    if (!serverEndpoints.empty())
    {
	ObjectAdapterPtr serverAdapter =
	    communicator()->createObjectAdapterFromProperty("Server", serverEndpointsProperty);
	serverAdapter->activate();
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
    // Initialize Router
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
	routerIdentity = "router";
    }

    ObjectAdapterPtr routerAdapter =
	communicator()->createObjectAdapterFromProperty("Router", routerEndpointsProperty);
    routerAdapter->add(new RouterI(clientAdapter, serverAdapter, routingTable), stringToIdentity(routerIdentity));
    routerAdapter->activate();

    //
    // We're done, let's wait for shutdown.
    //
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    addArgumentPrefix("Glacier");

    //
    // Make sure that the router doesn't use a router.
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
