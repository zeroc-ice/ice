// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>
#include <TestI.h>
#include <InstrumentationI.h>

DEFINE_TEST("collocated")

using namespace std;
using namespace Test;

void
setupObjectAdapter(const Ice::CommunicatorPtr& communicator)
{
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("");
    adapter->add(ICE_MAKE_SHARED(RetryI), Ice::stringToIdentity("retry"));
    //adapter->activate(); // Don't activate OA to ensure collocation is used.
}

int
run(int, char**, const Ice::CommunicatorPtr& communicator, const Ice::CommunicatorPtr& communicator2)
{
    setupObjectAdapter(communicator);
    setupObjectAdapter(communicator2);

    RetryPrxPtr allTests(const Ice::CommunicatorPtr&, const Ice::CommunicatorPtr&, const string&);
    RetryPrxPtr retry = allTests(communicator, communicator2, "retry");
    retry->shutdown();
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL();
#endif
    try
    {
        Ice::InitializationData initData = getTestInitData(argc, argv);
        initData.observer = getObserver();

        initData.properties->setProperty("Ice.RetryIntervals", "0 1 10 1");

        //
        // This test kills connections, so we don't want warnings.
        //
        initData.properties->setProperty("Ice.Warn.Connections", "0");
        initData.properties->setProperty("Ice.Warn.Dispatch", "0");

        Ice::CommunicatorHolder ich = Ice::initialize(argc, argv, initData);

        //
        // Configure a second communicator for the invocation timeout
        // + retry test, we need to configure a large retry interval
        // to avoid time-sensitive failures.
        //
        Ice::InitializationData initData2;
        initData2.properties = initData.properties->clone();
        initData2.properties->setProperty("Ice.RetryIntervals", "0 1 10000");
        initData2.observer = getObserver();

        Ice::CommunicatorHolder ich2 = Ice::initialize(initData2);

        return run(argc, argv, ich.communicator(), ich2.communicator());
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        return EXIT_FAILURE;
    }
}
