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
#include <TestCommon.h>
#include <Test.h>

using namespace std;

void
usage(const char* n)
{
    cerr << "Usage: " << n << " port port\n";
}

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    int port = 0;
    int lastPort = 0;
    for (int i = 1; i < argc; ++i)
    {
	if (argv[i][0] == '-')
	{
	    cerr << argv[0] << ": unknown option `" << argv[i] << "'" << endl;
	    usage(argv[0]);
	    return EXIT_FAILURE;
	}

	if (port == 0)
	{
	    port = atoi(argv[i]);
	}
	else if(lastPort == 0)
	{
	    lastPort = atoi(argv[i]);
	}
	else
	{
	    cerr << argv[0] << ": only two ports can be specified" << endl;
	    usage(argv[0]);
	    return EXIT_FAILURE;
	}
    }

    if (port == 0 || lastPort == 0)
    {
	cerr << argv[0] << ": no ports specified" << endl;
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    void allTests(const Ice::CommunicatorPtr&, int, int);
    allTests(communicator, port, lastPort);
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
    catch(const Ice::Exception& ex)
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
	catch(const Ice::Exception& ex)
	{
	    cerr << ex << endl;
	    status = EXIT_FAILURE;
	}
    }

    return status;
}
