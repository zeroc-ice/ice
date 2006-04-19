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
#include <IceUtil/UUID.h>

#include <ClockI.h>

#include <map>

using namespace std;
using namespace Demo;

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

    Ice::ObjectPrx base = communicator()->stringToProxy(proxy);
    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(base);
    if(!manager)
    {
	cerr << appName() << ": invalid proxy" << endl;
	return EXIT_FAILURE;
    }

    //
    // Gather the set of topics to which to subscribe. It is either
    // the set provided on the command line, or the topic "time".
    //
    Ice::StringSeq topics;
    if(argc > 1)
    {
	for(int i = 1; i < argc; ++i)
	{
	    topics.push_back(argv[i]);
	}
    }
    else
    {
	topics.push_back("time");
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
    Ice::ObjectPtr clock = new ClockI();

    //
    // List of all subscribers.
    //
    map<string, Ice::ObjectPrx> subscribers;

    //
    // Add the servant to the adapter for each topic. A ServantLocator
    // could have been used for the same purpose.
    //
    for(Ice::StringSeq::iterator p = topics.begin(); p != topics.end(); ++p)
    {
	//
	// Add a Servant for the Ice Object.
	//
	Ice::ObjectPrx object = adapter->addWithUUID(clock);
	try
	{
            IceStorm::TopicPrx topic = manager->retrieve(*p);
	    topic->subscribe(qos, object);
	}
	catch(const IceStorm::NoSuchTopic& e)
	{
	    cerr << appName() << ": " << e << " name: " << e.name << endl;
	    break;
	}

	//
	// Add to the set of subscribers _after_ subscribing. This
	// ensures that only subscribed subscribers are unsubscribed
	// in the case of an error.
	//
	subscribers[*p] = object;
    }

    //
    // Unless there is a subscriber per topic then there was some
    // problem. If there was an error the application should terminate
    // without accepting any events.
    //
    if(subscribers.size() == topics.size())
    {
	adapter->activate();
	shutdownOnInterrupt();
	communicator()->waitForShutdown();
    }

    //
    // Unsubscribe all subscribed objects.
    //
    for(map<string,Ice::ObjectPrx>::const_iterator q = subscribers.begin(); q != subscribers.end(); ++q)
    {
	try
	{
            IceStorm::TopicPrx topic = manager->retrieve(q->first);
	    topic->unsubscribe(q->second);
	}
	catch(const IceStorm::NoSuchTopic& e)
	{
	    cerr << appName() << ": " << e << " name: " << e.name << endl;
	}
    }

    return EXIT_SUCCESS;
}
