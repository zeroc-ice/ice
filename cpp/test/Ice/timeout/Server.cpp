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
#include <TestI.h>

DEFINE_TEST("server")

using namespace std;

int
run(int, char**, const Ice::CommunicatorPtr& communicator)
{
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint(communicator, 0) + ":udp");
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectPtr object = ICE_MAKE_SHARED(TimeoutI);
    adapter->add(object, Ice::stringToIdentity("timeout"));
    adapter->activate();
    TEST_READY
    communicator->waitForShutdown();
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
        // Limit the recv buffer size, this test relies on the socket
        // send() blocking after sending a given amount of data.
        //
        initData.properties->setProperty("Ice.TCP.RcvSize", "50000");

        Ice::CommunicatorHolder ich = Ice::initialize(argc, argv, initData);
        return run(argc, argv, ich.communicator());
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        return  EXIT_FAILURE;
    }
}
