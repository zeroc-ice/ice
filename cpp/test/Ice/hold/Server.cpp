//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/Ice.h"
#include "TestHelper.h"
#include "TestI.h"

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
    IceUtil::TimerPtr timer = make_shared<IceUtil::Timer>();
    auto properties = communicator->getProperties();

    properties->setProperty("Ice.IdleTimeout", "1");

    properties->setProperty("TestAdapter1.Endpoints", getTestEndpoint());
    properties->setProperty("TestAdapter1.ThreadPool.Size", "5");
    properties->setProperty("TestAdapter1.ThreadPool.SizeMax", "5");
    properties->setProperty("TestAdapter1.ThreadPool.SizeWarn", "0");
    properties->setProperty("TestAdapter1.ThreadPool.Serialize", "0");
    properties->setProperty("TestAdapter1.EnableIdleCheck", "1");

    Ice::ObjectAdapterPtr adapter1 = communicator->createObjectAdapter("TestAdapter1");
    adapter1->add(make_shared<HoldI>(timer, adapter1), Ice::stringToIdentity("hold"));

    properties->setProperty("TestAdapter2.Endpoints", getTestEndpoint(1));
    properties->setProperty("TestAdapter2.ThreadPool.Size", "5");
    properties->setProperty("TestAdapter2.ThreadPool.SizeMax", "5");
    properties->setProperty("TestAdapter2.ThreadPool.SizeWarn", "0");
    properties->setProperty("TestAdapter2.ThreadPool.Serialize", "1");
    properties->setProperty("TestAdapter2.EnableIdleCheck", "1");
    Ice::ObjectAdapterPtr adapter2 = communicator->createObjectAdapter("TestAdapter2");
    adapter2->add(make_shared<HoldI>(timer, adapter2), Ice::stringToIdentity("hold"));

    adapter1->activate();
    adapter2->activate();

    serverReady();

    communicator->waitForShutdown();

    timer->destroy();
}

DEFINE_TEST(Server)
