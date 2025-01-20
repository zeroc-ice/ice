// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

using namespace std;

class Client final : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Client::run(int argc, char** argv)
{
    auto properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.Warn.Connections", "0");

    // The test connects only to localhost and we want connection establishment to fail quickly when the server is not
    // running, especially on Windows.
    properties->setProperty("Ice.Connection.Client.ConnectTimeout", "1");

    Ice::CommunicatorHolder communicatorHolder = initialize(argc, argv, properties);
    communicatorHolder->getProperties()->parseCommandLineOptions("", Ice::argsToStringSeq(argc, argv));
    void allTests(Test::TestHelper*);
    allTests(this);
}

DEFINE_TEST(Client)
