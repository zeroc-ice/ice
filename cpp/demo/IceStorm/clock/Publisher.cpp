// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/Application.h>
#include <IceStorm/IceStorm.h>

#include <Clock.h>

using namespace std;

class Publisher : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    Publisher app;
    return app.main(argc, argv, "config");
}

int
Publisher::run(int argc, char* argv[])
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
    // Retrieve the topic named "time".
    //
    IceStorm::TopicPrx topic;
    try
    {
	topic = manager->retrieve("time");
    }
    catch(const IceStorm::NoSuchTopic& e)
    {
	cerr << appName() << ": " << e << " name: " << e.name << endl;
	return EXIT_FAILURE;
    }
    assert(topic);

    //
    // Get the topic's publisher object, verify that it supports
    // the Clock type, and create a oneway Clock proxy (for efficiency
    // reasons).
    //
    Ice::ObjectPrx obj = topic->getPublisher();
    assert(obj->ice_isA(Clock::ice_staticId()));
    ClockPrx clock = ClockPrx::uncheckedCast(obj->ice_oneway());

    cout << "publishing 10 tick events" << endl;
    for(int i = 0; i < 10; ++i)
    {
	clock->tick();
    }

    return EXIT_SUCCESS;
}
