// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "TestHelper.h"
#include "TestI.h"

using namespace std;

class Server : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Server::run(int argc, char** argv)
{
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    //
    // Its possible to have batch oneway requests dispatched after
    // the adapter is deactivated due to thread scheduling so we
    // suppress this warning.
    //
    properties->setProperty("Ice.Warn.Dispatch", "0");

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(make_shared<MyDerivedClassI>(), Ice::stringToIdentity("test"));
    adapter->activate();
    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
