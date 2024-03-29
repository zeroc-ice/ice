//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;

class Client : public Test::TestHelper
{
public:
    void run(int, char**);
};

void
Client::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    Test::MyClassPrx allTests(Test::TestHelper*);
    Test::MyClassPrx myClass = allTests(this);

    myClass->shutdown();
}

DEFINE_TEST(Client)
