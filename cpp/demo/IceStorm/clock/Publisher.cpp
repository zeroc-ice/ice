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

using namespace Ice;
using namespace IceStorm;
using namespace std;

class Publisher : public Application
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
    PropertiesPtr properties = communicator()->getProperties();
    const char* refProperty = "IceStorm.TopicManager";
    std::string ref = properties->getProperty(refProperty);
    if (ref.empty())
    {
	cerr << appName() << ": property `" << refProperty << "' not set" << endl;
	return EXIT_FAILURE;
    }

    ObjectPrx base = communicator()->stringToProxy(ref);
    TopicManagerPrx manager = TopicManagerPrx::checkedCast(base);
    if (!manager)
    {
	cerr << appName() << ": invalid object reference" << endl;
	return EXIT_FAILURE;
    }

    TopicPrx topic;
    try
    {
	topic = manager->retrieve("time");
    }
    catch(const NoSuchTopic& e)
    {
	cerr << appName() << ": NoSuchTopic: " << e.name << endl;
	return EXIT_FAILURE;
	
    }
    assert(topic);

    //
    // Get a publisher object, create a oneway proxy and then cast to
    // a Clock object
    //
    ObjectPrx obj = topic->getPublisher();
    obj = obj->ice_oneway();
    ClockPrx clock = ClockPrx::uncheckedCast(obj);

    cout << "publishing 10 tick events" << endl;
    for (int i = 0; i < 10; ++i)
	clock->tick();

    return EXIT_SUCCESS;
}
