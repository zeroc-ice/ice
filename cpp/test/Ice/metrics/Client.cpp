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
#include <InstrumentationI.h>

DEFINE_TEST("client")

using namespace std;
using namespace Test;

int
run(int, char**, const Ice::CommunicatorPtr& communicator, const CommunicatorObserverIPtr& observer)
{
    MetricsPrxPtr allTests(const Ice::CommunicatorPtr&, const CommunicatorObserverIPtr&);
    MetricsPrxPtr metrics = allTests(communicator, observer);
    metrics->shutdown();
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
        initData.properties->setProperty("Ice.Admin.Endpoints", "default");
        initData.properties->setProperty("Ice.Admin.InstanceName", "client");
        initData.properties->setProperty("Ice.Admin.DelayCreation", "1");
        initData.properties->setProperty("Ice.Warn.Connections", "0");
        CommunicatorObserverIPtr observer = ICE_MAKE_SHARED(CommunicatorObserverI);
        initData.observer = observer;
        Ice::CommunicatorHolder ich(argc, argv, initData);
        return run(argc, argv, ich.communicator(), observer);
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        return EXIT_FAILURE;
    }
}
