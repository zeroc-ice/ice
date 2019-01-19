//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;
using namespace Test;

class Client : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Client::run(int argc, char** argv)
{
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.NullHandleAbort", "0");
    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    void allTests(Test::TestHelper*);
    allTests(this);
}

DEFINE_TEST(Client)
