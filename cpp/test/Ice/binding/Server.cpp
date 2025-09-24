// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "TestHelper.h"
#include "TestI.h"

using namespace std;

namespace
{
    //
    // A no-op Logger, used when testing the Logger Admin
    //

    class NullLogger final : public Ice::Logger, public std::enable_shared_from_this<NullLogger>
    {
    public:
        void print(const string&) final {}
        void trace(const string&, const string&) final {}
        void warning(const string&) final {}
        void error(const string&) final {}
        string getPrefix() final { return "NullLogger"; }
        Ice::LoggerPtr cloneWithPrefix(string) final { return shared_from_this(); }
    };
}

class Server : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Server::run(int argc, char** argv)
{
    Ice::InitializationData initData;
    initData.properties = createTestProperties(argc, argv);
    initData.properties->setProperty("Ice.Warn.Connections", "0");
    initData.logger = std::make_shared<NullLogger>();
    Ice::CommunicatorHolder communicator = initialize(initData);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::Identity id = Ice::stringToIdentity("communicator");
    adapter->add(std::make_shared<RemoteCommunicatorI>(), id);
    adapter->activate();

    serverReady();

    // Disable ready print for further adapters.
    communicator->getProperties()->setProperty("Ice.PrintAdapterReady", "0");

    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
