//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

    void
    event(int, const Current&) override
    {
        lock_guard<mutex> lg(_mutex);
        if(++_count == 1000)
        {
            _condVar.notify_one();
        }
    }

    void
    waitForEvents()
    {
        unique_lock<mutex> lock(_mutex);
        while(_count < 1000)
        {
            if(_condVar.wait_for(lock, 20s) == cv_status::timeout)
            {
                test(false);
            }
        }
    }

private:

    int _count = 0;
    mutex _mutex;
    condition_variable _condVar;
};

class Client final : public Test::TestHelper
{
public:

    void run(int, char**) override;
};

void
Client::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    auto base = communicator->stringToProxy("Test.IceStorm/TopicManager");
    auto manager = checkedCast<IceStorm::TopicManagerPrx>(base);
    if(!manager)
    {
        ostringstream os;
        os << argv[0] << ": `Test.IceStorm/TopicManager' is not running";
        throw invalid_argument(os.str());
    }

    auto adapter = communicator->createObjectAdapterWithEndpoints("SingleAdapter", "default:udp");

    auto topic = manager->create("single");

    //
    // Create subscribers with different QoS.
    //
    auto sub = make_shared<SingleI>();
    topic->subscribeAndGetPublisher(IceStorm::QoS(), adapter->addWithUUID(sub));

    adapter->activate();

    // Ensure that getPublisher & getNonReplicatedPublisher work
    // correctly.
    auto p1 = topic->getPublisher();
    auto p2 = topic->getNonReplicatedPublisher();
    test(p1->ice_getAdapterId() == "PublishReplicaGroup");
    test(p2->ice_getAdapterId() == "Test.IceStorm1.Publish" ||
         p2->ice_getAdapterId() == "Test.IceStorm2.Publish" ||
         p2->ice_getAdapterId() == "Test.IceStorm3.Publish");

    //
    // Get a publisher object, create a twoway proxy and then cast to
    // a Single object.
    //
    auto single = uncheckedCast<SinglePrx>(topic->getPublisher()->ice_twoway());
    for(int i = 0; i < 1000; ++i)
    {
        single->event(i);
    }

    sub->waitForEvents();
}

DEFINE_TEST(Client)
