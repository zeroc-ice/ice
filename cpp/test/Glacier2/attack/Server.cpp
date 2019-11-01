//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <BackendI.h>

using namespace std;
using namespace Ice;
using namespace Test;

class BackendServer final : public Test::TestHelper
{
public:

    void run(int, char**) override;
};

void
BackendServer::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    communicator->getProperties()->setProperty("BackendAdapter.Endpoints", getTestEndpoint());
    auto adapter = communicator->createObjectAdapter("BackendAdapter");
    adapter->addDefaultServant(make_shared<BackendI>(), "");
    adapter->activate();
    communicator->waitForShutdown();
}

DEFINE_TEST(BackendServer)
