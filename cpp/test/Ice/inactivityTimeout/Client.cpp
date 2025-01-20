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
    // We configure a low idle timeout to make sure we send heartbeats frequently. It's the sending
    // of the heartbeats that schedules the inactivity timer task.
    initData.properties->setProperty("Ice.Connection.Client.IdleTimeout", "1");
    initData.properties->setProperty("Ice.Connection.Client.InactivityTimeout", "3");
    Ice::CommunicatorHolder communicator = initialize(argc, argv, initData);

    void allTests(Test::TestHelper*);
    allTests(this);
}

DEFINE_TEST(Client)
