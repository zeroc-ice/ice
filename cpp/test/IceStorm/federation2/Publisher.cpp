// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <IceUtil/Options.h>
#include <Ice/Ice.h>
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
    opts.addOpt("", "count", IceUtilInternal::Options::NeedArg);
    opts.parse(argc, (const char**)argv);

    PropertiesPtr properties = communicator->getProperties();
    string managerProxy = properties->getProperty("IceStormAdmin.TopicManager.Default");
    if(managerProxy.empty())
    {
        throw runtime_error("property `IceStormAdmin.TopicManager.Default' is not set");
    }

    ObjectPrx base = communicator->stringToProxy(managerProxy);
    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(base);
    if(!manager)
    {
        throw runtime_error("`" + managerProxy + "' is not running");
    }

    TopicPrx fed1 = manager->retrieve("fed1");
    EventPrx eventFed1 = EventPrx::uncheckedCast(fed1->getPublisher()->ice_oneway());

    string arg = opts.optArg("count");
    int count = 1;
    if(arg.empty())
    {
        count = atoi(arg.c_str());
    }

    while(true)
    {
        for(int i = 0; i < 10; ++i)
        {
            eventFed1->pub("fed1");
        }
        //
        // Before we exit, we ping all proxies as twoway, to make sure
        // that all oneways are delivered.
        //
        EventPrx::uncheckedCast(eventFed1->ice_twoway())->ice_ping();

        if(count == 0)
        {
            break;
        }
        --count;
        IceUtil::ThreadControl::sleep(IceUtil::Time::seconds(1));
    }
}

DEFINE_TEST(Publisher)
