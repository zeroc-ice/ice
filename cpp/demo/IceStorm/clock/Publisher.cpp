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

using namespace std;
using namespace Demo;

class Publisher : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    Publisher app;
    return app.main(argc, argv, "config.pub");
}

void
usage(const string& n)
{
    cerr << "Usage: " << n << " [--datagram|--twoway|--oneway] [topic]\n" << endl;
}

int
Publisher::run(int argc, char* argv[])
{
    enum Option { Datagram, Twoway, Oneway };
    Option option = Oneway;

    string topicName = "time";

    if(argc > 3)
    {
        cerr << appName() << ": too many arguments" << endl;
        usage(appName());
        return EXIT_FAILURE;
    }

    if(argc >= 2)
    {
        string optionString = argv[1];
        if(optionString == "--datagram")
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
        else if(argc == 3)
        {
            cerr << appName() << ": too many arguments" << endl;
            usage(appName());
            return EXIT_FAILURE;
        }
        else
        {
            topicName = optionString;
        }

        if(argc == 3)
        {
            topicName = argv[2];
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

    //
    // Retrieve the topic.
    //
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

    //
    // Get the topic's publisher object, and create a Clock proxy with
    // the mode specified as an argument of this application.
    //
    Ice::ObjectPrx publisher = topic->getPublisher();
    if(option == Datagram)
    {
        publisher = publisher->ice_datagram();
    }
    else if(option == Twoway)
    {
        // Do nothing.
    }
    else if(option == Oneway)
    {
        publisher = publisher->ice_oneway();
    }
    
    ClockPrx clock = ClockPrx::uncheckedCast(publisher);

    cout << "publishing tick events. Press ^C to terminate the application." << endl;
    try
    {
        while(true)
        {
            clock->tick(IceUtil::Time::now().toDateTime());
            IceUtil::ThreadControl::sleep(IceUtil::Time::seconds(1));
        }
    }
    catch(const Ice::CommunicatorDestroyedException&)
    {
        // Ignore
    }

    return EXIT_SUCCESS;
}
