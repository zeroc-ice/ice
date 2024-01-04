//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>
#include <Single.h>
#include <TestHelper.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Test;

class SingleI final : public Single
{
public:

    SingleI(const string& name) :
        _name(name),
        _count(0),
        _last(0)
    {
    }

    void
    event(int i, const Current& current) override
    {
        if((_name == "default" || _name == "oneway" || _name == "batch" || _name == "datagram" ||
            _name == "batch datagram") && current.requestId != 0)
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
        if((_name == "datagram" || _name == "batch datagram") && current.con->type() != "udp")
        {
            cerr << endl << "expected datagram to be received over udp";
            test(false);
        }
        lock_guard<mutex> lg(_mutex);
        if(_name == "per-request load balancing")
        {
            _connections.insert(current.con);
        }
        ++_last;
        if(++_count == 1000)
        {
            _condVar.notify_one();
        }
    }

    void
    waitForEvents()
    {
        unique_lock<mutex> lock(_mutex);
        cout << "testing " << _name << " ... " << flush;
        bool datagram = _name == "datagram" || _name == "batch datagram";
        while(_count < 1000)
        {
            if(_condVar.wait_for(lock, 30s) == cv_status::timeout)
            {
                if(datagram && _count > 0)
                {
                    if(_count < 100)
                    {
                        cout << "[" << _count << "/1000: This may be an error!!]";
                    }
                    else
                    {
                        cout << "[" << _count << "/1000] ";
                    }
                    break;
                }
                else
                {
                    test(false);
                }
            }
        }
        if(_name == "per-request load balancing")
        {
            test(_connections.size() == 2);
        }
        cout << "ok" << endl;
    }

private:

    const string _name;
    int _count;
    int _last;
    set<shared_ptr<Ice::Connection>> _connections;
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
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    auto properties = communicator->getProperties();
    string managerProxy = properties->getProperty("IceStormAdmin.TopicManager.Default");
    if(managerProxy.empty())
    {
        ostringstream os;
        os << argv[0] << ": property `IceStormAdmin.TopicManager.Default' is not set";
        throw invalid_argument(os.str());
    }

    auto base = communicator->stringToProxy(managerProxy);
    auto manager = checkedCast<IceStorm::TopicManagerPrx>(base);
    if(!manager)
    {
        ostringstream os;
        os << argv[0] << ": `" << managerProxy << "' is not running";
        throw invalid_argument(os.str());
    }

    // Use 2 default endpoints to test per-request load balancing
    auto adapter = communicator->createObjectAdapterWithEndpoints("SingleAdapter", "default:default:udp");

    //
    // Test topic name that is too long
    //
    if(string(argv[1]) != "transient")
    {
        try
        {
            manager->create(string(512, 'A'));
            test(false);
        }
        catch(const Ice::UnknownException&)
        {
        }
    }

    auto topic = manager->retrieve("single");

    //
    // Test subscriber identity that is too long
    //
    if(string(argv[1]) != "transient")
    {
        try
        {
            auto object = communicator->stringToProxy(string(512, 'A') + ":default -p 10000");
            topic->subscribeAndGetPublisher(IceStorm::QoS(), object);
            test(false);
        }
        catch(const Ice::UnknownException&)
        {
        }
    }

    //
    // Create subscribers with different QoS.
    //
    vector<shared_ptr<SingleI>> subscribers;
    vector<Ice::Identity> subscriberIdentities;

    {
        subscribers.push_back(make_shared<SingleI>("default"));
        auto object = adapter->addWithUUID(subscribers.back())->ice_oneway();
        subscriberIdentities.push_back(object->ice_getIdentity());
        topic->subscribeAndGetPublisher(IceStorm::QoS(), object);
    }
    {
        subscribers.push_back(make_shared<SingleI>("oneway"));
        auto object = adapter->addWithUUID(subscribers.back())->ice_oneway();
        subscriberIdentities.push_back(object->ice_getIdentity());
        topic->subscribeAndGetPublisher(IceStorm::QoS(), object);
    }
    {
        subscribers.push_back(make_shared<SingleI>("twoway"));
        auto object = adapter->addWithUUID(subscribers.back());
        subscriberIdentities.push_back(object->ice_getIdentity());
        topic->subscribeAndGetPublisher(IceStorm::QoS(), object);
    }
    {
        subscribers.push_back(make_shared<SingleI>("batch"));
        auto object = adapter->addWithUUID(subscribers.back())->ice_batchOneway();
        subscriberIdentities.push_back(object->ice_getIdentity());
        topic->subscribeAndGetPublisher(IceStorm::QoS(), object);
    }
    {
        subscribers.push_back(make_shared<SingleI>("twoway ordered")); // Ordered
        IceStorm::QoS qos;
        qos["reliability"] = "ordered";
        auto object = adapter->addWithUUID(subscribers.back());
        subscriberIdentities.push_back(object->ice_getIdentity());
        topic->subscribeAndGetPublisher(qos, object);
    }

    {
        subscribers.push_back(make_shared<SingleI>("per-request load balancing"));
        IceStorm::QoS qos;
        qos["locatorCacheTimeout"] = "10";
        qos["connectionCached"] = "0";
        auto object = adapter->addWithUUID(subscribers.back());
        subscriberIdentities.push_back(object->ice_getIdentity());
        topic->subscribeAndGetPublisher(qos, object);
    }
    {
        // Use a separate adapter to ensure a separate connection is used for the subscriber
        // (otherwise, if multiple UDP subscribers use the same connection we might get high
        // packet loss, see bug 1784).
        communicator->getProperties()->setProperty("UdpAdapter3.ThreadPool.Size", "1");
        auto adpt = communicator->createObjectAdapterWithEndpoints("UdpAdapter3", "udp");
        subscribers.push_back(make_shared<SingleI>("datagram"));
        auto object = adpt->addWithUUID(subscribers.back())->ice_datagram();
        subscriberIdentities.push_back(object->ice_getIdentity());
        adpt->activate();
        topic->subscribeAndGetPublisher(IceStorm::QoS(), object);
    }
    {
        // Use a separate adapter to ensure a separate connection is used for the subscriber
        // (otherwise, if multiple UDP subscribers use the same connection we might get high
        // packet loss, see bug 1784).
        communicator->getProperties()->setProperty("UdpAdapter4.ThreadPool.Size", "1");
        auto adpt = communicator->createObjectAdapterWithEndpoints("UdpAdapter4", "udp");
        subscribers.push_back(make_shared<SingleI>("batch datagram"));
        auto object = adpt->addWithUUID(subscribers.back())->ice_batchDatagram();
        subscriberIdentities.push_back(object->ice_getIdentity());
        adpt->activate();
        topic->subscribeAndGetPublisher(IceStorm::QoS(), object);
    }

    adapter->activate();

    vector<Ice::Identity> ids = topic->getSubscribers();
    test(ids.size() == subscriberIdentities.size());
    for(const auto& p: ids)
    {
        test(find(subscriberIdentities.begin(), subscriberIdentities.end(), p) != subscriberIdentities.end());
    }

    for(const auto& p: subscribers)
    {
        p->waitForEvents();
    }
}

DEFINE_TEST(Subscriber)
