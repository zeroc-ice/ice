//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>

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
    properties->parseCommandLineOptions("", Ice::argsToStringSeq(argc, argv));
    properties->setProperty("Ice.Warn.Connections", "0");
    initialize(argc, argv, properties);
    void allTests(Test::TestHelper*);
    allTests(this);
}

DEFINE_TEST(Client)
