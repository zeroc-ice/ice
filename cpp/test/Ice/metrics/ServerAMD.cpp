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
#include <TestAMDI.h>

DEFINE_TEST("serveramd")

using namespace std;

int
run(int, char**, const Ice::CommunicatorPtr& communicator)
{
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint(communicator, 0));
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(ICE_MAKE_SHARED(MetricsI), Ice::stringToIdentity("metrics"));
    adapter->activate();

    communicator->getProperties()->setProperty("ControllerAdapter.Endpoints", getTestEndpoint(communicator, 1));
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
#endif
    try
    {
        Ice::InitializationData initData = getTestInitData(argc, argv);
        //initData.properties->setProperty("Ice.ThreadPool.Server.Size", "1");
        //initData.properties->setProperty("Ice.ThreadPool.Server.SizeMax", "1");
        initData.properties->setProperty("Ice.Admin.Endpoints", "tcp");
        initData.properties->setProperty("Ice.Admin.InstanceName", "server");
        initData.properties->setProperty("Ice.Warn.Connections", "0");
        initData.properties->setProperty("Ice.Warn.Dispatch", "0");
        initData.properties->setProperty("Ice.MessageSizeMax", "50000");
        Ice::CommunicatorHolder ich(argc, argv, initData);
        return run(argc, argv, ich.communicator());
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        return EXIT_FAILURE;
    }
}
