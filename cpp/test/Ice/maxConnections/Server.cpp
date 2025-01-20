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
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    // Plain adapter with no limit.
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(std::make_shared<TestIntfI>(), Ice::stringToIdentity("test"));
    adapter->activate();

    // Accepts 10 connections.
    communicator->getProperties()->setProperty("TestAdapterMax10.Endpoints", getTestEndpoint(1));
    communicator->getProperties()->setProperty("TestAdapterMax10.MaxConnections", "10");
    auto adapterMax10 = communicator->createObjectAdapter("TestAdapterMax10");
    adapterMax10->add(std::make_shared<TestIntfI>(), Ice::stringToIdentity("test"));
    adapterMax10->activate();

    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
