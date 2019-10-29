//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <TestI.h>

using namespace std;

class Server final: public Test::TestHelper
{
public:

    void run(int, char**) override;
};

void
Server::run(int argc, char** argv)
{
    auto properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.Warn.Dispatch", "0");

    // Make sure requests are serialized, this is required for the ordering test.
    properties->setProperty("Ice.ThreadPool.Server.Serialize", "1");

    Ice::CommunicatorHolder communicatorHolder = initialize(argc, argv, properties);
    communicatorHolder->getProperties()->setProperty("TestAdapter.Endpoints",
                                                     getTestEndpoint() + ":" +
                                                     getTestEndpoint("udp"));
    auto adapter = communicatorHolder->createObjectAdapter("TestAdapter");
    adapter->add(make_shared<MyClassI>(), Ice::stringToIdentity("test"));
    adapter->activate();
    serverReady();
    communicatorHolder->waitForShutdown();
}

DEFINE_TEST(Server)
