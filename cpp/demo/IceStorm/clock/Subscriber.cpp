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

using namespace Ice;
using namespace IceStorm;
using namespace std;

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
    Ice::PropertiesPtr properties = communicator()->getProperties();
    const char* refProperty = "IceStorm.TopicManager";
    std::string ref = properties->getProperty(refProperty);
    if (ref.empty())
    {
	cerr << appName() << ": property `" << refProperty << "' not set" << endl;
	return EXIT_FAILURE;
    }

    Ice::ObjectPrx base = communicator()->stringToProxy(ref);
    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(base);
    if (!manager)
    {
	cerr << appName() << ": invalid object reference" << endl;
	return EXIT_FAILURE;
    }

    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapterWithEndpoints("ClockAdapter", "tcp");
    Ice::ObjectPtr clock = new ClockI();
    Ice::ObjectPrx object = adapter->add(clock, Ice::stringToIdentity("events#time"));

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
	cerr << appName() << ": NoSuchTopic: " << e.name << endl;
	return EXIT_FAILURE;
    }

    adapter->activate();

    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
