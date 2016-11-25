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
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    Ice::PropertiesPtr properties = communicator->getProperties();

    int num = argc == 2 ? atoi(argv[1]) : 0;

    properties->setProperty("ControlAdapter.Endpoints", getTestEndpoint(communicator, num, "tcp"));
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("ControlAdapter");
    adapter->add(ICE_MAKE_SHARED(TestIntfI), Ice::stringToIdentity("control"));
    adapter->activate();

    if(num == 0)
    {
        properties->setProperty("TestAdapter.Endpoints", getTestEndpoint(communicator, num, "udp"));
        Ice::ObjectAdapterPtr adapter2 = communicator->createObjectAdapter("TestAdapter");
        adapter2->add(ICE_MAKE_SHARED(TestIntfI), Ice::stringToIdentity("test"));
        adapter2->activate();
    }

    string endpoint;
    if(properties->getProperty("Ice.IPv6") == "1")
    {
#ifndef __APPLE__
        endpoint = "udp -h \"ff15::1:1\" -p 12020";
#else
        endpoint = "udp -h \"ff15::1:1\" -p 12020 --interface \"::1\"";
#endif
    }
    else
    {
        endpoint = "udp -h 239.255.1.1 -p 12020";
    }
    properties->setProperty("McastTestAdapter.Endpoints", endpoint);
    Ice::ObjectAdapterPtr mcastAdapter = communicator->createObjectAdapter("McastTestAdapter");
    mcastAdapter->add(ICE_MAKE_SHARED(TestIntfI), Ice::stringToIdentity("test"));
    mcastAdapter->activate();

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
        initData.properties->setProperty("Ice.Warn.Connections", "0");
        initData.properties->setProperty("Ice.UDP.SndSize", "16384");
        initData.properties->setProperty("Ice.UDP.RcvSize", "16384");

        Ice::CommunicatorHolder ich = Ice::initialize(argc, argv, initData);
        return run(argc, argv, ich.communicator());
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        return  EXIT_FAILURE;
    }
}

