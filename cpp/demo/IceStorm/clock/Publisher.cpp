// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <IceStorm/IceStorm.h>

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
    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(
	communicator()->propertyToProxy("IceStorm.TopicManager.Proxy"));
    if(!manager)
    {
	cerr << appName() << ": invalid proxy" << endl;
	return EXIT_FAILURE;
    }

    string topicName = "time";
    if(argc != 1)
    {
	topicName = argv[1];
    }

    //
    // Retrieve the topic.
    //
    IceStorm::TopicPrx topic;
    try
    {
	topic = manager->retrieve(topicName);
    }
    catch(const IceStorm::NoSuchTopic&)
    {
	try
	{
	    topic = manager->create(topicName);
	}
	catch(const IceStorm::TopicExists&)
	{
	    cerr << appName() << ": temporary failure. try again." << endl;
	    return EXIT_FAILURE;
	}
    }

    //
    // Get the topic's publisher object, the Clock type, and create a
    // oneway Clock proxy (for efficiency reasons).
    //
    ClockPrx clock = ClockPrx::uncheckedCast(topic->getPublisher()->ice_oneway());

    cout << "publishing tick events. Press ^C to terminate the application." << endl;
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
