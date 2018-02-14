// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>
#include <TestCommon.h>

using namespace std;

int
run(int, char**, const Ice::CommunicatorPtr& communicator)
{
    Ice::PropertiesPtr properties = communicator->getProperties();
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", "default -p 12010 -t 10000:udp");

    //
    // First try to use an invalid priority.
    //
    communicator->getProperties()->setProperty("Ice.ThreadPool.Server.ThreadPriority", "1024");
    try
    {
        Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
        test(false);
    }
    catch(const IceUtil::ThreadSyscallException&)
    {
        //expected
    }
    catch(...)
    {
        test(false);
    }

    //
    // Now set the priority correctly.
    //
#ifdef _WIN32
    communicator->getProperties()->setProperty("Ice.ThreadPool.Server.ThreadPriority", "1");
#else
    communicator->getProperties()->setProperty("Ice.ThreadPool.Server.ThreadPriority", "50");
#endif

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectPtr object = new PriorityI(adapter);
    adapter->add(object, communicator->stringToIdentity("test"));
    adapter->activate();
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
