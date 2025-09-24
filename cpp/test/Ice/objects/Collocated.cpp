// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "TestHelper.h"
#include "TestI.h"

using namespace std;
using namespace Test;

template<typename T>
function<shared_ptr<T>(string_view)>
makeFactory()
{
    return [](string_view) { return make_shared<T>(); };
}

class Collocated : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Collocated::run(int argc, char** argv)
{
    Ice::InitializationData initData;
    initData.properties = createTestProperties(argc, argv);
    initData.properties->setProperty("Ice.AcceptClassCycles", "1");
    initData.properties->setProperty("Ice.Warn.Dispatch", "0");
    initData.sliceLoader = make_shared<CustomSliceLoader>();

    Ice::CommunicatorHolder communicator = initialize(initData);

    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(make_shared<InitialI>(adapter), Ice::stringToIdentity("initial"));
    adapter->add(make_shared<TestIntfI>(), Ice::stringToIdentity("test"));
    adapter->add(make_shared<F2I>(), Ice::stringToIdentity("F21"));
    adapter->add(make_shared<UnexpectedObjectExceptionTestI>(), Ice::stringToIdentity("uoet"));
    InitialPrx allTests(Test::TestHelper*);
    InitialPrx initial = allTests(this);
    // We must call shutdown even in the collocated case for cyclic dependency cleanup
    initial->shutdown();
}

DEFINE_TEST(Collocated)
