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

Ice::SliceLoaderPtr createCustomSliceLoader();
InitialPrx allTests(Test::TestHelper*, const Ice::SliceLoaderPtr&);

void
Client::run(int argc, char** argv)
{
    Ice::InitializationData initData;
    initData.properties = createTestProperties(argc, argv);
    initData.properties->setProperty("Ice.AcceptClassCycles", "1");
    initData.sliceLoader = createCustomSliceLoader();

    Ice::CommunicatorHolder communicator = initialize(initData);

    InitialPrx initial = allTests(this, initData.sliceLoader);
    initial->shutdown();
}

DEFINE_TEST(Client)
