// **********************************************************************
//
// Copyright (c) 2003
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
#include <IceStorm/IceStorm.h>
#include <Event.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;

int
run(int argc, char* argv[], const CommunicatorPtr& communicator)
{
    PropertiesPtr properties = communicator->getProperties();

    StringSeq args = argsToStringSeq(argc, argv);
    args = properties->parseCommandLineOptions("IceStorm", args);
    stringSeqToArgs(args, argc, argv);

    const char* managerProxyProperty = "IceStorm.TopicManager.Proxy";
    string managerProxy = properties->getProperty(managerProxyProperty);
    if(managerProxy.empty())
    {
	cerr << argv[0] << ": property `" << managerProxyProperty << "' is not set" << endl;
	return EXIT_FAILURE;
    }

    ObjectPrx base = communicator->stringToProxy(managerProxy);
    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(base);
    if(!manager)
    {
	cerr << argv[0] << ": `" << managerProxy << "' is not running" << endl;
	return EXIT_FAILURE;
    }

    TopicPrx fed1;
    try
    {
	fed1 = manager->retrieve("fed1");
    }
    catch(const NoSuchTopic& e)
    {
	cerr << argv[0] << ": NoSuchTopic: " << e.name << endl;
	return EXIT_FAILURE;
	
    }

    TopicPrx fed2;
    try
    {
	fed2 = manager->retrieve("fed2");
    }
    catch(const NoSuchTopic& e)
    {
	cerr << argv[0] << ": NoSuchTopic: " << e.name << endl;
	return EXIT_FAILURE;
	
    }

    TopicPrx fed3;
    try
    {
	fed3 = manager->retrieve("fed3");
    }
    catch(const NoSuchTopic& e)
    {
	cerr << argv[0] << ": NoSuchTopic: " << e.name << endl;
	return EXIT_FAILURE;
	
    }

    ObjectPrx obj = fed1->getPublisher();
    if(!obj->ice_isDatagram())
    {
        if(!EventPrx::checkedCast(obj->ice_twoway()))
        {
            cerr << argv[0] << ": checkedCast failed for fed1 publisher" << endl;
            return EXIT_FAILURE;
        }
        obj = obj->ice_oneway();
    }
    EventPrx eventFed1 = EventPrx::uncheckedCast(obj);

    obj = fed2->getPublisher();
    if(!obj->ice_isDatagram())
    {
        if(!EventPrx::checkedCast(obj->ice_twoway()))
        {
            cerr << argv[0] << ": checkedCast failed for fed2 publisher" << endl;
            return EXIT_FAILURE;
        }
        obj = obj->ice_oneway();
    }
    EventPrx eventFed2 = EventPrx::uncheckedCast(obj);

    obj = fed3->getPublisher();
    if(!obj->ice_isDatagram())
    {
        if(!EventPrx::checkedCast(obj->ice_twoway()))
        {
            cerr << argv[0] << ": checkedCast failed for fed3 publisher" << endl;
            return EXIT_FAILURE;
        }
        obj = obj->ice_oneway();
    }
    EventPrx eventFed3 = EventPrx::uncheckedCast(obj);

    Ice::Context context;
    int i;
    context["cost"] = "0";
    for(i = 0; i < 10; ++i)
    {
	eventFed1->pub("fed1:0", context);
    }

    context["cost"] = "10";
    for(i = 0; i < 10; ++i)
    {
	eventFed1->pub("fed1:10", context);
    }

    context["cost"] = "15";
    for(i = 0; i < 10; ++i)
    {
	eventFed1->pub("fed1:15", context);
    }

    context["cost"] = "0";
    for(i = 0; i < 10; ++i)
    {
	eventFed2->pub("fed2:0", context);
    }

    context["cost"] = "5";
    for(i = 0; i < 10; ++i)
    {
	eventFed2->pub("fed2:5", context);
    }

    context["cost"] = "0";
    for(i = 0; i < 10; ++i)
    {
	eventFed3->pub("fed3:0", context);
    }

    //
    // Sleep for 4 seconds before shutting down.
    //
#ifdef _WIN32
    Sleep(4*1000);
#else
    sleep(4);
#endif

    eventFed3->pub("shutdown");

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    CommunicatorPtr communicator;

    try
    {
	communicator = initialize(argc, argv);
	status = run(argc, argv, communicator);
    }
    catch(const Exception& ex)
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
	catch(const Exception& ex)
	{
	    cerr << ex << endl;
	    status = EXIT_FAILURE;
	}
    }

    return status;
}
