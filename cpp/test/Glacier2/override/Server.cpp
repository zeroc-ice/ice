//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <CallbackI.h>

using namespace std;
using namespace Ice;
using namespace Test;

class CallbackServer final : public Test::TestHelper
{
public:

    void run(int, char**) override;
};

void
CallbackServer::run(int argc, char** argv)
{
    auto properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.Warn.Connections", "0");
    properties->setProperty("Ice.Warn.Dispatch", "0");
    properties->setProperty("Ice.ThreadPool.Server.Serialize", "1");

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    communicator->getProperties()->setProperty("CallbackAdapter.Endpoints", getTestEndpoint());
    auto adapter = communicator->createObjectAdapter("CallbackAdapter");
    adapter->add(make_shared<CallbackI>(), Ice::stringToIdentity("c/callback"));
    adapter->activate();
    communicator->waitForShutdown();
}

DEFINE_TEST(CallbackServer)
