// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>

using namespace std;

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    Ice::PropertiesPtr properties = communicator->getProperties();
    properties->setProperty("Ice.ConnectionWarnings", "0");
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", "default -p 12345 -t 2000");
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectPtr object = new ThrowerI(adapter);
    adapter->add(object, Ice::stringToIdentity("thrower"));
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
