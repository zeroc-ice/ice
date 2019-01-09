// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Glacier2/PermissionsVerifier.h>
#include <SessionI.h>

using namespace std;
using namespace Ice;
using namespace Test;

class SessionControlServer : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
SessionControlServer::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    communicator->getProperties()->setProperty("SessionControlAdapter.Endpoints", getTestEndpoint());
    ObjectAdapterPtr adapter = communicator->createObjectAdapter("SessionControlAdapter");
    adapter->add(new SessionManagerI, Ice::stringToIdentity("SessionManager"));
    adapter->activate();
    communicator->waitForShutdown();
}

DEFINE_TEST(SessionControlServer)
