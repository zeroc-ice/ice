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
#include <IcePack/AdminI.h>
#include <IcePack/Forward.h>
#ifndef _WIN32
#   include <signal.h>
#   include <sys/wait.h>
#endif

using namespace std;
using namespace Ice;
using namespace IcePack;

class Server : public Application
{
public:

    void usage();
    virtual int run(int, char*[]);
};

#ifndef _WIN32
static void
childHandler(int)
{
    wait(0);
}
#endif

int
main(int argc, char* argv[])
{
#ifndef _WIN32
    //
    // This application forks, so we need a signal handler for child
    // termination.
    //
    struct sigaction action;
    action.sa_handler = childHandler;
    sigemptyset(&action.sa_mask);
    sigaddset(&action.sa_mask, SIGCHLD);
    action.sa_flags = 0;
    sigaction(SIGCHLD, &action, 0);
#endif

    addArgumentPrefix("IcePack");
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
	"--nowarn             Don't print any security warnings.\n"
	;
}

int
Server::run(int argc, char* argv[])
{
    bool nowarn = false;
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
	else if (strcmp(argv[i], "--nowarn") == 0)
	{
	    nowarn = true;
	}
	else
	{
	    cerr << appName() << ": unknown option `" << argv[i] << "'" << endl;
	    usage();
	    return EXIT_FAILURE;
	}
    }

    PropertiesPtr properties = communicator()->getProperties();

    const char* adminEndpointsProperty = "IcePack.Admin.Endpoints";
    string adminEndpoints = properties->getProperty(adminEndpointsProperty);
    if (!adminEndpoints.empty() && !nowarn)
    {
	cerr << appName() << ": warning: administrative endpoints property `" << adminEndpointsProperty << "' enabled"
	     << endl;
    }

    const char* forwardEndpointsProperty = "IcePack.Forward.Endpoints";
    string forwardEndpoints = properties->getProperty(forwardEndpointsProperty);
    if (forwardEndpoints.empty())
    {
	cerr << appName() << ": property `" << forwardEndpointsProperty << "' is not set" << endl;
	return EXIT_FAILURE;
    }

    AdminPtr admin = new AdminI(communicator());
    ServantLocatorPtr forward = new Forward(communicator(), admin);

    if (adminEndpoints.length() != 0)
    {
	ObjectAdapterPtr adminAdapter = communicator()->createObjectAdapterFromProperty("Admin",
											adminEndpointsProperty);
	adminAdapter->add(admin, stringToIdentity("admin"));
	adminAdapter->activate();
    }

    ObjectAdapterPtr forwardAdapter = communicator()->createObjectAdapterFromProperty("Forward",
										      forwardEndpointsProperty);
    forwardAdapter->addServantLocator(forward, "");
    forwardAdapter->activate();

    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
