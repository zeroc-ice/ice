// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>
#include <TestCommon.h>
#include <Event.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Test;

int
run(int, char* argv[], const CommunicatorPtr& communicator)
{
    PropertiesPtr properties = communicator->getProperties();
    const char* managerProxyProperty = "IceStormAdmin.TopicManager.Default";
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

    EventPrx eventFed1 = EventPrx::uncheckedCast(fed1->getPublisher()->ice_oneway());
    EventPrx eventFed2 = EventPrx::uncheckedCast(fed2->getPublisher()->ice_oneway());
    EventPrx eventFed3 = EventPrx::uncheckedCast(fed3->getPublisher()->ice_oneway());

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
    // Before we exit, we ping all proxies as twoway, to make sure
    // that all oneways are delivered.
    //
    EventPrx::uncheckedCast(eventFed1->ice_twoway())->ice_ping();
    EventPrx::uncheckedCast(eventFed2->ice_twoway())->ice_ping();
    EventPrx::uncheckedCast(eventFed3->ice_twoway())->ice_ping();

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    CommunicatorPtr communicator;
    InitializationData initData = getTestInitData(argc, argv);
    try
    {
        communicator = initialize(argc, argv, initData);
        status = run(argc, argv, communicator);
    }
    catch(const Exception& ex)
    {
        cerr << ex << endl;
        status = EXIT_FAILURE;
    }

    if(communicator)
    {
        communicator->destroy();
    }

    return status;
}
