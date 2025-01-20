//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

template<typename T>
function<shared_ptr<T>(string_view)>
makeFactory()
{
    return [](string_view) { return make_shared<T>(); };
}

class Client : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Client::run(int argc, char** argv)
{
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.AcceptClassCycles", "1");

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    communicator->getValueFactoryManager()->add(makeFactory<BI>(), "::Test::B");
    communicator->getValueFactoryManager()->add(makeFactory<CI>(), "::Test::C");
    communicator->getValueFactoryManager()->add(makeFactory<DI>(), "::Test::D");

    InitialPrx allTests(Test::TestHelper*);
    InitialPrx initial = allTests(this);
    initial->shutdown();
}

DEFINE_TEST(Client)
