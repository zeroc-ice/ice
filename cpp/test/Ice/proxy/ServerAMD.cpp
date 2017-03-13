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

DEFINE_TEST("serveramd")

using namespace std;

int
run(int, char**, const Ice::CommunicatorPtr& communicator)
{
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint(communicator, 0));
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(ICE_MAKE_SHARED(MyDerivedClassI), Ice::stringToIdentity("test"));
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
        initData.properties->setProperty("Ice.Warn.Connections", "0");
        initData.properties->setProperty("Ice.Warn.Dispatch", "0");

        Ice::CommunicatorHolder ich(argc, argv, initData);
        return run(argc, argv, ich.communicator());
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        return  EXIT_FAILURE;
    }
}
