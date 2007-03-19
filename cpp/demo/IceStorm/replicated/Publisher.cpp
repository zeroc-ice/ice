// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
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

    IceStorm::TopicManagerPrx manager = 
        IceStorm::TopicManagerPrx::checkedCast(communicator()->stringToProxy("DemoIceStorm/TopicManager"));
    if(manager == 0)
    {
        cerr << appName() << ": no topic manager found, make sure application was deployed." << endl;
        return EXIT_FAILURE;
    }

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
    // Get the topic's publisher object, and configure a Clock proxy
    // with per-request load balancing.
    //
    ClockPrx clock = ClockPrx::uncheckedCast(topic->getPublisher()->ice_oneway()->ice_connectionCached(false));

    try
    {
        while(true)
        {
            clock->tick(IceUtil::Time::now().toDateTime());
            IceUtil::ThreadControl::sleep(IceUtil::Time::seconds(1));
        }
    }
    catch(const Ice::CommunicatorDestroyedException&)
    {
        // Ignore
    }

    return EXIT_SUCCESS;
}
