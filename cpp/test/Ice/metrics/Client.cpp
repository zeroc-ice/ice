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
#include <InstrumentationI.h>

DEFINE_TEST("client")

using namespace std;
using namespace Test;

int
run(int, char**, const Ice::CommunicatorPtr& communicator, const CommunicatorObserverIPtr& observer)
{
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
