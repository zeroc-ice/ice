// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>
#include <Single.h>
#include <TestCommon.h>

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

int
run(int, char* argv[], const CommunicatorPtr& communicator)
{
    ObjectPrx base = communicator->stringToProxy("Test.IceStorm/TopicManager");
    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(base);
    if(!manager)
    {
        cerr << argv[0] << ": `Test.IceStorm/TopicManager' is not running" << endl;
        return EXIT_FAILURE;
    }

    ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("SingleAdapter", "default:udp");

    TopicPrx topic;
    try
    {
        topic = manager->create("single");
    }
    catch(const IceStorm::TopicExists& e)
    {
        cerr << argv[0] << ": TopicExists: " << e.name << endl;
        return EXIT_FAILURE;
    }

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

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    CommunicatorPtr communicator;

    try
    {
        communicator = initialize(argc, argv);
        status = run(argc, argv, communicator);
    }
    catch(const Exception& ex)
    {
        cerr << ex << endl;
        status = EXIT_FAILURE;
    }

    if(communicator)
    {
        try
        {
            communicator->destroy();
        }
        catch(const Exception& ex)
        {
            cerr << ex << endl;
            status = EXIT_FAILURE;
        }
    }

    return status;
}
