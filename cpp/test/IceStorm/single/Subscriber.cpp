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
        bool datagram = _name == "datagram" || _name == "batch datagram";
        IceUtil::Time timeout = (datagram) ? IceUtil::Time::seconds(5) : IceUtil::Time::seconds(20);
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
        cout << "ok" << endl;
    }

private:

    CommunicatorPtr _communicator;
    const string _name;
    int _count;
    int _last;
};
typedef IceUtil::Handle<SingleI> SingleIPtr;

int
run(int, char* argv[], const CommunicatorPtr& communicator)
{
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

    ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("SingleAdapter", "default:udp");

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
        subscribers.push_back(new SingleI(communicator, "default"));
        Ice::ObjectPrx object = adapter->addWithUUID(subscribers.back())->ice_oneway();
        subscriberIdentities.push_back(object->ice_getIdentity());
        topic->subscribeAndGetPublisher(IceStorm::QoS(), object);
    }
    {
        subscribers.push_back(new SingleI(communicator, "oneway"));
        Ice::ObjectPrx object = adapter->addWithUUID(subscribers.back())->ice_oneway();
        subscriberIdentities.push_back(object->ice_getIdentity());
        topic->subscribeAndGetPublisher(IceStorm::QoS(), object);
    }
    {
        subscribers.push_back(new SingleI(communicator, "twoway"));
        Ice::ObjectPrx object = adapter->addWithUUID(subscribers.back());
        subscriberIdentities.push_back(object->ice_getIdentity());
        topic->subscribeAndGetPublisher(IceStorm::QoS(), object);
    }
    {
        subscribers.push_back(new SingleI(communicator, "batch"));
        Ice::ObjectPrx object = adapter->addWithUUID(subscribers.back())->ice_batchOneway();
        subscriberIdentities.push_back(object->ice_getIdentity());
        topic->subscribeAndGetPublisher(IceStorm::QoS(), object);
    }
    {
        subscribers.push_back(new SingleI(communicator, "twoway ordered")); // Ordered
        IceStorm::QoS qos;
        qos["reliability"] = "ordered";
        Ice::ObjectPrx object = adapter->addWithUUID(subscribers.back());
        subscriberIdentities.push_back(object->ice_getIdentity());
        topic->subscribeAndGetPublisher(qos, object);
    }
    {
        // Use a separate adapter to ensure a separate connection is used for the subscriber
        // (otherwise, if multiple UDP subscribers use the same connection we might get high
        // packet loss, see bug 1784).
        ObjectAdapterPtr adpt = communicator->createObjectAdapterWithEndpoints("UdpAdapter3", "udp");
        subscribers.push_back(new SingleI(communicator, "datagram"));
        Ice::ObjectPrx object = adpt->addWithUUID(subscribers.back())->ice_datagram();
        subscriberIdentities.push_back(object->ice_getIdentity());
        topic->subscribeAndGetPublisher(IceStorm::QoS(), object);
        adpt->activate();
    }
    {
        // Use a separate adapter to ensure a separate connection is used for the subscriber
        // (otherwise, if multiple UDP subscribers use the same connection we might get high
        // packet loss, see bug 1784).
        ObjectAdapterPtr adpt = communicator->createObjectAdapterWithEndpoints("UdpAdapter4", "udp");
        subscribers.push_back(new SingleI(communicator, "batch datagram"));
        Ice::ObjectPrx object = adpt->addWithUUID(subscribers.back())->ice_batchDatagram();
        subscriberIdentities.push_back(object->ice_getIdentity());
        topic->subscribeAndGetPublisher(IceStorm::QoS(), object);
        adpt->activate();
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

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    CommunicatorPtr communicator;

    try
    {
        Ice::InitializationData initData = getTestInitData(argc, argv);
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
