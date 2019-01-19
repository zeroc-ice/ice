//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/IceUtil.h>
#include <IceUtil/Random.h>
#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>
#include <PluginI.h>
#include <Configuration.h>

using namespace std;
using namespace Test;

class Callback : public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    Callback() : _called(false)
    {
    }

    void
    check()
    {
        Lock sync(*this);
        while(!_called)
        {
            wait();
        }
        _called = false;
    }

    void
    called()
    {
        Lock sync(*this);
        assert(!_called);
        _called = true;
        notify();
    }

    bool
    isCalled()
    {
        Lock sync(*this);
        return _called;
    }

private:

    bool _called;
};

class OpAMICallback : public IceUtil::Shared
{
public:

    void
    response()
    {
        _response.called();
    }

    void
    responseNoOp()
    {
    }

    void
    noResponse()
    {
        test(false);
    }

    void
    exception(const Ice::Exception&)
    {
        _response.called();
    }

    void
    noException(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        cerr << "stack: " << ex.ice_stackTrace() << endl;
        test(false);
    }

    void
    sent(bool)
    {
        _sent.called();
    }

    bool checkException(bool wait = true)
    {
        if(wait)
        {
            _response.check();
            return true;
        }
        else
        {
            return _response.isCalled();
        }
    }

    bool
    checkResponse(bool wait = true)
    {
        if(wait)
        {
            _response.check();
            return true;
        }
        else
        {
            return _response.isCalled();
        }
    }

    void
    checkResponseAndSent()
    {
        _sent.check();
        _response.check();
    }

private:

    Callback _response;
    Callback _sent;
};
typedef IceUtil::Handle<OpAMICallback> OpAMICallbackPtr;

class OpThread : public IceUtil::Thread, public IceUtil::Mutex
{
public:

    OpThread(const BackgroundPrxPtr& background) :
        _destroyed(false),
        _background(ICE_UNCHECKED_CAST(BackgroundPrx, background->ice_oneway()))
    {
        start();
    }

    void
    run()
    {
        int count = 0;
        while(true)
        {
            {
                IceUtil::Mutex::Lock sync(*this);
                if(_destroyed)
                {
                    return;
                }
            }

            try
            {
                if(++count == 10) // Don't blast the connection with only oneway's
                {
                    count = 0;
                    _background->ice_twoway()->ice_ping();
                }
#ifdef ICE_CPP11_MAPPING
                _background->opAsync();
#else
                _background->begin_op();
#endif
                IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(1));
            }
            catch(const Ice::LocalException&)
            {
            }
        }
    }

    void
    destroy()
    {
        IceUtil::Mutex::Lock sync(*this);
        _destroyed = true;
    }

private:

    bool _destroyed;
    BackgroundPrxPtr _background;
};
typedef IceUtil::Handle<OpThread> OpThreadPtr;

void connectTests(const ConfigurationPtr&, const Test::BackgroundPrxPtr&);
void initializeTests(const ConfigurationPtr&, const Test::BackgroundPrxPtr&, const Test::BackgroundControllerPrxPtr&);
void validationTests(const ConfigurationPtr&, const Test::BackgroundPrxPtr&, const Test::BackgroundControllerPrxPtr&);
void readWriteTests(const ConfigurationPtr&, const Test::BackgroundPrxPtr&, const Test::BackgroundControllerPrxPtr&);

BackgroundPrxPtr
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    const string endp = helper->getTestEndpoint();
    string sref = "background:" + endp;
    Ice::ObjectPrxPtr obj = communicator->stringToProxy(sref);
    test(obj);

    BackgroundPrxPtr background = ICE_UNCHECKED_CAST(BackgroundPrx, obj);

    sref = "backgroundController:" + helper->getTestEndpoint(1, "tcp");
    obj = communicator->stringToProxy(sref);
    test(obj);

    BackgroundControllerPrxPtr backgroundController = ICE_UNCHECKED_CAST(BackgroundControllerPrx, obj);

#ifdef ICE_CPP11_MAPPING
    auto plugin = dynamic_pointer_cast<PluginI>(communicator->getPluginManager()->getPlugin("Test"));
#else
    PluginI* plugin = dynamic_cast<PluginI*>(communicator->getPluginManager()->getPlugin("Test").get());
