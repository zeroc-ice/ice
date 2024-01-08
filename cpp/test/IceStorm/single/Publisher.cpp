//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>
#include <TestHelper.h>
#include <Single.h>

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
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    auto properties = communicator->getProperties();
    string managerProxy = properties->getProperty("IceStormAdmin.TopicManager.Default");
    if(managerProxy.empty())
    {
        ostringstream os;
        os << argv[0] << ": property `IceStormAdmin.TopicManager.Default' is not set";
        throw invalid_argument(os.str());
    }

    auto manager = checkedCast<IceStorm::TopicManagerPrx>(
        communicator->stringToProxy(managerProxy));
    if(!manager)
    {
        ostringstream os;
        os << argv[0] << ": `" << managerProxy << "' is not running";
        throw invalid_argument(os.str());
    }

    auto topic = manager->retrieve("single");
    assert(topic);

    //
    // Get a publisher object, create a twoway proxy and then cast to
    // a Single object.
    //
    auto single = uncheckedCast<SinglePrx>(topic->getPublisher()->ice_twoway());
    for(int i = 0; i < 1000; ++i)
    {
        single->event(i);
    }
}

DEFINE_TEST(Publisher)
