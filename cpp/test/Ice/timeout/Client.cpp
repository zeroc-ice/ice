// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
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
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);

    //
    // For this test, we want to disable retries.
    //
    properties->setProperty("Ice.RetryIntervals", "-1");

    properties->setProperty("Ice.Connection.Client.ConnectTimeout", "1");
    properties->setProperty("Ice.Connection.Client.CloseTimeout", "1");

    //
    // Limit the send buffer size, this test relies on the socket
    // send() blocking after sending a given amount of data.
    //
    properties->setProperty("Ice.TCP.SndSize", "50000");

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);

    void allTests(Test::TestHelper*);
    allTests(this);
}

DEFINE_TEST(Client)
