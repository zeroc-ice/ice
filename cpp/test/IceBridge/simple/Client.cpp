//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;

class Client final : public Test::TestHelper
{
public:

    void run(int argc, char** argv) override;
};

void
Client::run(int argc, char** argv)
{
    auto properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.RetryIntervals", "-1");
    Ice::CommunicatorHolder communicatorHolder = initialize(argc, argv, properties);
    void allTests(Test::TestHelper*);
    allTests(this);
}

DEFINE_TEST(Client)
