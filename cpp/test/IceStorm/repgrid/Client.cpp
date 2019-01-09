// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>
#include <Single.h>
#include <TestHelper.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Test;

class SingleI : public Single, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    SingleI() :
        _count(0)
    {
    }

    virtual void
    event(int, const Current&)
    {
        Lock sync(*this);
        if(++_count == 1000)
        {
            notify();
        }
    }

    virtual void
    waitForEvents()
    {
        Lock sync(*this);
        IceUtil::Time timeout = IceUtil::Time::seconds(20);
        while(_count < 1000)
        {
            if(!timedWait(timeout))
            {
                test(false);
            }
        }
    }

private:

    int _count;
};
typedef IceUtil::Handle<SingleI> SingleIPtr;

class Client : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Client::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    ObjectPrx base = communicator->stringToProxy("Test.IceStorm/TopicManager");
    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(base);
    if(!manager)
    {
        ostringstream os;
        os << argv[0] << ": `Test.IceStorm/TopicManager' is not running";
        throw invalid_argument(os.str());
    }

    ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("SingleAdapter", "default:udp");

    TopicPrx topic = manager->create("single");

    //
    // Create subscribers with different QoS.
    //
    SingleIPtr sub = new SingleI;
    topic->subscribeAndGetPublisher(IceStorm::QoS(), adapter->addWithUUID(sub));

    adapter->activate();

    // Ensure that getPublisher & getNonReplicatedPublisher work
    // correctly.
    Ice::ObjectPrx p1 = topic->getPublisher();
    Ice::ObjectPrx p2 = topic->getNonReplicatedPublisher();
    test(p1->ice_getAdapterId() == "PublishReplicaGroup");
    test(p2->ice_getAdapterId() == "Test.IceStorm1.Publish" ||
         p2->ice_getAdapterId() == "Test.IceStorm2.Publish" ||
         p2->ice_getAdapterId() == "Test.IceStorm3.Publish");

    //
    // Get a publisher object, create a twoway proxy and then cast to
    // a Single object.
    //
    SinglePrx single = SinglePrx::uncheckedCast(topic->getPublisher()->ice_twoway());
    for(int i = 0; i < 1000; ++i)
    {
        single->event(i);
    }

    sub->waitForEvents();
}

DEFINE_TEST(Client)
