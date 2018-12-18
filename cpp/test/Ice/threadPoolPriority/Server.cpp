// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>
#include <TestHelper.h>

using namespace std;

class Server : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Server::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    Ice::PropertiesPtr properties = communicator->getProperties();
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint() + " -t 10000");

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
    Ice::ObjectPtr object = ICE_MAKE_SHARED(PriorityI, adapter);
    adapter->add(object, Ice::stringToIdentity("test"));
    adapter->activate();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
