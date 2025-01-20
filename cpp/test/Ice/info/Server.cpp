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
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    communicator->getProperties()->setProperty(
        "TestAdapter.Endpoints",
        getTestEndpoint() + ":" + getTestEndpoint("udp"));
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(std::make_shared<TestI>(), Ice::stringToIdentity("test"));
    adapter->activate();
    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
