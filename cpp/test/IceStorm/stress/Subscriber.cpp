// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>
#include <IceUtil/Options.h>
#include <IceUtil/Thread.h>
#include <IceUtil/Time.h>
#include <IceUtil/Random.h>
#include <Event.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>
#include <TestCommon.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Test;

struct Subscription; // Forward declaration.

class EventI : public Event, public IceUtil::Mutex
{
public:

    EventI(const CommunicatorPtr& communicator, int total) :
        _communicator(communicator), _total(total), _count(0)
    {
    }

    int count() const
    {
        Lock sync(*this);
        return _count;
    }

    virtual void check(const Subscription&)
    {
    }

protected:

    const CommunicatorPtr _communicator;
    const int _total;
    int _count;
};
typedef IceUtil::Handle<EventI> EventIPtr;

struct Subscription
{
    Subscription() : activate(true)
    {
    }

    Ice::ObjectAdapterPtr adapter;
    Ice::ObjectPrx obj;
    EventIPtr servant;
    IceStorm::QoS qos;
    Ice::ObjectPrx publisher;
    bool activate;
};

class OrderEventI : public EventI
{
public:

    OrderEventI(const CommunicatorPtr& communicator, int total) :
        EventI(communicator, total)
    {
    }

    virtual void
    pub(int counter, const Ice::Current&)
    {
        Lock sync(*this);

        if(counter != _count || counter == _total-1)
        {
            if(counter != _count)
            {
                cerr << "failed! expected event: " << _count << " received event: " << counter << endl;
            }
            _communicator->shutdown();
        }
        _count++;
    }
};

class CountEventI : public EventI
{
public:

    CountEventI(const CommunicatorPtr& communicator, int total) :
        EventI(communicator, total)
    {
    }

    virtual void
    pub(int, const Ice::Current&)
    {
        Lock sync(*this);

        if(++_count == _total)
        {
            _communicator->shutdown();
        }
    }
};

class SlowEventI : public EventI
{
public:

    SlowEventI(const CommunicatorPtr& communicator, int total) :
        EventI(communicator, total)
    {
    }

    virtual void
    pub(int, const Ice::Current&)
    {
        Lock sync(*this);

        //
        // Ignore events over and above the expected.
        //
        if(_count >= _total)
        {
            return;
        }
        // Sleep for 3 seconds
        IceUtil::ThreadControl::sleep(IceUtil::Time::seconds(3));
        if(++_count == _total)
        {
            _communicator->shutdown();
        }
    }
};

class ErraticEventI : public EventI
{
public:

    ErraticEventI(const CommunicatorPtr& communicator, int total) :
        EventI(communicator, total), _done(false)
    {
        IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(_remainingMutex);
        ++_remaining;
    }

    virtual void
    pub(int, const Ice::Current& current)
    {
        Lock sync(*this);

        // Randomly close the connection.
        if(!_done && (IceUtilInternal::random(10) == 1 || ++_count == _total))
        {
            _done = true;
            current.con->close(true);
            // Deactivate the OA. This ensures that the subscribers
            // that have subscribed with oneway QoS will be booted.
            current.adapter->deactivate();
            _count = _total;
            {
                IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(_remainingMutex);
                --_remaining;
                if(_remaining == 0)
                {
                    _communicator->shutdown();
                }
            }
        }
    }

    static IceUtil::Mutex* _remainingMutex;

private:

    static int _remaining;
    bool _done;
};

IceUtil::Mutex* ErraticEventI::_remainingMutex = 0;
int ErraticEventI::_remaining = 0;

class MaxQueueEventI : public EventI
{
public:

    MaxQueueEventI(const CommunicatorPtr& communicator, int expected, int total, bool removeSubscriber) :
        EventI(communicator, total), _removeSubscriber(removeSubscriber), _expected(expected)
    {
    }

    virtual void
    pub(int counter, const Ice::Current&)
    {
        Lock sync(*this);

        if(counter != _count)
        {
            cerr << "failed! expected event: " << _count << " received event: " << counter << endl;
        }

        if(_removeSubscriber)
        {
            _count = _total;
            _communicator->shutdown();
            return;
        }

        if(_count == 0)
        {
            _count = _total - _expected;
        }
        else if(++_count == _total)
        {
            _communicator->shutdown();
        }
    }

    virtual void
    check(const Subscription& subscription)
    {
        if(_removeSubscriber)
        {
            try
            {
                subscription.publisher->ice_ping();
                test(false);
            }
            catch(const Ice::ObjectNotExistException&)
            {
            }
        }
    }

private:

    bool _removeSubscriber;
    int _expected;
};


class ControllerEventI: public EventI
{
public:

    ControllerEventI(const CommunicatorPtr& communicator, int total, const Ice::ObjectAdapterPtr& adapter) :
        EventI(communicator, total), _adapter(adapter)
    {
    }

    virtual void
    pub(int, const Ice::Current&)
    {
        Lock sync(*this);
        if(++_count == _total)
        {
            _adapter->activate();
        }
    }

private:

    const Ice::ObjectAdapterPtr _adapter;
};

namespace
{

class Init
{
public:

    Init()
    {
        ErraticEventI::_remainingMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete ErraticEventI::_remainingMutex;
        ErraticEventI::_remainingMutex = 0;
    }
};

Init init;

}

