// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "TestHelper.h"
#include "TestI.h"

using namespace std;
using namespace Test;

class Server : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Server::run(int argc, char** argv)
{
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.Warn.Dispatch", "0");

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);

    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(make_shared<InitialI>(adapter), Ice::stringToIdentity("initial"));
    adapter->add(make_shared<TestIntfI>(), Ice::stringToIdentity("test"));
    adapter->add(make_shared<F2I>(), Ice::stringToIdentity("F21"));

    adapter->add(make_shared<UnexpectedObjectExceptionTestI>(), Ice::stringToIdentity("uoet"));
    adapter->activate();
    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
