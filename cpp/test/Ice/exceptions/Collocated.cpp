//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <TestI.h>

using namespace std;
using namespace Test;

class Collocated : public Test::TestHelper
{
public:

    void run(int, char**);
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
    Ice::ObjectPtr object = ICE_MAKE_SHARED(ThrowerI);
    adapter->add(object, Ice::stringToIdentity("thrower"));

    ThrowerPrxPtr allTests(Test::TestHelper*);
    allTests(this);
}

DEFINE_TEST(Collocated)
