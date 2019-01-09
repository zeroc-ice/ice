// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <TestI.h>

using namespace std;

class Server : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Server::run(int argc, char** argv)
{
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    //
    // This test kills connections, so we don't want warnings.
    //
    properties->setProperty("Ice.Warn.Connections", "0");

    //
    // Limit the recv buffer size, this test relies on the socket
    // send() blocking after sending a given amount of data.
    //
    properties->setProperty("Ice.TCP.RcvSize", "50000");

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);

    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    communicator->getProperties()->setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
    communicator->getProperties()->setProperty("ControllerAdapter.ThreadPool.Size", "1");

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectAdapterPtr adapter2 = communicator->createObjectAdapter("ControllerAdapter");

    TestIntfControllerIPtr testController = ICE_MAKE_SHARED(TestIntfControllerI, adapter);

    adapter->add(ICE_MAKE_SHARED(TestIntfI), Ice::stringToIdentity("test"));
    adapter->add(ICE_MAKE_SHARED(TestIntfII), Ice::stringToIdentity("test2"));
    adapter->activate();

    adapter2->add(testController, Ice::stringToIdentity("testController"));
    adapter2->activate();

    serverReady();

    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
