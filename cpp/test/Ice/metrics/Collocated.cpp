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
#include <InstrumentationI.h>

DEFINE_TEST("client")

using namespace std;
using namespace Test;

int
run(int, char**, const Ice::CommunicatorPtr& communicator, const CommunicatorObserverIPtr& observer)
{
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", "default -p 12010");
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(ICE_MAKE_SHARED(MetricsI), Ice::stringToIdentity("metrics"));
    //adapter->activate(); // Don't activate OA to ensure collocation is used.

    communicator->getProperties()->setProperty("ControllerAdapter.Endpoints", "default -p 12011");
    Ice::ObjectAdapterPtr controllerAdapter = communicator->createObjectAdapter("ControllerAdapter");
    controllerAdapter->add(ICE_MAKE_SHARED(ControllerI, adapter), Ice::stringToIdentity("controller"));
    //controllerAdapter->activate(); // Don't activate OA to ensure collocation is used.

    MetricsPrxPtr allTests(const Ice::CommunicatorPtr&, const CommunicatorObserverIPtr&);
    MetricsPrxPtr metrics = allTests(communicator, observer);
    metrics->shutdown();
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
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties(argc, argv);
        initData.properties->setProperty("Ice.Admin.Endpoints", "tcp");
        initData.properties->setProperty("Ice.Admin.InstanceName", "client");
        initData.properties->setProperty("Ice.Admin.DelayCreation", "1");
        initData.properties->setProperty("Ice.Warn.Connections", "0");
        initData.properties->setProperty("Ice.Warn.Dispatch", "0");
        initData.properties->setProperty("Ice.MessageSizeMax", "50000");
        initData.properties->setProperty("Ice.Default.Host", "127.0.0.1");
        CommunicatorObserverIPtr observer = ICE_MAKE_SHARED(CommunicatorObserverI);
        initData.observer = observer;
        Ice::CommunicatorHolder ich = Ice::initialize(argc, argv, initData);
        return run(argc, argv, ich.communicator(), observer);
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        return EXIT_FAILURE;
    }
}
