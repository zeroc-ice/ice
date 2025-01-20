// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

using namespace std;
using namespace Test;

class Client : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Client::run(int argc, char** argv)
{
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.Warn.Connections", "0");
    properties->setProperty("Ice.MessageSizeMax", "10"); // 10KB max
    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);

    ThrowerPrx allTests(Test::TestHelper*);
    ThrowerPrx thrower = allTests(this);
    thrower->shutdown();
}

DEFINE_TEST(Client)
