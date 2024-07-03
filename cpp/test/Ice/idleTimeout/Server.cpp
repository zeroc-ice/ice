//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "TestHelper.h"
#include "TestI.h"

using namespace std;

class Server : public Test::TestHelper
{
public:
    void run(int, char**);
};

void
Server::run(int argc, char** argv)
{
    Ice::InitializationData initData;
    initData.properties = createTestProperties(argc, argv);
    initData.properties->setProperty("Ice.Warn.Connections", "0");
    initData.properties->setProperty("TestAdapter.Connection.IdleTimeout", "1"); // 1 second
    initData.properties->setProperty("TestAdapter.ThreadPool.Size", "1"); // dedicated thread pool with a single thread

    Ice::CommunicatorHolder communicator = initialize(argc, argv, initData);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(std::make_shared<TestIntfI>(), Ice::stringToIdentity("test"));
    adapter->activate();

    communicator->getProperties()->setProperty("TestAdapter3s.Endpoints", getTestEndpoint(1));
    communicator->getProperties()->setProperty("TestAdapter3s.Connection.IdleTimeout", "3");
    adapter = communicator->createObjectAdapter("TestAdapter3s");
    adapter->add(std::make_shared<TestIntfI>(), Ice::stringToIdentity("test"));
    adapter->activate();

    // Used by the JavaScript tests
    communicator->getProperties()->setProperty("TestAdapterNoIdleCheck.Endpoints", getTestEndpoint(2));
    communicator->getProperties()->setProperty("TestAdapterNoIdleCheck.Connection.IdleTimeout", "1");
    communicator->getProperties()->setProperty("TestAdapterNoIdleCheck.Connection.EnableIdleCheck", "0");
    adapter = communicator->createObjectAdapter("TestAdapterNoIdleCheck");
    adapter->add(std::make_shared<TestIntfI>(), Ice::stringToIdentity("test"));
    adapter->add(std::make_shared<TestIntfBidirI>(), Ice::stringToIdentity("test-bidir"));
    adapter->activate();

    // Used by the JavaScript tests
    communicator->getProperties()->setProperty("TestAdapterNoIdleCheck3s.Endpoints", getTestEndpoint(3));
    communicator->getProperties()->setProperty("TestAdapterNoIdleCheck3s.Connection.IdleTimeout", "3");
    communicator->getProperties()->setProperty("TestAdapterNoIdleCheck3s.Connection.EnableIdleCheck", "0");
    adapter = communicator->createObjectAdapter("TestAdapterNoIdleCheck3s");
    adapter->add(std::make_shared<TestIntfI>(), Ice::stringToIdentity("test"));
    adapter->add(std::make_shared<TestIntfBidirI>(), Ice::stringToIdentity("test-bidir"));
    adapter->activate();

    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
