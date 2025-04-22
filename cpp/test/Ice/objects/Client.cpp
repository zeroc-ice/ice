// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "TestHelper.h"
#include "TestI.h"

//
// Required to trigger initialization of Derived object factory.
//
#include "Derived.h"

//
// Required to trigger initialization of DerivedEx exception factory.
//
#include "DerivedEx.h"

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
    Ice::InitializationData initData;
    initData.properties = createTestProperties(argc, argv);
    initData.properties->setProperty("Ice.AcceptClassCycles", "1");
    initData.sliceLoader = make_shared<CustomSliceLoader>();

    Ice::CommunicatorHolder communicator = initialize(argc, argv, initData);
    InitialPrx allTests(Test::TestHelper*);
    InitialPrx initial = allTests(this);
    initial->shutdown();
}

DEFINE_TEST(Client)
