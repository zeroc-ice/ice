// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
    int port = 0;
    for (int i = 1; i < argc; ++i)
    {
	if (argv[i][0] == '-')
	{
	    cerr << argv[0] << ": unknown option `" << argv[i] << "'" << endl;
	    usage(argv[0]);
	    return EXIT_FAILURE;
	}

	if (port > 0)
	{
	    cerr << argv[0] << ": only one port can be specified" << endl;
	    usage(argv[0]);
	    return EXIT_FAILURE;
	}

	port = atoi(argv[i]);
    }

    if (port <= 0)
    {
	cerr << argv[0] << ": no port specified" << endl;
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    ostringstream endpts;
    endpts << "tcp -p " << port;
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("TestAdapter", endpts.str());
    Ice::ObjectPtr object = new TestI(adapter);
    adapter->add(object, "test");
    adapter->activate();
    communicator->waitForShutdown();
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
	communicator = Ice::initialize(argc, argv);
	status = run(argc, argv, communicator);

    }
    catch(const Ice::LocalException& ex)
    {
	cerr << ex << endl;
	status = EXIT_FAILURE;
    }

    if (communicator)
    {
	try
	{
	    communicator->destroy();
	}
	catch(const Ice::LocalException& ex)
	{
	    cerr << ex << endl;
	    status = EXIT_FAILURE;
	}
    }

    return status;
}
