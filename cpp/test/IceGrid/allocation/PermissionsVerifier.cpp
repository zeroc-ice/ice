//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <Glacier2/PermissionsVerifier.h>
#include <TestHelper.h>

using namespace std;

class PermissionsVerifierI : public Glacier2::PermissionsVerifier
{
public:

    virtual bool
    checkPermissions(const string&, const string&, string&, const Ice::Current&) const
    {
        return true;
    }
};

class Server : public Test::TestHelper
{
public:

    virtual void run(int argc, char** argv)
    {
        Ice::CommunicatorHolder communicator = initialize(argc, argv);
        Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("PermissionsVerifier");
        adapter->add(new PermissionsVerifierI, Ice::stringToIdentity("PermissionsVerifier"));
        adapter->activate();
        communicator->waitForShutdown();
    }
};

DEFINE_TEST(Server)
