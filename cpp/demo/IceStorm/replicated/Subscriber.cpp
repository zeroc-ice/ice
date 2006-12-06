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
#include <IceGrid/Query.h>
#include <IceUtil/UUID.h>

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

    IceGrid::QueryPrx query = IceGrid::QueryPrx::uncheckedCast(communicator()->stringToProxy("DemoIceGrid/Query"));
    Ice::ObjectProxySeq managers = query->findAllObjectsByType("::IceStorm::TopicManager");

    string topicName = "time";
    if(argc != 1)
    {
        topicName = argv[1];
    }

    //
    // Set the requested quality of service "reliability" =
    // "batch". This tells IceStorm to send events to the subscriber
    // in batches at regular intervals.
    //
    IceStorm::QoS qos;
    qos["reliability"] = "batch";

    //
    // Create the servant to receive the events.
    //
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Clock.Subscriber");
    Ice::ObjectPrx clock = adapter->addWithUUID(new ClockI);

    vector<IceStorm::TopicPrx> topics;
    for(Ice::ObjectProxySeq::const_iterator p = managers.begin(); p != managers.end(); ++p)
    {
	//
	// Add a Servant for the Ice Object.
	//
	IceStorm::TopicPrx topic;
	IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(*p);
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
	topic->subscribe(qos, clock);
	topics.push_back(topic);
    }

    adapter->activate();
    shutdownOnInterrupt();
    communicator()->waitForShutdown();

    //
    // Unsubscribe all subscribed objects.
    //
    for(unsigned int i = 0; i < topics.size(); ++i)
    {
        topics[i]->unsubscribe(clock);
    }

    return EXIT_SUCCESS;
}
