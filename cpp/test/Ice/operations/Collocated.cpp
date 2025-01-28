// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "TestHelper.h"
#include "TestI.h"

#include <iostream>

using namespace std;

class Collocated : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

namespace
{

    void testCollocatedIPv6Invocation()
    {
        cout << "testing collocated invocation with normalized IPv6 address... " << flush;
        Ice::CommunicatorHolder communicator = Ice::initialize();
        communicator->getProperties()->setProperty("TestAdapter.Endpoints", "tcp -h \"0:0:0:0:0:0:0:1\" -p 10000");
        Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
        adapter->add(std::make_shared<MyDerivedClassI>(), Ice::stringToIdentity("test"));

        auto prx = Ice::ObjectPrx(communicator.communicator(), "test:tcp -h \"::1\" -p 10000");
        prx = prx->ice_invocationTimeout(10ms);
        prx->ice_ping();

        prx = Ice::ObjectPrx(communicator.communicator(), "test:tcp -h \"0:0:0:0:0:0:0:1\" -p 10000");
        prx = prx->ice_invocationTimeout(10ms);
        prx->ice_ping();
        cout << "ok" << endl;
    }
}

void
Collocated::run(int argc, char** argv)
{
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.BatchAutoFlushSize", "100");
    {
        Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
        communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
        communicator->getProperties()->setProperty("TestAdapter.AdapterId", "test");
        Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
        Ice::ObjectPrx prx = adapter->add(std::make_shared<MyDerivedClassI>(), Ice::stringToIdentity("test"));
        // adapter->activate(); // Don't activate OA to ensure collocation is used.

        test(!prx->ice_getConnection());

        Test::MyClassPrx allTests(Test::TestHelper*);
        allTests(this);
    }
    testCollocatedIPv6Invocation();
}

DEFINE_TEST(Collocated)
