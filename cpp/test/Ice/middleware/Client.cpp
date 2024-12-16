// Copyright (c) ZeroC, Inc.

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
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    void allTests(Test::TestHelper*);
    allTests(this);
}

DEFINE_TEST(Client)
