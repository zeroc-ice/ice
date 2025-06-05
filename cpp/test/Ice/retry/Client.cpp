// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "InstrumentationI.h"
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
    initCounts();

    Ice::InitializationData initData;
    initData.properties = createTestProperties(argc, argv);
    initData.observer = getObserver();
    //
    // This test kills connections, so we don't want warnings.
    //
    initData.properties->setProperty("Ice.Warn.Connections", "0");

    // Disable PrintStackTraces otherwise the test can fail on Windows/Debug
    initData.properties->setProperty("Ice.PrintStackTraces", "0");

    initData.properties->setProperty("Ice.RetryIntervals", "0 1 10 1");
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

    RetryPrx allTests(const Ice::CommunicatorPtr&, const Ice::CommunicatorPtr&, const string&);
    RetryPrx retry = allTests(
        ich1.communicator(),
        ich2.communicator(),
        "retry:" + TestHelper::getTestEndpoint(ich1->getProperties()));
    retry->shutdown();
}

DEFINE_TEST(Client)
