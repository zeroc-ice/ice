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

#include <map>

using namespace std;
using namespace Demo;

class ClockI : public Clock
{
public:

    virtual void
    tick(const string& time, const Ice::Current&)
    {
	cout << time << endl;
    }
};

class Subscriber : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    Subscriber app;
    return app.main(argc, argv, "config.sub");
}

int
Subscriber::run(int argc, char* argv[])
{
    Ice::PropertiesPtr properties = communicator()->getProperties();

    const string proxyProperty = "IceStorm.TopicManager.Proxy";
    string proxy = properties->getProperty(proxyProperty);
    if(proxy.empty())
    {
	cerr << appName() << ": property `" << proxyProperty << "' not set" << endl;
	return EXIT_FAILURE;
    }

    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(
	communicator()->stringToProxy(proxy));
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

    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Clock.Subscriber");

    //
    // Add a Servant for the Ice Object.
    //
    Ice::ObjectPrx subscriber = adapter->addWithUUID(new ClockI);

    //
    // This demo requires no quality of service, so it will use
    // the defaults.
    //
    IceStorm::QoS qos;

    topic->subscribe(qos, subscriber);
    adapter->activate();

    shutdownOnInterrupt();
    communicator()->waitForShutdown();

    //
    // Unsubscribe all subscribed objects.
    //
    topic->unsubscribe(subscriber);

    return EXIT_SUCCESS;
}
