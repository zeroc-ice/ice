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
#include <TestI.h>

DEFINE_TEST("server")

using namespace std;

int
run(int, char**, const Ice::CommunicatorPtr& communicator)
{
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint(communicator, 0));
    communicator->getProperties()->setProperty("ControllerAdapter.Endpoints", getTestEndpoint(communicator, 1));
    communicator->getProperties()->setProperty("ControllerAdapter.ThreadPool.Size", "1");

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(ICE_MAKE_SHARED(TimeoutI), Ice::stringToIdentity("timeout"));
    adapter->activate();

    Ice::ObjectAdapterPtr controllerAdapter = communicator->createObjectAdapter("ControllerAdapter");
    controllerAdapter->add(ICE_MAKE_SHARED(ControllerI, adapter), Ice::stringToIdentity("controller"));
    controllerAdapter->activate();

    TEST_READY
    communicator->waitForShutdown();
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL(false);
    Ice::registerIceWS(true);
    Ice::registerIceUDP(true);
#endif

    try
    {
        Ice::InitializationData initData = getTestInitData(argc, argv);

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
        // The client sends large messages to cause the transport
        // buffers to fill up.
        //
        initData.properties->setProperty("Ice.MessageSizeMax", "20000");

        //
        // Limit the recv buffer size, this test relies on the socket
        // send() blocking after sending a given amount of data.
        //
        initData.properties->setProperty("Ice.TCP.RcvSize", "50000");

        Ice::CommunicatorHolder ich(argc, argv, initData);
        return run(argc, argv, ich.communicator());
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        return  EXIT_FAILURE;
    }
}
