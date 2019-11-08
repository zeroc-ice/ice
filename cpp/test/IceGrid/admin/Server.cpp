//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>

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
    auto adapter = communicator->createObjectAdapter("TestAdapter");
    try
    {
        adapter->activate();
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
    }
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
