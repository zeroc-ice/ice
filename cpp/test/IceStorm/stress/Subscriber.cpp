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
#include <IceUtil/Options.h>
#include <IceUtil/Thread.h>
#include <IceUtil/Time.h>
#include <IceUtil/Random.h>
#include <Event.h>

#include <TestCommon.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Test;

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

protected:

    const CommunicatorPtr _communicator;
    const int _total;
    int _count;
};
typedef IceUtil::Handle<EventI> EventIPtr;

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
        IceUtil::StaticMutex::Lock sync(_remainingMutex);
        ++_remaining;
    }

    virtual void
    pub(int, const Ice::Current& current)
    {
        Lock sync(*this);

        // Randomly close the connection.
        if(!_done && (IceUtil::random(10) == 1 || ++_count == _total))
        {
            _done = true;
            current.con->close(true);
            // Deactivate the OA. This ensures that the subscribers
            // that have subscribed with oneway QoS will be booted.
            current.adapter->deactivate();
            _count = _total;
            {
                IceUtil::StaticMutex::Lock sync(_remainingMutex);
                --_remaining;
                if(_remaining == 0)
                {
                    _communicator->shutdown();
                }
            }
        }
    }

private:

    static IceUtil::StaticMutex _remainingMutex;
    static int _remaining;
    bool _done;
};

IceUtil::StaticMutex ErraticEventI::_remainingMutex = ICE_STATIC_MUTEX_INITIALIZER;
int ErraticEventI::_remaining = 0;

struct Subscription
{
    Ice::ObjectAdapterPtr adapter;
    Ice::ObjectPrx obj;
    EventIPtr servant;
    IceStorm::QoS qos;
};

int
run(int argc, char* argv[], const CommunicatorPtr& communicator)
{
    IceUtil::Options opts;
    opts.addOpt("", "events", IceUtil::Options::NeedArg);
    opts.addOpt("", "qos", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("", "slow");
    opts.addOpt("", "erratic", IceUtil::Options::NeedArg);

    try
    {
        opts.parse(argc, (const char**)argv);
    }
    catch(const IceUtil::BadOptException& e)
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
    const char* managerProxyProperty = "IceStorm.TopicManager.Proxy";
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
            topic->subscribeAndGetPublisher(qos, p->obj);
        }
    }

    {
        for(vector<Subscription>::iterator p = subs.begin(); p != subs.end(); ++p)
        {
            p->adapter->activate();
        }
    }

    communicator->waitForShutdown();

    {
        for(vector<Subscription>::const_iterator p = subs.begin(); p != subs.end(); ++p)
        {
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
