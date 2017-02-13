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

DEFINE_TEST("server")

using namespace std;

int
run(int, char**, const Ice::CommunicatorPtr& communicator)
{
    IceUtil::TimerPtr timer = new IceUtil::Timer();

    communicator->getProperties()->setProperty("TestAdapter1.Endpoints", "default -p 12010:udp");
    communicator->getProperties()->setProperty("TestAdapter1.ThreadPool.Size", "5");
    communicator->getProperties()->setProperty("TestAdapter1.ThreadPool.SizeMax", "5");
    communicator->getProperties()->setProperty("TestAdapter1.ThreadPool.SizeWarn", "0");
    communicator->getProperties()->setProperty("TestAdapter1.ThreadPool.Serialize", "0");
    Ice::ObjectAdapterPtr adapter1 = communicator->createObjectAdapter("TestAdapter1");
    adapter1->add(new HoldI(timer, adapter1), communicator->stringToIdentity("hold"));

    communicator->getProperties()->setProperty("TestAdapter2.Endpoints", "default -p 12011:udp");
    communicator->getProperties()->setProperty("TestAdapter2.ThreadPool.Size", "5");
    communicator->getProperties()->setProperty("TestAdapter2.ThreadPool.SizeMax", "5");
    communicator->getProperties()->setProperty("TestAdapter2.ThreadPool.SizeWarn", "0");
    communicator->getProperties()->setProperty("TestAdapter2.ThreadPool.Serialize", "1");
    Ice::ObjectAdapterPtr adapter2 = communicator->createObjectAdapter("TestAdapter2");
    adapter2->add(new HoldI(timer, adapter2), communicator->stringToIdentity("hold"));

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
    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
        communicator = Ice::initialize(argc, argv);
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
