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
    // We configure a low idle timeout to make sure we send heartbeats frequently.
    initData.properties->setProperty("Ice.Connection.IdleTimeout", "1");
    initData.properties->setProperty("TestAdapter.Connection.InactivityTimeout", "2");
    initData.properties->setProperty("TestAdapter1s.Connection.InactivityTimeout", "1");
    // Message size large enough to receive the 10MB payload.
    initData.properties->setProperty("Ice.MessageSizeMax", "20000");
    Ice::CommunicatorHolder communicator = initialize(argc, argv, initData);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    communicator->getProperties()->setProperty("TestAdapter1s.Endpoints", getTestEndpoint(1));
    communicator->getProperties()->setProperty("ControllerAdapter.Endpoints", getTestEndpoint(2));

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(std::make_shared<TestIntfI>(), Ice::stringToIdentity("test"));
    adapter->activate();

    Ice::ObjectAdapterPtr adapter1s = communicator->createObjectAdapter("TestAdapter1s");
    adapter1s->add(std::make_shared<TestIntfI>(), Ice::stringToIdentity("test"));
    adapter1s->activate();

    Ice::ObjectAdapterPtr controllerAdapter = communicator->createObjectAdapter("ControllerAdapter");
    controllerAdapter->add(std::make_shared<TestIntfControllerI>(adapter), Ice::stringToIdentity("testController"));
    controllerAdapter->activate();

    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
