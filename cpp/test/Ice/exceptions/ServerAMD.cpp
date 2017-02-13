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
#include <TestAMDI.h>

DEFINE_TEST("serveramd");

using namespace std;

int
run(int, char**, const Ice::CommunicatorPtr& communicator)
{
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectAdapterPtr adapter2 = communicator->createObjectAdapter("TestAdapter2");
    Ice::ObjectAdapterPtr adapter3 = communicator->createObjectAdapter("TestAdapter3");
    Ice::ObjectPtr object = new ThrowerI();
    adapter->add(object, communicator->stringToIdentity("thrower"));
    adapter2->add(object, communicator->stringToIdentity("thrower"));
    adapter3->add(object, communicator->stringToIdentity("thrower"));
    adapter->activate();
    adapter2->activate();
    adapter3->activate();
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
    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties(argc, argv);
        initData.properties->setProperty("Ice.Warn.Dispatch", "0");
        initData.properties->setProperty("Ice.Warn.Connections", "0");
        initData.properties->setProperty("TestAdapter.Endpoints", "default -p 12010:udp");
        initData.properties->setProperty("Ice.MessageSizeMax", "10"); // 10KB max
        initData.properties->setProperty("TestAdapter2.Endpoints", "default -p 12011");
        initData.properties->setProperty("TestAdapter2.MessageSizeMax", "0");
        initData.properties->setProperty("TestAdapter3.Endpoints", "default -p 12012");
        initData.properties->setProperty("TestAdapter3.MessageSizeMax", "1");
        communicator = Ice::initialize(argc, argv, initData);
        status = run(argc, argv, communicator);
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
