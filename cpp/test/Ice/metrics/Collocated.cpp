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
    Ice::ObjectPtr object = new MetricsI;
    adapter->add(object, communicator->stringToIdentity("metrics"));
    //adapter->activate(); // Don't activate OA to ensure collocation is used.

    communicator->getProperties()->setProperty("ControllerAdapter.Endpoints", "default -p 12011");
    Ice::ObjectAdapterPtr controllerAdapter = communicator->createObjectAdapter("ControllerAdapter");
    controllerAdapter->add(new ControllerI(adapter), communicator->stringToIdentity("controller"));
    //controllerAdapter->activate(); // Don't activate OA to ensure collocation is used.

    MetricsPrx allTests(const Ice::CommunicatorPtr&, const CommunicatorObserverIPtr&);
    MetricsPrx metrics = allTests(communicator, observer);
    metrics->shutdown();
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
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties(argc, argv);
        initData.properties->setProperty("Ice.Admin.Endpoints", "tcp");
        initData.properties->setProperty("Ice.Admin.InstanceName", "client");
        initData.properties->setProperty("Ice.Admin.DelayCreation", "1");
        initData.properties->setProperty("Ice.Warn.Connections", "0");
        initData.properties->setProperty("Ice.Warn.Dispatch", "0");
        initData.properties->setProperty("Ice.MessageSizeMax", "50000");
        initData.properties->setProperty("Ice.Default.Host", "127.0.0.1");
        CommunicatorObserverIPtr observer = new CommunicatorObserverI();
        initData.observer = observer;
        communicator = Ice::initialize(argc, argv, initData);
        status = run(argc, argv, communicator, observer);
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
