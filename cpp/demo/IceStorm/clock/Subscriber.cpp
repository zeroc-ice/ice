// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Application.h>
#include <IceStorm/IceStorm.h>
#include <ClockI.h>

using namespace std;
using namespace Ice;

class Subscriber : public Application
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
    PropertiesPtr properties = communicator()->getProperties();
    const char* endpointsProperty = "IceStorm.TopicManager.Endpoints";
    std::string endpoints = properties->getProperty(endpointsProperty);
    if (endpoints.empty())
    {
	cerr << appName() << ": property `" << endpointsProperty << "' not set" << endl;
	return EXIT_FAILURE;
    }

    ObjectPrx base = communicator()->stringToProxy("TopicManager:" + endpoints);
    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(base);
    if (!manager)
    {
	cerr << appName() << ": invalid object reference" << endl;
	return EXIT_FAILURE;
    }

    //
    // Gather the set of topics to subscribe to. It is either the set
    // provided on the command line, or the topic "time".
    //
    IceStorm::StringSeq topics;
    if (argc > 1)
    {
	for (int i = 1; i < argc; ++i)
	{
	    topics.push_back(argv[i]);
	}
    }
    else
    {
	//
	// The set of topics to which to subscribe
	//
	topics.push_back("time");
    }

    //
    // Create the servant to receive the events. Then add the servant
    // to the adapter for the given topics. Alternatively we could
    // have used a ServantLocator for the same purpose. Note that any
    // of the activated proxies will do since it the proxy is only a
    // template from which the actual proxy is created by IceStorm.
    //
    ObjectAdapterPtr adapter = communicator()->createObjectAdapterWithEndpoints("ClockAdapter", "tcp");
    ObjectPtr clock = new ClockI();
    ObjectPrx object;

    assert(!topics.empty());
    Ice::Identity ident;
    ident.category = "events";

    for (IceStorm::StringSeq::iterator p = topics.begin(); p != topics.end(); ++p)
    {
	ident.name = *p;
	object = adapter->add(clock, ident);
    }

    //
    // The requested quality of service. This requests "reliability" =
    // "batch". This asks IceStorm to send events to the subscriber in
    // batches at regular intervals.
    //
    IceStorm::QoS qos;
    qos["reliability"] = "batch";
    try
    {
	manager->subscribe("events", qos, topics, object);
    }
    catch(const IceStorm::NoSuchTopic& e)
    {
	cerr << appName() << ": NoSuchTopic: " << e.name << endl;
	return EXIT_FAILURE;
    }

    adapter->activate();

    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
