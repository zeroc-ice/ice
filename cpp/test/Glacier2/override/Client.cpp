//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <Glacier2/Router.h>
#include <TestHelper.h>
#include <CallbackI.h>
#include <chrono>

using namespace std;
using namespace std::chrono_literals;

using namespace Ice;
using namespace Test;

class CallbackClient final : public Test::TestHelper
{
public:

    void run(int, char**) override;
};

void
CallbackClient::run(int argc, char** argv)
{
    auto properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.Warn.Connections", "0");
    properties->setProperty("Ice.ThreadPool.Client.Serialize", "1");

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    auto routerBase = communicator->stringToProxy("Glacier2/router:" + getTestEndpoint(50));
    auto router = checkedCast<Glacier2::RouterPrx>(routerBase);
    communicator->setDefaultRouter(router);

    auto base = communicator->stringToProxy("c/callback:" + getTestEndpoint());
    auto session = router->createSession("userid", "abc123");
    base->ice_ping();

    auto twoway = checkedCast<CallbackPrx>(base);
    auto oneway = twoway->ice_oneway();
    auto batchOneway = twoway->ice_batchOneway();

    communicator->getProperties()->setProperty("Ice.PrintAdapterReady", "0");
    auto adapter = communicator->createObjectAdapterWithRouter("CallbackReceiverAdapter", router);
    adapter->activate();

    string category = router->getCategoryForClient();

    auto callbackReceiver = make_shared<CallbackReceiverI>();

    Identity callbackReceiverIdent;
    callbackReceiverIdent.name = "callbackReceiver";
    callbackReceiverIdent.category = category;
    auto twowayR =
        uncheckedCast<CallbackReceiverPrx>(adapter->add(callbackReceiver, callbackReceiverIdent));
    auto onewayR = twowayR->ice_oneway();

    {
        cout << "testing client request override... " << flush;
        {
            for(int i = 0; i < 5; i++)
            {
                oneway->initiateCallback(twowayR, 0);
                oneway->initiateCallback(twowayR, 0);
                callbackReceiver->callbackOK(2, 0);
            }
        }

        {
            Ice::Context ctx;
            ctx["_ovrd"] = "test";
            for(int i = 0; i < 5; i++)
            {
                oneway->initiateCallback(twowayR, i, ctx);
                oneway->initiateCallback(twowayR, i, ctx);
                oneway->initiateCallback(twowayR, i, ctx);
                this_thread::sleep_for(100ms);
                test(callbackReceiver->callbackOK(1, i) < 3);
            }
        }
        cout << "ok" << endl;
    }

    {
        cout << "testing server request override... " << flush;
        Ice::Context ctx;
        ctx["serverOvrd"] = "test";
        for(int i = 0; i < 5; i++)
        {
            oneway->initiateCallback(onewayR, i, ctx);
            oneway->initiateCallback(onewayR, i, ctx);
            oneway->initiateCallback(onewayR, i, ctx);
            this_thread::sleep_for(100ms);
            test(callbackReceiver->callbackOK(1, i) < 3);
        }
        oneway->initiateCallback(twowayR, 0);
        test(callbackReceiver->callbackOK(1, 0) == 0);

        int count = 0;
        int nRetry = 0;
        do
        {
            callbackReceiver->hold();
            oneway->initiateCallbackWithPayload(onewayR, ctx);
            oneway->initiateCallbackWithPayload(onewayR, ctx);
            oneway->initiateCallbackWithPayload(onewayR, ctx);
            oneway->initiateCallbackWithPayload(onewayR, ctx);
            oneway->initiateCallbackWithPayload(onewayR, ctx);
            oneway->initiateCallbackWithPayload(onewayR, ctx);
            oneway->initiateCallbackWithPayload(onewayR, ctx);
            oneway->initiateCallbackWithPayload(onewayR, ctx);
            oneway->initiateCallbackWithPayload(onewayR, ctx);
            oneway->initiateCallbackWithPayload(onewayR, ctx);
            oneway->initiateCallback(twowayR, 0);
            this_thread::sleep_for(chrono::milliseconds(200 + nRetry * 200));
            callbackReceiver->activate();
            test(callbackReceiver->callbackOK(1, 0) == 0);
            count = callbackReceiver->callbackWithPayloadOK(0);
            callbackReceiver->callbackWithPayloadOK(count);
        }
        while(count == 10 && nRetry++ < 10);
        test(count < 10);

        oneway->initiateCallbackWithPayload(twowayR);
        oneway->initiateCallbackWithPayload(twowayR);
        callbackReceiver->hold();
        oneway->initiateCallbackWithPayload(twowayR);
        oneway->initiateCallback(onewayR, 0, ctx);
        oneway->initiateCallback(onewayR, 0, ctx);
        oneway->initiateCallback(onewayR, 0, ctx);
        oneway->initiateCallback(onewayR, 0, ctx);
        oneway->initiateCallback(onewayR, 0, ctx);
        oneway->initiateCallbackWithPayload(twowayR);
        this_thread::sleep_for(1000ms);
        callbackReceiver->activate();
        test(callbackReceiver->callbackWithPayloadOK(4) == 0);

        int remainingCallbacks = callbackReceiver->callbackOK(1, 0);
        //
        // Occasionally, Glacier2 flushes in the middle of our 5
        // callbacks, so we get more than 1 callback
        // (in theory we could get up to 5 total - more than 1 extra is extremely unlikely)
        //
        // The sleep above is also important as we want to have enough
        // time to receive this (these) extra callback(s).
        //
        test(remainingCallbacks <= 4);
        if(remainingCallbacks > 0)
        {
            test(callbackReceiver->callbackOK(remainingCallbacks, 0) == 0);
        }

        ctx["_fwd"] = "O";

        oneway->initiateCallbackWithPayload(twowayR);
        callbackReceiver->hold();
        oneway->initiateCallbackWithPayload(twowayR);
        oneway->initiateCallback(onewayR, 0, ctx);
        oneway->initiateCallback(onewayR, 0, ctx);
        oneway->initiateCallback(onewayR, 0, ctx);
        oneway->initiateCallback(onewayR, 0, ctx);
        oneway->initiateCallback(onewayR, 0, ctx);
        oneway->initiateCallbackWithPayload(twowayR);
        this_thread::sleep_for(200ms);
        callbackReceiver->activate();
        test(callbackReceiver->callbackWithPayloadOK(3) == 0);
        remainingCallbacks = callbackReceiver->callbackOK(1, 0);
        // Unlikely but sometime we get more than just one callback if the flush
        // occurs in the middle of our 5 callbacks.
        test(remainingCallbacks <= 3);
        if(remainingCallbacks > 0)
        {
            test(callbackReceiver->callbackOK(remainingCallbacks, 0) == 0);
        }

        cout << "ok" << endl;
    }

    {
        cout << "shutdown... " << flush;
        twoway->shutdown();

        try
        {
            router->destroySession();
        }
        catch(const Ice::LocalException&)
        {
            test(false);
        }

        communicator->setDefaultRouter(0);
        auto processBase = communicator->stringToProxy("Glacier2/admin -f Process:" + getTestEndpoint(51));
        auto process = checkedCast<Ice::ProcessPrx>(processBase);
        process->shutdown();
        try
        {
            process->ice_ping();
            test(false);
        }
        catch(const Ice::LocalException&)
        {
            cout << "ok" << endl;
        }
    }
}

DEFINE_TEST(CallbackClient)
