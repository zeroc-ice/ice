// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

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
    properties->setProperty("Ice.Warn.Dispatch", "0");
    properties->setProperty("Ice.Warn.Connections", "0");
    properties->setProperty("Ice.MessageSizeMax", "10"); // 10KB max

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
    communicator->getProperties()->setProperty("TestAdapter2.Endpoints", getTestEndpoint(1));
    communicator->getProperties()->setProperty("TestAdapter2.MessageSizeMax", "0");
    communicator->getProperties()->setProperty("TestAdapter3.Endpoints", getTestEndpoint(2));
    communicator->getProperties()->setProperty("TestAdapter3.MessageSizeMax", "1");
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectAdapterPtr adapter2 = communicator->createObjectAdapter("TestAdapter2");
    Ice::ObjectAdapterPtr adapter3 = communicator->createObjectAdapter("TestAdapter3");
    Ice::ObjectPtr object = ICE_MAKE_SHARED(ThrowerI);
    adapter->add(object, Ice::stringToIdentity("thrower"));
    adapter2->add(object, Ice::stringToIdentity("thrower"));
    adapter3->add(object, Ice::stringToIdentity("thrower"));
    adapter->activate();
    adapter2->activate();
    adapter3->activate();
    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(ServerAMD)
