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
#include <IceStorm/TopicManagerI.h>
#include <IceStorm/TraceLevels.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;

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

    //PropertiesPtr properties = communicator()->getProperties();

    TraceLevelsPtr traceLevels = new TraceLevels(communicator()->getProperties());
    ObjectAdapterPtr adapter = communicator()->createObjectAdapter("TopicManager");
    ObjectPtr object = new TopicManagerI(communicator(), adapter, traceLevels);
    adapter->add(object, "TopicManager");
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
