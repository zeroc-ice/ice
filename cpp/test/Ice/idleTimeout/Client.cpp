//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Test.h"
#include "TestHelper.h"

using namespace std;

class Client : public Test::TestHelper
{
public:
    void run(int, char**);
};

void
Client::run(int argc, char** argv)
{
    Ice::InitializationData initData;
    initData.properties = createTestProperties(argc, argv);
    initData.properties->setProperty("Ice.IdleTimeout", "2"); // 2 seconds
    Ice::CommunicatorHolder communicator = initialize(argc, argv, initData);

    void allTests(Test::TestHelper*);
    allTests(this);
}

DEFINE_TEST(Client)
