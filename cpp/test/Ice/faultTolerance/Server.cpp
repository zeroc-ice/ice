// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>

using namespace std;

void
usage(const char* n)
{
    cerr << "Usage: " << n << " port\n";
}

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    if(argc != 2)
    {
        usage(argv[0]);
	return EXIT_FAILURE;
    }

    int port = 0;
    for(int i = 1; i < argc; ++i)
    {
	if(argv[i][0] == '-')
	{
	    cerr << argv[0] << ": unknown option `" << argv[i] << "'" << endl;
	    usage(argv[0]);
	    return EXIT_FAILURE;
	}

	if(port > 0)
	{
	    cerr << argv[0] << ": only one port can be specified" << endl;
	    usage(argv[0]);
	    return EXIT_FAILURE;
	}

	port = atoi(argv[i]);
    }

    if(port <= 0)
    {
	cerr << argv[0] << ": no port specified" << endl;
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    CleanerPtr cleaner = new CleanerI(communicator);

    ostringstream endpts;
    endpts << "default  -p " << port;
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", endpts.str());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectPtr object = new TestI(adapter, cleaner);
    adapter->add(object, Ice::stringToIdentity("test"));

    int dummyArgc = 0;
    char* dummyArgv[] = { 0 };

    Ice::CommunicatorPtr cleanupCommunicator = Ice::initialize(dummyArgc, dummyArgv);

    ostringstream str;
    str << (port + 1);
    string cleanupPort = str.str();
    cleanupCommunicator->getProperties()->setProperty("CleanupAdapter.Endpoints", "default -p " + cleanupPort);

    Ice::ObjectAdapterPtr cleanupAdapter = cleanupCommunicator->createObjectAdapter("CleanupAdapter");
    cleanupAdapter->add(cleaner, Ice::stringToIdentity("Cleaner"));

    string adapterReady = cleanupCommunicator->getProperties()->getProperty("Ice.PrintAdapterReady");
    cleanupCommunicator->getProperties()->setProperty("Ice.PrintAdapterReady", "0");

    cleanupAdapter->activate();
    cleanupCommunicator->getProperties()->setProperty("Ice.PrintAdapterReady", adapterReady);

    adapter->activate();
    communicator->waitForShutdown();

    cleaner->cleanup();

    cleanupCommunicator->destroy();

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
        //
	// In this test, we need a longer server idle time, otherwise
	// our test servers may time out before they are used in the
	// test.
	//
	Ice::PropertiesPtr properties = Ice::getDefaultProperties(argc, argv);
	properties->setProperty("Ice.ServerIdleTime", "120"); // Two minutes.

	communicator = Ice::initialize(argc, argv);
	status = run(argc, argv, communicator);
    }
    catch(const Ice::Exception& ex)
    {
	cerr << ex << endl;
	status = EXIT_FAILURE;
    }

    if(communicator)
    {
	try
	{
	    communicator->destroy();
	}
	catch(const Ice::Exception& ex)
	{
	    cerr << ex << endl;
	    status = EXIT_FAILURE;
	}
    }

    return status;
}
