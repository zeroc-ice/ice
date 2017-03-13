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

DEFINE_TEST("client")

using namespace std;

int
run(int, char**, const Ice::CommunicatorPtr& communicator)
{
    Test::MyClassPrxPtr allTests(const Ice::CommunicatorPtr&);
    Test::MyClassPrxPtr myClass = allTests(communicator);

    myClass->shutdown();
    cout << "testing server shutdown... " << flush;
    try
    {
#ifdef _WIN32
        myClass = myClass->ice_timeout(100); // Workaround to speed up testing
#endif
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
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL();
#   if defined(__linux)
    Ice::registerIceBT();
#   endif
#endif

    try
    {
        //
        // In this test, we need at least two threads in the
        // client side thread pool for nested AMI.
        //
        Ice::InitializationData initData = getTestInitData(argc, argv);
        initData.properties->setProperty("Ice.ThreadPool.Client.Size", "2");
        initData.properties->setProperty("Ice.ThreadPool.Client.SizeWarn", "0");
        initData.properties->setProperty("Ice.BatchAutoFlushSize", "100");

        Ice::CommunicatorHolder ich(argc, argv, initData);
        return run(argc, argv, ich.communicator());
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        return EXIT_FAILURE;
    }
}
