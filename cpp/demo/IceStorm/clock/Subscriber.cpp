// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>

#include <Clock.h>

#include <map>

using namespace std;
using namespace Demo;

class ClockI : public Clock
{
public:

    virtual void
    tick(const string& time, const Ice::Current&)
    {
        cout << time << endl;
    }
};

class Subscriber : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    Subscriber app;
    return app.main(argc, argv, "config.sub");
}

void
usage(const string& n)
{
    cerr << "Usage: " << n << " [--batch] [--datagram|--twoway|--ordered|--oneway] [topic]" << endl;
}

int
Subscriber::run(int argc, char* argv[])
{
    IceUtil::Options opts;
    opts.addOpt("", "datagram");
    opts.addOpt("", "twoway");
    opts.addOpt("", "ordered");
    opts.addOpt("", "oneway");
    opts.addOpt("", "batch");

    IceUtil::Options::StringVector remaining;
    try
    {
        remaining = opts.parse(argc, (const char**)argv);
    }
    catch(const IceUtil::BadOptException& e)
    {
        cerr << argv[0] << ": " << e.reason << endl;
        usage(appName());
        return EXIT_FAILURE;
    }

    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(
        communicator()->propertyToProxy("IceStorm.TopicManager.Proxy"));
    if(!manager)
    {
        cerr << appName() << ": invalid proxy" << endl;
        return EXIT_FAILURE;
    }

    string topicName = "time";
    if(!remaining.empty())
    {
        topicName = remaining.front();
    }

    IceStorm::TopicPrx topic;
    try
    {
        topic = manager->retrieve(topicName);
    }
    catch(const IceStorm::NoSuchTopic&)
    {
        try
        {
            topic = manager->create(topicName);
        }
        catch(const IceStorm::TopicExists&)
        {
            cerr << appName() << ": temporary failure. try again." << endl;
            return EXIT_FAILURE;
        }
    }

    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Clock.Subscriber");

    //
    // Add a Servant for the Ice Object.
    //
    IceStorm::QoS qos;
    Ice::ObjectPrx subscriber = adapter->addWithUUID(new ClockI);
    //
    // Set up the proxy.
    //
    int optsSet = 0;
    if(opts.isSet("datagram"))
    {
        subscriber = subscriber->ice_datagram();
        ++optsSet;
    }
    if(opts.isSet("twoway"))
    {
        // Do nothing to the subscriber proxy. Its already twoway.
        ++optsSet;
    }
    if(opts.isSet("ordered"))
    {
        qos["reliability"] = "ordered";
        // Do nothing to the subscriber proxy. Its already twoway.
        ++optsSet;
    }
    if(opts.isSet("oneway") || optsSet == 0)
    {
        subscriber = subscriber->ice_oneway();
        ++optsSet;
    }

    if(optsSet != 1)
    {
        usage(appName());
        return EXIT_FAILURE;
    }

    if(opts.isSet("batch"))
    {
        if(opts.isSet("twoway") || opts.isSet("ordered"))
        {
            cerr << appName() << ": batch can only be set with oneway or datagram" << endl;
            return EXIT_FAILURE;
        }
        if(opts.isSet("datagram"))
        {
            subscriber = subscriber->ice_batchDatagram();
        }
        else
        {
            subscriber = subscriber->ice_batchOneway();
        }
    }

    topic->subscribeAndGetPublisher(qos, subscriber);
    adapter->activate();

    shutdownOnInterrupt();
    communicator()->waitForShutdown();

    topic->unsubscribe(subscriber);

    return EXIT_SUCCESS;
}
