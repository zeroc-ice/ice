// Copyright (c) ZeroC, Inc.

#include "../../src/Ice/Options.h"
#include "Ice/Ice.h"
#include "IceStorm/IceStorm.h"
#include "Single.h"
#include "TestHelper.h"

#include <stdexcept>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Test;

class SingleI final : public Single
{
public:
    void event(int, const Current&) override {}
};

class Sub final : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Sub::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv, make_shared<Ice::Properties>("IceStormAdmin"));
    IceInternal::Options opts;
    opts.addOpt("", "id", IceInternal::Options::NeedArg);
    opts.addOpt("", "unsub");

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

    auto properties = communicator->getProperties();
    auto managerProxy = properties->getIceProperty("IceStormAdmin.TopicManager.Default");
    if (managerProxy.empty())
    {
        ostringstream os;
        os << argv[0] << ": property `IceStormAdmin.TopicManager.Default' is not set";
        throw invalid_argument(os.str());
    }

    auto base = communicator->stringToProxy(managerProxy);
    auto manager = checkedCast<IceStorm::TopicManagerPrx>(base);
    if (!manager)
    {
        ostringstream os;
        os << argv[0] << ": `" << managerProxy << "' is not running";
        throw invalid_argument(os.str());
    }

    auto adapter = communicator->createObjectAdapterWithEndpoints("SingleAdapter", "default");

    auto topic = manager->retrieve("single");

    auto prx = adapter->add(make_shared<SingleI>(), stringToIdentity(opts.optArg("id")));
    if (opts.isSet("unsub"))
    {
        topic->unsubscribe(prx);
    }
    else
    {
        IceStorm::QoS qos;
        qos["persistent"] = "true";
        topic->subscribeAndGetPublisher(qos, prx);
    }
}

DEFINE_TEST(Sub)
