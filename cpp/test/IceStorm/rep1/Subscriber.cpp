// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
#include <TestHelper.h>

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
        IceUtil::Time timeout = IceUtil::Time::seconds(40);
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

class Subscriber : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Subscriber::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
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
        ostringstream os;
        os << argv[0] << ": error: " << e.reason;
        throw invalid_argument(os.str());
    }

    PropertiesPtr properties = communicator->getProperties();
    string managerProxy = properties->getProperty("IceStormAdmin.TopicManager.Default");
    if(managerProxy.empty())
    {
        ostringstream os;
        os << argv[0] << ": property `IceStormAdmin.TopicManager.Default' is not set";
        throw invalid_argument(os.str());
    }

    ObjectPrx base = communicator->stringToProxy(managerProxy);
    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(base);
    if(!manager)
    {
        ostringstream os;
        os << argv[0] << ": `" << managerProxy << "' is not running";
        throw invalid_argument(os.str());
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
            ostringstream os;
            os << argv[0] << ": NoSuchTopic: " << e.name;
            throw invalid_argument(os.str());
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
        sub = new SingleI(communicator.communicator(), "twoway ordered", events);
        qos["reliability"] = "ordered";
    }
    else
    {
        sub = new SingleI(communicator.communicator(), "twoway", events);
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
}

DEFINE_TEST(Subscriber)
