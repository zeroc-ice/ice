// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
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

int
main(int argc, char* argv[])
{
    PropertiesPtr defaultProperties;
    try
    {
	defaultProperties = getDefaultProperties(argc, argv);
        StringSeq args = argsToStringSeq(argc, argv);
        args = defaultProperties->parseCommandLineOptions("IceStorm", args);
        stringSeqToArgs(args, argc, argv);
    }
    catch(const Exception& ex)
    {
	cerr << argv[0] << ": " << ex << endl;
	return EXIT_FAILURE;
    }

    string dbEnvName = defaultProperties->getPropertyWithDefault("IceStorm.DBEnvName", "db");

    Server app(dbEnvName);
    return app.main(argc, argv);
}

void
IceStorm::Server::usage()
{
    cerr << "Usage: " << appName() << " [options]\n";
    cerr <<	
	"Options:\n"
	"-h, --help           Show this message.\n"
	"-v, --version        Display the Ice version.\n"
	;
}

int
IceStorm::Server::runFreeze(int argc, char* argv[], const Freeze::DBEnvironmentPtr& dbEnv)
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

    Freeze::DBPtr dbTopicManager = dbEnv->openDB("topicmanager", true);

    TraceLevelsPtr traceLevels = new TraceLevels("IceStorm", communicator()->getProperties(),
    						 communicator()->getLogger());
    ObjectAdapterPtr adapter = communicator()->createObjectAdapterFromProperty("TopicManager",
									       "IceStorm.TopicManager.Endpoints");
    TopicManagerIPtr manager = new TopicManagerI(communicator(), adapter, traceLevels, dbEnv, dbTopicManager);
    adapter->add(manager, stringToIdentity("TopicManager"));

    //
    // Everything ok, let's go.
    //
    shutdownOnInterrupt();
    adapter->activate();
    communicator()->waitForShutdown();
    ignoreInterrupt();

    //
    // It's necessary to reap all destroyed topics on shutdown.
    //
    manager->reap();

    return EXIT_SUCCESS;
}
