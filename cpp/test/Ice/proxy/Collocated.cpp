// Copyright (c) ZeroC, Inc.

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
    properties->setProperty("Ice.Warn.Dispatch", "0");
    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(std::make_shared<MyDerivedClassI>(), Ice::stringToIdentity("test"));
    adapter->add(std::make_shared<CI>(), Ice::stringToIdentity("c"));
    // adapter->activate(); // Don't activate OA to ensure collocation is used.

    Test::MyClassPrx allTests(Test::TestHelper*);
    allTests(this);
}

DEFINE_TEST(Collocated)
