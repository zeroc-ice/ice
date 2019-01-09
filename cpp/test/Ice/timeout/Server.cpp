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

#if TARGET_OS_IPHONE != 0
    //
    // COMPILERFIX: Disable connect timeout introduced for
    // workaround to iOS device hangs when using SSL
    //
    properties->setProperty("Ice.Override.ConnectTimeout", "");
#endif

    //
    // This test kills connections, so we don't want warnings.
    //
    properties->setProperty("Ice.Warn.Connections", "0");

    //
    // The client sends large messages to cause the transport
    // buffers to fill up.
    //
    properties->setProperty("Ice.MessageSizeMax", "20000");

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
    adapter->add(ICE_MAKE_SHARED(TimeoutI), Ice::stringToIdentity("timeout"));
    adapter->activate();

    Ice::ObjectAdapterPtr controllerAdapter = communicator->createObjectAdapter("ControllerAdapter");
    controllerAdapter->add(ICE_MAKE_SHARED(ControllerI, adapter), Ice::stringToIdentity("controller"));
    controllerAdapter->activate();

    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
