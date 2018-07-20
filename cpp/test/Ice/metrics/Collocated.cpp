// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <TestI.h>
#include <InstrumentationI.h>

using namespace std;
using namespace Test;

class Collocated : public Test::TestHelper
{
public:

    void run(int, char**);
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
    CommunicatorObserverIPtr observer = ICE_MAKE_SHARED(CommunicatorObserverI);
    initData.observer = observer;
    Ice::CommunicatorHolder communicator = initialize(argc, argv, initData);

    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(ICE_MAKE_SHARED(MetricsI), Ice::stringToIdentity("metrics"));
    //adapter->activate(); // Don't activate OA to ensure collocation is used.

    communicator->getProperties()->setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
    Ice::ObjectAdapterPtr controllerAdapter = communicator->createObjectAdapter("ControllerAdapter");
    controllerAdapter->add(ICE_MAKE_SHARED(ControllerI, adapter), Ice::stringToIdentity("controller"));
    //controllerAdapter->activate(); // Don't activate OA to ensure collocation is used.

    MetricsPrxPtr allTests(Test::TestHelper*, const CommunicatorObserverIPtr&);
    MetricsPrxPtr metrics = allTests(this, observer);
    metrics->shutdown();
}

DEFINE_TEST(Collocated)
