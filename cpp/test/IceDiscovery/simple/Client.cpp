// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Ice/RegisterPlugins.h"
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
#ifdef ICE_STATIC_LIBS
    //
    // Explicitly register the IceDiscovery plugin to test registerIceDiscovery.
    //
    Ice::registerIceDiscovery();
#endif

    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    int num = argc == 2 ? atoi(argv[1]) : 1;

    void allTests(Test::TestHelper*, int);
    allTests(this, num);
}

DEFINE_TEST(Client)