#endif
    assert(plugin);
    ConfigurationPtr configuration = plugin->getConfiguration();

    cout << "testing connect... " << flush;
    {
        connectTests(configuration, background);
    }
    cout << "ok" << endl;

    cout << "testing initialization... " << flush;
    {
        initializeTests(configuration, background, backgroundController);
    }
    cout << "ok" << endl;

    cout << "testing connection validation... " << flush;
    {
        validationTests(configuration, background, backgroundController);
    }
    cout << "ok" << endl;

    cout << "testing read/write... " << flush;
    {
        readWriteTests(configuration, background, backgroundController);
    }
    cout << "ok" << endl;

    cout << "testing locator... " << flush;
    {
        Ice::LocatorPrxPtr locator;
        obj = communicator->stringToProxy("locator:" + endp)->ice_invocationTimeout(250);
        locator = ICE_UNCHECKED_CAST(Ice::LocatorPrx, obj);
        obj = communicator->stringToProxy("background@Test")->ice_locator(locator)->ice_oneway();

        backgroundController->pauseCall("findAdapterById");
        try
        {
            obj->ice_ping();
            test(false);
        }
        catch(const Ice::TimeoutException&)
        {
        }
        backgroundController->resumeCall("findAdapterById");

        obj = communicator->stringToProxy("locator:" + endp);
        locator = ICE_UNCHECKED_CAST(Ice::LocatorPrx, obj);
        obj = obj->ice_locator(locator);
        obj->ice_ping();
        obj = communicator->stringToProxy("background@Test")->ice_locator(locator);
        BackgroundPrxPtr bg = ICE_UNCHECKED_CAST(BackgroundPrx, obj);

#ifdef ICE_CPP11_MAPPING
        backgroundController->pauseCall("findAdapterById");

        promise<void> p1;
        promise<void> p2;

        bg->opAsync([&p1](){ p1.set_value(); }, [&p1](exception_ptr e){ p1.set_exception(e); });
        bg->opAsync([&p2](){ p2.set_value(); }, [&p2](exception_ptr e){ p2.set_exception(e); });

        auto f1 = p1.get_future();
        auto f2 = p2.get_future();

        test(f1.wait_for(chrono::milliseconds(0)) != future_status::ready);
        test(f2.wait_for(chrono::milliseconds(0)) != future_status::ready);

        backgroundController->resumeCall("findAdapterById");

        f1.get();
        f2.get();
#else
        backgroundController->pauseCall("findAdapterById");
        Ice::AsyncResultPtr r1 = bg->begin_op();
        Ice::AsyncResultPtr r2 = bg->begin_op();
        test(!r1->isCompleted());
        test(!r2->isCompleted());
        backgroundController->resumeCall("findAdapterById");
        bg->end_op(r1);
        bg->end_op(r2);
        test(r1->isCompleted());
        test(r2->isCompleted());
#endif
    }
    cout << "ok" << endl;

    cout << "testing router... " << flush;
    {
        Ice::RouterPrxPtr router;
        obj = communicator->stringToProxy("router:" + endp)->ice_invocationTimeout(250);
        router = ICE_UNCHECKED_CAST(Ice::RouterPrx, obj);
        obj = communicator->stringToProxy("background@Test")->ice_router(router)->ice_oneway();

        backgroundController->pauseCall("getClientProxy");
        try
        {
            obj->ice_ping();
            test(false);
        }
        catch(const Ice::TimeoutException&)
        {
        }
        backgroundController->resumeCall("getClientProxy");

        obj = communicator->stringToProxy("router:" + endp);
        router = ICE_UNCHECKED_CAST(Ice::RouterPrx, obj);
        obj = communicator->stringToProxy("background@Test")->ice_router(router);
        BackgroundPrxPtr bg = ICE_UNCHECKED_CAST(BackgroundPrx, obj);
        test(bg->ice_getRouter());

#ifdef ICE_CPP11_MAPPING
        backgroundController->pauseCall("getClientProxy");

        promise<void> p1;
        promise<void> p2;

        bg->opAsync([&p1](){ p1.set_value(); }, [&p1](exception_ptr e){ p1.set_exception(e); });
        bg->opAsync([&p2](){ p2.set_value(); }, [&p2](exception_ptr e){ p2.set_exception(e); });

        auto f1 = p1.get_future();
        auto f2 = p2.get_future();

        test(f1.wait_for(chrono::milliseconds(0)) != future_status::ready);
        test(f2.wait_for(chrono::milliseconds(0)) != future_status::ready);

        backgroundController->resumeCall("getClientProxy");

        f1.get();
        f2.get();
#else
        backgroundController->pauseCall("getClientProxy");
        Ice::AsyncResultPtr r1 = bg->begin_op();
        Ice::AsyncResultPtr r2 = bg->begin_op();
        test(!r1->isCompleted());
        test(!r2->isCompleted());
        backgroundController->resumeCall("getClientProxy");
        bg->end_op(r1);
        bg->end_op(r2);
        test(r1->isCompleted());
        test(r2->isCompleted());
#endif
    }
    cout << "ok" << endl;

    const bool ws = communicator->getProperties()->getProperty("Ice.Default.Protocol") == "test-ws";
    const bool wss = communicator->getProperties()->getProperty("Ice.Default.Protocol") == "test-wss";
    if(!ws && !wss)
    {
        cout << "testing buffered transport... " << flush;

        configuration->buffered(true);
        backgroundController->buffered(true);

#ifdef ICE_CPP11_MAPPING
        background->opAsync();
        background->ice_getCachedConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, Forcefully));
        background->opAsync();

        vector<future<void>> results;
        for(int i = 0; i < 10000; ++i)
        {
            auto f = background->opAsync();
            if(i % 50 == 0)
            {
                backgroundController->holdAdapter();
                backgroundController->resumeAdapter();
            }
            if(i % 100 == 0)
            {
                f.get();
            }
            else
            {
                results.push_back(move(f));
            }
        }

        for(auto& f : results)
        {
            f.get(); // Ensure all the calls are completed before destroying the communicator
        }
#else
        background->begin_op();
        background->ice_getCachedConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, Forcefully));
        background->begin_op();

        vector<Ice::AsyncResultPtr> results;
        OpAMICallbackPtr cb = new OpAMICallback();
        Callback_Background_opPtr callback = newCallback_Background_op(cb,
                                                                       &OpAMICallback::responseNoOp,
                                                                       &OpAMICallback::noException);
        for(int i = 0; i < 10000; ++i)
        {
            Ice::AsyncResultPtr r = background->begin_op(callback);
            results.push_back(r);
            if(i % 50 == 0)
            {
                backgroundController->holdAdapter();
                backgroundController->resumeAdapter();
            }
            if(i % 100 == 0)
            {
                r->waitForCompleted();
            }
        }

        for(vector<Ice::AsyncResultPtr>::const_iterator p = results.begin(); p != results.end(); ++p)
        {
            (*p)->waitForCompleted(); // Ensure all the calls are completed before destroying the communicator
        }
