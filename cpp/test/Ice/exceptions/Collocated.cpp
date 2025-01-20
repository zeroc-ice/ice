// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "TestHelper.h"
#include "TestI.h"

using namespace std;
using namespace Test;

class Collocated : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Collocated::run(int argc, char** argv)
{
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.MessageSizeMax", "10"); // 10KB max
    properties->setProperty("Ice.Warn.Connections", "0");
    properties->setProperty("Ice.Warn.Dispatch", "0");

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);

    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectPtr object = std::make_shared<ThrowerI>();
    adapter->add(object, Ice::stringToIdentity("thrower"));

    ThrowerPrx allTests(Test::TestHelper*);
    allTests(this);
}

DEFINE_TEST(Collocated)
