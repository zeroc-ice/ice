// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
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
using namespace Test;

class EventI : public Event
{
public:

    EventI(const CommunicatorPtr& communicator) :
	_communicator(communicator)
    {
    }

    virtual void
    pub(const string& data, const Ice::Current&)
    {
	IceUtil::StaticMutex::Lock sync(_countMutex);

	if(++_count == 30 + 40 + 30)
	{
	    _communicator->shutdown();
	}
    }

private:

    CommunicatorPtr _communicator;

    static int _count;
    static IceUtil::StaticMutex _countMutex;
};

typedef IceUtil::Handle<EventI> EventIPtr;

int EventI::_count = 0;
IceUtil::StaticMutex EventI::_countMutex = ICE_STATIC_MUTEX_INITIALIZER;

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
#   ifndef NDEBUG
    int ret = 
#   endif
	unlink(name.c_str());
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

    TopicPrx fed1;
    TopicPrx fed2;
    TopicPrx fed3;

    try
    {
        fed1 = manager->retrieve("fed1");
        fed2 = manager->retrieve("fed2");
        fed3 = manager->retrieve("fed3");
    }
    catch(const IceStorm::NoSuchTopic& e)
    {
	cerr << argv[0] << ": NoSuchTopic: " << e.name << endl;
	return EXIT_FAILURE;
    }

    fed1->subscribe(qos, objFed1);
    fed2->subscribe(qos, objFed2);
    fed3->subscribe(qos, objFed3);

    createLock(lockfile);

    communicator->waitForShutdown();

    fed1->unsubscribe(objFed1);
    fed2->unsubscribe(objFed2);
    fed3->unsubscribe(objFed3);

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
