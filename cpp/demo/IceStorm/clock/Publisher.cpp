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

    static const string endpointsProperty = "IceStorm.TopicManager.Endpoints";
    string endpoints = properties->getProperty(endpointsProperty);
    if (endpoints.empty())
    {
	cerr << appName() << ": property `" << endpointsProperty << "' not set" << endl;
	return EXIT_FAILURE;
    }

    Ice::ObjectPrx base = communicator()->stringToProxy("TopicManager:" + endpoints);
    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(base);
    if (!manager)
    {
	cerr << appName() << ": invalid object reference" << endl;
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
    // Get a publisher object, create a oneway proxy (for efficiency
    // reasons) and then cast to a Clock object. Note that the cast
    // must be unchecked.
    //
    Ice::ObjectPrx obj = topic->getPublisher();
    obj = obj->ice_oneway();
    ClockPrx clock = ClockPrx::uncheckedCast(obj);

    cout << "publishing 10 tick events" << endl;
    for (int i = 0; i < 10; ++i)
    {
	clock->tick();
    }

    return EXIT_SUCCESS;
}
