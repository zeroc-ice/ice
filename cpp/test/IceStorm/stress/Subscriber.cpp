// Copyright (c) ZeroC, Inc.

#include "../../src/Ice/Options.h"
#include "Event.h"
#include "Ice/Ice.h"
#include "IceStorm/IceStorm.h"
#include "TestHelper.h"
#include <random>
#include <thread>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Test;

struct Subscription;

class EventI : public Event
{
public:
    EventI(CommunicatorPtr communicator, int total) : _communicator(std::move(communicator)), _total(total) {}

    int count()
    {
        lock_guard<mutex> lg(_mutex);
        return _count;
    }

    virtual void check(const Subscription&) {}

protected:
    CommunicatorPtr _communicator;
    const int _total;
    int _count{0};
    mutex _mutex;
};

struct Subscription final
{
    Ice::ObjectAdapterPtr adapter;
    optional<Ice::ObjectPrx> obj;
    shared_ptr<EventI> servant;
    IceStorm::QoS qos;
    optional<Ice::ObjectPrx> publisher;
    bool activate = true;
};

class OrderEventI final : public EventI
{
public:
    OrderEventI(CommunicatorPtr communicator, int total) : EventI(std::move(communicator), total) {}

    void pub(int counter, const Ice::Current&) override
    {
        lock_guard<mutex> lg(_mutex);
        if (counter != _count || counter == _total - 1)
        {
            if (counter != _count)
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
    CountEventI(CommunicatorPtr communicator, int total) : EventI(std::move(communicator), total) {}

    void pub(int, const Ice::Current&) override
    {
        lock_guard<mutex> lg(_mutex);
        if (++_count == _total)
        {
            _communicator->shutdown();
        }
    }
};

class SlowEventI final : public EventI
{
public:
    SlowEventI(CommunicatorPtr communicator, int total) : EventI(std::move(communicator), total) {}

    void pub(int, const Ice::Current&) override
    {
        lock_guard<mutex> lg(_mutex);
        //
        // Ignore events over and above the expected.
        //
        if (_count >= _total)
        {
            return;
        }
        // Sleep for 3 seconds
        this_thread::sleep_for(3s);
        if (++_count == _total)
        {
            _communicator->shutdown();
        }
    }
};

class ErraticEventI final : public EventI
{
public:
    ErraticEventI(CommunicatorPtr communicator, int total) : EventI(std::move(communicator), total) { ++_remaining; }

    void pub(int, const Ice::Current& current) override
    {
        lock_guard<mutex> lg(_mutex);

        // Randomly close the connection.
        if (!_done && (_rd() % 10 == 1 || ++_count == _total))
        {
            _done = true;
            current.con->abort();
            // Deactivate the OA. This ensures that the subscribers
            // that have subscribed with oneway QoS will be booted.
            current.adapter->deactivate();
            _count = _total;
            {
                if (--_remaining == 0)
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
    MaxQueueEventI(CommunicatorPtr communicator, int expected, int total, bool removeSubscriber)
        : EventI(std::move(communicator), total),
          _removeSubscriber(removeSubscriber),
          _expected(expected)
    {
    }

    void pub(int counter, const Ice::Current&) override
    {
        lock_guard<mutex> lg(_mutex);

        if (counter != _count)
        {
            cerr << "failed! expected event: " << _count << " received event: " << counter << endl;
        }

        if (_removeSubscriber)
        {
            _count = _total;
            _communicator->shutdown();
            return;
        }

        if (_count == 0)
        {
            _count = _total - _expected;
        }
        else if (++_count == _total)
        {
            _communicator->shutdown();
        }
    }

    void check(const Subscription& subscription) override
    {
        if (_removeSubscriber)
        {
            try
            {
                //
                // check might be invoked before IceStorm got a chance to process the close connection
                // message from this subscriber, retry if the ice_ping still succeeds.
                //
                int nRetry = 10;
                while (--nRetry > 0)
                {
                    subscription.publisher->ice_ping();
                    this_thread::sleep_for(200ms);
                }
                test(false);
            }
            catch (const Ice::ObjectNotExistException&)
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
    ControllerEventI(CommunicatorPtr communicator, int total, ObjectAdapterPtr adapter)
        : EventI(std::move(communicator), total),
          _adapter(std::move(adapter))
    {
    }

    void pub(int, const Ice::Current&) override
    {
        lock_guard<mutex> lg(_mutex);
        if (++_count == _total)
        {
            _adapter->activate();
        }
    }

private:
    const Ice::ObjectAdapterPtr _adapter;
};

class Subscriber final : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Subscriber::run(int argc, char** argv)
{
    InitializationData initData;
    initData.properties = make_shared<Ice::Properties>(vector<string>{"IceStormAdmin"});
    initData.properties = createTestProperties(argc, argv, initData.properties);

    // override the test default since we abort connections
    initData.properties->setProperty("Ice.Warn.Connections", "0");
    Ice::CommunicatorHolder ich = initialize(initData);
    const auto& communicator = ich.communicator();
    IceInternal::Options opts;
    opts.addOpt("", "events", IceInternal::Options::NeedArg);
    opts.addOpt("", "qos", IceInternal::Options::NeedArg, "", IceInternal::Options::Repeat);
    opts.addOpt("", "slow");
    opts.addOpt("", "erratic", IceInternal::Options::NeedArg);
    opts.addOpt("", "maxQueueDropEvents", IceInternal::Options::NeedArg);
    opts.addOpt("", "maxQueueRemoveSub", IceInternal::Options::NeedArg);

    try
    {
        opts.parse(argc, (const char**)argv);
    }
    catch (const IceInternal::BadOptException& e)
    {
        ostringstream os;
        os << argv[0] << ": error: " << e.what();
        throw invalid_argument(os.str());
    }

    int events = 1000;
    string s = opts.optArg("events");
    if (!s.empty())
    {
        events = stoi(s);
    }
    if (events <= 0)
    {
        ostringstream os;
        os << argv[0] << ": events must be > 0.";
        throw invalid_argument(os.str());
    }

    IceStorm::QoS cmdLineQos;

    vector<string> sqos = opts.argVec("qos");
    for (const auto& q : sqos)
    {
        string::size_type off = q.find(",");
        if (off == string::npos)
        {
            ostringstream os;
            os << argv[0] << ": parse error: no , in QoS";
            throw invalid_argument(os.str());
        }
        cmdLineQos[q.substr(0, off)] = q.substr(off + 1);
    }

    bool slow = opts.isSet("slow");
    int maxQueueDropEvents = opts.isSet("maxQueueDropEvents") ? stoi(opts.optArg("maxQueueDropEvents")) : 0;
    int maxQueueRemoveSub = opts.isSet("maxQueueRemoveSub") ? stoi(opts.optArg("maxQueueRemoveSub")) : 0;
    bool erratic = false;
    int erraticNum = 0;
    s = opts.optArg("erratic");
    if (!s.empty())
    {
        erratic = true;
        erraticNum = stoi(s);
    }
    if (events <= 0)
    {
        ostringstream os;
        os << argv[0] << ": events must be > 0.";
        throw invalid_argument(os.str());
    }

    auto properties = communicator->getProperties();
    string managerProxyProperty = "IceStormAdmin.TopicManager.Default";
    string managerProxy = properties->getProperty(managerProxyProperty);
    if (managerProxy.empty())
    {
        ostringstream os;
        os << argv[0] << ": property `" << managerProxyProperty << "' is not set";
        throw invalid_argument(os.str());
    }

    IceStorm::TopicManagerPrx manager(communicator, managerProxy);

    vector<Subscription> subs;

    if (erratic)
    {
        for (int i = 0; i < erraticNum; ++i)
        {
            ostringstream os;
            os << "SubscriberAdapter" << i;
            Subscription item;
            item.adapter = communicator->createObjectAdapterWithEndpoints(os.str(), "default");
            item.servant = make_shared<ErraticEventI>(communicator, events);
            item.qos["reliability"] = "twoway";
            subs.push_back(item);
        }
    }
    else if (slow)
    {
        Subscription item;
        item.adapter = communicator->createObjectAdapterWithEndpoints("SubscriberAdapter", "default");
        item.servant = make_shared<SlowEventI>(communicator, events);
        item.qos = cmdLineQos;
        subs.push_back(item);
    }
    else if (maxQueueDropEvents || maxQueueRemoveSub)
    {
        Subscription item1;
        item1.adapter = communicator->createObjectAdapterWithEndpoints("MaxQueueAdapter", "default");
        if (maxQueueDropEvents)
        {
            item1.servant = make_shared<MaxQueueEventI>(communicator, maxQueueDropEvents, events, false);
        }
        else
        {
            item1.servant = make_shared<MaxQueueEventI>(communicator, maxQueueRemoveSub, events, true);
        }
        item1.qos = cmdLineQos;
        item1.activate = false;
        subs.push_back(item1);

        Subscription item2;
        item2.adapter = communicator->createObjectAdapterWithEndpoints("ControllerAdapter", "default");
        item2.servant = make_shared<ControllerEventI>(communicator, events, item1.adapter);
        item2.qos["reliability"] = "oneway";
        subs.push_back(item2);
    }
    else
    {
        Subscription item;
        item.adapter = communicator->createObjectAdapterWithEndpoints("SubscriberAdapter", "default");
        item.qos = cmdLineQos;
        auto p = item.qos.find("reliability");
        if (p != item.qos.end() && p->second == "ordered")
        {
            item.servant = make_shared<OrderEventI>(communicator, events);
        }
        else
        {
            item.servant = make_shared<CountEventI>(communicator, events);
        }
        subs.push_back(item);
    }

    auto topic = manager->retrieve("fed1");

    {
        for (auto& p : subs)
        {
            p.obj = p.adapter->addWithUUID(p.servant);

            IceStorm::QoS qos;
            string reliability = "";
            auto q = p.qos.find("reliability");
            if (q != p.qos.end())
            {
                reliability = q->second;
            }
            if (reliability == "twoway")
            {
                // Do nothing.
            }
            else if (reliability == "ordered")
            {
                qos["reliability"] = "ordered";
            }
            else if (reliability == "batch")
            {
                p.obj = p.obj->ice_batchOneway();
            }
            else // if(reliability == "oneway")
            {
                p.obj = p.obj->ice_oneway();
            }
            p.publisher = topic->subscribeAndGetPublisher(qos, p.obj);
        }
    }

    {
        for (const auto& p : subs)
        {
            if (p.activate)
            {
                p.adapter->activate();
            }
        }
    }

    communicator->waitForShutdown();

    {
        for (const auto& p : subs)
        {
            p.servant->check(p);
            topic->unsubscribe(p.obj);
            if (p.servant->count() != events)
            {
                ostringstream os;
                os << "expected " << events << " events but got " << p.servant->count() << " events.";
                throw invalid_argument(os.str());
            }
        }
    }
}

DEFINE_TEST(Subscriber)
