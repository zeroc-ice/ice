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

#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>
#include <Event.h>

#include <TestCommon.h>

#include <fcntl.h>
#ifdef _WIN32
#   include <io.h>
#else
#   include <sys/types.h>
#   include <sys/stat.h>
#endif

using namespace std;
using namespace Ice;
using namespace IceStorm;

class EventI : public Event, public IceUtil::Mutex
{
public:

    EventI(const CommunicatorPtr& communicator) :
	_communicator(communicator),
	_count(0)
    {
    }

    virtual void
    pub(const string& data, const Ice::Current&)
    {
	IceUtil::Mutex::Lock sync(*this);

	if(data == "shutdown")
	{
	    _communicator->shutdown();
	    return;
	}
	++_count;
    }

    int
    count() const
    {
	IceUtil::Mutex::Lock sync(*this);

	return _count;
    }

private:

    CommunicatorPtr _communicator;
    int _count;
};

typedef IceUtil::Handle<EventI> EventIPtr;

void
createLock(const string& name)
{
    int fd = open(name.c_str(), O_CREAT | O_WRONLY | O_EXCL, 0777);
    assert(fd != -1);
    close(fd);
}

void
deleteLock(const string& name)
{
#ifdef _WIN32
    int ret = _unlink(name.c_str());
#else
    int ret = unlink(name.c_str());
#endif
    assert(ret != -1);
}

void
usage(const char* appName)
{
    cerr << "Usage: " << appName << " [options] [lockfile]\n";
    cerr <<	
	"Options:\n"
	"-h, --help           Show this message.\n"
	"-b                   Use batch reliability.\n"
	;
}

int
run(int argc, char* argv[], const CommunicatorPtr& communicator)
{
    string lockfile = "subscriber.lock";
    bool batch = false;

    int idx = 1;
    while(idx < argc)
    {
	if(strcmp(argv[idx], "-b") == 0)
	{
            batch = true;

	    for(int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
	}
	else if(strcmp(argv[idx], "-h") == 0 || strcmp(argv[idx], "--help") == 0)
	{
	    usage(argv[0]);
	    return EXIT_SUCCESS;
	}
	else if(argv[idx][0] == '-')
	{
	    cerr << argv[0] << ": unknown option `" << argv[idx] << "'" << endl;
	    usage(argv[0]);
	    return EXIT_FAILURE;
	}
	else
	{
            lockfile = argv[idx];
	    ++idx;
	}
    }

    PropertiesPtr properties = communicator->getProperties();
    const char* managerProxyProperty = "IceStorm.TopicManager.Proxy";
    string managerProxy = properties->getProperty(managerProxyProperty);
    if(managerProxy.empty())
    {
	cerr << argv[0] << ": property `" << managerProxyProperty << "' is not set" << endl;
	return EXIT_FAILURE;
    }

    ObjectPrx base = communicator->stringToProxy(managerProxy);
    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(base);
    if(!manager)
    {
	cerr << argv[0] << ": `" << managerProxy << "' is not running" << endl;
	return EXIT_FAILURE;
    }

    ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("SubscriberAdapter", "default");
    EventIPtr eventFed1 = new EventI(communicator);
    EventIPtr eventFed2 = new EventI(communicator);
    EventIPtr eventFed3 = new EventI(communicator);

    //
    // Activate the servants.
    //
    ObjectPrx objFed1 = adapter->addWithUUID(eventFed1);
    ObjectPrx objFed2 = adapter->addWithUUID(eventFed2);
    ObjectPrx objFed3 = adapter->addWithUUID(eventFed3);

    adapter->activate();

    IceStorm::QoS qos;
    if(batch)
    {
        qos["reliability"] = "batch";
    }
    try
    {
        TopicPrx topic;
        topic = manager->retrieve("fed1");
	topic->subscribe(qos, objFed1);
        topic = manager->retrieve("fed2");
	topic->subscribe(qos, objFed2);
        topic = manager->retrieve("fed3");
	topic->subscribe(qos, objFed3);
    }
    catch(const IceStorm::NoSuchTopic& e)
    {
	cerr << argv[0] << ": NoSuchTopic: " << e.name << endl;
	return EXIT_FAILURE;
    }

    createLock(lockfile);

    communicator->waitForShutdown();

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
	communicator = initialize(argc, argv);
	status = run(argc, argv, communicator);
    }
    catch(const Exception& ex)
    {
	cerr << ex << endl;
	status = EXIT_FAILURE;
    }

    if(communicator)
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
