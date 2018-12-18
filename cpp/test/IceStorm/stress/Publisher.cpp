// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/Options.h>
#include <IceStorm/IceStorm.h>
#include <Event.h>
#include <TestHelper.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Test;

class Publisher : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Publisher::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    IceUtilInternal::Options opts;
    opts.addOpt("", "events", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "oneway");
    opts.addOpt("", "maxQueueTest");

    try
    {
        opts.parse(argc, (const char**)argv);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        ostringstream os;
        os << argv[0] << ": error: " << e.reason;
        throw invalid_argument(os.str());
    }

    int events = 1000;
    string s = opts.optArg("events");
    if(!s.empty())
    {
        events = atoi(s.c_str());
    }
    if(events <= 0)
    {
        ostringstream os;
        os << argv[0] << ": events must be > 0.";
        throw invalid_argument(os.str());
    }

    bool oneway = opts.isSet("oneway");
    bool maxQueueTest = opts.isSet("maxQueueTest");

    PropertiesPtr properties = communicator->getProperties();
    const char* managerProxyProperty = "IceStormAdmin.TopicManager.Default";
    string managerProxy = properties->getProperty(managerProxyProperty);
    if(managerProxy.empty())
    {
        ostringstream os;
        os << argv[0] << ": property `" << managerProxyProperty << "' is not set";
        throw invalid_argument(os.str());
    }

    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(
        communicator->stringToProxy(managerProxy));
    if(!manager)
    {
        ostringstream os;
        os << argv[0] << ": `" << managerProxy << "' is not running";
        throw invalid_argument(os.str());
    }

    TopicPrx topic = manager->retrieve("fed1");

    EventPrx twowayProxy = EventPrx::uncheckedCast(topic->getPublisher()->ice_twoway());
    EventPrx proxy;
    if(oneway)
    {
        proxy = EventPrx::uncheckedCast(twowayProxy->ice_oneway());
    }
    else
    {
        proxy = twowayProxy;
    }

    for(int i = 0; i < events; ++i)
    {
        if(maxQueueTest && i == 10)
        {
            // Sleep one seconds to give some time to IceStorm to connect to the subscriber
            IceUtil::ThreadControl::sleep(IceUtil::Time::seconds(1));
        }
        proxy->pub(i);
    }

    if(oneway)
    {
        //
        // Before we exit, we ping all proxies as twoway, to make sure
        // that all oneways are delivered.
        //
        twowayProxy->ice_ping();
    }
}

DEFINE_TEST(Publisher)
