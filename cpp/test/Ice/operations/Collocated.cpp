//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/Ice.h"
#include "TestHelper.h"
#include "TestI.h"

using namespace std;

class Collocated : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Collocated::run(int argc, char** argv)
{
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.BatchAutoFlushSize", "100");
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

DEFINE_TEST(Collocated)
