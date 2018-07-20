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
#include <Test.h>
#include <InstrumentationI.h>

using namespace std;
using namespace Test;

class Client : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Client::run(int argc, char** argv)
{
    Ice::InitializationData initData;
    initData.properties = createTestProperties(argc, argv);
    initData.properties->setProperty("Ice.Admin.Endpoints", "default");
    initData.properties->setProperty("Ice.Admin.InstanceName", "client");
    initData.properties->setProperty("Ice.Admin.DelayCreation", "1");
    initData.properties->setProperty("Ice.Warn.Connections", "0");
    CommunicatorObserverIPtr observer = ICE_MAKE_SHARED(CommunicatorObserverI);
    initData.observer = observer;
    Ice::CommunicatorHolder communicator = initialize(argc, argv, initData);

    MetricsPrxPtr allTests(Test::TestHelper*, const CommunicatorObserverIPtr&);
    MetricsPrxPtr metrics = allTests(this, observer);
    metrics->shutdown();
}

DEFINE_TEST(Client)
