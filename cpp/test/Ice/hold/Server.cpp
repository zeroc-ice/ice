// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
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
    initData.properties->setProperty("Ice.Connection.Server.IdleTimeout", "1");

    Ice::CommunicatorHolder communicator = initialize(initData);
    auto properties = communicator->getProperties();

    properties->setProperty("TestAdapter1.Endpoints", getTestEndpoint());
    properties->setProperty("TestAdapter1.ThreadPool.Size", "5");
    properties->setProperty("TestAdapter1.ThreadPool.SizeMax", "5");
    properties->setProperty("TestAdapter1.ThreadPool.SizeWarn", "0");
    properties->setProperty("TestAdapter1.ThreadPool.Serialize", "0");

    Ice::ObjectAdapterPtr adapter1 = communicator->createObjectAdapter("TestAdapter1");
    adapter1->add(make_shared<HoldI>(adapter1), Ice::stringToIdentity("hold"));

    properties->setProperty("TestAdapter2.Endpoints", getTestEndpoint(1));
    properties->setProperty("TestAdapter2.ThreadPool.Size", "5");
    properties->setProperty("TestAdapter2.ThreadPool.SizeMax", "5");
    properties->setProperty("TestAdapter2.ThreadPool.SizeWarn", "0");
    properties->setProperty("TestAdapter2.ThreadPool.Serialize", "1");
    Ice::ObjectAdapterPtr adapter2 = communicator->createObjectAdapter("TestAdapter2");
    adapter2->add(make_shared<HoldI>(adapter2), Ice::stringToIdentity("hold"));

    adapter1->activate();
    adapter2->activate();

    serverReady();

    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
