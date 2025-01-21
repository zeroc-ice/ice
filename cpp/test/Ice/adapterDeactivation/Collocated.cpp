// Copyright (c) ZeroC, Inc.

#include "ServantLocatorI.h"
#include "TestHelper.h"

using namespace std;
using namespace Ice;
using namespace Test;

class Collocated : public Test::TestHelper
{
public:
    void run(int, char**) override;
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
    ServantLocatorPtr locator = make_shared<ServantLocatorI>();
    adapter->addServantLocator(locator, "");

    void allTests(TestHelper*);
    allTests(this);

    adapter->waitForDeactivate();
}

DEFINE_TEST(Collocated)
