// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Freeze/Application.h>
#include <IceStorm/TopicManagerI.h>
#include <IceStorm/TraceLevels.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;

namespace IceStorm
{

class Server : public Freeze::Application
{
public:

    Server(const string& dbEnvName) :
	Freeze::Application(dbEnvName)
    {
    }

    void usage();
    virtual int runFreeze(int, char*[], const Freeze::DBEnvironmentPtr&);
};

} // End namespace IceStorm

//
// TODO: get rid of this when the prescanning of the argument list is
// not needed.
//
static void
usage(const char* appName)
{
    cerr << "Usage: " << appName << " [options]\n";
    cerr <<	
	"Options:\n"
	"-h, --help           Show this message.\n"
	"-v, --version        Display the Ice version.\n"
        "--dbdir directory    Set the database location.\n"
	;
}

int
main(int argc, char* argv[])
{
    //
    // TODO: This is a hack.
    //
    // Prescan the argument list for --dbdir. Process & consume if
    // found.
    //
    string dbdir = "db";
    int i = 1;
    while (i < argc)
    {
	if (strcmp(argv[i], "--dbdir") == 0)
	{
	    if (i +1 >= argc)
	    {
		usage(argv[0]);
		return EXIT_FAILURE;
	    }
	    dbdir = argv[i + 1];
	    //
	    // Consume arguments
	    //
	    while (i < argc - 2)
	    {
		argv[i] = argv[i + 2];
		++i;
	    }
	    argc -= 2;
	}
	else
	{
	    ++i;
	}
    }
    
    addArgumentPrefix("IceStorm");
    Server app(dbdir);
    return app.main(argc, argv);
}

void
IceStorm::Server::usage()
{
    //
    // TODO: This is only needed because of the forced prescan of the
    // arguments.
    //
    ::usage(appName());
}

int
IceStorm::Server::runFreeze(int argc, char* argv[], const Freeze::DBEnvironmentPtr& dbEnv)
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

    Freeze::DBPtr dbTopicManager = dbEnv->openDB("topicmanager");

    TraceLevelsPtr traceLevels = new TraceLevels(communicator()->getProperties());
    ObjectAdapterPtr adapter = communicator()->createObjectAdapterFromProperty("TopicManager",
									       "IceStorm.TopicManager.Endpoints");
    ObjectPtr object = new TopicManagerI(communicator(), adapter, traceLevels, dbTopicManager);
    adapter->add(object, stringToIdentity("TopicManager"));
    adapter->activate();

    shutdownOnInterrupt();
    communicator()->waitForShutdown();
    ignoreInterrupt();

    // TODO: topic manager ::reap()

    return EXIT_SUCCESS;
}
