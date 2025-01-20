// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "TestHelper.h"
#include "TestI.h"

using namespace std;

class Server final : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Server::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    auto properties = communicator->getProperties();

    string name = properties->getIceProperty("Ice.ProgramName");

    auto adapter = communicator->createObjectAdapter("Server");
    adapter->add(make_shared<TestI>(adapter, properties), Ice::stringToIdentity(name));

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
