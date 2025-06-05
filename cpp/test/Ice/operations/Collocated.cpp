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
    void testCollocatedIPv6Invocation(Test::TestHelper* helper)
    {
        int port = helper->getTestPort(1);

        string endpoint1 = "tcp -h \"::1\" -p " + to_string(port);
        string endpoint2 = "tcp -h \"0:0:0:0:0:0:0:1\" -p " + to_string(port);

        cout << "testing collocated invocation with normalized IPv6 address... " << flush;
        Ice::CommunicatorHolder communicator = Ice::initialize();
        communicator->getProperties()->setProperty("TestAdapter.Endpoints", endpoint1);
        Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
        adapter->add(std::make_shared<MyDerivedClassI>(), Ice::stringToIdentity("test"));
        // Don't activate OA to ensure collocation is used.

        auto prx = Ice::ObjectPrx{communicator.communicator(), "test:" + endpoint1};
        prx->ice_ping();

        prx = Ice::ObjectPrx{communicator.communicator(), "test:" + endpoint2};
        prx->ice_ping();
        cout << "ok" << endl;
    }
}

void
Collocated::run(int argc, char** argv)
{
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.BatchAutoFlushSize", "100");
    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectPrx prx = adapter->add(std::make_shared<MyDerivedClassI>(), Ice::stringToIdentity("test"));
    // adapter->activate(); // Don't activate OA to ensure collocation is used.

    test(!prx->ice_getConnection());

    Test::MyClassPrx allTests(Test::TestHelper*);
    allTests(this);

    testCollocatedIPv6Invocation(this);
}

DEFINE_TEST(Collocated)
