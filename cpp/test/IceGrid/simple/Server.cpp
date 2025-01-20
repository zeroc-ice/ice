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

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    string id = communicator->getProperties()->getPropertyWithDefault("Identity", "test");
    adapter->add(std::make_shared<TestI>(), Ice::stringToIdentity(id));

    try
    {
        adapter->activate();
    }
    catch (const Ice::ObjectAdapterDeactivatedException&)
    {
    }
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
