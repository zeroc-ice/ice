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
#include <IceWall/Router.h>

using namespace std;
using namespace Ice;
using namespace IceWall;

class Server : public Application
{
public:

    void usage();
    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    Server app;
    return app.main(argc, argv);
}

void
Server::usage()
{
    cerr << "Usage: " << appName() << " [options]\n";
    cerr <<     
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        ;
}

int
Server::run(int argc, char* argv[])
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

    const char* routerEndpointsProperty = "Ice.Adapter.Router.Endpoints";
    string routerEndpoints = properties->getProperty(routerEndpointsProperty);
    if (routerEndpoints.empty())
    {
	cerr << appName() << ": property `" << routerEndpointsProperty << "' is not set" << endl;
	return EXIT_FAILURE;
    }

    ServantLocatorPtr router = new Router;

    ObjectAdapterPtr routerAdapter = communicator()->createObjectAdapter("Router");
    routerAdapter->addServantLocator(router, "");
    routerAdapter->activate();

    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
