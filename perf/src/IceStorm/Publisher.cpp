// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Thread.h>
#include <Ice/Application.h>
#include <IceStorm/IceStorm.h>

#include <Perf.h>
#include <Sync.h>

using namespace std;
using namespace Perf;

//
// Time types do not give us sufficient resolution on Windows, so the
// high resolution performance counters are used.
//
Ice::Long
getTick()
{
#ifdef WIN32
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return t.QuadPart;
#else
    return IceUtil::Time::now().toMicroSeconds();
#endif
}

class SyncI : public Sync, IceUtil::Monitor<IceUtil::Mutex>
{
public:

    SyncI() : _notified(false) 
    {
    }

    virtual void 
    waitNotification()
    {
        Lock sync(*this);
        while(!_notified)
        {
            wait();
        }
    }

    virtual void
    notify(const Ice::Current&)
    {
        Lock sync(*this);
        _notified = true;
        notifyAll();
    }

private:

    bool _notified;
};
typedef IceUtil::Handle<SyncI> SyncIPtr;

class Publisher : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    Publisher app;
    return app.main(argc, argv, "config");
}

int
Publisher::run(int argc, char* argv[])
{
    int period = 0;
    int repetitions = 10000;
    bool twoway = false;
    bool payload = false;
    for(int i = 0; i < argc; i++)
    {
        if(strcmp(argv[i], "-p") == 0)
        {
            period = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "-r") == 0)
        {
            repetitions = atoi(argv[++i]);
        }
        if(strcmp(argv[i], "-t") == 0)
        {
            twoway = true;
        }
        if(strcmp(argv[i], "-w") == 0)
        {
            payload = true;
        }
    }

    Ice::PropertiesPtr properties = communicator()->getProperties();

    const string proxyProperty = "IceStorm.TopicManager.Proxy";
    string proxy = properties->getProperty(proxyProperty);
    if(proxy.empty())
    {
        cerr << appName() << ": property `" << proxyProperty << "' not set" << endl;
        return EXIT_FAILURE;
    }

    Ice::ObjectPrx base = communicator()->stringToProxy(proxy);
    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(base);
    if(!manager)
    {
        cerr << appName() << ": invalid proxy" << endl;
        return EXIT_FAILURE;
    }

    //
    // Retrieve the topic named "time".
    //
    IceStorm::TopicPrx topic;
    try
    {
        topic = manager->retrieve("time");
    }
    catch(const IceStorm::NoSuchTopic& e)
    {
        cerr << appName() << ": " << e << " name: " << e.name << endl;
        return EXIT_FAILURE;
    }
    assert(topic);

    //
    // Create a proxy for the synchronization object.
    //
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapterWithEndpoints("Publisher", "tcp");
    SyncIPtr sync = new SyncI();
    Ice::ObjectPrx obj = adapter->addWithUUID(sync);

    PingPrx ping;
    if(twoway)
    {
        ping = PingPrx::uncheckedCast(topic->getPublisher()->ice_twoway());
    }
    else
    {
        ping = PingPrx::uncheckedCast(topic->getPublisher()->ice_oneway());
    }
    ping->ice_ping();
    cout << communicator()->proxyToString(obj) << endl;
    adapter->activate();

    //
    // Wait for the notification to start publishing.
    //
    sync->waitNotification();

    IceUtil::ThreadControl::yield();

    if(!payload)
    {
        ping->tickVoid(0);
        for(int i = 0; i < repetitions; ++i)
        {
            ping->tickVoid(getTick());
            if(period > 0)
            {
                IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(period));
            }
        }
        ping->tickVoid(-1);
    }
    else
    {
        ping->tick(0, A, 10, AStruct());
        for(int i = 0; i < repetitions; ++i)
        {
            AStruct s;
            s.s = "TEST";
            ping->tick(getTick(), A, 10, s);
            if(period > 0)
            {
                IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(period));
            }
        }
        ping->tick(-1, A, 10, AStruct());
    }

    return EXIT_SUCCESS;
}
