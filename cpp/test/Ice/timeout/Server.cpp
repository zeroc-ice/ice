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

    // The client sends large messages to cause the transport buffers to fill up.
    properties->setProperty("Ice.MessageSizeMax", "20000");

    // Limit the recv buffer size, this test relies on the socket send() blocking after sending a given amount of data.
    properties->setProperty("Ice.TCP.RcvSize", "50000");

#ifdef _WIN32
    // Turn off stack trace collection and printing on Windows with ws(s): they slow down the logging so much that this
    // test can fail. See #3048.
    if (getTestProtocol(properties).find("ws") == 0)
    {
        properties->setProperty("Ice.PrintStackTraces", "0");
    }
#endif

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);

    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    communicator->getProperties()->setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
    communicator->getProperties()->setProperty("ControllerAdapter.ThreadPool.Size", "1");

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(make_shared<TimeoutI>(), Ice::stringToIdentity("timeout"));
    adapter->activate();

    Ice::ObjectAdapterPtr controllerAdapter = communicator->createObjectAdapter("ControllerAdapter");
    controllerAdapter->add(make_shared<ControllerI>(adapter), Ice::stringToIdentity("controller"));
    controllerAdapter->activate();

    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
