//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;
using namespace Test;

class Client : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Client::run(int argc, char** argv)
{
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.Warn.Connections", "0");
    properties->setProperty("Ice.MessageSizeMax", "10"); // 10KB max
    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);

    ThrowerPrxPtr allTests(Test::TestHelper*);
    ThrowerPrxPtr thrower = allTests(this);
    thrower->shutdown();
}

DEFINE_TEST(Client)
