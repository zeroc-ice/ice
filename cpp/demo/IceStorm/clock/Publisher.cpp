// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>

#include <Clock.h>

using namespace std;

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    Ice::PropertiesPtr properties = communicator->getProperties();
    const char* refProperty = "IceStorm.TopicManager";
    std::string ref = properties->getProperty(refProperty);
    if (ref.empty())
    {
	cerr << argv[0] << ": property `" << refProperty << "' not set" << endl;
	return EXIT_FAILURE;
    }

    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(base);
    if (!manager)
    {
	cerr << argv[0] << ": invalid object reference" << endl;
	return EXIT_FAILURE;
    }

    IceStorm::TopicPrx topic;
    try
    {
	topic = manager->retrieve("time");
    }
    catch(const IceStorm::NoSuchTopic& e)
    {
	cerr << argv[0] << ": NoSuchTopic: " << e.name << endl;
	return EXIT_FAILURE;
	
    }
    assert(topic);

    //
    // Get a publisher object, create a oneway proxy and then cast to
    // a Clock object
    //
    Ice::ObjectPrx obj = topic->getPublisher();
    obj = obj->ice_oneway();
    ClockPrx clock = ClockPrx::uncheckedCast(obj);

    cout << "publishing 10 tick events" << endl;
    for (int i = 0; i < 10; ++i)
	clock->tick();

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
	Ice::PropertiesPtr properties = Ice::createPropertiesFromFile(argc, argv, "config");
	communicator = Ice::initializeWithProperties(properties);
	status = run(argc, argv, communicator);
    }
    catch(const Ice::Exception& ex)
    {
	cerr << ex << endl;
	status = EXIT_FAILURE;
    }

    if (communicator)
    {
	try
	{
	    communicator->destroy();
	}
	catch(const Ice::Exception& ex)
	{
	    cerr << ex << endl;
	    status = EXIT_FAILURE;
	}
    }

    return status;
}
