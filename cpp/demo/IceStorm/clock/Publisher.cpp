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
    IceUtil::Options opts;
    opts.addOpt("", "datagram");
    opts.addOpt("", "twoway");
    opts.addOpt("", "oneway");

    IceUtil::Options::StringVector remaining;
    try
    {
        remaining = opts.parse(argc, (const char**)argv);
    }
    catch(const IceUtil::BadOptException& e)
    {
        cerr << argv[0] << ": " << e.reason << endl;
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
    int optsSet = 0;
    if(opts.isSet("datagram"))
    {
        publisher = publisher->ice_datagram();
        ++optsSet;
    }
    else if(opts.isSet("twoway"))
    {
        // Do nothing.
        ++optsSet;
    }
    else if(opts.isSet("oneway") || optsSet == 0)
    {
        publisher = publisher->ice_oneway();
        ++optsSet;
    }
    if(optsSet != 1)
    {
        usage(appName());
        return EXIT_FAILURE;
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
