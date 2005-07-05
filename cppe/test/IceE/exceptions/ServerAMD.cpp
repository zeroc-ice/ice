// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <TestAMDI.h>

using namespace std;

int
run(int argc, char* argv[], const IceE::CommunicatorPtr& communicator)
{
    IceE::PropertiesPtr properties = communicator->getProperties();
    properties->setProperty("IceE.Warn.Dispatch", "0");
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", "default -p 12345 -t 10000");
    IceE::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    IceE::ObjectPtr object = new ThrowerI(adapter);
    adapter->add(object, IceE::stringToIdentity("thrower"));
    adapter->activate();
    communicator->waitForShutdown();
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    IceE::CommunicatorPtr communicator;

    try
    {
	communicator = IceE::initialize(argc, argv);
	status = run(argc, argv, communicator);
    }
    catch(const IceE::Exception& ex)
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
	catch(const IceE::Exception& ex)
	{
	    cerr << ex << endl;
	    status = EXIT_FAILURE;
	}
    }

    return status;
}
