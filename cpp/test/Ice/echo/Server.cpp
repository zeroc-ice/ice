// Copyright (c) ZeroC, Inc.

#include "BlobjectI.h"
#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

using namespace std;

class EchoI : public Test::Echo
{
public:
    EchoI(BlobjectIPtr blob) : _blob(std::move(blob)) {}

    void setConnection(const Ice::Current& current) override { _blob->setConnection(current.con); }

    void startBatch(const Ice::Current&) override { _blob->startBatch(); }

    void flushBatch(const Ice::Current&) override { _blob->flushBatch(); }

    void shutdown(const Ice::Current& current) override { current.adapter->getCommunicator()->shutdown(); }

private:
    BlobjectIPtr _blob;
};

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
    BlobjectIPtr blob = make_shared<BlobjectI>();
    adapter->addDefaultServant(blob, "");
    adapter->add(make_shared<EchoI>(blob), Ice::Identity{.name = "__echo"});
    adapter->activate();
    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
