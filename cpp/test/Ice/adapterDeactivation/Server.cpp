// Copyright (c) ZeroC, Inc.

#include "ServantLocatorI.h"
#include "TestHelper.h"

using namespace std;
using namespace Ice;

class Server : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Server::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    ServantLocatorPtr locator = make_shared<ServantLocatorI>();
    adapter->addServantLocator(locator, "");
    adapter->activate();
    serverReady();
    adapter->waitForDeactivate();
}

DEFINE_TEST(Server)
