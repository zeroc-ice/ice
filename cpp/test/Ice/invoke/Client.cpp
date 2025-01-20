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
    Ice::CommunicatorHolder holder = initialize(argc, argv);
    Test::MyClassPrx allTests(Test::TestHelper*);
    Test::MyClassPrx myClass = allTests(this);
    myClass->shutdown();
}

DEFINE_TEST(Client)
