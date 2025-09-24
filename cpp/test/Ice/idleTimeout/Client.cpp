// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
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
    Ice::InitializationData initData;
    initData.properties = createTestProperties(argc, argv);
    initData.properties->setProperty("Ice.Connection.Client.IdleTimeout", "1");
    Ice::CommunicatorHolder communicator = initialize(initData);

    void allTests(Test::TestHelper*);
    allTests(this);
}

DEFINE_TEST(Client)
