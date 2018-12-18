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
    properties->setProperty("Ice.Admin.Endpoints", "tcp");
    properties->setProperty("Ice.Admin.InstanceName", "server");
    properties->setProperty("Ice.Warn.Connections", "0");
    properties->setProperty("Ice.Warn.Dispatch", "0");
    properties->setProperty("Ice.MessageSizeMax", "50000");
    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);

    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(ICE_MAKE_SHARED(MetricsI), Ice::stringToIdentity("metrics"));
    adapter->activate();

    communicator->getProperties()->setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
    Ice::ObjectAdapterPtr controllerAdapter = communicator->createObjectAdapter("ControllerAdapter");
    controllerAdapter->add(ICE_MAKE_SHARED(ControllerI, adapter), Ice::stringToIdentity("controller"));
    controllerAdapter->activate();

    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
