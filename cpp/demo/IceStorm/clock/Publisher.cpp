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
    if(!obj->ice_isDatagram())
    {
        obj = obj->ice_oneway();
    }
    ClockPrx clock = ClockPrx::uncheckedCast(obj);

    cout << "publishing 10 tick events" << endl;
    for(int i = 0; i < 10; ++i)
    {
	clock->tick();
    }

    return EXIT_SUCCESS;
}
