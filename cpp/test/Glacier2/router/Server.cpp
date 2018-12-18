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
#include <CallbackI.h>

using namespace std;
using namespace Ice;
using namespace Test;

class CallbackServer : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
CallbackServer::run(int argc, char** argv)
{
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.Warn.Connections", "0");
    properties->setProperty("Ice.Warn.Dispatch", "0");
    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    communicator->getProperties()->setProperty("CallbackAdapter.Endpoints", getTestEndpoint());
    ObjectAdapterPtr adapter = communicator->createObjectAdapter("CallbackAdapter");
    adapter->add(new CallbackI(), Ice::stringToIdentity("c1/callback")); // The test allows "c1" as category.
    adapter->add(new CallbackI(), Ice::stringToIdentity("c2/callback")); // The test allows "c2" as category.
    adapter->add(new CallbackI(), Ice::stringToIdentity("c3/callback")); // The test rejects "c3" as category.
    adapter->add(new CallbackI(), Ice::stringToIdentity("_userid/callback")); // The test allows the prefixed userid.
    adapter->activate();
    communicator->waitForShutdown();
}

DEFINE_TEST(CallbackServer)
