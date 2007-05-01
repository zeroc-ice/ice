// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;

int
run(int argc, char* argv[], 
    const Ice::CommunicatorPtr& communicator,
    const Ice::InitializationData& initData)
{
    Test::MyClassPrx allTests(const Ice::CommunicatorPtr&, bool);
    Test::MyClassPrx myClass = allTests(communicator, false);

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

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
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

        //
        // We must set MessageSizeMax to an explicit values, because
        // we run tests to check whether Ice.MemoryLimitException is
        // raised as expected.
        //
        initData.properties->setProperty("Ice.MessageSizeMax", "100");

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
