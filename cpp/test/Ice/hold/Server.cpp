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
    IceUtil::TimerPtr timer = new IceUtil::Timer();

    communicator->getProperties()->setProperty("TestAdapter1.Endpoints", getTestEndpoint(communicator, 0) + ":udp");
    communicator->getProperties()->setProperty("TestAdapter1.ThreadPool.Size", "5");
    communicator->getProperties()->setProperty("TestAdapter1.ThreadPool.SizeMax", "5");
    communicator->getProperties()->setProperty("TestAdapter1.ThreadPool.SizeWarn", "0");
    communicator->getProperties()->setProperty("TestAdapter1.ThreadPool.Serialize", "0");
    Ice::ObjectAdapterPtr adapter1 = communicator->createObjectAdapter("TestAdapter1");
    adapter1->add(ICE_MAKE_SHARED(HoldI, timer, adapter1), Ice::stringToIdentity("hold"));

    communicator->getProperties()->setProperty("TestAdapter2.Endpoints", getTestEndpoint(communicator, 1) + ":udp");
    communicator->getProperties()->setProperty("TestAdapter2.ThreadPool.Size", "5");
    communicator->getProperties()->setProperty("TestAdapter2.ThreadPool.SizeMax", "5");
    communicator->getProperties()->setProperty("TestAdapter2.ThreadPool.SizeWarn", "0");
    communicator->getProperties()->setProperty("TestAdapter2.ThreadPool.Serialize", "1");
    Ice::ObjectAdapterPtr adapter2 = communicator->createObjectAdapter("TestAdapter2");
    adapter2->add(ICE_MAKE_SHARED(HoldI, timer, adapter2), Ice::stringToIdentity("hold"));

    adapter1->activate();
    adapter2->activate();

    TEST_READY

    communicator->waitForShutdown();

    timer->destroy();

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
        Ice::CommunicatorHolder ich = Ice::initialize(argc, argv, initData);
        return run(argc, argv, ich.communicator());
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        return EXIT_FAILURE;
    }
}
