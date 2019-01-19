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

class SingleI : public Single, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    SingleI(const string& name) :
        _name(name),
        _count(0),
        _last(0)
    {
    }

    virtual void
    event(int i, const Current& current)
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
        Lock sync(*this);
        if(_name == "per-request load balancing")
        {
            _connections.insert(current.con);
        }
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
        cout << "testing " << _name << " ... " << flush;
        bool datagram = _name == "datagram" || _name == "batch datagram";
        IceUtil::Time timeout = (datagram) ? IceUtil::Time::seconds(5) : IceUtil::Time::seconds(30);
        while(_count < 1000)
        {
            if(!timedWait(timeout))
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
    set<Ice::ConnectionPtr> _connections;
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

    // Use 2 default endpoints to test per-request load balancing
    ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("SingleAdapter", "default:default:udp");

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

    TopicPrx topic = manager->retrieve("single");

    //
    // Test subscriber identity that is too long
    //
    if(string(argv[1]) != "transient")
    {
        try
        {
            Ice::ObjectPrx object = communicator->stringToProxy(string(512, 'A') + ":default -p 10000");
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
    vector<SingleIPtr> subscribers;
    vector<Ice::Identity> subscriberIdentities;

    {
        subscribers.push_back(new SingleI("default"));
        Ice::ObjectPrx object = adapter->addWithUUID(subscribers.back())->ice_oneway();
        subscriberIdentities.push_back(object->ice_getIdentity());
        topic->subscribeAndGetPublisher(IceStorm::QoS(), object);
    }
    {
        subscribers.push_back(new SingleI("oneway"));
        Ice::ObjectPrx object = adapter->addWithUUID(subscribers.back())->ice_oneway();
        subscriberIdentities.push_back(object->ice_getIdentity());
        topic->subscribeAndGetPublisher(IceStorm::QoS(), object);
    }
    {
        subscribers.push_back(new SingleI("twoway"));
        Ice::ObjectPrx object = adapter->addWithUUID(subscribers.back());
        subscriberIdentities.push_back(object->ice_getIdentity());
        topic->subscribeAndGetPublisher(IceStorm::QoS(), object);
    }
    {
        subscribers.push_back(new SingleI("batch"));
        Ice::ObjectPrx object = adapter->addWithUUID(subscribers.back())->ice_batchOneway();
        subscriberIdentities.push_back(object->ice_getIdentity());
        topic->subscribeAndGetPublisher(IceStorm::QoS(), object);
    }
    {
        subscribers.push_back(new SingleI("twoway ordered")); // Ordered
        IceStorm::QoS qos;
        qos["reliability"] = "ordered";
        Ice::ObjectPrx object = adapter->addWithUUID(subscribers.back());
        subscriberIdentities.push_back(object->ice_getIdentity());
        topic->subscribeAndGetPublisher(qos, object);
    }

    {
        subscribers.push_back(new SingleI("per-request load balancing"));
        IceStorm::QoS qos;
        qos["locatorCacheTimeout"] = "10";
        qos["connectionCached"] = "0";
        Ice::ObjectPrx object = adapter->addWithUUID(subscribers.back());
        subscriberIdentities.push_back(object->ice_getIdentity());
        topic->subscribeAndGetPublisher(qos, object);
    }
    {
        // Use a separate adapter to ensure a separate connection is used for the subscriber
        // (otherwise, if multiple UDP subscribers use the same connection we might get high
        // packet loss, see bug 1784).
        communicator->getProperties()->setProperty("UdpAdapter3.ThreadPool.Size", "1");
        ObjectAdapterPtr adpt = communicator->createObjectAdapterWithEndpoints("UdpAdapter3", "udp");
        subscribers.push_back(new SingleI("datagram"));
        Ice::ObjectPrx object = adpt->addWithUUID(subscribers.back())->ice_datagram();
        subscriberIdentities.push_back(object->ice_getIdentity());
        adpt->activate();
        topic->subscribeAndGetPublisher(IceStorm::QoS(), object);
    }
    {
        // Use a separate adapter to ensure a separate connection is used for the subscriber
        // (otherwise, if multiple UDP subscribers use the same connection we might get high
        // packet loss, see bug 1784).
        communicator->getProperties()->setProperty("UdpAdapter4.ThreadPool.Size", "1");
        ObjectAdapterPtr adpt = communicator->createObjectAdapterWithEndpoints("UdpAdapter4", "udp");
        subscribers.push_back(new SingleI("batch datagram"));
        Ice::ObjectPrx object = adpt->addWithUUID(subscribers.back())->ice_batchDatagram();
        subscriberIdentities.push_back(object->ice_getIdentity());
        adpt->activate();
        topic->subscribeAndGetPublisher(IceStorm::QoS(), object);
    }

    adapter->activate();

    vector<Ice::Identity> ids = topic->getSubscribers();
    test(ids.size() == subscriberIdentities.size());
    for(vector<Ice::Identity>::const_iterator i = ids.begin(); i != ids.end(); ++i)
    {
        test(find(subscriberIdentities.begin(), subscriberIdentities.end(), *i) != subscriberIdentities.end());
    }

    for(vector<SingleIPtr>::const_iterator p = subscribers.begin(); p != subscribers.end(); ++p)
    {
        (*p)->waitForEvents();
    }
}

DEFINE_TEST(Subscriber)
