//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/Ice.h"
#include "TestHelper.h"
#include "TestI.h"

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
    properties->setProperty("Ice.AcceptClassCycles", "1");
    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    InitialPrx allTests(Test::TestHelper*, bool);
    InitialPrx initial = allTests(this, false);
    initial->shutdown();
}

DEFINE_TEST(Client)