#endif
        cout << "ok" << endl;
    }

    return background;
}

void
connectTests(const ConfigurationPtr& configuration, const Test::BackgroundPrxPtr& background)
{
    try
    {
        background->op();
    }
    catch(const Ice::LocalException&)
    {
        test(false);
    }
    background->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));

    for(int i = 0; i < 4; ++i)
    {
        if(i == 0 || i == 2)
        {
            configuration->connectorsException(new Ice::DNSException(__FILE__, __LINE__));
        }
        else
        {
            configuration->connectException(new Ice::SocketException(__FILE__, __LINE__));
        }
        BackgroundPrxPtr prx = (i == 1 || i == 3) ? background : background->ice_oneway();

        try
        {
            prx->op();
            test(false);
        }
        catch(const Ice::Exception&)
        {
        }
#ifdef ICE_CPP11_MAPPING
        {
            promise<void> completed;
            promise<bool> sent;
            prx->opAsync(
                []()
                {
                    test(false);
                },
                [&completed](exception_ptr)
                {
                    completed.set_value();
                },
                [&sent](bool value)
                {
                    sent.set_value(value);
                });
            test(sent.get_future().wait_for(chrono::milliseconds(0)) != future_status::ready);
            completed.get_future().get();
        }

        {
            promise<void> completed;
            promise<bool> sent;

            prx->opAsync(
                []()
                {
                    test(false);
                },
                [&completed](exception_ptr)
                {
                    completed.set_value();
                },
                [&sent](bool value)
                {
                    sent.set_value(value);
                });
            test(sent.get_future().wait_for(chrono::milliseconds(0)) != future_status::ready);
            completed.get_future().get();
        }
#else
        Ice::AsyncResultPtr r = prx->begin_op();
        test(!r->sentSynchronously());
        try
        {
            prx->end_op(r);
            test(false);
        }
        catch(const Ice::Exception&)
        {
        }
        test(r->isCompleted());

        OpAMICallbackPtr cbEx = new OpAMICallback();
        r = prx->begin_op(Test::newCallback_Background_op(cbEx, &OpAMICallback::exception));
        test(!r->sentSynchronously());
        cbEx->checkException();
        test(r->isCompleted());
#endif
        if(i == 0 || i == 2)
        {
            configuration->connectorsException(0);
        }
        else
        {
            configuration->connectException(0);
        }
    }

    OpThreadPtr thread1 = new OpThread(background);
    OpThreadPtr thread2 = new OpThread(background);

    for(int i = 0; i < 5; i++)
    {
        try
        {
            background->ice_ping();
        }
        catch(const Ice::LocalException&)
        {
            test(false);
        }

        configuration->connectException(new Ice::SocketException(__FILE__, __LINE__));
        background->ice_getCachedConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, Forcefully));
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(10));
        configuration->connectException(0);
        try
        {
            background->ice_ping();
        }
        catch(const Ice::LocalException&)
        {
        }
    }

    thread1->destroy();
    thread2->destroy();

    thread1->getThreadControl().join();
    thread2->getThreadControl().join();
}

void
initializeTests(const ConfigurationPtr& configuration,
                const Test::BackgroundPrxPtr& background,
                const Test::BackgroundControllerPrxPtr& ctl)
{
    try
    {
        background->op();
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        cerr << "stack: " << ex.ice_stackTrace() << endl;
        test(false);
    }
    background->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));

    for(int i = 0; i < 4; i++)
    {
        if(i == 0 || i == 2)
        {
            configuration->initializeException(new Ice::SocketException(__FILE__, __LINE__));
        }
        else
        {
#if defined(ICE_USE_IOCP) || defined(ICE_USE_CFSTREAM)
            continue;
#else
            configuration->initializeSocketOperation(IceInternal::SocketOperationWrite);
            configuration->initializeException(new Ice::SocketException(__FILE__, __LINE__));
#endif
        }
        BackgroundPrxPtr prx = (i == 1 || i == 3) ? background : background->ice_oneway();

        try
        {
            prx->op();
            test(false);
        }
        catch(const Ice::SocketException&)
        {
        }

#ifdef ICE_CPP11_MAPPING
        promise<bool> sent;
        promise<void> completed;

        prx->opAsync(
            []()
            {
                test(false);
            },
            [&completed](exception_ptr)
            {
                completed.set_value();
            },
            [&sent](bool value)
            {
                sent.set_value(value);
            });
        test(sent.get_future().wait_for(chrono::milliseconds(0)) != future_status::ready);
        completed.get_future().get();
#else
        Ice::AsyncResultPtr r = prx->begin_op();
        test(!r->sentSynchronously());
        try
        {
            prx->end_op(r);
            test(false);
        }
        catch(const Ice::Exception&)
        {
        }
        test(r->isCompleted());

        OpAMICallbackPtr cbEx = new OpAMICallback();
        r = prx->begin_op(Test::newCallback_Background_op(cbEx, &OpAMICallback::exception));
        test(!r->sentSynchronously());
        cbEx->checkException();
        test(r->isCompleted());
#endif
        if(i == 0 || i == 2)
        {
            configuration->initializeException(0);
        }
        else
        {
            configuration->initializeSocketOperation(IceInternal::SocketOperationNone);
            configuration->initializeException(0);
        }
    }

