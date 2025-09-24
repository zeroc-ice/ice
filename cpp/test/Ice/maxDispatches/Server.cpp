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
    initData.properties->setProperty("Ice.ThreadPool.Server.Size", "10"); // plenty of threads to handle the requests
    Ice::CommunicatorHolder communicator = initialize(initData);

    auto responder = make_shared<ResponderI>();
    auto testIntf = make_shared<TestIntfI>(responder);

    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(testIntf, Ice::stringToIdentity("test"));
    adapter->activate();

    communicator->getProperties()->setProperty("ResponderAdapter.Endpoints", getTestEndpoint(1));
    adapter = communicator->createObjectAdapter("ResponderAdapter");
    adapter->add(responder, Ice::stringToIdentity("responder"));
    adapter->activate();

    communicator->getProperties()->setProperty("TestAdapterMax10.Endpoints", getTestEndpoint(2));
    communicator->getProperties()->setProperty("TestAdapterMax10.Connection.MaxDispatches", "10");
    adapter = communicator->createObjectAdapter("TestAdapterMax10");
    adapter->add(testIntf, Ice::stringToIdentity("test"));
    adapter->activate();

    communicator->getProperties()->setProperty("TestAdapterMax1.Endpoints", getTestEndpoint(3));
    communicator->getProperties()->setProperty("TestAdapterMax1.Connection.MaxDispatches", "1");
    adapter = communicator->createObjectAdapter("TestAdapterMax1");
    adapter->add(testIntf, Ice::stringToIdentity("test"));
    adapter->activate();

    communicator->getProperties()->setProperty("TestAdapterSerialize.Endpoints", getTestEndpoint(4));
    communicator->getProperties()->setProperty("TestAdapterSerialize.ThreadPool.Size", "10");
    communicator->getProperties()->setProperty("TestAdapterSerialize.ThreadPool.Serialize", "1");
    adapter = communicator->createObjectAdapter("TestAdapterSerialize");
    adapter->add(testIntf, Ice::stringToIdentity("test"));
    adapter->activate();

    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
