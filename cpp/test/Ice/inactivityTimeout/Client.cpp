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
    // We configure a low idle timeout to make sure we send heartbeats frequently.
    initData.properties->setProperty("Ice.Connection.IdleTimeout", "1");
    initData.properties->setProperty("Ice.Connection.InactivityTimeout", "1");
    // Limit the send buffer size, this test relies on the socket send() blocking after sending a given amount of data.
    initData.properties->setProperty("Ice.TCP.SndSize", "50000");
    Ice::CommunicatorHolder communicator = initialize(argc, argv, initData);


    void allTests(Test::TestHelper*);
    allTests(this);
}

DEFINE_TEST(Client)
