// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Callback.h>

using namespace std;
using namespace Test;

namespace
{

class CallbackI : public Callback
{

public:

    virtual void
    initiateCallback(ICE_IN(CallbackReceiverPrxPtr) proxy, const Ice::Current& current)
    {
        proxy->callback(current.ctx);
    }

    virtual void
    shutdown(const Ice::Current& current)
    {
        current.adapter->getCommunicator()->shutdown();
    }
};

}

class Server : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Server::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    communicator->getProperties()->setProperty("DeactivatedAdapter.Endpoints", getTestEndpoint(1));
    communicator->createObjectAdapter("DeactivatedAdapter");

    communicator->getProperties()->setProperty("CallbackAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("CallbackAdapter");
    adapter->add(ICE_MAKE_SHARED(CallbackI), Ice::stringToIdentity("callback"));
    adapter->activate();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
