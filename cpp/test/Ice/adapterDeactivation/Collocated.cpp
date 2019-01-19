//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <ServantLocatorI.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;
using namespace Ice;
using namespace Test;

class Collocated : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Collocated::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());

    //
    // 2 threads are necessary to dispatch the collocated transient() call with AMI
    //
    communicator->getProperties()->setProperty("TestAdapter.ThreadPool.Size", "2");

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    ServantLocatorPtr locator = ICE_MAKE_SHARED(ServantLocatorI);
    adapter->addServantLocator(locator, "");

    TestIntfPrxPtr allTests(TestHelper*);
    allTests(this);

    adapter->waitForDeactivate();
}

DEFINE_TEST(Collocated)
