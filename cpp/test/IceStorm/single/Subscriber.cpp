// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>
#include <Single.h>

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

class SingleI : public Single, public IceUtil::Mutex
{
public:

    SingleI(const CommunicatorPtr& communicator) :
	_communicator(communicator),
	_count(0)
    {
    }

    virtual void event(const Current&)
    {
	IceUtil::Mutex::Lock sync(*this);

	if(++_count == 10)
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

int
run(int argc, char* argv[], const CommunicatorPtr& communicator)
{
    string lockfile = "subscriber.lock";

    if(argc != 1)
    {
	lockfile = argv[1];
    }

    createLock(lockfile);

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

    ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("SingleAdapter", "default");
    ObjectPtr single = new SingleI(communicator);
    ObjectPrx object = adapter->addWithUUID(single);

    IceStorm::QoS qos;
    //TODO: qos["reliability"] = "batch";
    try
    {
        TopicPrx topic = manager->retrieve("single");
	topic->subscribe(qos, object);
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
