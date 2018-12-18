// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>
#include <TestI.h>
#include <InstrumentationI.h>

using namespace std;
using namespace Test;

void
setupObjectAdapter(const Ice::CommunicatorPtr& communicator)
{
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("");
    adapter->add(ICE_MAKE_SHARED(RetryI), Ice::stringToIdentity("retry"));
    //adapter->activate(); // Don't activate OA to ensure collocation is used.
}

class Collocated : public Test::TestHelper
{
public:

    void run(int, char**);
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

    Ice::CommunicatorHolder ich1 = Ice::initialize(argc, argv, initData);

    //
    // Configure a second communicator for the invocation timeout
    // + retry test, we need to configure a large retry interval
    // to avoid time-sensitive failures.
    //
    initData.properties = initData.properties->clone();
    initData.properties->setProperty("Ice.RetryIntervals", "0 1 10000");
    initData.observer = getObserver();

    Ice::CommunicatorHolder ich2 = Ice::initialize(initData);;

    setupObjectAdapter(ich1.communicator());
    setupObjectAdapter(ich2.communicator());

    RetryPrxPtr allTests(const Ice::CommunicatorPtr&, const Ice::CommunicatorPtr&, const string&);
    RetryPrxPtr retry = allTests(ich1.communicator(), ich2.communicator(), "retry");
    retry->shutdown();
}

DEFINE_TEST(Collocated)
