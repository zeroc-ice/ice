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
    properties->setProperty("Ice.Warn.Connections", "0");
    properties->setProperty("Ice.UDP.RcvSize", "16384");
    properties->setProperty("Ice.UDP.SndSize", "16384");

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    void allTests(Test::TestHelper*);
    allTests(this);

    int num = argc == 2 ? atoi(argv[1]) : 1;
    for(int i = 0; i < num; i++)
    {
        ostringstream os;
        os << "control:" << getTestEndpoint(i, "tcp");
        ICE_UNCHECKED_CAST(TestIntfPrx, communicator->stringToProxy(os.str()))->shutdown();
    }
}

DEFINE_TEST(Client)
