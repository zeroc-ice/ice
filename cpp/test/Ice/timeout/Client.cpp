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
using namespace Test;

int
run(int, char**, const Ice::CommunicatorPtr& communicator)
{
    TimeoutPrxPtr allTests(const Ice::CommunicatorPtr&);
    TimeoutPrxPtr timeout = allTests(communicator);
    timeout->shutdown();
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

        //
        // For this test, we want to disable retries.
        //
        initData.properties->setProperty("Ice.RetryIntervals", "-1");

#if TARGET_OS_IPHONE != 0
        //
        // COMPILERFIX: Disable connect timeout introduced for
        // workaround to iOS device hangs when using SSL
        //
        initData.properties->setProperty("Ice.Override.ConnectTimeout", "");
#endif

        //
        // This test kills connections, so we don't want warnings.
        //
        initData.properties->setProperty("Ice.Warn.Connections", "0");

        //
        // We need to send messages large enough to cause the transport
        // buffers to fill up.
        //
        initData.properties->setProperty("Ice.MessageSizeMax", "20000");

        //
        // Limit the send buffer size, this test relies on the socket
        // send() blocking after sending a given amount of data.
        //
        initData.properties->setProperty("Ice.TCP.SndSize", "50000");

        Ice::CommunicatorHolder ich(argc, argv, initData);
        return run(argc, argv, ich.communicator());
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        return  EXIT_FAILURE;
    }
}
