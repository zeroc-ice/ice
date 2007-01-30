// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>
#include <Single.h>
#include <TestCommon.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Test;

class SingleI : public Single, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    SingleI(const CommunicatorPtr& communicator, const string& name) :
	_communicator(communicator),
	_name(name),
	_count(0),
	_last(0)
    {
    }

    virtual void
    event(int i, const Current& current)
    {

	if((_name == "default" || _name == "oneway" || _name == "batch") && current.requestId != 0)
	{
	    cerr << endl << "expected oneway request";
	    test(false);
	}
	else if((_name == "twoway" || _name == "twoway ordered") && current.requestId == 0)
	{
	    cerr << endl << "expected twoway request";
	}
	if(_name == "twoway ordered" && i != _last)
	{
	    cerr << endl << "received unordered event for `" << _name << "': " << i << " " << _last;
	    test(false);
	}
	Lock sync(*this);
	++_last;
	if(++_count == 1000)
	{
	    notify();
	}
    }

    virtual void
    waitForEvents()
    {
	Lock sync(*this);
	cout << "testing " << _name << " reliability... " << flush;
	while(_count < 1000)
	{
	    if(!timedWait(IceUtil::Time::seconds(20)))
	    {
		test(false);
	    }
	}
	cout << "ok" << endl;
    }

private:

    CommunicatorPtr _communicator;
    const string _name;
    int _count;
    bool _ordered;
    int _last;
};
typedef IceUtil::Handle<SingleI> SingleIPtr;

int
run(int argc, char* argv[], const CommunicatorPtr& communicator)
{
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

    TopicPrx topic;
    try
    {
        topic = manager->retrieve("single");
    }
    catch(const IceStorm::NoSuchTopic& e)
    {
	cerr << argv[0] << ": NoSuchTopic: " << e.name << endl;
	return EXIT_FAILURE;
    }

    //
    // Create subscribers with different QoS.
    //
    vector<SingleIPtr> subscribers;
    //
    // First we use the old deprecated API.
    //
    {
	subscribers.push_back(new SingleI(communicator, "default"));
	topic->subscribe(IceStorm::QoS(), adapter->addWithUUID(subscribers.back()));
    }
    {
	subscribers.push_back(new SingleI(communicator, "oneway"));
	IceStorm::QoS qos;
	qos["reliability"] = "oneway";
	topic->subscribe(qos, adapter->addWithUUID(subscribers.back()));
    }
    {
	subscribers.push_back(new SingleI(communicator, "twoway"));
	IceStorm::QoS qos;
	qos["reliability"] = "twoway";
	topic->subscribe(qos, adapter->addWithUUID(subscribers.back()));
    }
    {
	subscribers.push_back(new SingleI(communicator, "batch"));
	IceStorm::QoS qos;
	qos["reliability"] = "batch";
	topic->subscribe(qos, adapter->addWithUUID(subscribers.back()));
    }
    {
	subscribers.push_back(new SingleI(communicator, "twoway ordered")); // Ordered
	IceStorm::QoS qos;
	qos["reliability"] = "twoway ordered";
	topic->subscribe(qos, adapter->addWithUUID(subscribers.back()));
    }
    //
    // Next we use the new API call with the new proxy semantics.
    //
    {
	subscribers.push_back(new SingleI(communicator, "default"));
	topic->subscribeAndGetPublisher(IceStorm::QoS(), adapter->addWithUUID(subscribers.back())->ice_oneway());
    }
    {
	subscribers.push_back(new SingleI(communicator, "oneway"));
	topic->subscribeAndGetPublisher(IceStorm::QoS(), adapter->addWithUUID(subscribers.back())->ice_oneway());
    }
    {
	subscribers.push_back(new SingleI(communicator, "twoway"));
	topic->subscribeAndGetPublisher(IceStorm::QoS(), adapter->addWithUUID(subscribers.back()));
    }
    {
	subscribers.push_back(new SingleI(communicator, "batch"));
	topic->subscribeAndGetPublisher(IceStorm::QoS(), adapter->addWithUUID(subscribers.back())->ice_batchOneway());
    }
    {
	subscribers.push_back(new SingleI(communicator, "twoway ordered")); // Ordered
	IceStorm::QoS qos;
	qos["reliability"] = "ordered";
	topic->subscribeAndGetPublisher(qos, adapter->addWithUUID(subscribers.back()));
    }

    adapter->activate();

    for(vector<SingleIPtr>::const_iterator p = subscribers.begin(); p != subscribers.end(); ++p)
    {
	(*p)->waitForEvents();
    }

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
