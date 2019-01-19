//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <ServantLocatorI.h>
#include <TestHelper.h>

using namespace std;
using namespace Ice;

class Server : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Server::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    ServantLocatorPtr locator = ICE_MAKE_SHARED(ServantLocatorI);
    adapter->addServantLocator(locator, "");
    adapter->activate();
    serverReady();
    adapter->waitForDeactivate();
}

DEFINE_TEST(Server)
