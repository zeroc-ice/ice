// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
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
    properties->setProperty("Ice.Warn.Connections", "0");
    properties->setProperty("Ice.UDP.RcvSize", "16384");
    properties->setProperty("Ice.UDP.SndSize", "16384");

    Ice::CommunicatorHolder ich = initialize(argc, argv, properties);
    const auto& communicator = ich.communicator();
    void allTests(Test::TestHelper*);
    allTests(this);

    int num = argc == 2 ? stoi(argv[1]) : 1;
    for (int i = 0; i < num; i++)
    {
        ostringstream os;
        os << "control:" << getTestEndpoint(i, "tcp");
        TestIntfPrx(communicator, os.str())->shutdown();
    }
}

DEFINE_TEST(Client)