int
run(int argc, char* argv[], const CommunicatorPtr& communicator)
{
    IceUtilInternal::Options opts;
    opts.addOpt("", "events", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "qos", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("", "slow");
    opts.addOpt("", "erratic", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "maxQueueDropEvents", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "maxQueueRemoveSub", IceUtilInternal::Options::NeedArg);

    try
    {
        opts.parse(argc, (const char**)argv);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        cerr << argv[0] << ": " << e.reason << endl;
        return EXIT_FAILURE;
    }

    int events = 1000;
    string s = opts.optArg("events");
    if(!s.empty())
    {
        events = atoi(s.c_str());
    }
    if(events <= 0)
    {
        cerr << argv[0] << ": events must be > 0." << endl;
        return EXIT_FAILURE;
    }

    IceStorm::QoS cmdLineQos;

    vector<string> sqos = opts.argVec("qos");
    for(vector<string>::const_iterator q = sqos.begin(); q != sqos.end(); ++q)
    {
        string::size_type off = q->find(",");
        if(off == string::npos)
        {
            cerr << argv[0] << ": parse error: no , in QoS" << endl;
            return EXIT_FAILURE;
        }
        cmdLineQos[q->substr(0, off)] = q->substr(off+1);
    }

    bool slow = opts.isSet("slow");
    int maxQueueDropEvents = opts.isSet("maxQueueDropEvents") ? atoi(opts.optArg("maxQueueDropEvents").c_str()) : 0;
    int maxQueueRemoveSub = opts.isSet("maxQueueRemoveSub") ? atoi(opts.optArg("maxQueueRemoveSub").c_str()) : 0;
    bool erratic = false;
    int erraticNum = 0;
    s = opts.optArg("erratic");
    if(!s.empty())
    {
        erratic = true;
        erraticNum = atoi(s.c_str());
    }
    if(events <= 0)
    {
        cerr << argv[0] << ": events must be > 0." << endl;
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

    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(
        communicator->stringToProxy(managerProxy));
    if(!manager)
    {
        cerr << argv[0] << ": `" << managerProxy << "' is not running" << endl;
        return EXIT_FAILURE;
    }

    vector<Subscription> subs;

    if(erratic)
    {
        for(int i = 0 ; i < erraticNum; ++i)
        {
            ostringstream os;
            os << "SubscriberAdapter" << i;
            Subscription item;
            item.adapter = communicator->createObjectAdapterWithEndpoints(os.str(), "default");
            item.servant = new ErraticEventI(communicator, events);
            item.qos["reliability"] = "twoway";
            subs.push_back(item);
        }
    }
    else if(slow)
    {
        Subscription item;
        item.adapter = communicator->createObjectAdapterWithEndpoints("SubscriberAdapter", "default");
        item.servant = new SlowEventI(communicator, events);
        item.qos = cmdLineQos;
        subs.push_back(item);
    }
    else if(maxQueueDropEvents || maxQueueRemoveSub)
    {
        Subscription item1;
        item1.adapter = communicator->createObjectAdapterWithEndpoints("MaxQueueAdapter", "default");
        if(maxQueueDropEvents)
        {
            item1.servant = new MaxQueueEventI(communicator, maxQueueDropEvents, events, false);
        }
        else
        {
            item1.servant = new MaxQueueEventI(communicator, maxQueueRemoveSub, events, true);
        }
        item1.qos = cmdLineQos;
        item1.activate = false;
        subs.push_back(item1);

        Subscription item2;
        item2.adapter = communicator->createObjectAdapterWithEndpoints("ControllerAdapter", "default");
        item2.servant = new ControllerEventI(communicator, events, item1.adapter);
        item2.qos["reliability"] = "oneway";
        subs.push_back(item2);
    }
    else
    {
        Subscription item;
        item.adapter = communicator->createObjectAdapterWithEndpoints("SubscriberAdapter", "default");
        item.qos = cmdLineQos;
        map<string, string>::const_iterator p = item.qos.find("reliability");
        if(p != item.qos.end() && p->second == "ordered")
        {
            item.servant = new OrderEventI(communicator, events);
        }
        else
        {
            item.servant = new CountEventI(communicator, events);
        }
        subs.push_back(item);
    }

    TopicPrx topic;
    try
    {
        topic = manager->retrieve("fed1");
    }
    catch(const IceStorm::NoSuchTopic& e)
    {
        cerr << argv[0] << ": NoSuchTopic: " << e.name << endl;
        return EXIT_FAILURE;
    }

    {
        for(vector<Subscription>::iterator p = subs.begin(); p != subs.end(); ++p)
        {
            p->obj = p->adapter->addWithUUID(p->servant);

            IceStorm::QoS qos;
            string reliability = "";
            IceStorm::QoS::const_iterator q = p->qos.find("reliability");
            if(q != p->qos.end())
            {
                reliability = q->second;
            }
            if(reliability == "twoway")
            {
                // Do nothing.
            }
            else if(reliability == "ordered")
            {
                qos["reliability"] = "ordered";
            }
            else if(reliability == "batch")
            {
                p->obj = p->obj->ice_batchOneway();
            }
            else //if(reliability == "oneway")
            {
                p->obj = p->obj->ice_oneway();
            }
            p->publisher = topic->subscribeAndGetPublisher(qos, p->obj);
        }
    }

    {
        for(vector<Subscription>::iterator p = subs.begin(); p != subs.end(); ++p)
        {
            if(p->activate)
            {
                p->adapter->activate();
            }
        }
    }

    communicator->waitForShutdown();

    {
        for(vector<Subscription>::const_iterator p = subs.begin(); p != subs.end(); ++p)
        {
            p->servant->check(*p);
            topic->unsubscribe(p->obj);
            if(p->servant->count() != events)
            {
                cerr << "expected " << events << " events but got " << p->servant->count() << " events." << endl;
                return EXIT_FAILURE;
            }
        }
    }

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
