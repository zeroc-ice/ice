// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <Glacier2/SessionRouterI.h>

using namespace std;
using namespace Ice;
using namespace Glacier2;

namespace Glacier2
{

class RouterApp : public Application
{
public:

    void usage();
    virtual int run(int, char*[]);
};

};

void
Glacier2::RouterApp::usage()
{
    cerr << "Usage: " << appName() << " [options]\n";
    cerr <<     
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        ;
}

int
Glacier2::RouterApp::run(int argc, char* argv[])
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

    //
    // Initialize the client object adapter and servant locator.
    //
    const char* clientEndpointsProperty = "Glacier2.Client.Endpoints";
    if(properties->getProperty(clientEndpointsProperty).empty())
    {
	cerr << appName() << ": property `" << clientEndpointsProperty << "' is not set" << endl;
	return EXIT_FAILURE;
    }
    ObjectAdapterPtr clientAdapter = communicator()->createObjectAdapter("Glacier2.Client");

    //
    // Create a router implementation that can handle sessions, and
    // add it to the client object adapter.
    //
    SessionRouterIPtr sessionRouter = new SessionRouterI(clientAdapter);
    const char* routerIdProperty = "Glacier2.RouterIdentity";
    Identity routerId = stringToIdentity(properties->getPropertyWithDefault(routerIdProperty, "Glacier2/router"));
    clientAdapter->add(sessionRouter, routerId);

    //
    // Everything ok, let's go.
    //
    shutdownOnInterrupt();
    clientAdapter->activate();
    communicator()->waitForShutdown();
    ignoreInterrupt();

    //
    // Destroy the session router, which destroys everything else.
    //
    sessionRouter->destroy();

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    try
    {
	PropertiesPtr defaultProperties = getDefaultProperties(argc, argv);

	//
	// Make sure that Glacier2 doesn't use a router.
	//
	defaultProperties->setProperty("Ice.Default.Router", "");

	//
	// No active connection management is permitted with
	// Glacier2. Connections must remain established.
	//
	defaultProperties->setProperty("Ice.ConnectionIdleTime", "0");

	//
	// Ice.MonitorConnections defaults to Ice.ConnectionIdleTime,
	// which we set to 0 above. However, we still want the
	// connection monitor thread for AMI timeouts. We only set
	// this value if it hasn't been set explicitly already.
	//
	if(defaultProperties->getProperty("Ice.MonitorConnections").empty())
	{
	    defaultProperties->setProperty("Ice.MonitorConnections", "60");
	}

	//
	// We do not need to set Ice.RetryIntervals to -1, i.e., we do
	// not have to disable connection retry. It is safe for
	// Glacier2 to retry outgoing connections to servers. Retry
	// for incoming connections from clients must be disabled in
	// the clients.
	//
    }
    catch(const Exception& e)
    {
	cerr << e << endl;
	exit(EXIT_FAILURE);
    }

    RouterApp app;
    return app.main(argc, argv);
}