#if !defined(ICE_USE_IOCP) && !defined(ICE_USE_CFSTREAM)
    try
    {
        configuration->initializeSocketOperation(IceInternal::SocketOperationWrite);
        background->op();
        configuration->initializeSocketOperation(IceInternal::SocketOperationNone);
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        cerr << "stack: " << ex.ice_stackTrace() << endl;
        test(false);
    }
    background->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));

    try
    {
        configuration->initializeSocketOperation(IceInternal::SocketOperationConnect);
        background->op();
        configuration->initializeSocketOperation(IceInternal::SocketOperationNone);
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        cerr << "stack: " << ex.ice_stackTrace() << endl;
        test(false);
    }
    background->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
#endif

    //
    // Now run the same tests with the server side.
    //

    try
    {
        ctl->initializeException(true);
        background->op();
        test(false);
    }
    catch(const Ice::ConnectionLostException&)
    {
        ctl->initializeException(false);
    }
    catch(const Ice::SecurityException&)
    {
        ctl->initializeException(false);
    }

#if !defined(ICE_USE_IOCP) && !defined(ICE_USE_CFSTREAM)
    try
    {
        ctl->initializeSocketOperation(IceInternal::SocketOperationWrite);
        background->op();
        ctl->initializeSocketOperation(IceInternal::SocketOperationNone);
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        cerr << "stack: " << ex.ice_stackTrace() << endl;
        test(false);
    }
    background->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));

    try
    {
        ctl->initializeSocketOperation(IceInternal::SocketOperationWrite);
        ctl->initializeException(true);
        background->op();
        test(false);
    }
    catch(const Ice::ConnectionLostException&)
    {
        ctl->initializeException(false);
        ctl->initializeSocketOperation(IceInternal::SocketOperationNone);
    }
    catch(const Ice::SecurityException&)
    {
        ctl->initializeException(false);
        ctl->initializeSocketOperation(IceInternal::SocketOperationNone);
    }
#endif

    OpThreadPtr thread1 = new OpThread(background);
    OpThreadPtr thread2 = new OpThread(background);

    for(int i = 0; i < 5; i++)
    {
        try
        {
            background->ice_ping();
        }
        catch(const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            cerr << "stack: " << ex.ice_stackTrace() << endl;
            test(false);
        }

        configuration->initializeException(new Ice::SocketException(__FILE__, __LINE__));
        background->ice_getCachedConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, Forcefully));
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(10));
        configuration->initializeException(0);
        try
        {
            background->ice_ping();
        }
        catch(const Ice::LocalException&)
        {
        }
        try
        {
            background->ice_ping();
        }
        catch(const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            cerr << "stack: " << ex.ice_stackTrace() << endl;
            test(false);
        }

        configuration->initializeSocketOperation(IceInternal::SocketOperationWrite);
        background->ice_getCachedConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, Forcefully));
        background->ice_ping();
        configuration->initializeSocketOperation(IceInternal::SocketOperationNone);

        ctl->initializeException(true);
        background->ice_getCachedConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, Forcefully));
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(10));
        ctl->initializeException(false);
        try
        {
            background->ice_ping();
        }
        catch(const Ice::LocalException&)
        {
        }
        try
        {
            background->ice_ping();
        }
        catch(const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            cerr << "stack: " << ex.ice_stackTrace() << endl;
            test(false);
        }

        try
        {
#if !defined(ICE_USE_IOCP) && !defined(ICE_USE_CFSTREAM)
            ctl->initializeSocketOperation(IceInternal::SocketOperationWrite);
            background->ice_getCachedConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, Forcefully));
            background->op();
            ctl->initializeSocketOperation(IceInternal::SocketOperationNone);
#else
            background->ice_getCachedConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, Forcefully));
            background->op();
#endif
        }
        catch(const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            cerr << "stack: " << ex.ice_stackTrace() << endl;
            test(false);
        }
    }

    thread1->destroy();
    thread2->destroy();

    thread1->getThreadControl().join();
    thread2->getThreadControl().join();
}

