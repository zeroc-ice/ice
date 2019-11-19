//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <Glacier2/Router.h>
#include <TestHelper.h>
#include <CallbackI.h>
#include <mutex>
#include <random>

using namespace std;
using namespace std::chrono_literals;
using namespace Ice;
using namespace Test;

static Ice::InitializationData initData;

class AsyncCallback final
{
public:

    void
    response(int val)
    {
        {
            lock_guard<mutex> lg(_mutex);
            _haveResponse = true;
            _response = val;
        }
        _condVar.notify_one();
    }

    void
    error(exception_ptr e)
    {
        {
            lock_guard<mutex> lg(_mutex);
            _haveResponse = true;
            _exception = e;
        }
        _condVar.notify_one();
    }

    int
    waitResponse()
    {
        unique_lock<mutex> lock(_mutex);
        while(!_haveResponse)
        {
            _condVar.wait(lock);
        }
        if(_exception)
        {
            rethrow_exception(_exception);
        }
        return _response;
    }

private:

    bool _haveResponse = false;
    exception_ptr _exception = nullptr;
    int _response = -1;
    mutex _mutex;
    condition_variable _condVar;
};

class MisbehavedClient final
{
public:

    explicit MisbehavedClient(int id) : _id(id)
    {
    }

    void run()
    {
        auto communicator = initialize(initData);
        auto routerBase = communicator->stringToProxy(
            "Glacier2/router:" + TestHelper::getTestEndpoint(communicator->getProperties(), 50));
        auto router = checkedCast<Glacier2::RouterPrx>(routerBase);
        communicator->setDefaultRouter(router);

        ostringstream os;
        os << "userid-" << _id;
        auto session = router->createSession(os.str(), "abc123");
        communicator->getProperties()->setProperty("Ice.PrintAdapterReady", "");
        auto adapter = communicator->createObjectAdapterWithRouter("CallbackReceiverAdapter", router);
        adapter->activate();

        string category = router->getCategoryForClient();
        {
            lock_guard<mutex> lg(_mutex);
            _callbackReceiver = make_shared<CallbackReceiverI>();
        }
        _condVar.notify_one();

        Identity ident = {"callbackReceiver", category};
        auto receiver = uncheckedCast<CallbackReceiverPrx>(adapter->add(_callbackReceiver, ident));

        auto base = communicator->stringToProxy(
            "c1/callback:" + TestHelper::getTestEndpoint(communicator->getProperties()));
        base = base->ice_oneway();
        auto callback = uncheckedCast<CallbackPrx>(base);

        //
        // Block the CallbackReceiver in wait() to prevent the client from
        // processing other incoming calls and wait to receive the callback.
        //
        callback->initiateWaitCallback(receiver);
        _callbackReceiver->waitCallbackOK();

        //
        // Notify the main thread that the callback was received.
        //
        {
            lock_guard<mutex> lg(_mutex);
            _callback = true;
        }
        _condVar.notify_one();

        //
        // Callback the client with a large payload. This should cause
        // the Glacier2 request queue thread to block trying to send the
        // callback to the client because the client is currently blocked
        // in CallbackReceiverI::waitCallback() and can't process more
        // requests.
        //
        callback->initiateCallbackWithPayload(receiver);
        _callbackReceiver->callbackWithPayloadOK();

        try
        {
            router->destroySession();
        }
        catch(const Ice::LocalException&)
        {
            test(false);
        }
        communicator->destroy();
    }

    void
    notifyWaitCallback()
    {
        _callbackReceiver->notifyWaitCallback();
    }

    void
    waitForCallback()
    {
        unique_lock<mutex> lock(_mutex);
        while(!_callback)
        {
            _condVar.wait(lock);
        }
    }

private:

    int _id;
    shared_ptr<CallbackReceiverI> _callbackReceiver;
    bool _callback = false;
    mutex _mutex;
    condition_variable _condVar;
};

class StressClient
{
public:

    explicit StressClient(int id) : _id(id)
    {
    }

