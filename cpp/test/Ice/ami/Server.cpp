// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "TestHelper.h"
#include "TestI.h"

#include <future>

using namespace std;

class Server : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Server::run(int argc, char** argv)
{
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);

    //
    // This test kills connections, so we don't want warnings.
    //
    properties->setProperty("Ice.Warn.Connections", "0");

    //
    // Limit the recv buffer size, this test relies on the socket
    // send() blocking after sending a given amount of data.
    //
    properties->setProperty("Ice.TCP.RcvSize", "50000");

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);

    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
    communicator->getProperties()->setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
    communicator->getProperties()->setProperty("ControllerAdapter.ThreadPool.Size", "1");

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectAdapterPtr adapter2 = communicator->createObjectAdapter("ControllerAdapter");

    TestIntfControllerIPtr testController = make_shared<TestIntfControllerI>(adapter);

    adapter->add(make_shared<TestIntfI>(), Ice::stringToIdentity("test"));
    adapter->add(make_shared<TestIntfII>(), Ice::stringToIdentity("test2"));
    adapter->activate();

    adapter2->add(testController, Ice::stringToIdentity("testController"));
    adapter2->activate();

    serverReady();

    promise<void> shutdownCompleted;
    communicator->waitForShutdownAsync([&shutdownCompleted]() { shutdownCompleted.set_value(); });
    shutdownCompleted.get_future().wait();
}

DEFINE_TEST(Server)