void
validationTests(const ConfigurationPtr& configuration,
                const Test::BackgroundPrxPtr& background,
                const Test::BackgroundControllerPrxPtr& ctl)
{
    try
    {
        background->op();
    }
    catch(const Ice::LocalException&)
    {
        test(false);
    }
    background->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));

    try
    {
        // Get the read() of connection validation to throw right away.
        configuration->readException(new Ice::SocketException(__FILE__, __LINE__));
        background->op();
        test(false);
    }
    catch(const Ice::SocketException&)
    {
        configuration->readException(0);
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        cerr << "stack: " << ex.ice_stackTrace() << endl;
        test(false);
    }

    for(int i = 0; i < 2; i++)
    {
        configuration->readException(new Ice::SocketException(__FILE__, __LINE__));
        BackgroundPrxPtr prx = i == 0 ? background : background->ice_oneway();
#ifdef ICE_CPP11_MAPPING
        promise<bool> sent;
        promise<void> completed;

        prx->opAsync(
            []()
            {
                test(false);
            },
            [&completed](exception_ptr)
            {
                completed.set_value();
            },
            [&sent](bool value)
            {
                sent.set_value(value);
            });
        test(sent.get_future().wait_for(chrono::milliseconds(0)) != future_status::ready);
        completed.get_future().get();
#else
        Ice::AsyncResultPtr r = prx->begin_op();
        test(!r->sentSynchronously());
        try
        {
            prx->end_op(r);
            test(false);
        }
        catch(const Ice::SocketException&)
        {
        }
        test(r->isCompleted());
#endif
        configuration->readException(0);
    }

    if(background->ice_getCommunicator()->getProperties()->getProperty("Ice.Default.Protocol") != "test-ssl" &&
       background->ice_getCommunicator()->getProperties()->getProperty("Ice.Default.Protocol") != "test-wss")
    {
        try
        {
            // Get the read() of the connection validation to return "would block"
            configuration->readReady(false);
            background->op();
            configuration->readReady(true);
        }
        catch(const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            cerr << "stack: " << ex.ice_stackTrace() << endl;
            test(false);
        }
        background->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));

        try
        {
            // Get the read() of the connection validation to return "would block" and then throw.
            configuration->readReady(false);
            configuration->readException(new Ice::SocketException(__FILE__, __LINE__));
            background->op();
            test(false);
        }
        catch(const Ice::SocketException&)
        {
            configuration->readException(0);
            configuration->readReady(true);
        }
        catch(const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            cerr << "stack: " << ex.ice_stackTrace() << endl;
            test(false);
        }

        for(int i = 0; i < 2; i++)
        {
            configuration->readReady(false);
            configuration->readException(new Ice::SocketException(__FILE__, __LINE__));
#ifdef ICE_CPP11_MAPPING
            promise<void> completed;
            background->opAsync(
                []()
                {
                    test(false);
                },
                [&completed](exception_ptr err)
                {
                    try
                    {
                        rethrow_exception(err);
                    }
                    catch(const Ice::SocketException&)
                    {
                        completed.set_value();
                    }
                    catch(...)
                    {
                        test(false);
                    }
                });
            completed.get_future().get();
#else
            Ice::AsyncResultPtr r = background->begin_op();
            test(!r->sentSynchronously());
            try
            {
                background->end_op(r);
                test(false);
            }
            catch(const Ice::SocketException&)
            {
            }
            test(r->isCompleted());
#endif
            configuration->readException(0);
            configuration->readReady(true);
        }
    }

    ctl->holdAdapter(); // Hold to block in connection validation
#ifdef ICE_CPP11_MAPPING
    promise<void> p1;
    promise<void> p2;

    promise<bool> s1;
    promise<bool> s2;

    background->opAsync(
        [&p1]()
        {
            p1.set_value();
        },
        [&p1](exception_ptr e)
        {
            p1.set_exception(e);
        },
        [&s1](bool value)
        {
            s1.set_value(value);
        });

    background->opAsync(
        [&p2]()
        {
            p2.set_value();
        },
        [&p2](exception_ptr e)
        {
            p2.set_exception(e);
        },
        [&s2](bool value)
        {
            s2.set_value(value);
        });

    test(s1.get_future().wait_for(chrono::milliseconds(0)) != future_status::ready);
    test(s2.get_future().wait_for(chrono::milliseconds(0)) != future_status::ready);

    auto f1 = p1.get_future();
    auto f2 = p2.get_future();

    test(f1.wait_for(chrono::milliseconds(0)) != future_status::ready);
    test(f2.wait_for(chrono::milliseconds(0)) != future_status::ready);

    ctl->resumeAdapter();

    f1.get();
    f2.get();
#else
    Ice::AsyncResultPtr r = background->begin_op();
    Ice::AsyncResultPtr r2 = background->begin_op();
    test(!r->sentSynchronously() && !r2->sentSynchronously());
    test(!r->isCompleted() && !r2->isCompleted());
    ctl->resumeAdapter();
    background->end_op(r);
    background->end_op(r2);
    test(r->isCompleted() && r2->isCompleted());
#endif

#if defined(ICE_USE_IOCP) || defined(ICE_USE_CFSTREAM)
    if(background->ice_getCommunicator()->getProperties()->getProperty("Ice.Default.Protocol") != "test-ssl" &&
       background->ice_getCommunicator()->getProperties()->getProperty("Ice.Default.Protocol") != "test-wss")
    {
#endif
    try
    {
        // Get the write() of connection validation to throw right away.
        ctl->writeException(true);
        background->op();
        test(false);
    }
    catch(const Ice::ConnectionLostException&)
    {
        ctl->writeException(false);
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        cerr << "stack: " << ex.ice_stackTrace() << endl;
        test(false);
    }

    try
    {
        // Get the write() of the connection validation to return "would block"
        ctl->writeReady(false);
        background->op();
        ctl->writeReady(true);
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        cerr << "stack: " << ex.ice_stackTrace() << endl;
        test(false);
    }
    background->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));

    try
    {
        // Get the write() of the connection validation to return "would block" and then throw.
        ctl->writeReady(false);
        ctl->writeException(true);
        background->op();
        test(false);
    }
    catch(const Ice::ConnectionLostException&)
    {
        ctl->writeException(false);
        ctl->writeReady(true);
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        cerr << "stack: " << ex.ice_stackTrace() << endl;
        test(false);
    }
#if defined(ICE_USE_IOCP) || defined(ICE_USE_CFSTREAM)
    }
