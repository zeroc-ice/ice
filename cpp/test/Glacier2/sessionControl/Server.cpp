//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Glacier2/PermissionsVerifier.h>
#include <SessionI.h>

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
