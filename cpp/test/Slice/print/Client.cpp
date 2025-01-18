// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

using namespace std;
using namespace Test;

class Client final : public Test::TestHelper
{
public:
    void run(int, char**) final;
};

void
Client::run(int argc, char** argv)
{
    // TODO: we don't need/use a communicator in this test.
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    void allTests(Test::TestHelper*);
    allTests(this);
}

DEFINE_TEST(Client)
