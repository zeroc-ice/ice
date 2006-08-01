// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <IceStorm/IceStorm.h>
#include <IceUtil/UUID.h>

#include <PerfI.h>

#include <map>

#include <math.h>

using namespace std;
using namespace Perf;

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
    int repetitions = 10000;
    bool batch = false;
    bool twoway = false;
    bool ordered = false;
    int nPublishers = 1;
    for(int i = 0; i < argc; i++)
    {
	if(strcmp(argv[i], "-r") == 0)
	{
	    repetitions = atoi(argv[++i]);
	}
	if(strcmp(argv[i], "-b") == 0)
	{
	    batch = true;
	}
	if(strcmp(argv[i], "-t") == 0)
	{
	    twoway = true;
	}
	if(strcmp(argv[i], "-o") == 0)
	{
	    ordered = true;
	}
	if(strcmp(argv[i], "-c") == 0)
	{
	    nPublishers = atoi(argv[++i]);
	}
    }

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
    // Set the requested quality of service "reliability" =
    // "batch". This tells IceStorm to send events to the subscriber
    // in batches at regular intervals.
    //
    IceStorm::QoS qos;
    if(batch)
    {
	qos["reliability"] = "batch";
    }
    else if(twoway)
    {
	qos["reliability"] = "twoway";
    }
    else if(ordered)
    {
	qos["reliability"] = "twoway-ordered";
    }

    //
    // Create the servant to receive the events.
    //
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Ping.Subscriber");
    Ice::ObjectPtr ping = new PingI(repetitions, nPublishers);

    //
    // Add a Servant for the Ice Object.
    //
    Ice::ObjectPrx object = adapter->addWithUUID(ping);
    IceStorm::TopicPrx topic;
    try
    {
	topic = manager->retrieve("time");
    }
    catch(const IceStorm::NoSuchTopic&)
    {
	try
	{
	    topic = manager->create("time");
	}
	catch(const IceStorm::TopicExists&)
	{
	    topic = manager->retrieve("time");
	}
    }
    topic->subscribe(qos, object);

    adapter->activate();
    shutdownOnInterrupt();
    communicator()->waitForShutdown();

    topic->unsubscribe(object);

    return EXIT_SUCCESS;
}
