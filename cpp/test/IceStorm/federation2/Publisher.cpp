// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <IceUtil/Options.h>
#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>
#include <Event.h>
#include <TestCommon.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Test;

int
run(int argc, char* argv[], const CommunicatorPtr& communicator)
{
    IceUtilInternal::Options opts;
    opts.addOpt("", "count", IceUtilInternal::Options::NeedArg);

    try
    {
        opts.parse(argc, (const char**)argv);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        cerr << argv[0] << ": " << e.reason << endl;
        return EXIT_FAILURE;
    }

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

    EventPrx eventFed1 = EventPrx::uncheckedCast(fed1->getPublisher()->ice_oneway());

    string arg = opts.optArg("count");
    int count = 1;
    if(arg.empty())
    {
        count = atoi(arg.c_str());
    }

    while(true)
    {
        for(int i = 0; i < 10; ++i)
        {
            eventFed1->pub("fed1");
        }
        //
        // Before we exit, we ping all proxies as twoway, to make sure
        // that all oneways are delivered.
        //
        EventPrx::uncheckedCast(eventFed1->ice_twoway())->ice_ping();

        if(count == 0)
        {
            break;
        }
        --count;
        IceUtil::ThreadControl::sleep(IceUtil::Time::seconds(1));
    }

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
