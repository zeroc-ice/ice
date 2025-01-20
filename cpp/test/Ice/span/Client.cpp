// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

using namespace std;

class Client : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Client::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    Test::TestIntfPrx allTests(Test::TestHelper*);
    Test::TestIntfPrx test = allTests(this);
    test->shutdown();
}

DEFINE_TEST(Client)
