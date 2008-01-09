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
    bool batch = false;

    enum Option { Datagram, Twoway, Oneway, Ordered};
    Option option = Oneway;

    string topicName = "time";

    if(argc > 4)
    {
        cerr << appName() << ": too many arguments" << endl;
        usage(appName());
        return EXIT_FAILURE;
    }
    
    if(argc >= 2)
    {
        int argIndex = 1;
        string optionString = argv[argIndex];
  
        if(optionString == "--batch")
        {
            batch = true;
            if(argc >= 3)
            {
                argIndex++;
                optionString = argv[argIndex];
            }
            else
            {
                optionString = "";
            }
        }

        if(optionString == "")
        {
            // done
        }
        else if(optionString == "--datagram")
        {
            option = Datagram;
        }
        else if(optionString == "--twoway")
        {
            option = Twoway;
        }
        else if(optionString == "--oneway")
        {
            option = Oneway;
        }
        else if(optionString == "--ordered")
        {
            option = Ordered;
        }
        else if(argIndex == argc - 2)
        {
            cerr << appName() << ": too many arguments" << endl;
            usage(appName());
            return EXIT_FAILURE;
        }
        else
        {
            topicName = optionString;
        }

        argIndex++;
        if(argIndex < argc)
        {
            topicName = argv[argIndex];
        }

        if(topicName[0] == '-')
        {
            cerr << appName() << ": invalid topic name" << endl;
            usage(appName());
            return EXIT_FAILURE;
        }
    }

    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(
        communicator()->propertyToProxy("IceStorm.TopicManager.Proxy"));
    if(!manager)
    {
        cerr << appName() << ": invalid proxy" << endl;
        return EXIT_FAILURE;
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
  
    if(option == Datagram)
    {
        subscriber = subscriber->ice_datagram();
    }
    else if(option == Twoway)
    {
        // Do nothing to the subscriber proxy. Its already twoway.
       
    }
    else if(option == Ordered)
    {
        qos["reliability"] = "ordered";
        // Do nothing to the subscriber proxy. Its already twoway.
      
    }
    else if(option == Oneway)
    {
        subscriber = subscriber->ice_oneway();
    }

    if(batch)
    {
        if(option == Twoway || option == Ordered)
        {
            cerr << appName() << ": batch can only be set with oneway or datagram" << endl;
            return EXIT_FAILURE;
        }
        if(option == Datagram)
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