    void run()
    {
        auto communicator = initialize(initData);
        auto routerBase = communicator->stringToProxy(
            "Glacier2/router:" + TestHelper::getTestEndpoint(communicator->getProperties(), 50));
        _router = checkedCast<Glacier2::RouterPrx>(routerBase);
        communicator->setDefaultRouter(_router);

        ostringstream os;
        os << "userid-" << _id;
        auto session = _router->createSession(os.str(), "abc123");
        communicator->getProperties()->setProperty("Ice.PrintAdapterReady", "");
        auto adapter = communicator->createObjectAdapterWithRouter("CallbackReceiverAdapter", _router);
        adapter->activate();

        string category = _router->getCategoryForClient();
        _callbackReceiver = make_shared<CallbackReceiverI>();
        Identity ident = {"callbackReceiver", category};
        auto receiver = uncheckedCast<CallbackReceiverPrx>(adapter->add(_callbackReceiver, ident));

        auto base = communicator->stringToProxy(
            "c1/callback:" + TestHelper::getTestEndpoint(communicator->getProperties()));
        base = base->ice_oneway();
        auto callback = uncheckedCast<CallbackPrx>(base);

        {
            lock_guard<mutex> lg(_mutex);
            _initialized = true;
        }
        _condVar.notify_all();

        {
            unique_lock<mutex> lock(_mutex);
            while(!_notified)
            {
                _condVar.wait(lock);
            }
        }

        //
        // Stress the router until the connection is closed.
        //
        stress(move(callback), move(receiver));
        communicator->destroy();
    }

    virtual void stress(shared_ptr<CallbackPrx> callback, shared_ptr<CallbackReceiverPrx>) = 0;

    void
    notifyThread()
    {
        {
            unique_lock<mutex> lock(_mutex);
            while(!_initialized)
            {
                _condVar.wait(lock);
            }
            _notified = true;
        }
        _condVar.notify_one();
    }

    void
    kill()
    {
        try
        {
            _router->destroySession();
        }
        catch(const Ice::ConnectionLostException&)
        {
            // Expected if the thread invokes shortly after the session is destroyed.
            // In this case, Glacier2 closes forcefully the connection.
        }
        catch(const Ice::CommunicatorDestroyedException&)
        {
        }
        catch(const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
    }

protected:

    shared_ptr<Glacier2::RouterPrx> _router;
    int _id;
    shared_ptr<CallbackReceiverI> _callbackReceiver;
    bool _initialized = false;
    bool _notified = false;
    mutex _mutex;
    condition_variable _condVar;
};

class PingStressClient final : public StressClient
{
public:

    explicit PingStressClient(int id) : StressClient(id)
    {
    }

    void
    stress(shared_ptr<CallbackPrx> callback, shared_ptr<CallbackReceiverPrx>) override
    {
        try
        {
            auto cb = callback->ice_twoway();
            Context context;
            context["_fwd"] = "t";
            while(true)
            {
                cb->ice_ping(context);
                this_thread::sleep_for(1ms);
            }
        }
        catch(const Ice::ConnectionLostException&)
        {
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }
        catch(const Ice::CommunicatorDestroyedException&)
        {
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }
    }
};

class CallbackStressClient final : public StressClient
{
public:

    explicit CallbackStressClient(int id) : StressClient(id)
    {
    }

    void
    stress(shared_ptr<CallbackPrx> callback, shared_ptr<CallbackReceiverPrx> receiver) override
    {
        try
        {
            auto cb = callback->ice_twoway();
            Context context;
            context["_fwd"] = "t";
            while(true)
            {
                cb->initiateCallback(receiver, context);
                _callbackReceiver->callbackOK();
                this_thread::sleep_for(1ms);
            }
        }
        catch(const Ice::ConnectionLostException&)
        {
            // Session was destroyed.
        }
        catch(const Ice::ObjectNotExistException&)
        {
            // This might be raised by the CallbackI implementation if it can't invoke on the
            // callback receiver because the session is being destroyed concurrently.
        }
        catch(const Ice::CommunicatorDestroyedException&)
        {
            // This might happen if the retry fails because the communicator is destroyed.
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }
    }
};

class CallbackWithPayloadStressClient final : public StressClient
{
public:

