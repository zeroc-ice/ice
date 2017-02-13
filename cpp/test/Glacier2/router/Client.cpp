// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Random.h>
#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <Glacier2/Router.h>
#include <TestCommon.h>
#include <CallbackI.h>

using namespace std;
using namespace Ice;
using namespace Test;

static Ice::InitializationData initData;

class AsyncCallback : public IceUtil::Monitor<IceUtil::Mutex>, public IceUtil::Shared
{
public:

    AsyncCallback() :
        _haveResponse(false)
    {
    }

    void
    response(Int response)
    {
        Lock sync(*this);
        _haveResponse = true;
        _response = response;
        notify();
    }

    void
    exception(const Exception& e)
    {
        Lock sync(*this);
        _haveResponse = true;
        _ex.reset(e.ice_clone());
        notify();
    }

    int
    waitResponse() const
    {
        Lock sync(*this);
        while(!_haveResponse)
        {
            wait();
        }
        if(_ex.get())
        {
            _ex->ice_throw();
        }
        return _response;
    }

private:

    bool _haveResponse;
    IceInternal::UniquePtr<Exception> _ex;
    Int _response;
};
typedef IceUtil::Handle<AsyncCallback> AsyncCallbackPtr;

class MisbehavedClient : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    MisbehavedClient(int id) : _id(id), _callback(false)
    {
    }

    virtual
    void run()
    {
        CommunicatorPtr communicator = initialize(initData);
        ObjectPrx routerBase = communicator->stringToProxy("Glacier2/router:" + getTestEndpoint(communicator, 10));
        Glacier2::RouterPrx router = Glacier2::RouterPrx::checkedCast(routerBase);
        communicator->setDefaultRouter(router);

        ostringstream os;
        os << "userid-" << _id;
        Glacier2::SessionPrx session = router->createSession(os.str(), "abc123");
        communicator->getProperties()->setProperty("Ice.PrintAdapterReady", "");
        ObjectAdapterPtr adapter = communicator->createObjectAdapterWithRouter("CallbackReceiverAdapter", router);
        adapter->activate();

        //
        // Verify that the generated end_ method is exported properly - see bug 4719.
        //
        Ice::AsyncResultPtr r = router->begin_getCategoryForClient();
        string category = router->end_getCategoryForClient(r);
        {
            Lock sync(*this);
            _callbackReceiver = new CallbackReceiverI;
            notify();
        }

        Identity ident;
        ident.name = "callbackReceiver";
        ident.category = category;
        CallbackReceiverPrx receiver = CallbackReceiverPrx::uncheckedCast(adapter->add(_callbackReceiver, ident));

        ObjectPrx base = communicator->stringToProxy("c1/callback:" + getTestEndpoint(communicator, 0));
        base = base->ice_oneway();
        CallbackPrx callback = CallbackPrx::uncheckedCast(base);


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
            Lock sync(*this);
            _callback = true;
            notify();
        }

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
        {
            Lock sync(*this);
            while(!_callback)
            {
                wait();
            }
        }
    }

private:

    int _id;
    CallbackReceiverIPtr _callbackReceiver;
    bool _callback;
};
typedef IceUtil::Handle<MisbehavedClient> MisbehavedClientPtr;

class StressClient : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    StressClient(int id) : _id(id), _initialized(false), _notified(false)
    {
    }

    virtual
    void run()
    {
        CommunicatorPtr communicator = initialize(initData);
        ObjectPrx routerBase = communicator->stringToProxy("Glacier2/router:" + getTestEndpoint(communicator, 10));
        _router = Glacier2::RouterPrx::checkedCast(routerBase);
        communicator->setDefaultRouter(_router);

        ostringstream os;
        os << "userid-" << _id;
        Glacier2::SessionPrx session = _router->createSession(os.str(), "abc123");
        communicator->getProperties()->setProperty("Ice.PrintAdapterReady", "");
        ObjectAdapterPtr adapter = communicator->createObjectAdapterWithRouter("CallbackReceiverAdapter", _router);
        adapter->activate();

        string category = _router->getCategoryForClient();
        _callbackReceiver = new CallbackReceiverI;
        Identity ident;
        ident.name = "callbackReceiver";
        ident.category = category;
        CallbackReceiverPrx receiver = CallbackReceiverPrx::uncheckedCast(adapter->add(_callbackReceiver, ident));

        ObjectPrx base = communicator->stringToProxy("c1/callback:" + getTestEndpoint(communicator, 0));
        base = base->ice_oneway();
        CallbackPrx callback = CallbackPrx::uncheckedCast(base);

        {
            Lock sync(*this);
            _initialized = true;
            notifyAll();
        }
        {
            Lock sync(*this);
            while(!_notified)
            {
                wait();
            }
        }

        //
        // Stress the router until the connection is closed.
        //
        stress(callback, receiver);
        communicator->destroy();
    }

    virtual void stress(const CallbackPrx& callback, const CallbackReceiverPrx&) = 0;

    void
    notifyThread()
    {
        {
            Lock sync(*this);
            while(!_initialized)
            {
                wait();
            }
        }
        {
            Lock sync(*this);
            _notified = true;
            notify();
        }
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

    Glacier2::RouterPrx _router;
    int _id;
    CallbackReceiverIPtr _callbackReceiver;
    bool _initialized;
    bool _notified;
};
typedef IceUtil::Handle<StressClient> StressClientPtr;

