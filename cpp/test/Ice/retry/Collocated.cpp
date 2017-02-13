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
#include <TestI.h>
#include <InstrumentationI.h>

DEFINE_TEST("collocated")

using namespace std;
using namespace Test;

void
setupObjectAdapter(const Ice::CommunicatorPtr& communicator)
{
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("");
    Ice::ObjectPtr object = new RetryI;
    adapter->add(object, communicator->stringToIdentity("retry"));
    //adapter->activate(); // Don't activate OA to ensure collocation is used.
}

int
run(int, char**, const Ice::CommunicatorPtr& communicator, const Ice::CommunicatorPtr& communicator2)
{
    setupObjectAdapter(communicator);
    setupObjectAdapter(communicator2);

    RetryPrx allTests(const Ice::CommunicatorPtr&, const Ice::CommunicatorPtr&, const string&);
    RetryPrx retry = allTests(communicator, communicator2, "retry");
    retry->shutdown();
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL();
#endif

    int status;
    Ice::CommunicatorPtr communicator;
    Ice::CommunicatorPtr communicator2;

    try
    {
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties(argc, argv);
        initData.observer = getObserver();

        initData.properties->setProperty("Ice.RetryIntervals", "0 1 10 1");

        //
        // This test kills connections, so we don't want warnings.
        //
        initData.properties->setProperty("Ice.Warn.Connections", "0");
        initData.properties->setProperty("Ice.Warn.Dispatch", "0");

        communicator = Ice::initialize(argc, argv, initData);

        //
        // Configure a second communicator for the invocation timeout
        // + retry test, we need to configure a large retry interval
        // to avoid time-sensitive failures.
        //
        Ice::InitializationData initData2;
        initData2.properties = initData.properties->clone();
        initData2.properties->setProperty("Ice.RetryIntervals", "0 1 10000");
        initData2.observer = getObserver();

        communicator2 = Ice::initialize(initData2);

        status = run(argc, argv, communicator, communicator2);
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        status = EXIT_FAILURE;
    }

    if(communicator)
    {
        try
        {
            communicator->destroy();
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            status = EXIT_FAILURE;
        }
    }

    if(communicator2)
    {
        try
        {
            communicator2->destroy();
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            status = EXIT_FAILURE;
        }
    }

    return status;
}
