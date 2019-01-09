// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <TestI.h>

using namespace std;

class Collocated : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Collocated::run(int argc, char** argv)
{
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.Warn.AMICallback", "0");
    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    communicator->getProperties()->setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
    communicator->getProperties()->setProperty("ControllerAdapter.ThreadPool.Size", "1");

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectAdapterPtr adapter2 = communicator->createObjectAdapter("ControllerAdapter");

    TestIntfControllerIPtr testController = ICE_MAKE_SHARED(TestIntfControllerI, adapter);

    adapter->add(ICE_MAKE_SHARED(TestIntfI), Ice::stringToIdentity("test"));
    adapter->add(ICE_MAKE_SHARED(TestIntfII), Ice::stringToIdentity("test2"));
    //adapter->activate(); // Collocated test doesn't need to activate the OA

    adapter2->add(testController, Ice::stringToIdentity("testController"));
    //adapter2->activate(); // Collocated test doesn't need to activate the OA

    void allTests(Test::TestHelper*, bool);
    allTests(this, true);
}

DEFINE_TEST(Collocated)
