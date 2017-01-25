// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <IceUtil/Options.h>
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

    SingleI(const CommunicatorPtr& communicator, const string& name, int max) :
        _communicator(communicator),
        _name(name),
        _max(max),
        _count(0),
        _last(0)
    {
    }

    virtual void
    event(int i, const Current&)
    {
        if(_name == "twoway ordered" && i != _last)
        {
            cerr << endl << "received unordered event for `" << _name << "': " << i << " " << _last;
            test(false);
        }
        Lock sync(*this);
        ++_last;
        if(++_count == _max)
        {
            notify();
        }
    }

    virtual void
    waitForEvents()
    {
        Lock sync(*this);
        IceUtil::Time timeout = IceUtil::Time::seconds(20);
        while(_count < _max)
        {
            if(!timedWait(timeout))
            {
                test(false);
            }
        }
    }

private:

    CommunicatorPtr _communicator;
    const string _name;
    const int _max;
    int _count;
    int _last;
};
typedef IceUtil::Handle<SingleI> SingleIPtr;

int
run(int argc, char* argv[], const CommunicatorPtr& communicator)
{
    IceUtilInternal::Options opts;
    opts.addOpt("", "ordered");
    opts.addOpt("", "twoway");
    opts.addOpt("", "events", IceUtilInternal::Options::NeedArg);

    try
    {
        opts.parse(argc, (const char**)argv);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        cerr << argv[0] << ": " << e.reason << endl;
        return EXIT_FAILURE;
    }

    PropertiesPtr properties = communicator->getProperties();
    const char* managerProxyProperty = "IceStormAdmin.TopicManager.Default";
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
    while(true)
    {
        try
        {
            topic = manager->retrieve("single");
            break;
        }
        // This can happen if the replica group loses the majority
        // during retrieve. In this case we retry.
        catch(const Ice::UnknownException&)
        {
            continue;
        }
        catch(const IceStorm::NoSuchTopic& e)
        {
            cerr << argv[0] << ": NoSuchTopic: " << e.name << endl;
            return EXIT_FAILURE;
        }
    }

    int events = 1000;
    if(opts.isSet("events"))
    {
        events = atoi(opts.optArg("events").c_str());
    }
    //
    // Create subscribers with different QoS.
    //
    SingleIPtr sub;
    IceStorm::QoS qos;
    if(opts.isSet("ordered"))
    {
        sub = new SingleI(communicator, "twoway ordered", events);
        qos["reliability"] = "ordered";
    }
    else
    {
        sub = new SingleI(communicator, "twoway", events);
    }

    Ice::ObjectPrx prx = adapter->addWithUUID(sub);

    while(true)
    {
        try
        {
            topic->subscribeAndGetPublisher(qos, prx);
            break;
        }
        // If we're already subscribed then we're done (previously we
        // got an UnknownException which succeeded).
        catch(const IceStorm::AlreadySubscribed&)
        {
            break;
        }
        // This can happen if the replica group loses the majority
        // during subscription. In this case we retry.
        catch(const Ice::UnknownException&)
        {
        }
    }

    adapter->activate();

    sub->waitForEvents();

    topic->unsubscribe(prx);

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    CommunicatorPtr communicator;
    InitializationData initData = getTestInitData(argc, argv);
    try
    {
        communicator = initialize(argc, argv, initData);
        status = run(argc, argv, communicator);
    }
    catch(const Exception& ex)
    {
        cerr << ex << endl;
        status = EXIT_FAILURE;
    }

    if(communicator)
    {
        communicator->destroy();
    }

    return status;
}
