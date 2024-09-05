//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
    Ice::CommunicatorHolder communicatorHolder = initialize(argc, argv);
    communicatorHolder->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    communicatorHolder->getProperties()->setProperty("TestAdapter.Connection.MaxDispatches", "1");
    auto adapter = communicatorHolder->createObjectAdapter("TestAdapter");
    adapter->add(make_shared<MyClassI>(), Ice::stringToIdentity("test"));
    adapter->activate();
    serverReady();
    communicatorHolder->waitForShutdown();
}

DEFINE_TEST(Server)
