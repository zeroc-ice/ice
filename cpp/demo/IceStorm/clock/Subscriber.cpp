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
#include <ClockI.h>

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

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("ClockAdapter", "tcp");
    Ice::ObjectPtr clock = new ClockI();
    const char* id = "events#time";
    adapter->add(clock, id);

    Ice::ObjectPrx object = adapter->createProxy(id);

    //
    // The set of topics to which to subscribe
    //
    IceStorm::StringSeq topics;
    topics.push_back("time");

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
	cerr << argv[0] << ": NoSuchTopic: " << e.name << endl;
	return EXIT_FAILURE;
    }

    adapter->activate();

    communicator->waitForShutdown();
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
