// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Application.h>
#include <IcePatch/NodeLocator.h>

using namespace std;
using namespace Ice;
using namespace IcePatch;

namespace IcePatch
{

class Server : public Application
{
public:

    void usage();
    virtual int run(int, char*[]);
};

};

void
IcePatch::Server::usage()
{
    cerr << "Usage: " << appName() << " [options]\n";
    cerr <<     
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        ;
}

int
IcePatch::Server::run(int argc, char* argv[])
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
    // Get the working directory and change to this directory.
    //
    const char* directoryProperty = "IcePatch.Directory";
    string directory = properties->getProperty(directoryProperty);
    if (directory.empty())
    {
	cerr << appName() << ": property `" << directoryProperty << "' is not set" << endl;
	return EXIT_FAILURE;
    }
    if(chdir(directory.c_str()) == -1)
    {
	cerr << appName() << ": can't change to directory `" << directory << "': " << strerror(errno) << endl;
	return EXIT_FAILURE;
    }

    //
    // Initialize Object Adapter
    //
    const char* endpointsProperty = "IcePatch.Endpoints";
    string endpoints = properties->getProperty(endpointsProperty);
    if (endpoints.empty())
    {
	cerr << appName() << ": property `" << endpointsProperty << "' is not set" << endl;
	return EXIT_FAILURE;
    }
    ObjectAdapterPtr adapter = communicator()->createObjectAdapterFromProperty("IcePatch", endpointsProperty);

    //
    // Create and add the node locator.
    //
    ServantLocatorPtr nodeLocator = new NodeLocator;
    adapter->addServantLocator(nodeLocator, "IcePatch");

    //
    // We're done, let's wait for shutdown.
    //
    communicator()->waitForShutdown();

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    addArgumentPrefix("IcePatch");
    Server app;
    return app.main(argc, argv);
}
