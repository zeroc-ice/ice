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

DEFINE_TEST("server");

using namespace std;

int
run(int, char**, const Ice::CommunicatorPtr& communicator)
{
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint(communicator, 0) + ":udp");
    communicator->getProperties()->setProperty("TestAdapter2.Endpoints", getTestEndpoint(communicator, 1));
    communicator->getProperties()->setProperty("TestAdapter2.MessageSizeMax", "0");
    communicator->getProperties()->setProperty("TestAdapter3.Endpoints", getTestEndpoint(communicator, 2));
    communicator->getProperties()->setProperty("TestAdapter3.MessageSizeMax", "1");
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectAdapterPtr adapter2 = communicator->createObjectAdapter("TestAdapter2");
    Ice::ObjectAdapterPtr adapter3 = communicator->createObjectAdapter("TestAdapter3");
    Ice::ObjectPtr object = ICE_MAKE_SHARED(ThrowerI);
    adapter->add(object, Ice::stringToIdentity("thrower"));
    adapter2->add(object, Ice::stringToIdentity("thrower"));
    adapter3->add(object, Ice::stringToIdentity("thrower"));
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

    try
    {
        Ice::InitializationData initData = getTestInitData(argc, argv);
        initData.properties->setProperty("Ice.Warn.Dispatch", "0");
        initData.properties->setProperty("Ice.Warn.Connections", "0");
        initData.properties->setProperty("Ice.MessageSizeMax", "10"); // 10KB max

        Ice::CommunicatorHolder ich = Ice::initialize(argc, argv, initData);
        return run(argc, argv, ich.communicator());
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        return  EXIT_FAILURE;
    }
}
