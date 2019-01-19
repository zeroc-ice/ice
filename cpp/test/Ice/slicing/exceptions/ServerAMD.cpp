//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestAMDI.h>
#include <TestHelper.h>

using namespace std;

class ServerAMD : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
ServerAMD::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    Ice::PropertiesPtr properties = communicator->getProperties();
    properties->setProperty("Ice.Warn.Dispatch", "0");
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint() + " -t 2000");
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(ICE_MAKE_SHARED(TestI), Ice::stringToIdentity("Test"));
    adapter->activate();
    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(ServerAMD)
