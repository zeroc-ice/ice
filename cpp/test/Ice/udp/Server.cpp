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
    properties->setProperty("Ice.Warn.Connections", "0");
    properties->setProperty("Ice.UDP.SndSize", "16384");
    properties->setProperty("Ice.UDP.RcvSize", "16384");

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);

    int num = argc == 2 ? stoi(argv[1]) : 0;

    communicator->getProperties()->setProperty("ControlAdapter.Endpoints", getTestEndpoint(num, "tcp"));
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("ControlAdapter");
    adapter->add(std::make_shared<TestIntfI>(), Ice::stringToIdentity("control"));
    adapter->activate();

    if (num == 0)
    {
        communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint(num, "udp"));
        Ice::ObjectAdapterPtr adapter2 = communicator->createObjectAdapter("TestAdapter");
        adapter2->add(std::make_shared<TestIntfI>(), Ice::stringToIdentity("test"));
        adapter2->activate();
    }

    ostringstream endpoint;
    if (communicator->getProperties()->getIceProperty("Ice.IPv6") == "1")
    {
        endpoint << "udp -h \"ff15::1:1\" -p " << getTestPort(10);
    }
    else
    {
        endpoint << "udp -h 239.255.1.1 -p " << getTestPort(10);
    }
    communicator->getProperties()->setProperty("McastTestAdapter.Endpoints", endpoint.str());

    Ice::ObjectAdapterPtr mcastAdapter = communicator->createObjectAdapter("McastTestAdapter");
    mcastAdapter->add(std::make_shared<TestIntfI>(), Ice::stringToIdentity("test"));
    mcastAdapter->activate();

    serverReady();

    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
