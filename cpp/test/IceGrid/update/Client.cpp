//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;

class Client final : public Test::TestHelper
{
public:

    void run(int, char**) override;
};

void
Client::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicatorHolder = initialize(argc, argv);
    communicatorHolder->getProperties()->parseCommandLineOptions("", Ice::argsToStringSeq(argc, argv));
    void allTests(Test::TestHelper*);
    allTests(this);
}

DEFINE_TEST(Client)
