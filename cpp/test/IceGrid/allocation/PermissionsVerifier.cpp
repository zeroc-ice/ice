//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <Glacier2/PermissionsVerifier.h>
#include <TestHelper.h>

using namespace std;

class PermissionsVerifierI final : public Glacier2::PermissionsVerifier
{
public:

    bool
    checkPermissions(string, string, string&, const Ice::Current&) const override
    {
        return true;
    }
};

class Server : public Test::TestHelper
{
public:

    void run(int argc, char** argv) override
    {
        Ice::CommunicatorHolder communicator = initialize(argc, argv);
        auto adapter = communicator->createObjectAdapter("PermissionsVerifier");
        adapter->add(make_shared<PermissionsVerifierI>(), Ice::stringToIdentity("PermissionsVerifier"));
        adapter->activate();
        communicator->waitForShutdown();
    }
};

DEFINE_TEST(Server)
