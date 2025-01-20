// Copyright (c) ZeroC, Inc.

#include "ClientPrivate.h"
#include "Ice/Ice.h"
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
    properties->setProperty("Ice.AcceptClassCycles", "1");

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    TestIntfPrx allTests(Test::TestHelper*);
    TestIntfPrx prx = allTests(this);
    prx->shutdown();
}

DEFINE_TEST(Client)