class PingStressClient : public StressClient
{
public:

    PingStressClient(int id) : StressClient(id)
    {
    }

    virtual void
    stress(const CallbackPrx& callback, const CallbackReceiverPrx&)
    {
        try
        {
            CallbackPrx cb = CallbackPrx::uncheckedCast(callback->ice_twoway());
            Context context;
            context["_fwd"] = "t";
            while(true)
            {
                cb->ice_ping(context);
                IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(1));
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

class CallbackStressClient : public StressClient
{
public:

    CallbackStressClient(int id) : StressClient(id)
    {
    }

    virtual void
    stress(const CallbackPrx& callback, const CallbackReceiverPrx& receiver)
    {
        try
        {
            CallbackPrx cb = CallbackPrx::uncheckedCast(callback->ice_twoway());
            Context context;
            context["_fwd"] = "t";
            while(true)
            {
                cb->initiateCallback(receiver, context);
                _callbackReceiver->callbackOK();
                IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(1));
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

class CallbackWithPayloadStressClient : public StressClient
{
public:

    CallbackWithPayloadStressClient(int id) : StressClient(id)
    {
    }

    virtual void
    stress(const CallbackPrx& callback, const CallbackReceiverPrx& receiver)
    {
        try
        {
            CallbackPrx cb = CallbackPrx::uncheckedCast(callback->ice_twoway());
            Context context;
            context["_fwd"] = "t";
            while(true)
            {
                cb->initiateCallbackWithPayload(receiver, context);
                _callbackReceiver->callbackWithPayloadOK();
                IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(10));
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

class CallbackClient : public Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL();
#endif

    //
    // We must disable connection warnings, because we attempt to ping
    // the router before session establishment, as well as after
    // session destruction. Both will cause a ConnectionLostException.
    //
    initData = getTestInitData(argc, argv);
    initData.properties->setProperty("Ice.Warn.Connections", "0");

    CallbackClient app;
    return app.main(argc, argv, initData);
}

int
CallbackClient::run(int argc, char* argv[])
{
    ObjectPrx routerBase;

    {
        cout << "testing stringToProxy for router... " << flush;
        routerBase = communicator()->stringToProxy("Glacier2/router:" + getTestEndpoint(communicator(), 10));
        cout << "ok" << endl;
    }

    Glacier2::RouterPrx router;

    {
        cout << "testing checked cast for router... " << flush;
        router = Glacier2::RouterPrx::checkedCast(routerBase);
        test(router);
        cout << "ok" << endl;
    }

    {
        cout << "testing router finder... " << flush;
        Ice::RouterFinderPrx finder =
            RouterFinderPrx::uncheckedCast(communicator()->stringToProxy("Ice/RouterFinder:" +
                                                                         getTestEndpoint(communicator(), 10)));
        test(finder->getRouter()->ice_getIdentity() == router->ice_getIdentity());
        cout << "ok" << endl;
    }

    {
        cout << "installing router with communicator... " << flush;
        communicator()->setDefaultRouter(router);
        cout << "ok" << endl;
    }

    {
        cout << "getting the session timeout... " << flush;
        Ice::Long timeout = router->getSessionTimeout();
        test(timeout == 30);
        cout << "ok" << endl;
    }

    ObjectPrx base;

    {
        cout << "testing stringToProxy for server object... " << flush;
        base = communicator()->stringToProxy("c1/callback:" + getTestEndpoint(communicator(), 0));
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

    Glacier2::SessionPrx session;

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

    CallbackPrx twoway;

    {
        cout << "testing checked cast for server object... " << flush;
        twoway = CallbackPrx::checkedCast(base);
        test(twoway);
        cout << "ok" << endl;
    }

    ObjectAdapterPtr adapter;

    {
        cout << "creating and activating callback receiver adapter with router... " << flush;
        communicator()->getProperties()->setProperty("Ice.PrintAdapterReady", "0");
        adapter = communicator()->createObjectAdapterWithRouter("CallbackReceiverAdapter", router);
        adapter->activate();
        cout << "ok" << endl;
    }

    string category;

    {
        cout << "getting category from router... " << flush;
        category = router->getCategoryForClient();
        cout << "ok" << endl;
    }

    CallbackReceiverI* callbackReceiverImpl;
    ObjectPtr callbackReceiver;
    CallbackReceiverPrx twowayR;
    CallbackReceiverPrx fakeTwowayR;

    {
        cout << "creating and adding callback receiver object... " << flush;
        callbackReceiverImpl = new CallbackReceiverI;
        callbackReceiver = callbackReceiverImpl;
        Identity callbackReceiverIdent;
        callbackReceiverIdent.name = "callbackReceiver";
        callbackReceiverIdent.category = category;
        twowayR = CallbackReceiverPrx::uncheckedCast(adapter->add(callbackReceiver, callbackReceiverIdent));
        Identity fakeCallbackReceiverIdent;
        fakeCallbackReceiverIdent.name = "callbackReceiver";
        fakeCallbackReceiverIdent.category = "dummy";
        fakeTwowayR = CallbackReceiverPrx::uncheckedCast(adapter->add(callbackReceiver, fakeCallbackReceiverIdent));
        cout << "ok" << endl;
    }

    {
        cout << "testing oneway callback... " << flush;
        CallbackPrx oneway = CallbackPrx::uncheckedCast(twoway->ice_oneway());
        CallbackReceiverPrx onewayR = CallbackReceiverPrx::uncheckedCast(twowayR->ice_oneway());
        Context context;
        context["_fwd"] = "o";
        oneway->initiateCallback(onewayR, context);
        oneway->initiateCallback(onewayR, context);
        oneway->initiateCallback(onewayR, context);
        oneway->initiateCallback(onewayR, context);
        callbackReceiverImpl->callbackOK(4);
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
        callbackReceiverImpl->callbackOK(4);
        cout << "ok" << endl;
    }

    {
        cout << "testing batch oneway callback... " << flush;
        Context context;
        context["_fwd"] = "O";
        CallbackPrx batchOneway = CallbackPrx::uncheckedCast(twoway->ice_batchOneway());
        CallbackReceiverPrx onewayR = CallbackReceiverPrx::uncheckedCast(twowayR->ice_oneway());
        batchOneway->initiateCallback(onewayR, context);
        batchOneway->initiateCallback(onewayR, context);
        batchOneway->initiateCallback(onewayR, context);
        batchOneway->initiateCallback(onewayR, context);
        batchOneway->initiateCallback(onewayR, context);
        batchOneway->initiateCallback(onewayR, context);
        batchOneway->ice_flushBatchRequests();
        callbackReceiverImpl->callbackOK(6);
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
        AsyncCallbackPtr cb0 = new AsyncCallback();
        twoway->begin_initiateConcurrentCallback(0, twowayR, context,
            newCallback_Callback_initiateConcurrentCallback(cb0, &AsyncCallback::response, &AsyncCallback::exception));
        AsyncCallbackPtr cb1 = new AsyncCallback();
        twoway->begin_initiateConcurrentCallback(1, twowayR, context,
            newCallback_Callback_initiateConcurrentCallback(cb1, &AsyncCallback::response, &AsyncCallback::exception));
        AsyncCallbackPtr cb2 = new AsyncCallback();
        twoway->begin_initiateConcurrentCallback(2, twowayR, context,
            newCallback_Callback_initiateConcurrentCallback(cb2, &AsyncCallback::response, &AsyncCallback::exception));
        callbackReceiverImpl->answerConcurrentCallbacks(3);
        test(cb0->waitResponse() == 0);
        test(cb1->waitResponse() == 1);
        test(cb2->waitResponse() == 2);
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
        callbackReceiverImpl->callbackOK();
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
        CallbackPrx otherCategoryTwoway = CallbackPrx::uncheckedCast(
            twoway->ice_identity(stringToIdentity("c2/callback")));
        otherCategoryTwoway->initiateCallback(twowayR, context);
        callbackReceiverImpl->callbackOK();
        cout << "ok" << endl;
    }

    {
        cout << "testing whether disallowed category gets rejected... " << flush;
        Context context;
        context["_fwd"] = "t";
        try
        {
            CallbackPrx otherCategoryTwoway = CallbackPrx::uncheckedCast(
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
        CallbackPrx otherCategoryTwoway = CallbackPrx::uncheckedCast(
            twoway->ice_identity(stringToIdentity("_userid/callback")));
        otherCategoryTwoway->initiateCallback(twowayR, context);
        callbackReceiverImpl->callbackOK();
        cout << "ok" << endl;
    }

    {
        cout << "testing with blocking clients... " << flush;

        //
        // Start 3 misbehaving clients.
        //
        const int nClients = 3; // Passwords need to be added to the password file if more clients are needed.
        int i;
        vector<MisbehavedClientPtr> clients;
        for(i = 0; i < nClients; ++i)
        {
            clients.push_back(new MisbehavedClient(i));
            clients.back()->start();
            clients.back()->waitForCallback();
        }

        //
        // Sleep for one second to make sure the router starts sending
        // the callback with the payload to the clients.
        //
        IceUtil::ThreadControl::sleep(IceUtil::Time::seconds(1));

        //
        // Initiate few callbacks with a large payload. Because of
        // the buffered mode, this shouldn't block even though the
        // misbehaved client are not answering their callback
        // requests.
        //
        Context context;
        context["_fwd"] = "t";
        twoway->initiateCallbackWithPayload(twowayR, context);
        callbackReceiverImpl->callbackWithPayloadOK();
        twoway->initiateCallbackWithPayload(twowayR, context);
        callbackReceiverImpl->callbackWithPayloadOK();
        twoway->initiateCallbackWithPayload(twowayR, context);
        callbackReceiverImpl->callbackWithPayloadOK();
        twoway->initiateCallbackWithPayload(twowayR, context);
        callbackReceiverImpl->callbackWithPayloadOK();

        for(vector<MisbehavedClientPtr>::const_iterator p = clients.begin(); p != clients.end(); ++p)
        {
            (*p)->notifyWaitCallback();
            (*p)->getThreadControl().join();
        }

        cout << "ok" << endl;
    }

    {
        cout << "stress test... " << flush;
        const int nClients = 3; // Passwords need to be added to the password file if more clients are needed.
        int i;
        vector<StressClientPtr> clients;
        for(i = 0; i < nClients; ++i)
        {
            switch(IceUtilInternal::random(3))
            {
            case 0:
                clients.push_back(new PingStressClient(i));
                break;
            case 1:
                clients.push_back(new CallbackStressClient(i));
                break;
            case 2:
                clients.push_back(new CallbackWithPayloadStressClient(i));
                break;
            default:
                assert(false);
                break;
            }
            clients.back()->start();
        }
        for(vector<StressClientPtr>::const_iterator p = clients.begin(); p != clients.end(); ++p)
        {
            (*p)->notifyThread();
        }

        //
        // Let the stress client run for a bit.
        //
        IceUtil::ThreadControl::sleep(IceUtil::Time::seconds(3));

        //
        // Send some callbacks.
        //
        Context context;
        context["_fwd"] = "t";
        twoway->initiateCallback(twowayR);
        callbackReceiverImpl->callbackOK();

        //
        // Kill the stress clients.
        //
        for(vector<StressClientPtr>::const_iterator q = clients.begin(); q != clients.end(); ++q)
        {
            (*q)->kill();
            (*q)->getThreadControl().join();
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
        catch(const Ice::LocalException&)
        {
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
            communicator()->setDefaultRouter(0);
            cout << "ok" << endl;
        }

        ObjectPrx processBase;

        {
            cout << "testing stringToProxy for admin process facet... " << flush;
            processBase = communicator()->stringToProxy("Glacier2/admin -f Process:" +
                                                        getTestEndpoint(communicator(), 11));
            cout << "ok" << endl;
        }

        Ice::ProcessPrx process;

        {
            cout << "testing checked cast for process facet... " << flush;
            process = Ice::ProcessPrx::checkedCast(processBase);
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

    return EXIT_SUCCESS;
}
