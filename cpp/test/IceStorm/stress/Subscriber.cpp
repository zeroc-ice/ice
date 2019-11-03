//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>
#include <IceUtil/Options.h>
#include <Event.h>
#include <TestHelper.h>
#include <random>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Test;

struct Subscription;

class EventI : public Event
{
public:

    EventI(shared_ptr<Communicator> communicator, int total) :
        _communicator(move(communicator)), _total(total), _count(0)
    {
    }

    int count()
    {
        lock_guard<mutex> lg(_mutex);
        return _count;
    }

    virtual void check(const Subscription&)
    {
    }

protected:

    shared_ptr<Communicator> _communicator;
    const int _total;
    int _count;
    mutex _mutex;
};

struct Subscription final
{
    shared_ptr<Ice::ObjectAdapter> adapter;
    shared_ptr<Ice::ObjectPrx> obj;
    shared_ptr<EventI> servant;
    IceStorm::QoS qos;
    shared_ptr<Ice::ObjectPrx> publisher;
    bool activate = true;
};

class OrderEventI final : public EventI
{
public:

    OrderEventI(shared_ptr<Communicator> communicator, int total) :
        EventI(move(communicator), total)
    {
    }

    void
    pub(int counter, const Ice::Current&) override
    {
        lock_guard<mutex> lg(_mutex);
        if(counter != _count || counter == _total - 1)
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

class CountEventI final : public EventI
{
public:

    CountEventI(shared_ptr<Communicator> communicator, int total) :
        EventI(move(communicator), total)
    {
    }

    void
    pub(int, const Ice::Current&) override
    {
        lock_guard<mutex> lg(_mutex);
        if(++_count == _total)
        {
            _communicator->shutdown();
        }
    }
};

class SlowEventI final : public EventI
{
public:

    SlowEventI(shared_ptr<Communicator> communicator, int total) :
        EventI(move(communicator), total)
    {
    }

    void
    pub(int, const Ice::Current&) override
    {
        lock_guard<mutex> lg(_mutex);
        //
        // Ignore events over and above the expected.
        //
        if(_count >= _total)
        {
            return;
        }
        // Sleep for 3 seconds
        this_thread::sleep_for(3s);
        if(++_count == _total)
        {
            _communicator->shutdown();
        }
    }
};

class ErraticEventI final : public EventI
{
public:

    ErraticEventI(shared_ptr<Communicator> communicator, int total) :
        EventI(move(communicator), total)
    {
        ++_remaining;
    }

    void
    pub(int, const Ice::Current& current) override
    {
        lock_guard<mutex> lg(_mutex);

        // Randomly close the connection.
        if(!_done && (_rd() % 10 == 1 || ++_count == _total))
        {
            _done = true;
            current.con->close(ConnectionClose::Forcefully);
            // Deactivate the OA. This ensures that the subscribers
            // that have subscribed with oneway QoS will be booted.
            current.adapter->deactivate();
            _count = _total;
            {
                if(--_remaining == 0)
                {
                    _communicator->shutdown();
                }
            }
        }
    }

private:

    static atomic_int _remaining;
    bool _done = false;
    random_device _rd;
};

atomic_int ErraticEventI::_remaining = 0;

class MaxQueueEventI final : public EventI
{
public:

    MaxQueueEventI(shared_ptr<Communicator> communicator, int expected, int total, bool removeSubscriber) :
        EventI(move(communicator), total), _removeSubscriber(removeSubscriber), _expected(expected)
    {
    }

    void
    pub(int counter, const Ice::Current&) override
    {
        lock_guard<mutex> lg(_mutex);

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

    void
    check(const Subscription& subscription) override
    {
        if(_removeSubscriber)
        {
            try
            {
                //
                // check might be invoked before IceStorm got a chance to process the close connection
                // message from this subscriber, retry if the ice_ping still succeeds.
                //
                int nRetry = 10;
                while(--nRetry > 0)
                {
                    subscription.publisher->ice_ping();
                    this_thread::sleep_for(200ms);
                }
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

class ControllerEventI final : public EventI
{
public:

    ControllerEventI(shared_ptr<Communicator> communicator, int total, shared_ptr<ObjectAdapter> adapter) :
        EventI(move(communicator), total), _adapter(move(adapter))
    {
    }

    void
    pub(int, const Ice::Current&) override
    {
        lock_guard<mutex> lg(_mutex);
        if(++_count == _total)
        {
            _adapter->activate();
        }
    }

private:

    const shared_ptr<Ice::ObjectAdapter> _adapter;
};

class Subscriber final : public Test::TestHelper
{
public:

    void run(int, char**) override;
};

void
Subscriber::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
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
        ostringstream os;
        os << argv[0] << ": error: " << e.reason;
        throw invalid_argument(os.str());
    }

    int events = 1000;
    string s = opts.optArg("events");
    if(!s.empty())
    {
        events = atoi(s.c_str());
    }
    if(events <= 0)
    {
        ostringstream os;
        os << argv[0] << ": events must be > 0.";
        throw invalid_argument(os.str());
    }

    IceStorm::QoS cmdLineQos;

    vector<string> sqos = opts.argVec("qos");
    for(const auto& q: sqos)
    {
        string::size_type off = q.find(",");
        if(off == string::npos)
        {
            ostringstream os;
            os << argv[0] << ": parse error: no , in QoS";
            throw invalid_argument(os.str());
        }
        cmdLineQos[q.substr(0, off)] = q.substr(off+1);
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
        ostringstream os;
        os << argv[0] << ": events must be > 0.";
        throw invalid_argument(os.str());
    }

    auto properties = communicator->getProperties();
    string managerProxyProperty = "IceStormAdmin.TopicManager.Default";
    string managerProxy = properties->getProperty(managerProxyProperty);
    if(managerProxy.empty())
    {
        ostringstream os;
        os << argv[0] << ": property `" << managerProxyProperty << "' is not set";
        throw invalid_argument(os.str());
    }

    auto manager = checkedCast<IceStorm::TopicManagerPrx>(
        communicator->stringToProxy(managerProxy));
    if(!manager)
    {
        ostringstream os;
        os << argv[0] << ": `" << managerProxy << "' is not running";
        throw invalid_argument(os.str());
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
            item.servant = make_shared<ErraticEventI>(communicator.communicator(), events);
            item.qos["reliability"] = "twoway";
            subs.push_back(item);
        }
    }
    else if(slow)
    {
        Subscription item;
        item.adapter = communicator->createObjectAdapterWithEndpoints("SubscriberAdapter", "default");
        item.servant = make_shared<SlowEventI>(communicator.communicator(), events);
        item.qos = cmdLineQos;
        subs.push_back(item);
    }
    else if(maxQueueDropEvents || maxQueueRemoveSub)
    {
        Subscription item1;
        item1.adapter = communicator->createObjectAdapterWithEndpoints("MaxQueueAdapter", "default");
        if(maxQueueDropEvents)
        {
            item1.servant = make_shared<MaxQueueEventI>(communicator.communicator(), maxQueueDropEvents, events, false);
        }
        else
        {
            item1.servant = make_shared<MaxQueueEventI>(communicator.communicator(), maxQueueRemoveSub, events, true);
        }
        item1.qos = cmdLineQos;
        item1.activate = false;
        subs.push_back(item1);

        Subscription item2;
        item2.adapter = communicator->createObjectAdapterWithEndpoints("ControllerAdapter", "default");
        item2.servant = make_shared<ControllerEventI>(communicator.communicator(), events, item1.adapter);
        item2.qos["reliability"] = "oneway";
        subs.push_back(item2);
    }
    else
    {
        Subscription item;
        item.adapter = communicator->createObjectAdapterWithEndpoints("SubscriberAdapter", "default");
        item.qos = cmdLineQos;
        auto p = item.qos.find("reliability");
        if(p != item.qos.end() && p->second == "ordered")
        {
            item.servant = make_shared<OrderEventI>(communicator.communicator(), events);
        }
        else
        {
            item.servant = make_shared<CountEventI>(communicator.communicator(), events);
        }
        subs.push_back(item);
    }

    auto topic = manager->retrieve("fed1");

    {
        for(auto& p: subs)
        {
            p.obj = p.adapter->addWithUUID(p.servant);

            IceStorm::QoS qos;
            string reliability = "";
            IceStorm::QoS::const_iterator q = p.qos.find("reliability");
            if(q != p.qos.end())
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
                p.obj = p.obj->ice_batchOneway();
            }
            else //if(reliability == "oneway")
            {
                p.obj = p.obj->ice_oneway();
            }
            p.publisher = topic->subscribeAndGetPublisher(qos, p.obj);
        }
    }

    {
        for(const auto& p: subs)
        {
            if(p.activate)
            {
                p.adapter->activate();
            }
        }
    }

    communicator->waitForShutdown();

    {
        for(const auto& p: subs)
        {
            p.servant->check(p);
            topic->unsubscribe(p.obj);
            if(p.servant->count() != events)
            {
                ostringstream os;
                os << "expected " << events << " events but got " << p.servant->count() << " events.";
                throw invalid_argument(os.str());
            }
        }
    }
}

DEFINE_TEST(Subscriber)