#endif

    Ice::ByteSeq seq;
    seq.resize(512 * 1024);

    BackgroundPrxPtr backgroundBatchOneway = ICE_UNCHECKED_CAST(BackgroundPrx, background->ice_batchOneway());

    //
    // First send small requests to test without auto-flushing.
    //
    ctl->holdAdapter();
    backgroundBatchOneway->op();
    backgroundBatchOneway->op();
    backgroundBatchOneway->op();
    backgroundBatchOneway->op();
    ctl->resumeAdapter();
    try
    {
        backgroundBatchOneway->ice_flushBatchRequests();
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        cerr << "stack: " << ex.ice_stackTrace() << endl;
        test(false);
    }

    //
    // Send bigger requests to test with auto-flushing.
    //
    ctl->holdAdapter();
    backgroundBatchOneway->opWithPayload(seq);
    backgroundBatchOneway->opWithPayload(seq);
    backgroundBatchOneway->opWithPayload(seq);
    backgroundBatchOneway->opWithPayload(seq);
    ctl->resumeAdapter();
    try
    {
        backgroundBatchOneway->ice_flushBatchRequests();
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        cerr << "stack: " << ex.ice_stackTrace() << endl;
        test(false);
    }

    //
    // Then try the same thing with async flush.
    //

    ctl->holdAdapter();
    backgroundBatchOneway->op();
    backgroundBatchOneway->op();
    backgroundBatchOneway->op();
    backgroundBatchOneway->op();
    ctl->resumeAdapter();
#ifdef ICE_CPP11_MAPPING
    backgroundBatchOneway->ice_flushBatchRequestsAsync();
#else
    backgroundBatchOneway->begin_ice_flushBatchRequests();
#endif
    backgroundBatchOneway->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));

    ctl->holdAdapter();
    backgroundBatchOneway->opWithPayload(seq);
    backgroundBatchOneway->opWithPayload(seq);
    backgroundBatchOneway->opWithPayload(seq);
    backgroundBatchOneway->opWithPayload(seq);
    ctl->resumeAdapter();
#ifdef ICE_CPP11_MAPPING
    backgroundBatchOneway->ice_flushBatchRequestsAsync().get();
#else
    r = backgroundBatchOneway->begin_ice_flushBatchRequests();
    //
    // We can't close the connection before ensuring all the batches
    // have been sent since with auto-flushing the close connection
    // message might be sent once the first call opWithPayload is sent
    // and before the flushBatchRequests (this would therefore result
    // in the flush to report a CloseConnectionException). Instead we
    // wait for the first flush to complete.
    //
    //backgroundBatchOneway->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
    backgroundBatchOneway->end_ice_flushBatchRequests(r);
#endif
    backgroundBatchOneway->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
}

