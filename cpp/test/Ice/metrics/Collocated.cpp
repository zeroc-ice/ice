// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "InstrumentationI.h"
#include "TestHelper.h"
#include "TestI.h"

using namespace std;
using namespace Test;

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
    initData.properties->setProperty("Ice.Admin.Endpoints", "tcp");
    initData.properties->setProperty("Ice.Admin.InstanceName", "client");
    initData.properties->setProperty("Ice.Admin.DelayCreation", "1");
    initData.properties->setProperty("Ice.Warn.Connections", "0");
    initData.properties->setProperty("Ice.Warn.Dispatch", "0");
    CommunicatorObserverIPtr observer = make_shared<CommunicatorObserverI>();
    initData.observer = observer;
    Ice::CommunicatorHolder communicator = initialize(argc, argv, initData);

    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(make_shared<MetricsI>(), Ice::stringToIdentity("metrics"));
    // adapter->activate(); // Don't activate OA to ensure collocation is used.

    communicator->getProperties()->setProperty("ForwardingAdapter.Endpoints", getTestEndpoint(1));
    Ice::ObjectAdapterPtr forwardingAdapter = communicator->createObjectAdapter("ForwardingAdapter");
    forwardingAdapter->addDefaultServant(adapter->dispatchPipeline(), "");

    communicator->getProperties()->setProperty("ControllerAdapter.Endpoints", getTestEndpoint(2));
    Ice::ObjectAdapterPtr controllerAdapter = communicator->createObjectAdapter("ControllerAdapter");
    controllerAdapter->add(make_shared<ControllerI>(adapter), Ice::stringToIdentity("controller"));
    // controllerAdapter->activate(); // Don't activate OA to ensure collocation is used.

    MetricsPrx allTests(Test::TestHelper*, const CommunicatorObserverIPtr&);
    MetricsPrx metrics = allTests(this, observer);
    metrics->shutdown();
}

DEFINE_TEST(Collocated)
