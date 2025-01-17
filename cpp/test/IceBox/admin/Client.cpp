//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
    Ice::CommunicatorHolder ich = initialize(argc, argv);
    auto communicator = ich.communicator();

    void allTests(Test::TestHelper*);
    allTests(this);

    //
    // Shutdown the IceBox server.
    //
    Ice::ProcessPrx(communicator, "DemoIceBox/admin -f Process:default -p 9996")->shutdown();
}

DEFINE_TEST(Client)
