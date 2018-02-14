// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

DEFINE_TEST("client")

using namespace std;

int
run(int, char**, const Ice::CommunicatorPtr& communicator, const Ice::InitializationData&)
{
    Test::MyClassPrx allTests(const Ice::CommunicatorPtr&);
    Test::MyClassPrx myClass = allTests(communicator);

#ifndef ICE_OS_WINRT
    cout << "testing server shutdown... " << flush;
    myClass->shutdown();
    try
    {
        myClass->opVoid();
        test(false);
    }
    catch(const Ice::LocalException&)
    {
        cout << "ok" << endl;
    }
#else
    //
    // When using SSL the run.py script starts a new server after shutdown
    // and the call to opVoid will success.
    //
    myClass->shutdown();
#endif
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

    try
    {
        //
        // In this test, we need at least two threads in the
        // client side thread pool for nested AMI.
        //
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties(argc, argv);
        initData.properties->setProperty("Ice.ThreadPool.Client.Size", "2");
        initData.properties->setProperty("Ice.ThreadPool.Client.SizeWarn", "0");

        initData.properties->setProperty("Ice.BatchAutoFlushSize", "100");

        communicator = Ice::initialize(argc, argv, initData);
        status = run(argc, argv, communicator, initData);
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

    return status;
}
