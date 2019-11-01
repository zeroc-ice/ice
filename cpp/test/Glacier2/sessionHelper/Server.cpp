//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Callback.h>

using namespace std;
using namespace Test;

namespace
{

class CallbackI final : public Callback
{
public:

    void
    initiateCallback(shared_ptr<CallbackReceiverPrx> proxy, const Ice::Current& current) override
    {
        proxy->callback(current.ctx);
    }

    void
    initiateCallbackEx(shared_ptr<CallbackReceiverPrx> proxy, const Ice::Current& current) override
    {
        proxy->callbackEx(current.ctx);
    }

    void
    shutdown(const Ice::Current& current) override
    {
        current.adapter->getCommunicator()->shutdown();
    }
};

}

class Server final : public Test::TestHelper
{
public:

    void run(int, char**) override;
};

void
Server::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    communicator->getProperties()->setProperty("DeactivatedAdapter.Endpoints", getTestEndpoint(1));
    communicator->createObjectAdapter("DeactivatedAdapter");

    communicator->getProperties()->setProperty("CallbackAdapter.Endpoints", getTestEndpoint());
    auto adapter = communicator->createObjectAdapter("CallbackAdapter");
    adapter->add(ICE_MAKE_SHARED(CallbackI), Ice::stringToIdentity("callback"));
    adapter->activate();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
