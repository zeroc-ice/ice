// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "SessionI.h"
#include "TestHelper.h"

using namespace std;
using namespace Ice;
using namespace Test;

class SessionControlServer final : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
SessionControlServer::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    communicator->getProperties()->setProperty("SessionControlAdapter.Endpoints", getTestEndpoint());
    auto adapter = communicator->createObjectAdapter("SessionControlAdapter");
    adapter->add(make_shared<SessionManagerI>(), Ice::stringToIdentity("SessionManager"));
    adapter->activate();
    communicator->waitForShutdown();
}

DEFINE_TEST(SessionControlServer)
