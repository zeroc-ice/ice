//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.AcceptClassCycles", "1");
    properties->setProperty("Ice.Warn.Dispatch", "0");
    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);

    communicator->getValueFactoryManager()->add(makeFactory<BI>(), "::Test::B");
    communicator->getValueFactoryManager()->add(makeFactory<CI>(), "::Test::C");
    communicator->getValueFactoryManager()->add(makeFactory<DI>(), "::Test::D");
    communicator->getValueFactoryManager()->add(makeFactory<EI>(), "::Test::E");
    communicator->getValueFactoryManager()->add(makeFactory<FI>(), "::Test::F");

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
