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
    cerr << "Usage: " << n << " port...\n";
}

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    vector<int> ports;
    for (int i = 1; i < argc; ++i)
    {
	if (argv[i][0] == '-')
	{
	    cerr << argv[0] << ": unknown option `" << argv[i] << "'" << endl;
	    usage(argv[0]);
	    return EXIT_FAILURE;
	}

	ports.push_back(atoi(argv[i]));
    }

    if (ports.empty())
    {
	cerr << argv[0] << ": no ports specified" << endl;
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    void allTests(const Ice::CommunicatorPtr, const vector<int>&&);
    allTests(communicator, ports);
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
