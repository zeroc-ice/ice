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
#include <Single.h>
#include <fstream>

#ifdef WIN32
#include <io.h>
#endif

using namespace Ice;
using namespace IceStorm;
using namespace std;

class SingleI : public Single
{
public:

    SingleI(const CommunicatorPtr& communicator) :
	_communicator(communicator),
	_count(0)
    {
    }

    virtual void event(const Ice::Current&)
    {
	++_count;
	if (_count == 10)
	{
	    _communicator->shutdown();
	}
    }

private:

    CommunicatorPtr _communicator;
    int _count;
};

void
createLock(const string& name)
{
    ofstream f(name.c_str());
}

void
deleteLock(const string& name)
{
#ifdef HAVE__UNLINK
    _unlink(name.c_str());
#else
    unlink(name.c_str());
#endif
}

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    string lockfile = "subscriber.lock";

    if (argc != 1)
    {
	lockfile = argv[1];
    }
    createLock(lockfile);

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
    Ice::ObjectPtr single = new SingleI(communicator);
    Ice::ObjectPrx object = adapter->add(single, Ice::stringToIdentity("events#single"));

    //
    // The set of topics to which to subscribe
    //
    IceStorm::StringSeq topics;
    topics.push_back("single");

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

    deleteLock(lockfile);

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
	communicator = Ice::initialize(argc, argv);
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

