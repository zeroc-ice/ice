// Copyright (c) ZeroC, Inc.

#include "Glacier2/PermissionsVerifier.h"
#include "Ice/Ice.h"
#include "TestHelper.h"

using namespace std;

class AdminPermissionsVerifierI final : public Glacier2::PermissionsVerifier
{
public:
    bool checkPermissions(string userId, string passwd, string&, const Ice::Current& c) const override
    {
        if (c.ctx.find("throw") != c.ctx.end())
        {
            throw Glacier2::PermissionDeniedException("reason");
        }
        return (userId == "admin1" && passwd == "test1") || (userId == "admin2" && passwd == "test2") ||
               (userId == "admin3" && passwd == "test3");
    }
};

class PermissionsVerifierServer final : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
PermissionsVerifierServer::run(int argc, char** argv)
{
    shutdownOnInterrupt();
    auto properties = createTestProperties(argc, argv);
    properties->parseCommandLineOptions("", Ice::argsToStringSeq(argc, argv));
    Ice::CommunicatorHolder communicatorHolder = initialize(argc, argv, properties);
    auto adapter = communicatorHolder->createObjectAdapter("PermissionsVerifier");
    adapter->add(make_shared<AdminPermissionsVerifierI>(), Ice::stringToIdentity("AdminPermissionsVerifier"));
    adapter->activate();
    communicatorHolder->waitForShutdown();
}

DEFINE_TEST(PermissionsVerifierServer)
