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
#include <Event.h>
#include <fstream>

#include <TestCommon.h>

#ifdef WIN32
#   include <io.h>
#endif

using namespace std;
using namespace Ice;
using namespace IceStorm;

class EventI : public Event
{
public:

    EventI(const CommunicatorPtr& communicator) :
	_communicator(communicator),
	_count(0)
    {
    }

    virtual void pub(const string& data, const Ice::Current&)
    {
	if (data == "shutdown")
	{
	    _communicator->shutdown();
	    return;
	}
	++_count;
    }

    int count() const { return _count; }

private:

    CommunicatorPtr _communicator;
    int _count;
};

typedef IceUtil::Handle<EventI> EventIPtr;

void
createLock(const string& name)
{
    ofstream f(name.c_str());
}

void
deleteLock(const string& name)
{
#ifdef WIN32
    _unlink(name.c_str());
#else
    unlink(name.c_str());
#endif
}

int
run(int argc, char* argv[], const CommunicatorPtr& communicator)
{
    string lockfile = "subscriber.lock";

    if (argc != 1)
    {
	lockfile = argv[1];
    }
    createLock(lockfile);

    PropertiesPtr properties = communicator->getProperties();
    const char* managerEndpointsProperty = "IceStorm.TopicManager.Endpoints";
    string managerEndpoints = properties->getProperty(managerEndpointsProperty);
    if (managerEndpoints.empty())
    {
	cerr << argv[0] << ": property `" << managerEndpointsProperty << "' is not set" << endl;
	return EXIT_FAILURE;
    }

    ObjectPrx base = communicator->stringToProxy("TopicManager:" + managerEndpoints);
    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(base);
    if (!manager)
    {
	cerr << argv[0] << ": `" << managerEndpoints << "' is not running" << endl;
	return EXIT_FAILURE;
    }

    ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("SubscriberAdapter", "default");
    EventIPtr eventFed1 = new EventI(communicator);
    EventIPtr eventFed2 = new EventI(communicator);
    EventIPtr eventFed3 = new EventI(communicator);
    //
    // Any of the objects will do as long as they are all activated
    //
    ObjectPrx object = adapter->add(eventFed1, stringToIdentity("events#fed1"));
    adapter->add(eventFed2, stringToIdentity("events#fed2"));
    adapter->add(eventFed3, stringToIdentity("events#fed3"));

    //
    // The set of topics to which to subscribe
    //
    IceStorm::StringSeq topics;
    topics.push_back("fed1");
    topics.push_back("fed2");
    topics.push_back("fed3");

    IceStorm::QoS qos;
    //TODO: qos["reliability"] = "batch";
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

    //cout << "eventFed1->count(): " << eventFed1->getCount() << endl;
    //cout << "eventFed2->count(): " << eventFed2->getCount() << endl;
    //cout << "eventFed3->count(): " << eventFed3->getCount() << endl;

    test(eventFed1->count() == 30);
    test(eventFed2->count() == 40);
    test(eventFed3->count() == 30);

    deleteLock(lockfile);

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    CommunicatorPtr communicator;

    try
    {
	addArgumentPrefix("IceStorm");
	communicator = initialize(argc, argv);
	status = run(argc, argv, communicator);
    }
    catch(const Exception& ex)
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
	catch(const Exception& ex)
	{
	    cerr << ex << endl;
	    status = EXIT_FAILURE;
	}
    }

    return status;
}
