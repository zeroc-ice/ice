// Copyright (c) ZeroC, Inc.

#include "../../src/Ice/Options.h"
#include "Event.h"
#include "Ice/Ice.h"
#include "IceStorm/IceStorm.h"
#include "TestHelper.h"

#include <stdexcept>
#include <thread>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Test;

class Publisher final : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Publisher::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv, make_shared<Ice::Properties>("IceStormAdmin"));
    IceInternal::Options opts;
    opts.addOpt("", "events", IceInternal::Options::NeedArg);
    opts.addOpt("", "oneway");
    opts.addOpt("", "maxQueueTest");

    try
    {
        opts.parse(argc, (const char**)argv);
    }
    catch (const IceInternal::BadOptException& e)
    {
        ostringstream os;
        os << argv[0] << ": error: " << e.what();
        throw invalid_argument(os.str());
    }

    int events = 1000;
    string s = opts.optArg("events");
    if (!s.empty())
    {
        events = stoi(s);
    }
    if (events <= 0)
    {
        ostringstream os;
        os << argv[0] << ": events must be > 0.";
        throw invalid_argument(os.str());
    }

    bool oneway = opts.isSet("oneway");
    bool maxQueueTest = opts.isSet("maxQueueTest");

    auto properties = communicator->getProperties();
    string managerProxyProperty = "IceStormAdmin.TopicManager.Default";
    string managerProxy = properties->getProperty(managerProxyProperty);
    if (managerProxy.empty())
    {
        ostringstream os;
        os << argv[0] << ": property `" << managerProxyProperty << "' is not set";
        throw invalid_argument(os.str());
    }

    auto manager = checkedCast<IceStorm::TopicManagerPrx>(communicator->stringToProxy(managerProxy));
    if (!manager)
    {
        ostringstream os;
        os << argv[0] << ": `" << managerProxy << "' is not running";
        throw invalid_argument(os.str());
    }

    auto topic = manager->retrieve("fed1");

    auto twowayProxy = uncheckedCast<EventPrx>(topic->getPublisher()->ice_twoway());
    EventPrx proxy = oneway ? twowayProxy->ice_oneway() : twowayProxy;

    for (int i = 0; i < events; ++i)
    {
        if (maxQueueTest && i == 10)
        {
            // Sleep one seconds to give some time to IceStorm to connect to the subscriber
            this_thread::sleep_for(1s);
        }
        proxy->pub(i);
    }

    if (oneway)
    {
        //
        // Before we exit, we ping all proxies as twoway, to make sure
        // that all oneways are delivered.
        //
        twowayProxy->ice_ping();
    }
}

DEFINE_TEST(Publisher)
