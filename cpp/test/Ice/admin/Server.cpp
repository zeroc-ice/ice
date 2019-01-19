//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestI.h>
#include <TestHelper.h>

using namespace std;

class Server : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Server::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint() + " -t 10000");
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::Identity id = Ice::stringToIdentity("factory");
    adapter->add(ICE_MAKE_SHARED(RemoteCommunicatorFactoryI), id);
    adapter->activate();
    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
