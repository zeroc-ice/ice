// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>

using namespace std;

void
usage(const char* n)
{
    cerr << "Usage: " << n << " [--fwd reference] port\n";
}

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    int port = 0;
    Ice::ObjectPrx fwd;
    for(int i = 1; i < argc; ++i)
    {
	if(strcmp(argv[i], "--fwd") == 0)
	{
	    if(i + 1 >= argc)
            {
		cerr << argv[0] << ": argument expected for`" << argv[i] << "'" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
            }
	    
	    fwd = communicator->stringToProxy(argv[++i]);
	}
	else if(argv[i][0] == '-')
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

    ostringstream endpts;
    endpts << "default  -p " << port;
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", endpts.str());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectPtr object = new TestI(adapter, fwd);
    adapter->add(object, Ice::stringToIdentity("test"));
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
