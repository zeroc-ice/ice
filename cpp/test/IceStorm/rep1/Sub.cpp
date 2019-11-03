//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/Options.h>
#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>
#include <Single.h>
#include <TestHelper.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Test;

class SingleI final : public Single
{
public:

    void event(int, const Current&) override
    {
    }
};

class Sub final : public Test::TestHelper
{
public:

    void run(int, char**) override;
};

void
Sub::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    IceUtilInternal::Options opts;
    opts.addOpt("", "id", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "unsub");

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

    auto properties = communicator->getProperties();
    auto managerProxy = properties->getProperty("IceStormAdmin.TopicManager.Default");
    if(managerProxy.empty())
    {
        ostringstream os;
        os << argv[0] << ": property `IceStormAdmin.TopicManager.Default' is not set";
        throw invalid_argument(os.str());
    }

    auto base = communicator->stringToProxy(managerProxy);
    auto manager = checkedCast<IceStorm::TopicManagerPrx>(base);
    if(!manager)
    {
        ostringstream os;
        os << argv[0] << ": `" << managerProxy << "' is not running";
        throw invalid_argument(os.str());
    }

    auto adapter = communicator->createObjectAdapterWithEndpoints("SingleAdapter", "default");

    auto topic = manager->retrieve("single");

    auto prx = adapter->add(make_shared<SingleI>(), stringToIdentity(opts.optArg("id")));
    if(opts.isSet("unsub"))
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
