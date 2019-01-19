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
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.Default.Locator", "locator:" + getTestEndpoint(properties));
    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    void allTests(Test::TestHelper*, const string&);
    allTests(this, "ServerManager:" + getTestEndpoint());
}

DEFINE_TEST(Client)
