// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "TestHelper.h"
#include "TestI.h"

class Server : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Server::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectPtr d = std::make_shared<DI>();
    adapter->add(d, Ice::stringToIdentity("d"));
    adapter->addFacet(d, Ice::stringToIdentity("d"), "facetABCD");
    Ice::ObjectPtr f = std::make_shared<FI>();
    adapter->addFacet(f, Ice::stringToIdentity("d"), "facetEF");
    Ice::ObjectPtr h = std::make_shared<HI>();
    adapter->addFacet(h, Ice::stringToIdentity("d"), "facetGH");
    serverReady();
    adapter->activate();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
