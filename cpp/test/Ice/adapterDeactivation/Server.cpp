// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <ServantLocatorI.h>
#include <TestHelper.h>

using namespace std;
using namespace Ice;

class Server : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Server::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    ServantLocatorPtr locator = ICE_MAKE_SHARED(ServantLocatorI);
    adapter->addServantLocator(locator, "");
    adapter->activate();
    serverReady();
    adapter->waitForDeactivate();
}

DEFINE_TEST(Server)
