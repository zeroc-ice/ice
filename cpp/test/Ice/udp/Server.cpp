// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
    properties->setProperty("Ice.Warn.Connections", "0");
    properties->setProperty("Ice.UDP.SndSize", "16384");
    properties->setProperty("Ice.UDP.RcvSize", "16384");

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);

    int num = argc == 2 ? atoi(argv[1]) : 0;

    communicator->getProperties()->setProperty("ControlAdapter.Endpoints", getTestEndpoint(num, "tcp"));
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("ControlAdapter");
    adapter->add(ICE_MAKE_SHARED(TestIntfI), Ice::stringToIdentity("control"));
    adapter->activate();

    if(num == 0)
    {
        communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint(num, "udp"));
        Ice::ObjectAdapterPtr adapter2 = communicator->createObjectAdapter("TestAdapter");
        adapter2->add(ICE_MAKE_SHARED(TestIntfI), Ice::stringToIdentity("test"));
        adapter2->activate();
    }

    ostringstream endpoint;
    if(communicator->getProperties()->getProperty("Ice.IPv6") == "1")
    {
        endpoint << "udp -h \"ff15::1:1\" -p " << getTestPort(10);
#if defined(__APPLE__) || defined(_WIN32)
        endpoint << " --interface \"::1\""; // Use loopback to prevent other machines to answer.
#endif
    }
    else
    {
        endpoint << "udp -h 239.255.1.1 -p " << getTestPort(10);
#if defined(__APPLE__) || defined(_WIN32)
        endpoint << " --interface 127.0.0.1"; // Use loopback to prevent other machines to answer.
#endif
    }
    communicator->getProperties()->setProperty("McastTestAdapter.Endpoints", endpoint.str());
    Ice::ObjectAdapterPtr mcastAdapter = communicator->createObjectAdapter("McastTestAdapter");
    mcastAdapter->add(ICE_MAKE_SHARED(TestIntfI), Ice::stringToIdentity("test"));
    mcastAdapter->activate();

    serverReady();

    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
