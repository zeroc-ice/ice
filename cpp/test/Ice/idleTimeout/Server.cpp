// Copyright (c) ZeroC, Inc.

#include "TestHelper.h"
#include "TestI.h"

using namespace std;

class Server : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Server::run(int argc, char** argv)
{
    Ice::InitializationData initData;
    initData.properties = createTestProperties(argc, argv);
    initData.properties->setProperty("Ice.Warn.Connections", "0");
    initData.properties->setProperty("TestAdapter.Connection.IdleTimeout", "1"); // 1 second
    // Serialize dispatches on each incoming connection.
    initData.properties->setProperty("TestAdapter.Connection.MaxDispatches", "1");

    Ice::CommunicatorHolder communicator = initialize(argc, argv, initData);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(std::make_shared<TestIntfI>(), Ice::stringToIdentity("test"));
    adapter->activate();

    communicator->getProperties()->setProperty("TestAdapterDefaultMax.Endpoints", getTestEndpoint(1));
    communicator->getProperties()->setProperty("TestAdapterDefaultMax.Connection.IdleTimeout", "1");
    adapter = communicator->createObjectAdapter("TestAdapterDefaultMax");
    adapter->add(std::make_shared<TestIntfI>(), Ice::stringToIdentity("test"));
    adapter->activate();

    communicator->getProperties()->setProperty("TestAdapter3s.Endpoints", getTestEndpoint(2));
    communicator->getProperties()->setProperty("TestAdapter3s.Connection.IdleTimeout", "3");
    adapter = communicator->createObjectAdapter("TestAdapter3s");
    adapter->add(std::make_shared<TestIntfI>(), Ice::stringToIdentity("test"));
    adapter->activate();

    communicator->getProperties()->setProperty("TestAdapterNoIdleTimeout.Endpoints", getTestEndpoint(3));
    communicator->getProperties()->setProperty("TestAdapterNoIdleTimeout.Connection.IdleTimeout", "0");
    adapter = communicator->createObjectAdapter("TestAdapterNoIdleTimeout");
    adapter->add(std::make_shared<TestIntfI>(), Ice::stringToIdentity("test"));
    adapter->activate();

    // Used by the JavaScript tests
    communicator->getProperties()->setProperty("TestAdapterNoIdleCheck.Endpoints", getTestEndpoint(4));
    communicator->getProperties()->setProperty("TestAdapterNoIdleCheck.Connection.IdleTimeout", "1");
    communicator->getProperties()->setProperty("TestAdapterNoIdleCheck.Connection.EnableIdleCheck", "0");
    adapter = communicator->createObjectAdapter("TestAdapterNoIdleCheck");
    adapter->add(std::make_shared<TestIntfI>(), Ice::stringToIdentity("test"));
    adapter->add(std::make_shared<TestIntfBidirI>(), Ice::stringToIdentity("test-bidir"));
    adapter->activate();

    // Used by the JavaScript tests
    communicator->getProperties()->setProperty("TestAdapterNoIdleCheck3s.Endpoints", getTestEndpoint(5));
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
