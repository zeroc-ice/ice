// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/Options.h>
#include <IceStorm/IceStorm.h>
#include <Event.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Test;

int
run(int argc, char* argv[], const CommunicatorPtr& communicator)
{
    IceUtilInternal::Options opts;
    opts.addOpt("", "events", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "oneway");
    opts.addOpt("", "maxQueueTest");

    try
    {
        opts.parse(argc, (const char**)argv);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        cerr << argv[0] << ": " << e.reason << endl;
        return EXIT_FAILURE;
    }

    int events = 1000;
    string s = opts.optArg("events");
    if(!s.empty())
    {
        events = atoi(s.c_str());
    }
    if(events <= 0)
    {
        cerr << argv[0] << ": events must be > 0." << endl;
        return EXIT_FAILURE;
    }

    bool oneway = opts.isSet("oneway");
    bool maxQueueTest = opts.isSet("maxQueueTest");

    PropertiesPtr properties = communicator->getProperties();
    const char* managerProxyProperty = "IceStormAdmin.TopicManager.Default";
    string managerProxy = properties->getProperty(managerProxyProperty);
    if(managerProxy.empty())
    {
        cerr << argv[0] << ": property `" << managerProxyProperty << "' is not set" << endl;
        return EXIT_FAILURE;
    }

    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(
        communicator->stringToProxy(managerProxy));
    if(!manager)
    {
        cerr << argv[0] << ": `" << managerProxy << "' is not running" << endl;
        return EXIT_FAILURE;
    }

    TopicPrx topic;
    try
    {
        topic = manager->retrieve("fed1");
    }
    catch(const NoSuchTopic& e)
    {
        cerr << argv[0] << ": NoSuchTopic: " << e.name << endl;
        return EXIT_FAILURE;

    }

    EventPrx twowayProxy = EventPrx::uncheckedCast(topic->getPublisher()->ice_twoway());
    EventPrx proxy;
    if(oneway)
    {
        proxy = EventPrx::uncheckedCast(twowayProxy->ice_oneway());
    }
    else
    {
        proxy = twowayProxy;
    }

    for(int i = 0; i < events; ++i)
    {
        if(maxQueueTest && i == 10)
        {
            // Sleep one seconds to give some time to IceStorm to connect to the subscriber
            IceUtil::ThreadControl::sleep(IceUtil::Time::seconds(1));
        }
        proxy->pub(i);
    }

    if(oneway)
    {
        //
        // Before we exit, we ping all proxies as twoway, to make sure
        // that all oneways are delivered.
        //
        twowayProxy->ice_ping();
    }

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
