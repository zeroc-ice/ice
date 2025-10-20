// Copyright (c) ZeroC, Inc.

#include "../../src/Ice/Options.h"
#include "Ice/Ice.h"
#include "IceStorm/IceStorm.h"
#include "Single.h"
#include "TestHelper.h"

#include <stdexcept>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Test;

class SingleI final : public Single
{
public:
    SingleI(CommunicatorPtr communicator, string name, int max)
        : _communicator(std::move(communicator)),
          _name(std::move(name)),
          _max(max)

    {
    }

    void event(int i, const Current&) override
    {
        if (_name == "twoway ordered" && i != _last)
        {
            cerr << endl << "received unordered event for `" << _name << "': " << i << " " << _last;
            test(false);
        }
        lock_guard<mutex> lg(_mutex);
        ++_last;
        if (++_count == _max)
        {
            _condVar.notify_one();
        }
    }

    void waitForEvents()
    {
        unique_lock<mutex> lock(_mutex);
        while (_count < _max)
        {
            if (_condVar.wait_for(lock, 40s) == cv_status::timeout)
            {
                test(false);
            }
        }
    }

private:
    CommunicatorPtr _communicator;
    const string _name;
    const int _max;
    int _count{0};
    int _last{0};
    mutex _mutex;
    condition_variable _condVar;
};

class Subscriber final : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Subscriber::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv, make_shared<Ice::Properties>("IceStormAdmin"));
    IceInternal::Options opts;
    opts.addOpt("", "ordered");
    opts.addOpt("", "twoway");
    opts.addOpt("", "events", IceInternal::Options::NeedArg);

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

    auto properties = communicator->getProperties();
    auto managerProxy = properties->getIceProperty("IceStormAdmin.TopicManager.Default");
    if (managerProxy.empty())
    {
        ostringstream os;
        os << argv[0] << ": property `IceStormAdmin.TopicManager.Default' is not set";
        throw invalid_argument(os.str());
    }

    auto base = communicator->stringToProxy(managerProxy);
    auto manager = checkedCast<IceStorm::TopicManagerPrx>(base);
    if (!manager)
    {
        ostringstream os;
        os << argv[0] << ": `" << managerProxy << "' is not running";
        throw invalid_argument(os.str());
    }

    auto adapter = communicator->createObjectAdapterWithEndpoints("SingleAdapter", "default");

    optional<TopicPrx> topic;
    while (true)
    {
        try
        {
            topic = manager->retrieve("single");
            break;
        }
        // This can happen if the replica group loses the majority
        // during retrieve. In this case we retry.
        catch (const Ice::UnknownException&)
        {
            continue;
        }
        catch (const IceStorm::NoSuchTopic& e)
        {
            ostringstream os;
            os << argv[0] << ": NoSuchTopic: " << e.name;
            throw invalid_argument(os.str());
        }
    }

    int events = 1000;
    if (opts.isSet("events"))
    {
        events = stoi(opts.optArg("events"));
    }
    //
    // Create subscribers with different QoS.
    //
    shared_ptr<SingleI> sub;
    IceStorm::QoS qos;
    if (opts.isSet("ordered"))
    {
        sub = make_shared<SingleI>(communicator.communicator(), "twoway ordered", events);
        qos["reliability"] = "ordered";
    }
    else
    {
        sub = make_shared<SingleI>(communicator.communicator(), "twoway", events);
    }

    auto prx = adapter->addWithUUID(sub);

    while (true)
    {
        try
        {
            topic->subscribeAndGetPublisher(qos, prx);
            break;
        }
        // If we're already subscribed then we're done (previously we
        // got an UnknownException which succeeded).
        catch (const IceStorm::AlreadySubscribed&)
        {
            break;
        }
        // This can happen if the replica group loses the majority
        // during subscription. In this case we retry.
        catch (const Ice::UnknownException&)
        {
        }
    }

    adapter->activate();

    sub->waitForEvents();

    topic->unsubscribe(prx);
}

DEFINE_TEST(Subscriber)
