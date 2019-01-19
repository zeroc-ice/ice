//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <TestAMDI.h>

using namespace std;

class ServerAMD : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
ServerAMD::run(int argc, char** argv)
{
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.Warn.Connections", "0");
    properties->setProperty("Ice.Warn.Dispatch", "0");

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(ICE_MAKE_SHARED(MyDerivedClassI), Ice::stringToIdentity("test"));
    adapter->activate();
    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(ServerAMD)
