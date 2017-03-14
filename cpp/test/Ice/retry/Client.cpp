// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>
#include <InstrumentationI.h>

DEFINE_TEST("client")

using namespace std;
using namespace Test;

int
run(int, char**, const Ice::CommunicatorPtr& communicator, const Ice::CommunicatorPtr& communicator2)
{
    RetryPrxPtr allTests(const Ice::CommunicatorPtr&, const Ice::CommunicatorPtr&, const string&);
    RetryPrxPtr retry = allTests(communicator, communicator2, "retry:" + getTestEndpoint(communicator, 0));
    retry->shutdown();
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL(false);
#endif
    try
    {
        initCounts();

        Ice::InitializationData initData = getTestInitData(argc, argv);
        initData.observer = getObserver();
        //
        // This test kills connections, so we don't want warnings.
        //
        initData.properties->setProperty("Ice.Warn.Connections", "0");

        // Disable PrintStackTraces otherwise the test can fail on Windows/Debug
        initData.properties->setProperty("Ice.PrintStackTraces", "0");

        initData.properties->setProperty("Ice.RetryIntervals", "0 1 10 1");
        Ice::CommunicatorHolder ich(argc, argv, initData);

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
