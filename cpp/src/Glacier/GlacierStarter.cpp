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
#include <Glacier/GlacierI.h>

using namespace std;
using namespace Ice;
//using namespace Glacier;

namespace Glacier
{

class Router : public Application
{
public:

    void usage();
    virtual int run(int, char*[]);
};

};

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
    // Initialize Starter Object Adapter
    //
    const char* starterEndpointsProperty = "Glacier.Starter.Endpoints";
    string starterEndpoints = properties->getProperty(starterEndpointsProperty);
    if (starterEndpoints.empty())
    {
	cerr << appName() << ": property `" << starterEndpointsProperty << "' is not set" << endl;
	return EXIT_FAILURE;
    }

    ObjectAdapterPtr starterAdapter = communicator()->createObjectAdapterFromProperty("Starter",
										      starterEndpointsProperty);
    StarterPtr starter = new StarterI;
    starterAdapter->add(starter, stringToIdentity("Glacier#starter"));
    starterAdapter->activate();

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
