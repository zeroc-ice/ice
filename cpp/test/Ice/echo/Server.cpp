// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <BlobjectI.h>
#include <Test.h>

using namespace std;

class EchoI : public Test::Echo
{
public:

    EchoI(const BlobjectIPtr& blob) :
        _blob(blob)
    {
    }

    virtual void setConnection(const Ice::Current& current)
    {
        _blob->setConnection(current.con);
    }

    virtual void startBatch(const Ice::Current&)
    {
        _blob->startBatch();
    }

    virtual void flushBatch(const Ice::Current&)
    {
        _blob->flushBatch();
    }

    virtual void shutdown(const Ice::Current& current)
    {
        current.adapter->getCommunicator()->shutdown();
    }

private:

    BlobjectIPtr _blob;
};

class Server : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Server::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    BlobjectIPtr blob = ICE_MAKE_SHARED(BlobjectI);
    adapter->addDefaultServant(blob, "");
    adapter->add(ICE_MAKE_SHARED(EchoI, blob), Ice::stringToIdentity("__echo"));
    adapter->activate();
    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
