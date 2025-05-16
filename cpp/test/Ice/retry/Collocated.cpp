// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "InstrumentationI.h"
#include "Test.h"
#include "TestHelper.h"
#include "TestI.h"

using namespace std;
using namespace Test;

void
setupObjectAdapter(const Ice::CommunicatorPtr& communicator)
{
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("");
    adapter->add(std::make_shared<RetryI>(), Ice::stringToIdentity("retry"));
    // adapter->activate(); // Don't activate OA to ensure collocation is used.
}

class Collocated : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Collocated::run(int argc, char** argv)
{
    initCounts();

    Ice::InitializationData initData;
    initData.properties = createTestProperties(argc, argv);
    initData.observer = getObserver();

    initData.properties->setProperty("Ice.RetryIntervals", "0 1 10 1");

    //
    // This test kills connections, so we don't want warnings.
    //
    initData.properties->setProperty("Ice.Warn.Connections", "0");
    initData.properties->setProperty("Ice.Warn.Dispatch", "0");

    installTransport(initData);

    Ice::CommunicatorHolder ich1 = initialize(initData);

    //
    // Configure a second communicator for the invocation timeout
    // + retry test, we need to configure a large retry interval
    // to avoid time-sensitive failures.
    //
    initData.properties = initData.properties->clone();
    initData.properties->setProperty("Ice.RetryIntervals", "0 1 10000");
    initData.observer = getObserver();

    Ice::CommunicatorHolder ich2 = initialize(initData);

    setupObjectAdapter(ich1.communicator());
    setupObjectAdapter(ich2.communicator());

    RetryPrx allTests(const Ice::CommunicatorPtr&, const Ice::CommunicatorPtr&, const string&);
    RetryPrx retry = allTests(ich1.communicator(), ich2.communicator(), "retry");
    retry->shutdown();
}

DEFINE_TEST(Collocated)
