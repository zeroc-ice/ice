// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>

using namespace std;

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", "default -p 12345 -t 10000");
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectPtr object = new MyDerivedClassI(adapter, Ice::stringToIdentity("test"));
    adapter->add(object, Ice::stringToIdentity("test"));
    adapter->activate();

    //
    // Make a separate adapter with a servant locator. We use this to test
    // that ::Ice::Context is correctly passed to checkedCast() operation.
    //
    communicator->getProperties()->setProperty("CheckedCastAdapter.Endpoints", "default -p 12346 -t 10000");
    adapter = communicator->createObjectAdapter("CheckedCastAdapter");
    Ice::ServantLocatorPtr checkedCastLocator = new CheckedCastLocator();
    adapter->addServantLocator(checkedCastLocator, "");
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
