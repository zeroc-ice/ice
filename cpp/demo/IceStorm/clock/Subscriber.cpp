// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Application.h>
#include <IceStorm/IceStorm.h>
#include <IceUtil/UUID.h>

#include <ClockI.h>

#include <list>

using namespace std;

class Subscriber : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    Subscriber app;
    return app.main(argc, argv, "config");
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
    // Create a UUID to use in the name field of the the object
    // identity. The same UUID can be used for the identity of each
    // Subscriber object since the category field will differ.
    //
    string uuid = IceUtil::generateUUID();

    //
    // List of all subscribers.
    //
    list<Ice::ObjectPrx> subscribers;

    //
    // Add the servant to the adapter for the given
    // topics. Alternatively a ServantLocator could have been used for
    // the same purpose.
    //
    for(Ice::StringSeq::iterator p = topics.begin(); p != topics.end(); ++p)
    {
	//
	// The category is the name of the topic.
	//
	Ice::Identity ident;
	ident.category = *p;
	ident.name = uuid;

	//
	// Add a Servant for the Ice Object.
	//
	Ice::ObjectPrx object = adapter->add(clock, ident);
	try
	{
	    manager->subscribe(qos, object);
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
	subscribers.push_back(object);
    }

    //
    // Unless there is a subscriber per topic then there was some
    // problem. If there was an error the application should terminate
    // without accepting any events.
    //
    if(subscribers.size() == topics.size())
    {
	adapter->activate();
	communicator()->waitForShutdown();
    }

    //
    // Unsubscribe all subscribed objects.
    //
    for(list<Ice::ObjectPrx>::const_iterator q = subscribers.begin(); q != subscribers.end(); ++q)
    {
	try
	{
	    manager->unsubscribe(*q);
	}
	catch(const IceStorm::NoSuchTopic& e)
	{
	    cerr << appName() << ": " << e << " name: " << e.name << endl;
	}
    }

    return EXIT_SUCCESS;
}