    explicit CallbackWithPayloadStressClient(int id) : StressClient(id)
    {
    }

    void
    stress(shared_ptr<CallbackPrx> callback, shared_ptr<CallbackReceiverPrx> receiver) override
    {
        try
        {
            auto cb = callback->ice_twoway();
            Context context;
            context["_fwd"] = "t";
            while(true)
            {
                cb->initiateCallbackWithPayload(receiver, context);
                _callbackReceiver->callbackWithPayloadOK();
                this_thread::sleep_for(10ms);
            }
        }
        catch(const Ice::ConnectionLostException&)
        {
            // Session was destroyed.
        }
        catch(const Ice::ObjectNotExistException&)
        {
            // This might be raised by the CallbackI implementation if it can't invoke on the
            // callback receiver because the session is being destroyed concurrently.
        }
        catch(const Ice::CommunicatorDestroyedException&)
        {
            // This might happen if the retry fails because the communicator is destroyed.
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }
    }
};

class CallbackClient final : public Test::TestHelper
{
public:

    void run(int, char**) override;
};

void
CallbackClient::run(int argc, char** argv)
{
    //
    // We must disable connection warnings, because we attempt to ping
    // the router before session establishment, as well as after
    // session destruction. Both will cause a ConnectionLostException.
    //
    initData.properties = createTestProperties(argc, argv);
    initData.properties->setProperty("Ice.Warn.Connections", "0");

    Ice::CommunicatorHolder communicator = initialize(argc, argv, initData);
    shared_ptr<ObjectPrx> routerBase;
    {
        cout << "testing stringToProxy for router... " << flush;
        routerBase = communicator->stringToProxy("Glacier2/router:" + getTestEndpoint(50));
        cout << "ok" << endl;
    }

    shared_ptr<Glacier2::RouterPrx> router;

    {
        cout << "testing checked cast for router... " << flush;
        router = checkedCast<Glacier2::RouterPrx>(routerBase);
        test(router);
        cout << "ok" << endl;
    }

    {
        cout << "testing router finder... " << flush;
        auto finder =
            uncheckedCast<RouterFinderPrx>(communicator->stringToProxy("Ice/RouterFinder:" + getTestEndpoint(50)));
        test(finder->getRouter()->ice_getIdentity() == router->ice_getIdentity());
        cout << "ok" << endl;
    }

    {
        cout << "installing router with communicator... " << flush;
        communicator->setDefaultRouter(router);
        cout << "ok" << endl;
    }

    {
        cout << "getting the session timeout... " << flush;
        auto sessionTimeout = router->getSessionTimeout();
        auto acmTimeout = router->getACMTimeout();
        test(sessionTimeout == 30 && sessionTimeout == acmTimeout);
        cout << "ok" << endl;
    }

    shared_ptr<ObjectPrx> base;

    {
        cout << "testing stringToProxy for server object... " << flush;
        base = communicator->stringToProxy("c1/callback:" + getTestEndpoint());
        cout << "ok" << endl;
    }

    {
        cout << "trying to ping server before session creation... " << flush;
        try
        {
            base->ice_ping();
            test(false);
        }
        catch(const ConnectionLostException&)
        {
            cout << "ok" << endl;
        }
    }

    shared_ptr<Glacier2::SessionPrx> session;

    {
        cout << "trying to create session with wrong password... " << flush;
        try
        {
            session = router->createSession("userid", "xxx");
            test(false);
        }
        catch(const Glacier2::PermissionDeniedException&)
        {
            cout << "ok" << endl;
        }
    }

    {
        cout << "trying to destroy non-existing session... " << flush;
        try
        {
            router->destroySession();
            test(false);
        }
        catch(const Glacier2::SessionNotExistException&)
        {
            cout << "ok" << endl;
        }
    }

    {
        cout << "creating session with correct password... " << flush;
        try
        {
            session = router->createSession("userid", "abc123");
            cout << "ok" << endl;
        }
        catch(const Glacier2::PermissionDeniedException& ex)
        {
            cerr << ex << ":\n" << ex.reason << endl;
            test(false);
        }
    }

    {
        cout << "trying to create a second session... " << flush;
        try
        {
            router->createSession("userid", "abc123");
            test(false);
        }
        catch(const Glacier2::CannotCreateSessionException&)
        {
            cout << "ok" << endl;
        }
    }

    {
        cout << "pinging server after session creation... " << flush;
        base->ice_ping();
        cout << "ok" << endl;
    }

    {
        cout << "pinging object with client endpoint... " << flush;
        auto baseC = communicator->stringToProxy("collocated:" + getTestEndpoint(50));
        try
        {
            baseC->ice_ping();
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }
        cout << "ok" << endl;
    }

    shared_ptr<CallbackPrx> twoway;

    {
        cout << "testing checked cast for server object... " << flush;
        twoway = checkedCast<CallbackPrx>(base);
        test(twoway);
        cout << "ok" << endl;
    }

    shared_ptr<ObjectAdapter> adapter;

    {
        cout << "creating and activating callback receiver adapter with router... " << flush;
        communicator->getProperties()->setProperty("Ice.PrintAdapterReady", "0");
        adapter = communicator->createObjectAdapterWithRouter("CallbackReceiverAdapter", router);
        adapter->activate();
        cout << "ok" << endl;
    }

    string category;

    {
        cout << "getting category from router... " << flush;
        category = router->getCategoryForClient();
        cout << "ok" << endl;
    }

    shared_ptr<CallbackReceiverI> callbackReceiver;
    shared_ptr<CallbackReceiverPrx> twowayR;
    shared_ptr<CallbackReceiverPrx> fakeTwowayR;

    {
        cout << "creating and adding callback receiver object... " << flush;
        callbackReceiver = make_shared<CallbackReceiverI>();
        Identity callbackReceiverIdent = {"callbackReceiver", category};
        twowayR = uncheckedCast<CallbackReceiverPrx>(adapter->add(callbackReceiver, callbackReceiverIdent));
        Identity fakeCallbackReceiverIdent = {"callbackReceiver", "dummy"};
        fakeTwowayR = uncheckedCast<CallbackReceiverPrx>(adapter->add(callbackReceiver, fakeCallbackReceiverIdent));
        cout << "ok" << endl;
    }

    {
        cout << "testing oneway callback... " << flush;
        auto oneway = twoway->ice_oneway();
        auto onewayR = twowayR->ice_oneway();
        Context context;
        context["_fwd"] = "o";
        oneway->initiateCallback(onewayR, context);
        oneway->initiateCallback(onewayR, context);
        oneway->initiateCallback(onewayR, context);
        oneway->initiateCallback(onewayR, context);
        callbackReceiver->callbackOK(4);
        cout << "ok" << endl;
    }

    {
        cout << "testing twoway callback... " << flush;
        Context context;
        context["_fwd"] = "t";
        twoway->initiateCallback(twowayR, context);
        twoway->initiateCallback(twowayR, context);
        twoway->initiateCallback(twowayR, context);
        twoway->initiateCallback(twowayR, context);
        callbackReceiver->callbackOK(4);
        cout << "ok" << endl;
    }

    {
        cout << "testing batch oneway callback... " << flush;
        Context context;
        context["_fwd"] = "O";
        auto batchOneway = twoway->ice_batchOneway();
        auto onewayR = twowayR->ice_oneway();
        batchOneway->initiateCallback(onewayR, context);
        batchOneway->initiateCallback(onewayR, context);
        batchOneway->initiateCallback(onewayR, context);
        batchOneway->initiateCallback(onewayR, context);
        batchOneway->initiateCallback(onewayR, context);
        batchOneway->initiateCallback(onewayR, context);
        batchOneway->ice_flushBatchRequests();
        callbackReceiver->callbackOK(6);
        cout << "ok" << endl;
    }

    //
    // Send 3 twoway request to callback the receiver. The callback
    // receiver only reply to the callback once it received the 3
    // callbacks. This test ensures that Glacier2 doesn't serialize
    // twoway requests (see bug 337 for more information).
    //
    {
        cout << "testing concurrent twoway callback... " << flush;
        Context context;
        context["_fwd"] = "t";
        AsyncCallback cb0;
        twoway->initiateConcurrentCallbackAsync(0, twowayR,
                                                [&cb0](int val){ cb0.response(val); },
                                                [&cb0](exception_ptr e){ cb0.error(e); },
                                                nullptr, context);

        AsyncCallback cb1;
        twoway->initiateConcurrentCallbackAsync(1, twowayR,
                                                [&cb1](int val){ cb1.response(val); },
                                                [&cb1](exception_ptr e){ cb1.error(e); },
                                                nullptr, context);

        AsyncCallback cb2;
        twoway->initiateConcurrentCallbackAsync(2, twowayR,
                                                [&cb2](int val){ cb2.response(val); },
                                                [&cb2](exception_ptr e){ cb2.error(e); },
                                                nullptr, context);

        callbackReceiver->answerConcurrentCallbacks(3);
        test(cb0.waitResponse() == 0);
        test(cb1.waitResponse() == 1);
        test(cb2.waitResponse() == 2);
        cout << "ok" << endl;
    }

    {
        cout << "ditto, but with user exception... " << flush;
        Context context;
        context["_fwd"] = "t";
        try
        {
            twoway->initiateCallbackEx(twowayR, context);
            test(false);
        }
        catch(const CallbackException& ex)
        {
            test(ex.someValue == 3.14);
            test(ex.someString == "3.14");
        }
        callbackReceiver->callbackOK();
        cout << "ok" << endl;
    }

    {
        cout << "trying twoway callback with fake category... " << flush;
        Context context;
        context["_fwd"] = "t";
        try
        {
            twoway->initiateCallback(fakeTwowayR, context);
            test(false);
        }
        catch(const ObjectNotExistException&)
        {
            cout << "ok" << endl;
        }
    }

    {
        cout << "testing whether other allowed category is accepted... " << flush;
        Context context;
        context["_fwd"] = "t";
        auto otherCategoryTwoway = uncheckedCast<CallbackPrx>(
            twoway->ice_identity(stringToIdentity("c2/callback")));
        otherCategoryTwoway->initiateCallback(twowayR, context);
        callbackReceiver->callbackOK();
        cout << "ok" << endl;
    }

    {
        cout << "testing whether disallowed category gets rejected... " << flush;
        Context context;
        context["_fwd"] = "t";
        try
        {
            auto otherCategoryTwoway = uncheckedCast<CallbackPrx>(
                twoway->ice_identity(stringToIdentity("c3/callback")));
            otherCategoryTwoway->initiateCallback(twowayR, context);
            test(false);
        }
        catch(const ObjectNotExistException&)
        {
            cout << "ok" << endl;
        }
    }

    {
        cout << "testing whether user-id as category is accepted... " << flush;
        Context context;
        context["_fwd"] = "t";
        auto otherCategoryTwoway = uncheckedCast<CallbackPrx>(
            twoway->ice_identity(stringToIdentity("_userid/callback")));
        otherCategoryTwoway->initiateCallback(twowayR, context);
        callbackReceiver->callbackOK();
        cout << "ok" << endl;
    }

    {
        cout << "testing with blocking clients... " << flush;

        //
        // Start 3 misbehaving clients.
        //
        MisbehavedClient clients[] = {MisbehavedClient(0), MisbehavedClient(1), MisbehavedClient(2)};
        std::future<void> futures[3] = {};
        const int nClients = 3; // Passwords need to be added to the password file if more clients are needed.
        for(int i = 0; i < nClients; ++i)
        {
            auto& client = clients[i];
            futures[i] = std::async(launch::async, [&client]{ client.run(); });
            client.waitForCallback();
        }

        //
        // Sleep for one second to make sure the router starts sending
        // the callback with the payload to the clients.
        //
        this_thread::sleep_for(1s);

        //
        // Initiate few callbacks with a large payload. Because of
        // the buffered mode, this shouldn't block even though the
        // misbehaved client are not answering their callback
        // requests.
        //
        Context context;
        context["_fwd"] = "t";
        twoway->initiateCallbackWithPayload(twowayR, context);
        callbackReceiver->callbackWithPayloadOK();
        twoway->initiateCallbackWithPayload(twowayR, context);
        callbackReceiver->callbackWithPayloadOK();
        twoway->initiateCallbackWithPayload(twowayR, context);
        callbackReceiver->callbackWithPayloadOK();
        twoway->initiateCallbackWithPayload(twowayR, context);
        callbackReceiver->callbackWithPayloadOK();

        for(int i = 0; i < nClients; ++i)
        {
            clients[i].notifyWaitCallback();
            futures[i].get();
        }

        cout << "ok" << endl;
    }

    {
        cout << "stress test... " << flush;
        const int nClients = 3; // Passwords need to be added to the password file if more clients are needed.
        vector<shared_ptr<StressClient>> clients;
        vector<future<void>> futures;
        random_device rd;
        for(int i = 0; i < nClients; ++i)
        {
            switch(rd() % 3)
            {
            case 0:
                clients.push_back(make_shared<PingStressClient>(i));
                break;
            case 1:
                clients.push_back(make_shared<CallbackStressClient>(i));
                break;
            case 2:
                clients.push_back(make_shared<CallbackWithPayloadStressClient>(i));
                break;
            default:
                assert(false);
                break;
            }
            auto client = clients.back();
            futures.push_back(std::async(launch::async, [client = move(client)]{ client->run(); }));
        }
        for(const auto& p: clients)
        {
            p->notifyThread();
        }

        //
        // Let the stress client run for a bit.
        //
        this_thread::sleep_for(3s);

        //
        // Send some callbacks.
        //
        Context context;
        context["_fwd"] = "t";
        twoway->initiateCallback(twowayR);
        callbackReceiver->callbackOK();

        //
        // Kill the stress clients.
        //
        for(const auto& p: clients)
        {
            p->kill();
        }
        for(auto& f: futures)
        {
            f.get();
        }

        cout << "ok" << endl;
    }

    if(argc >= 2 && strcmp(argv[1], "--shutdown") == 0)
    {
        cout << "testing server shutdown... " << flush;
        twoway->shutdown();
        // No ping, otherwise the router prints a warning message if it's
        // started with --Ice.Warn.Connections.
        cout << "ok" << endl;
        /*
          try
          {
          base->ice_ping();
          test(false);
          }
          // If we use the glacier router, the exact exception reason gets
          // lost.
          catch(const UnknownLocalException&)
          {
          cout << "ok" << endl;
          }
        */
    }

    {
        cout << "destroying session... " << flush;
        try
        {
            router->destroySession();
        }
        catch(const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        cout << "ok" << endl;
    }

    {
        cout << "trying to ping server after session destruction... " << flush;
        try
        {
            base->ice_ping();
            test(false);
        }
        catch(const ConnectionLostException&)
        {
            cout << "ok" << endl;
        }
    }

    if(argc >= 2 && strcmp(argv[1], "--shutdown") == 0)
    {
        {
            cout << "uninstalling router with communicator... " << flush;
            communicator->setDefaultRouter(0);
            cout << "ok" << endl;
        }

        shared_ptr<ObjectPrx> processBase;

        {
            cout << "testing stringToProxy for admin process facet... " << flush;
            processBase = communicator->stringToProxy("Glacier2/admin -f Process:" + getTestEndpoint(51));
            cout << "ok" << endl;
        }

        shared_ptr<Ice::ProcessPrx> process;

        {
            cout << "testing checked cast for process facet... " << flush;
            process = checkedCast<Ice::ProcessPrx>(processBase);
            test(process);
            cout << "ok" << endl;
        }

        cout << "testing Glacier2 shutdown... " << flush;
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
