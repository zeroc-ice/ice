// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>
#include <TestHelper.h>

using namespace std;

class ServerCustomThreadPool : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
ServerCustomThreadPool::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    Ice::PropertiesPtr properties = communicator->getProperties();
    properties->setProperty("Ice.Warn.Dispatch", "0");
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint() + " -t 10000");
    communicator->getProperties()->setProperty("TestAdapter.ThreadPool.Size", "1");

    //
    // First try to use an invalid priority.
    //
    communicator->getProperties()->setProperty("TestAdapter.ThreadPool.ThreadPriority", "1024");
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
    communicator->getProperties()->setProperty("Ice.ThreadPool.Server.ThreadPriority", "-1");
    communicator->getProperties()->setProperty("TestAdapter.ThreadPool.ThreadPriority", "1");
#else
    communicator->getProperties()->setProperty("Ice.ThreadPool.Server.ThreadPriority", "10");
    communicator->getProperties()->setProperty("TestAdapter.ThreadPool.ThreadPriority", "50");
#endif
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectPtr object = ICE_MAKE_SHARED(PriorityI, adapter);
    adapter->add(object, Ice::stringToIdentity("test"));
    adapter->activate();
    communicator->waitForShutdown();
}

DEFINE_TEST(ServerCustomThreadPool)
