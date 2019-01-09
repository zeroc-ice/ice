// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <TestI.h>

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
    IceUtil::TimerPtr timer = new IceUtil::Timer();

    communicator->getProperties()->setProperty("TestAdapter1.Endpoints", getTestEndpoint());
    communicator->getProperties()->setProperty("TestAdapter1.ThreadPool.Size", "5");
    communicator->getProperties()->setProperty("TestAdapter1.ThreadPool.SizeMax", "5");
    communicator->getProperties()->setProperty("TestAdapter1.ThreadPool.SizeWarn", "0");
    communicator->getProperties()->setProperty("TestAdapter1.ThreadPool.Serialize", "0");
    Ice::ObjectAdapterPtr adapter1 = communicator->createObjectAdapter("TestAdapter1");
    adapter1->add(ICE_MAKE_SHARED(HoldI, timer, adapter1), Ice::stringToIdentity("hold"));

    communicator->getProperties()->setProperty("TestAdapter2.Endpoints", getTestEndpoint(1));
    communicator->getProperties()->setProperty("TestAdapter2.ThreadPool.Size", "5");
    communicator->getProperties()->setProperty("TestAdapter2.ThreadPool.SizeMax", "5");
    communicator->getProperties()->setProperty("TestAdapter2.ThreadPool.SizeWarn", "0");
    communicator->getProperties()->setProperty("TestAdapter2.ThreadPool.Serialize", "1");
    Ice::ObjectAdapterPtr adapter2 = communicator->createObjectAdapter("TestAdapter2");
    adapter2->add(ICE_MAKE_SHARED(HoldI, timer, adapter2), Ice::stringToIdentity("hold"));

    adapter1->activate();
    adapter2->activate();

    serverReady();

    communicator->waitForShutdown();

    timer->destroy();
}

DEFINE_TEST(Server)
