// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
    properties->setProperty("Ice.ACM.Timeout", "1");
    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    communicator->getProperties()->setProperty("TestAdapter.ACM.Timeout", "0");
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::Identity id = Ice::stringToIdentity("communicator");
    adapter->add(ICE_MAKE_SHARED(RemoteCommunicatorI), id);
    adapter->activate();

    serverReady();

    // Disable ready print for further adapters.
    communicator->getProperties()->setProperty("Ice.PrintAdapterReady", "0");

    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