void
readWriteTests(const ConfigurationPtr& configuration,
               const Test::BackgroundPrxPtr& background,
               const Test::BackgroundControllerPrxPtr& ctl)
{
    try
    {
        background->op();
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        cerr << "stack: " << ex.ice_stackTrace() << endl;
        test(false);
    }

    for(int i = 0; i < 2; i++)
    {
        BackgroundPrxPtr prx = i == 0 ? background : background->ice_oneway();

        try
        {
            background->ice_ping();
            configuration->writeException(new Ice::SocketException(__FILE__, __LINE__));
            prx->op();
            test(false);
        }
        catch(const Ice::SocketException&)
        {
            configuration->writeException(0);
        }

        background->ice_ping();
        configuration->writeException(new Ice::SocketException(__FILE__, __LINE__));
#ifdef ICE_CPP11_MAPPING
        promise<bool> sent;
        promise<void> completed;
        prx->opAsync(
            []()
            {
                test(false);
            },
            [&completed](exception_ptr e)
            {
                try
                {
                    rethrow_exception(e);
                }
                catch(const Ice::SocketException&)
                {
                    completed.set_value();
                }
                catch(...)
                {
                    test(false);
                }
            },
            [&sent](bool value)
            {
                sent.set_value(value);
            });
        test(sent.get_future().wait_for(chrono::milliseconds(0)) != future_status::ready);
        completed.get_future().get();
#else
        Ice::AsyncResultPtr r = prx->begin_op();
        test(!r->sentSynchronously());
        try
        {
            prx->end_op(r);
            test(false);
        }
        catch(const Ice::SocketException&)
        {
        }
        test(r->isCompleted());
#endif
        configuration->writeException(0);
    }

    try
    {
        background->ice_ping();
        configuration->readException(new Ice::SocketException(__FILE__, __LINE__));
        background->op();
        test(false);
    }
    catch(const Ice::SocketException&)
    {
        configuration->readException(0);
    }

    background->ice_ping();
    configuration->readReady(false); // Required in C# to make sure beginRead() doesn't throw too soon.
    configuration->readException(new Ice::SocketException(__FILE__, __LINE__));
#ifdef ICE_CPP11_MAPPING
    {
        promise<void> completed;
        background->opAsync(
            []()
            {
                test(false);
            },
            [&completed](exception_ptr e)
            {
                try
                {
                    rethrow_exception(e);
                }
                catch(const Ice::SocketException&)
                {
                    completed.set_value();
                }
                catch(...)
                {
                    test(false);
                }
            });
        completed.get_future().get();
    }
#else
    Ice::AsyncResultPtr r = background->begin_op();
    try
    {
        background->end_op(r);
        test(false);
    }
    catch(const Ice::SocketException&)
    {
    }
    test(r->isCompleted());
#endif
    configuration->readException(0);
    configuration->readReady(true);

#if defined(ICE_USE_IOCP) || defined(ICE_USE_CFSTREAM)
    if(background->ice_getCommunicator()->getProperties()->getProperty("Ice.Default.Protocol") != "test-ssl" &&
       background->ice_getCommunicator()->getProperties()->getProperty("Ice.Default.Protocol") != "test-wss")
    {
#endif
        try
        {
            background->ice_ping();
            configuration->writeReady(false);
            background->op();
            configuration->writeReady(true);
        }
        catch(const Ice::LocalException&)
        {
            test(false);
        }

        try
        {
            background->ice_ping();
            configuration->readReady(false);
            background->op();
            configuration->readReady(true);
        }
        catch(const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            cerr << "stack: " << ex.ice_stackTrace() << endl;
            test(false);
        }

        try
        {
            background->ice_ping();
            configuration->writeReady(false);
            configuration->writeException(new Ice::SocketException(__FILE__, __LINE__));
            background->op();
            test(false);
        }
        catch(const Ice::SocketException&)
        {
            configuration->writeReady(true);
            configuration->writeException(0);
        }

        for(int i = 0; i < 2; ++i)
        {
            BackgroundPrxPtr prx = i == 0 ? background : background->ice_oneway();

            background->ice_ping();
            configuration->writeReady(false);
            configuration->writeException(new Ice::SocketException(__FILE__, __LINE__));
#ifdef ICE_CPP11_MAPPING
            promise<void> completed;
            promise<bool> sent;
            prx->opAsync(
                []()
                {
                    test(false);
                },
                [&completed](exception_ptr e)
                {
                    try
                    {
                        rethrow_exception(e);
                    }
                    catch(const Ice::SocketException&)
                    {
                        completed.set_value();
                    }
                    catch(...)
                    {
                        test(false);
                    }
                },
                [&sent](bool value)
                {
                    sent.set_value(value);
                });
            test(sent.get_future().wait_for(chrono::milliseconds(0)) != future_status::ready);
            completed.get_future().get();
#else
            r = prx->begin_op();
            test(!r->sentSynchronously());
            try
            {
                prx->end_op(r);
                test(false);
            }
            catch(const Ice::SocketException&)
            {
            }
            test(r->isCompleted());
#endif
            configuration->writeReady(true);
            configuration->writeException(0);
        }

        try
        {
            background->ice_ping();
            configuration->readReady(false);
            configuration->readException(new Ice::SocketException(__FILE__, __LINE__));
            background->op();
            test(false);
        }
        catch(const Ice::SocketException&)
        {
            configuration->readException(0);
            configuration->readReady(true);
        }

        {
            background->ice_ping();
            configuration->readReady(false);
            configuration->readException(new Ice::SocketException(__FILE__, __LINE__));
#ifdef ICE_CPP11_MAPPING
            promise<void> completed;
            background->opAsync(
                []()
                {
                    test(false);
                },
                [&completed](exception_ptr e)
                {
                    try
                    {
                        rethrow_exception(e);
                    }
                    catch(const Ice::SocketException&)
                    {
                        completed.set_value();
                    }
                    catch(...)
                    {
                        test(false);
                    }
                });
            completed.get_future().get();

#else
            r = background->begin_op();
            try
            {
                background->end_op(r);
                test(false);
            }
            catch(const Ice::SocketException&)
            {
            }
            test(r->isCompleted());
#endif
            configuration->readReady(true);
            configuration->readException(0);
        }

        {
            background->ice_ping();
            configuration->readReady(false);
            configuration->writeReady(false);
            configuration->readException(new Ice::SocketException(__FILE__, __LINE__));
#ifdef ICE_CPP11_MAPPING
            promise<void> completed;
            background->opAsync(
                []()
                {
                    test(false);
                },
                [&](exception_ptr e)
                {
                    try
                    {
                        rethrow_exception(e);
                    }
                    catch(const Ice::SocketException&)
                    {
                        completed.set_value();
                    }
                    catch(...)
                    {
                        test(false);
                    }
                });
            completed.get_future().get();
#else
            r = background->begin_op();
            // The read exception might propagate before the message send is seen as completed on IOCP.
#   ifndef ICE_USE_IOCP
            r->waitForSent();
#   endif
            try
            {
                background->end_op(r);
                test(false);
            }
            catch(const Ice::SocketException&)
            {
            }
            test(r->isCompleted());
#endif
            configuration->writeReady(true);
            configuration->readReady(true);
            configuration->readException(0);
        }
#if defined(ICE_USE_IOCP) || defined(ICE_USE_CFSTREAM)
    }
#endif

    background->ice_ping(); // Establish the connection

    BackgroundPrxPtr backgroundOneway = ICE_UNCHECKED_CAST(BackgroundPrx, background->ice_oneway());
    test(backgroundOneway->ice_getConnection() == background->ice_getConnection());

    ctl->holdAdapter(); // Hold to block in request send.

    Ice::ByteSeq seq;
    seq.resize(10024); // Make sure the request doesn't compress too well.
    for(Ice::ByteSeq::iterator p = seq.begin(); p != seq.end(); ++p)
    {
        *p = static_cast<Ice::Byte>(IceUtilInternal::random(255));
    }

#ifdef ICE_CPP11_MAPPING

    // Fill up the receive and send buffers
    for(int i = 0; i < 200; ++i) // 2MB
    {
        backgroundOneway->opWithPayloadAsync(
            seq,
            []()
            {
                test(false);
            },
            [](exception_ptr)
            {
                test(false);
            });
    }
    promise<void> c1;
    promise<bool> s1;

    background->opAsync(
        [&c1]()
        {
            c1.set_value();
        },
        [](exception_ptr)
        {
            test(false);
        },
        [&s1](bool value)
        {
            s1.set_value(value);
        });
    auto fs1 = s1.get_future();
    test(fs1.wait_for(chrono::milliseconds(0)) != future_status::ready);

    promise<void> c2;
    promise<bool> s2;
    background->opAsync(
        [&c2]()
        {
            c2.set_value();
        },
        [](exception_ptr)
        {
            test(false);
        },
        [&s2](bool value)
        {
            s2.set_value(value);
        });

    auto fs2 = s2.get_future();
    test(fs2.wait_for(chrono::milliseconds(0)) != future_status::ready);

    promise<bool> s3;
    backgroundOneway->opWithPayloadAsync(seq,
                                          [](){ test(false); },
                                          [](exception_ptr){ test(false); },
                                          [&s3](bool value){ s3.set_value(value); });
    auto fs3 = s3.get_future();
    test(fs3.wait_for(chrono::milliseconds(0)) != future_status::ready);

    promise<bool> s4;
    backgroundOneway->opWithPayloadAsync(seq,
                                          [](){ test(false); },
                                          [](exception_ptr){ test(false); },
                                          [&s4](bool value){ s4.set_value(value); });
    auto fs4 = s4.get_future();
    test(fs4.wait_for(chrono::milliseconds(0)) != future_status::ready);

    auto fc1 = c1.get_future();
    test(fc1.wait_for(chrono::milliseconds(0)) != future_status::ready);

    auto fc2 = c2.get_future();
    test(fc2.wait_for(chrono::milliseconds(0)) != future_status::ready);

    ctl->resumeAdapter();

    fs1.get();
    fs2.get();

    fc1.get();
    fc2.get();
#else
    OpAMICallbackPtr cb = new OpAMICallback();
    Callback_Background_opWithPayloadPtr callbackWP = newCallback_Background_opWithPayload(cb,
                                                                                           &OpAMICallback::noResponse,
                                                                                           &OpAMICallback::noException);

    // Fill up the receive and send buffers
    for(int i = 0; i < 200; ++i) // 2MB
    {
        backgroundOneway->begin_opWithPayload(seq, callbackWP);
    }

    Callback_Background_opPtr callback;
    cb = new OpAMICallback();
    Ice::AsyncResultPtr r1 = background->begin_op(newCallback_Background_op(cb,
                                                                            &OpAMICallback::response,
                                                                            &OpAMICallback::noException,
                                                                            &OpAMICallback::sent));
    test(!r1->sentSynchronously() && !r1->isSent());

    OpAMICallbackPtr cb2 = new OpAMICallback();
    Ice::AsyncResultPtr r2 = background->begin_op(newCallback_Background_op(cb2,
                                                                            &OpAMICallback::response,
                                                                            &OpAMICallback::noException,
                                                                            &OpAMICallback::sent));
    test(!r2->sentSynchronously() && !r2->isSent());

    test(!backgroundOneway->begin_opWithPayload(seq, callbackWP)->sentSynchronously());
    test(!backgroundOneway->begin_opWithPayload(seq, callbackWP)->sentSynchronously());

    test(!cb->checkResponse(false));
    test(!cb2->checkResponse(false));
    ctl->resumeAdapter();
    cb->checkResponseAndSent();
    cb2->checkResponseAndSent();
    test(r1->isSent() && r1->isCompleted());
    test(r2->isSent() && r2->isCompleted());
#endif

    try
    {
        background->ice_ping();
        ctl->writeException(true);
        background->op();
        test(false);
    }
    catch(const Ice::ConnectionLostException&)
    {
        ctl->writeException(false);
    }

    try
    {
        background->ice_ping();
        ctl->readException(true);
        background->op();
        test(false);
    }
    catch(const Ice::ConnectionLostException&)
    {
        ctl->readException(false);
    }

#if defined(ICE_USE_IOCP) || defined(ICE_USE_CFSTREAM)
    if(background->ice_getCommunicator()->getProperties()->getProperty("Ice.Default.Protocol") != "test-ssl")
    {
#endif
    try
    {
        background->ice_ping();
        ctl->writeReady(false);
        background->op();
        ctl->writeReady(true);
    }
    catch(const Ice::LocalException&)
    {
        test(false);
    }

    try
    {
        background->ice_ping();
        ctl->readReady(false);
        background->op();
        ctl->readReady(true);
    }
    catch(const Ice::LocalException&)
    {
        test(false);
    }

    try
    {
        background->ice_ping();
        ctl->writeReady(false);
        ctl->writeException(true);
        background->op();
        test(false);
    }
    catch(const Ice::ConnectionLostException&)
    {
        ctl->writeException(false);
        ctl->writeReady(true);
    }

    try
    {
        background->ice_ping();
        ctl->readReady(false);
        ctl->readException(true);
        background->op();
        test(false);
    }
    catch(const Ice::ConnectionLostException&)
    {
        ctl->readException(false);
        ctl->readReady(true);
    }
#if defined(ICE_USE_IOCP) || defined(ICE_USE_CFSTREAM)
    }
#endif

    OpThreadPtr thread1 = new OpThread(background);
    OpThreadPtr thread2 = new OpThread(background);

    for(int i = 0; i < 5; i++)
    {
        try
        {
            background->ice_ping();
        }
        catch(const Ice::LocalException&)
        {
            test(false);
        }

        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(10));
        configuration->writeException(new Ice::SocketException(__FILE__, __LINE__));
        try
        {
            background->op();
        }
        catch(const Ice::LocalException&)
        {
        }
        configuration->writeException(0);

        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(10));

        background->ice_ping();
        background->ice_getCachedConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, Forcefully));
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(10));

        background->ice_getCachedConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, Forcefully));
    }

    thread1->destroy();
    thread2->destroy();

    thread1->getThreadControl().join();
    thread2->getThreadControl().join();
}
