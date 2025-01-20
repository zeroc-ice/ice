// Copyright (c) ZeroC, Inc.

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
    auto properties = createTestProperties(argc, argv);
    // We disable retries to make the logs clearer and avoid hiding potential issues.
    properties->setProperty("Ice.RetryIntervals", "-1");

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);

    void allTests(Test::TestHelper*);
    allTests(this);
}

DEFINE_TEST(Client)
