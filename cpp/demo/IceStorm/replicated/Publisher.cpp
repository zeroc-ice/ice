// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <IceStorm/IceStorm.h>
#include <IceGrid/Query.h>

#include <Clock.h>

using namespace std;
using namespace Demo;

class Publisher : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    Publisher app;
    return app.main(argc, argv, "config.pub");
}

int
Publisher::run(int argc, char* argv[])
{
    Ice::PropertiesPtr properties = communicator()->getProperties();

    IceGrid::QueryPrx query = IceGrid::QueryPrx::uncheckedCast(communicator()->stringToProxy("DemoIceGrid/Query"));
    IceStorm::TopicManagerPrx manager = 
        IceStorm::TopicManagerPrx::checkedCast(query->findObjectByType("::IceStorm::TopicManager"));

    string topicName = "time";
    if(argc != 1)
    {
        topicName = argv[1];
    }

    IceStorm::TopicPrx topic;
    try
    {
        topic = manager->retrieve(topicName);
    }
    catch(const IceStorm::NoSuchTopic&)
    {
        cerr << appName() << ": topics not created yet, run subscriber." << endl;
	return EXIT_FAILURE;
    }

    //
    // Get the topic's publisher object, verify that it supports
    // the Clock type, and create a oneway Clock proxy (for efficiency
    // reasons). Use per-request load balancing with round robin from the
    // IceGrid locator for the publisher object.
    //
    ClockPrx clock = ClockPrx::uncheckedCast(topic->getPublisher()->ice_oneway()->
    					     ice_locatorCacheTimeout(0)->ice_connectionCached(false));

    try
    {
        while(true)
        {
            clock->tick(IceUtil::Time::now().toDateTime());
#ifdef _WIN32
            Sleep(1000);
#else
            sleep(1);
#endif
        }
    }
    catch(const Ice::CommunicatorDestroyedException&)
    {
        // Ignore
    }

    return EXIT_SUCCESS;
}
