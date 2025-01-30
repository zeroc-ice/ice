// Copyright (c) ZeroC, Inc.

#include "../../src/Ice/Random.h"
#include "Configuration.h"
#include "Ice/Ice.h"
#include "PluginI.h"
#include "Test.h"
#include "TestHelper.h"

#include <chrono>
#include <thread>

using namespace std;
using namespace Ice;
using namespace Test;

namespace
{

    const char* socketErrorMessage = "simulated socket error";

    class OpThread final
    {
    public:
        OpThread(const BackgroundPrx& background) : _background(background->ice_oneway()) {}

        void run()
        {
            int count = 0;
            while (true)
            {
                {
                    lock_guard lock(_mutex);
                    if (_destroyed)
                    {
                        return;
                    }
                }

                try
                {
                    if (++count == 10) // Don't blast the connection with only oneway's
                    {
                        count = 0;
                        _background->ice_twoway()->ice_ping();
                    }
                    _background->opAsync(nullptr); // don't wait
                    this_thread::sleep_for(chrono::milliseconds(1));
                }
                catch (const Ice::LocalException&)
                {
                }
            }
        }

        void destroy()
        {
            lock_guard lock(_mutex);
            _destroyed = true;
        }

    private:
        bool _destroyed{false};
        BackgroundPrx _background;
        mutex _mutex;
    };
    using OpThreadPtr = shared_ptr<OpThread>;

}

void connectTests(const ConfigurationPtr&, const BackgroundPrx&);
void initializeTests(const ConfigurationPtr&, const BackgroundPrx&, const BackgroundControllerPrx&);
void validationTests(const ConfigurationPtr&, const BackgroundPrx&, const BackgroundControllerPrx&);
void readWriteTests(const ConfigurationPtr&, const BackgroundPrx&, const BackgroundControllerPrx&);

BackgroundPrx
allTests(TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    const string endp = helper->getTestEndpoint();

    BackgroundPrx background(communicator, "background:" + endp);
    BackgroundControllerPrx backgroundController(
        communicator,
        "backgroundController:" + helper->getTestEndpoint(1, "tcp"));

    auto plugin = dynamic_pointer_cast<PluginI>(communicator->getPluginManager()->getPlugin("Test"));
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
        auto locator = LocatorPrx(communicator, "locator:" + endp)->ice_invocationTimeout(250);
        auto obj = ObjectPrx(communicator, "background@Test")->ice_locator(locator)->ice_oneway();

        backgroundController->pauseCall("findAdapterById");
        try
        {
            obj->ice_ping();
            test(false);
        }
        catch (const Ice::TimeoutException&)
        {
        }
        backgroundController->resumeCall("findAdapterById");

        locator = LocatorPrx(communicator, "locator:" + endp);
        locator->ice_ping();
        auto bg = BackgroundPrx(communicator, "background@Test")->ice_locator(locator);

        backgroundController->pauseCall("findAdapterById");

        promise<void> p1;
        promise<void> p2;

        bg->opAsync([&p1]() { p1.set_value(); }, [&p1](exception_ptr e) { p1.set_exception(e); });
        bg->opAsync([&p2]() { p2.set_value(); }, [&p2](exception_ptr e) { p2.set_exception(e); });

        auto f1 = p1.get_future();
        auto f2 = p2.get_future();

        test(f1.wait_for(chrono::milliseconds(0)) != future_status::ready);
        test(f2.wait_for(chrono::milliseconds(0)) != future_status::ready);

        backgroundController->resumeCall("findAdapterById");

        f1.get();
        f2.get();
    }
    cout << "ok" << endl;

    cout << "testing router... " << flush;
    {
        auto router = RouterPrx(communicator, "router:" + endp)->ice_invocationTimeout(250);
        auto obj = ObjectPrx(communicator, "background@Test")->ice_router(router)->ice_oneway();

        backgroundController->pauseCall("getClientProxy");
        try
        {
            obj->ice_ping();
            test(false);
        }
        catch (const Ice::TimeoutException&)
        {
        }
        backgroundController->resumeCall("getClientProxy");

        router = RouterPrx(communicator, "router:" + endp);
        auto bg = BackgroundPrx(communicator, "background@Test")->ice_router(router);
        test(bg->ice_getRouter());
        backgroundController->pauseCall("getClientProxy");

        promise<void> p1;
        promise<void> p2;

        bg->opAsync([&p1]() { p1.set_value(); }, [&p1](exception_ptr e) { p1.set_exception(e); });
        bg->opAsync([&p2]() { p2.set_value(); }, [&p2](exception_ptr e) { p2.set_exception(e); });

        auto f1 = p1.get_future();
        auto f2 = p2.get_future();

        test(f1.wait_for(chrono::milliseconds(0)) != future_status::ready);
        test(f2.wait_for(chrono::milliseconds(0)) != future_status::ready);

        backgroundController->resumeCall("getClientProxy");

        f1.get();
        f2.get();
    }
    cout << "ok" << endl;

    const bool ws = communicator->getProperties()->getIceProperty("Ice.Default.Protocol") == "test-ws";
    const bool wss = communicator->getProperties()->getIceProperty("Ice.Default.Protocol") == "test-wss";
    if (!ws && !wss)
    {
        cout << "testing buffered transport... " << flush;

        configuration->buffered(true);
        backgroundController->buffered(true);
        background->opAsync(nullptr);
        background->ice_getCachedConnection()->abort();
        background->opAsync(nullptr);

        vector<future<void>> results;
        for (int i = 0; i < 10000; ++i)
        {
            auto f = background->opAsync();
            if (i % 50 == 0)
            {
                backgroundController->holdAdapter();
                backgroundController->resumeAdapter();
            }
            if (i % 100 == 0)
            {
                f.get();
            }
            else
            {
                results.push_back(std::move(f));
            }
        }

        for (auto& f : results)
        {
            f.get(); // Ensure all the calls are completed before destroying the communicator
        }
        cout << "ok" << endl;
    }

    return background;
}

void
connectTests(const ConfigurationPtr& configuration, const BackgroundPrx& background)
{
    try
    {
        background->op();
    }
    catch (const Ice::LocalException&)
    {
        test(false);
    }
    background->ice_getConnection()->close().get();

    for (int i = 0; i < 4; ++i)
    {
        if (i == 0 || i == 2)
        {
            configuration->connectorsException(
                make_exception_ptr(Ice::DNSException{__FILE__, __LINE__, 0, "host.fake"}));
        }
        else
        {
            configuration->connectException(
                make_exception_ptr(Ice::SocketException{__FILE__, __LINE__, socketErrorMessage}));
        }
        BackgroundPrx prx = (i == 1 || i == 3) ? background : background->ice_oneway();

        try
        {
            prx->op();
            test(false);
        }
        catch (const Ice::Exception&)
        {
        }

        {
            promise<void> completed;
            promise<bool> sent;
            prx->opAsync(
                []() { test(false); },
                [&completed](exception_ptr) { completed.set_value(); },
                [&sent](bool value) { sent.set_value(value); });
            test(sent.get_future().wait_for(chrono::milliseconds(0)) != future_status::ready);
            completed.get_future().get();
        }

        {
            promise<void> completed;
            promise<bool> sent;

            prx->opAsync(
                []() { test(false); },
                [&completed](exception_ptr) { completed.set_value(); },
                [&sent](bool value) { sent.set_value(value); });
            test(sent.get_future().wait_for(chrono::milliseconds(0)) != future_status::ready);
            completed.get_future().get();
        }

        if (i == 0 || i == 2)
        {
            configuration->connectorsException(nullptr);
        }
        else
        {
            configuration->connectException(nullptr);
        }
    }

    auto opThread1 = make_shared<OpThread>(background);
    auto worker1 = thread([opThread1] { opThread1->run(); });

    OpThreadPtr opThread2 = make_shared<OpThread>(background);
    auto worker2 = thread([opThread2] { opThread2->run(); });

    for (int i = 0; i < 5; i++)
    {
        try
        {
            background->ice_ping();
        }
        catch (const Ice::LocalException&)
        {
            test(false);
        }

        configuration->connectException(
            make_exception_ptr(Ice::SocketException{__FILE__, __LINE__, socketErrorMessage}));
        background->ice_getCachedConnection()->abort();
        this_thread::sleep_for(chrono::milliseconds(10));
        configuration->connectException(nullptr);
        try
        {
            background->ice_ping();
        }
        catch (const Ice::LocalException&)
        {
        }
    }

    opThread1->destroy();
    opThread2->destroy();

    worker1.join();
    worker2.join();
}

void
initializeTests(
    const ConfigurationPtr& configuration,
    const BackgroundPrx& background,
    const BackgroundControllerPrx& ctl)
{
    try
    {
        background->op();
    }
    catch (const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    background->ice_getConnection()->close().get();

    for (int i = 0; i < 4; i++)
    {
        if (i == 0 || i == 2)
        {
            configuration->initializeException(
                make_exception_ptr(Ice::SocketException{__FILE__, __LINE__, socketErrorMessage}));
        }
        else
        {
#if defined(ICE_USE_IOCP) || defined(ICE_USE_CFSTREAM)
            continue;
#else
            configuration->initializeSocketOperation(IceInternal::SocketOperationWrite);
            configuration->initializeException(
                make_exception_ptr(Ice::SocketException{__FILE__, __LINE__, socketErrorMessage}));
#endif
        }
        BackgroundPrx prx = (i == 1 || i == 3) ? background : background->ice_oneway();

        try
        {
            prx->op();
            test(false);
        }
        catch (const Ice::SocketException&)
        {
        }

        promise<bool> sent;
        promise<void> completed;

        prx->opAsync(
            []() { test(false); },
            [&completed](exception_ptr) { completed.set_value(); },
            [&sent](bool value) { sent.set_value(value); });
        test(sent.get_future().wait_for(chrono::milliseconds(0)) != future_status::ready);
        completed.get_future().get();

        if (i == 0 || i == 2)
        {
            configuration->initializeException(nullptr);
        }
        else
        {
            configuration->initializeSocketOperation(IceInternal::SocketOperationNone);
            configuration->initializeException(nullptr);
        }
    }

#if !defined(ICE_USE_IOCP) && !defined(ICE_USE_CFSTREAM)
    try
    {
        configuration->initializeSocketOperation(IceInternal::SocketOperationWrite);
        background->op();
        configuration->initializeSocketOperation(IceInternal::SocketOperationNone);
    }
    catch (const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    background->ice_getConnection()->close().get();

    try
    {
        configuration->initializeSocketOperation(IceInternal::SocketOperationConnect);
        background->op();
        configuration->initializeSocketOperation(IceInternal::SocketOperationNone);
    }
    catch (const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    background->ice_getConnection()->close().get();
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
    catch (const Ice::ConnectionLostException&)
    {
        ctl->initializeException(false);
    }
    catch (const Ice::SecurityException&)
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
    catch (const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    background->ice_getConnection()->close().get();

    try
    {
        ctl->initializeSocketOperation(IceInternal::SocketOperationWrite);
        ctl->initializeException(true);
        background->op();
        test(false);
    }
    catch (const Ice::ConnectionLostException&)
    {
        ctl->initializeException(false);
        ctl->initializeSocketOperation(IceInternal::SocketOperationNone);
    }
    catch (const Ice::SecurityException&)
    {
        ctl->initializeException(false);
        ctl->initializeSocketOperation(IceInternal::SocketOperationNone);
    }
#endif

    auto opThread1 = make_shared<OpThread>(background);
    thread worker1 = thread([opThread1] { opThread1->run(); });
    auto opThread2 = make_shared<OpThread>(background);
    thread worker2 = thread([opThread2] { opThread2->run(); });

    for (int i = 0; i < 5; i++)
    {
        try
        {
            background->ice_ping();
        }
        catch (const Ice::LocalException& ex)
        {
            cerr << ex << endl;

            test(false);
        }

        configuration->initializeException(
            make_exception_ptr(Ice::SocketException{__FILE__, __LINE__, socketErrorMessage}));
        background->ice_getCachedConnection()->abort();
        this_thread::sleep_for(chrono::milliseconds(10));
        configuration->initializeException(nullptr);
        try
        {
            background->ice_ping();
        }
        catch (const Ice::LocalException&)
        {
        }
        try
        {
            background->ice_ping();
        }
        catch (const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        configuration->initializeSocketOperation(IceInternal::SocketOperationWrite);
        background->ice_getCachedConnection()->abort();
        background->ice_ping();
        configuration->initializeSocketOperation(IceInternal::SocketOperationNone);

        ctl->initializeException(true);
        background->ice_getCachedConnection()->abort();
        this_thread::sleep_for(chrono::milliseconds(10));
        ctl->initializeException(false);
        try
        {
            background->ice_ping();
        }
        catch (const Ice::LocalException&)
        {
        }
        try
        {
            background->ice_ping();
        }
        catch (const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        try
        {
#if !defined(ICE_USE_IOCP) && !defined(ICE_USE_CFSTREAM)
            ctl->initializeSocketOperation(IceInternal::SocketOperationWrite);
            background->ice_getCachedConnection()->abort();
            background->op();
            ctl->initializeSocketOperation(IceInternal::SocketOperationNone);
#else
            background->ice_getCachedConnection()->abort();
            background->op();
#endif
        }
        catch (const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
    }

    opThread1->destroy();
    opThread2->destroy();

    worker1.join();
    worker2.join();
}

void
validationTests(
    const ConfigurationPtr& configuration,
    const BackgroundPrx& background,
    const BackgroundControllerPrx& ctl)
{
    try
    {
        background->op();
    }
    catch (const Ice::LocalException&)
    {
        test(false);
    }
    background->ice_getConnection()->close().get();

    try
    {
        // Get the read() of connection validation to throw right away.
        configuration->readException(make_exception_ptr(Ice::SocketException{__FILE__, __LINE__, socketErrorMessage}));
        background->op();
        test(false);
    }
    catch (const Ice::SocketException&)
    {
        configuration->readException(nullptr);
    }
    catch (const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }

    for (int i = 0; i < 2; i++)
    {
        configuration->readException(make_exception_ptr(Ice::SocketException{__FILE__, __LINE__, socketErrorMessage}));
        BackgroundPrx prx = i == 0 ? background : background->ice_oneway();
        promise<bool> sent;
        promise<void> completed;

        prx->opAsync(
            []() { test(false); },
            [&completed](exception_ptr) { completed.set_value(); },
            [&sent](bool value) { sent.set_value(value); });
        test(sent.get_future().wait_for(chrono::milliseconds(0)) != future_status::ready);
        completed.get_future().get();
        configuration->readException(nullptr);
    }

    if (background->ice_getCommunicator()->getProperties()->getIceProperty("Ice.Default.Protocol") != "test-ssl" &&
        background->ice_getCommunicator()->getProperties()->getIceProperty("Ice.Default.Protocol") != "test-wss")
    {
        try
        {
            // Get the read() of the connection validation to return "would block"
            configuration->readReady(false);
            background->op();
            configuration->readReady(true);
        }
        catch (const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        background->ice_getConnection()->close().get();

        try
        {
            // Get the read() of the connection validation to return "would block" and then throw.
            configuration->readReady(false);
            configuration->readException(
                make_exception_ptr(Ice::SocketException{__FILE__, __LINE__, socketErrorMessage}));
            background->op();
            test(false);
        }
        catch (const Ice::SocketException&)
        {
            configuration->readException(nullptr);
            configuration->readReady(true);
        }
        catch (const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        for (int i = 0; i < 2; i++)
        {
            configuration->readReady(false);
            configuration->readException(
                make_exception_ptr(Ice::SocketException{__FILE__, __LINE__, socketErrorMessage}));
            promise<void> completed;
            background->opAsync(
                []() { test(false); },
                [&completed](exception_ptr err)
                {
                    try
                    {
                        rethrow_exception(err);
                    }
                    catch (const Ice::SocketException&)
                    {
                        completed.set_value();
                    }
                    catch (...)
                    {
                        test(false);
                    }
                });
            completed.get_future().get();
            configuration->readException(nullptr);
            configuration->readReady(true);
        }
    }

    ctl->holdAdapter(); // Hold to block in connection validation
    promise<void> p1;
    promise<void> p2;

    promise<bool> s1;
    promise<bool> s2;

    background->opAsync(
        [&p1]() { p1.set_value(); },
        [&p1](exception_ptr e) { p1.set_exception(e); },
        [&s1](bool value) { s1.set_value(value); });

    background->opAsync(
        [&p2]() { p2.set_value(); },
        [&p2](exception_ptr e) { p2.set_exception(e); },
        [&s2](bool value) { s2.set_value(value); });

    test(s1.get_future().wait_for(chrono::milliseconds(0)) != future_status::ready);
    test(s2.get_future().wait_for(chrono::milliseconds(0)) != future_status::ready);

    auto f1 = p1.get_future();
    auto f2 = p2.get_future();

    test(f1.wait_for(chrono::milliseconds(0)) != future_status::ready);
    test(f2.wait_for(chrono::milliseconds(0)) != future_status::ready);

    ctl->resumeAdapter();

    f1.get();
    f2.get();

#if defined(ICE_USE_IOCP) || defined(ICE_USE_CFSTREAM)
    if (background->ice_getCommunicator()->getProperties()->getIceProperty("Ice.Default.Protocol") != "test-ssl" &&
        background->ice_getCommunicator()->getProperties()->getIceProperty("Ice.Default.Protocol") != "test-wss")
    {
#endif
        try
        {
            // Get the write() of connection validation to throw right away.
            ctl->writeException(true);
            background->op();
            test(false);
        }
        catch (const Ice::ConnectionLostException&)
        {
            ctl->writeException(false);
        }
        catch (const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        try
        {
            // Get the write() of the connection validation to return "would block"
            ctl->writeReady(false);
            background->op();
            ctl->writeReady(true);
        }
        catch (const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        background->ice_getConnection()->close().get();

        try
        {
            // Get the write() of the connection validation to return "would block" and then throw.
            ctl->writeReady(false);
            ctl->writeException(true);
            background->op();
            test(false);
        }
        catch (const Ice::ConnectionLostException&)
        {
            ctl->writeException(false);
            ctl->writeReady(true);
        }
        catch (const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
#if defined(ICE_USE_IOCP) || defined(ICE_USE_CFSTREAM)
    }
#endif

    Ice::ByteSeq seq;
    seq.resize(512 * 1024);

    BackgroundPrx backgroundBatchOneway(background->ice_batchOneway());

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
    catch (const Ice::LocalException& ex)
    {
        cerr << ex << endl;
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
    catch (const Ice::LocalException& ex)
    {
        cerr << ex << endl;
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
    backgroundBatchOneway->ice_flushBatchRequestsAsync(nullptr);
    backgroundBatchOneway->ice_getConnection()->close().get();

    ctl->holdAdapter();
    backgroundBatchOneway->opWithPayload(seq);
    backgroundBatchOneway->opWithPayload(seq);
    backgroundBatchOneway->opWithPayload(seq);
    backgroundBatchOneway->opWithPayload(seq);
    ctl->resumeAdapter();
    backgroundBatchOneway->ice_flushBatchRequestsAsync().get();
    backgroundBatchOneway->ice_getConnection()->close().get();
}

void
readWriteTests(
    const ConfigurationPtr& configuration,
    const BackgroundPrx& background,
    const BackgroundControllerPrx& ctl)
{
    try
    {
        background->op();
    }
    catch (const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }

    for (int i = 0; i < 2; i++)
    {
        BackgroundPrx prx = i == 0 ? background : background->ice_oneway();

        try
        {
            background->ice_ping();
            configuration->writeException(
                make_exception_ptr(Ice::SocketException{__FILE__, __LINE__, socketErrorMessage}));
            prx->op();
            test(false);
        }
        catch (const Ice::SocketException&)
        {
            configuration->writeException(nullptr);
        }

        background->ice_ping();
        configuration->writeException(make_exception_ptr(Ice::SocketException{__FILE__, __LINE__, socketErrorMessage}));
        promise<bool> sent;
        promise<void> completed;
        prx->opAsync(
            []() { test(false); },
            [&completed](exception_ptr e)
            {
                try
                {
                    rethrow_exception(e);
                }
                catch (const Ice::SocketException&)
                {
                    completed.set_value();
                }
                catch (...)
                {
                    test(false);
                }
            },
            [&sent](bool value) { sent.set_value(value); });
        test(sent.get_future().wait_for(chrono::milliseconds(0)) != future_status::ready);
        completed.get_future().get();
        configuration->writeException(nullptr);
    }

    try
    {
        background->ice_ping();
        configuration->readException(make_exception_ptr(Ice::SocketException{__FILE__, __LINE__, socketErrorMessage}));
        background->op();
        test(false);
    }
    catch (const Ice::SocketException&)
    {
        configuration->readException(nullptr);
    }

    background->ice_ping();
    configuration->readReady(false); // Required in C# to make sure beginRead() doesn't throw too soon.
    configuration->readException(make_exception_ptr(Ice::SocketException{__FILE__, __LINE__, socketErrorMessage}));
    {
        promise<void> completed;
        background->opAsync(
            []() { test(false); },
            [&completed](exception_ptr e)
            {
                try
                {
                    rethrow_exception(e);
                }
                catch (const Ice::SocketException&)
                {
                    completed.set_value();
                }
                catch (...)
                {
                    test(false);
                }
            });
        completed.get_future().get();
    }
    configuration->readException(nullptr);
    configuration->readReady(true);

#if defined(ICE_USE_IOCP) || defined(ICE_USE_CFSTREAM)
    if (background->ice_getCommunicator()->getProperties()->getIceProperty("Ice.Default.Protocol") != "test-ssl" &&
        background->ice_getCommunicator()->getProperties()->getIceProperty("Ice.Default.Protocol") != "test-wss")
    {
#endif
        try
        {
            background->ice_ping();
            configuration->writeReady(false);
            background->op();
            configuration->writeReady(true);
        }
        catch (const Ice::LocalException&)
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
        catch (const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        try
        {
            background->ice_ping();
            configuration->writeReady(false);
            configuration->writeException(
                make_exception_ptr(Ice::SocketException{__FILE__, __LINE__, socketErrorMessage}));
            background->op();
            test(false);
        }
        catch (const Ice::SocketException&)
        {
            configuration->writeReady(true);
            configuration->writeException(nullptr);
        }

        for (int i = 0; i < 2; ++i)
        {
            BackgroundPrx prx = i == 0 ? background : background->ice_oneway();

            background->ice_ping();
            configuration->writeReady(false);
            configuration->writeException(
                make_exception_ptr(Ice::SocketException{__FILE__, __LINE__, socketErrorMessage}));
            promise<void> completed;
            promise<bool> sent;
            prx->opAsync(
                []() { test(false); },
                [&completed](exception_ptr e)
                {
                    try
                    {
                        rethrow_exception(e);
                    }
                    catch (const Ice::SocketException&)
                    {
                        completed.set_value();
                    }
                    catch (...)
                    {
                        test(false);
                    }
                },
                [&sent](bool value) { sent.set_value(value); });
            test(sent.get_future().wait_for(chrono::milliseconds(0)) != future_status::ready);
            completed.get_future().get();
            configuration->writeReady(true);
            configuration->writeException(nullptr);
        }

        try
        {
            background->ice_ping();
            configuration->readReady(false);
            configuration->readException(
                make_exception_ptr(Ice::SocketException{__FILE__, __LINE__, socketErrorMessage}));
            background->op();
            test(false);
        }
        catch (const Ice::SocketException&)
        {
            configuration->readException(nullptr);
            configuration->readReady(true);
        }

        {
            background->ice_ping();
            configuration->readReady(false);
            configuration->readException(
                make_exception_ptr(Ice::SocketException{__FILE__, __LINE__, socketErrorMessage}));
            promise<void> completed;
            background->opAsync(
                []() { test(false); },
                [&completed](exception_ptr e)
                {
                    try
                    {
                        rethrow_exception(e);
                    }
                    catch (const Ice::SocketException&)
                    {
                        completed.set_value();
                    }
                    catch (...)
                    {
                        test(false);
                    }
                });
            completed.get_future().get();
            configuration->readReady(true);
            configuration->readException(nullptr);
        }

        {
            background->ice_ping();
            configuration->readReady(false);
            configuration->writeReady(false);
            configuration->readException(
                make_exception_ptr(Ice::SocketException{__FILE__, __LINE__, socketErrorMessage}));
            promise<void> completed;
            background->opAsync(
                []() { test(false); },
                [&](exception_ptr e)
                {
                    try
                    {
                        rethrow_exception(e);
                    }
                    catch (const Ice::SocketException&)
                    {
                        completed.set_value();
                    }
                    catch (...)
                    {
                        test(false);
                    }
                });
            completed.get_future().get();
            configuration->writeReady(true);
            configuration->readReady(true);
            configuration->readException(nullptr);
        }
#if defined(ICE_USE_IOCP) || defined(ICE_USE_CFSTREAM)
    }
#endif

    background->ice_ping(); // Establish the connection

    BackgroundPrx backgroundOneway(background->ice_oneway());
    test(backgroundOneway->ice_getConnection() == background->ice_getConnection());

    ctl->holdAdapter(); // Hold to block in request send.

    Ice::ByteSeq seq;
    seq.resize(10024); // Make sure the request doesn't compress too well.
    for (auto& p : seq)
    {
        p = static_cast<byte>(IceInternal::random(255));
    }

    // Fill up the receive and send buffers
    for (int i = 0; i < 200; ++i) // 2MB
    {
        backgroundOneway->opWithPayloadAsync(seq, []() { test(false); }, [](exception_ptr) { test(false); });
    }
    promise<void> c1;
    promise<bool> s1;

    background->opAsync(
        [&c1]() { c1.set_value(); },
        [](exception_ptr) { test(false); },
        [&s1](bool value) { s1.set_value(value); });
    auto fs1 = s1.get_future();
    test(fs1.wait_for(chrono::milliseconds(0)) != future_status::ready);

    promise<void> c2;
    promise<bool> s2;
    background->opAsync(
        [&c2]() { c2.set_value(); },
        [](exception_ptr) { test(false); },
        [&s2](bool value) { s2.set_value(value); });

    auto fs2 = s2.get_future();
    test(fs2.wait_for(chrono::milliseconds(0)) != future_status::ready);

    promise<bool> s3;
    backgroundOneway->opWithPayloadAsync(
        seq,
        []() { test(false); },
        [](exception_ptr) { test(false); },
        [&s3](bool value) { s3.set_value(value); });
    auto fs3 = s3.get_future();
    test(fs3.wait_for(chrono::milliseconds(0)) != future_status::ready);

    promise<bool> s4;
    backgroundOneway->opWithPayloadAsync(
        seq,
        []() { test(false); },
        [](exception_ptr) { test(false); },
        [&s4](bool value) { s4.set_value(value); });
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

    try
    {
        background->ice_ping();
        ctl->writeException(true);
        background->op();
        test(false);
    }
    catch (const Ice::ConnectionLostException&)
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
    catch (const Ice::ConnectionLostException&)
    {
        ctl->readException(false);
    }

#if defined(ICE_USE_IOCP) || defined(ICE_USE_CFSTREAM)
    if (background->ice_getCommunicator()->getProperties()->getIceProperty("Ice.Default.Protocol") != "test-ssl")
    {
#endif
        try
        {
            background->ice_ping();
            ctl->writeReady(false);
            background->op();
            ctl->writeReady(true);
        }
        catch (const Ice::LocalException&)
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
        catch (const Ice::LocalException&)
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
        catch (const Ice::ConnectionLostException&)
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
        catch (const Ice::ConnectionLostException&)
        {
            ctl->readException(false);
            ctl->readReady(true);
        }
#if defined(ICE_USE_IOCP) || defined(ICE_USE_CFSTREAM)
    }
#endif

    auto opThread1 = make_shared<OpThread>(background);
    auto worker1 = thread([opThread1] { opThread1->run(); });
    auto opThread2 = make_shared<OpThread>(background);
    auto worker2 = thread([opThread2] { opThread2->run(); });

    for (int i = 0; i < 5; i++)
    {
        try
        {
            background->ice_ping();
        }
        catch (const Ice::LocalException&)
        {
            test(false);
        }

        this_thread::sleep_for(chrono::milliseconds(10));
        configuration->writeException(make_exception_ptr(Ice::SocketException{__FILE__, __LINE__, socketErrorMessage}));
        try
        {
            background->op();
        }
        catch (const Ice::LocalException&)
        {
        }
        configuration->writeException(nullptr);

        this_thread::sleep_for(chrono::milliseconds(10));

        background->ice_ping();
        background->ice_getCachedConnection()->abort();
        this_thread::sleep_for(chrono::milliseconds(10));

        background->ice_getCachedConnection()->abort();
    }

    opThread1->destroy();
    opThread2->destroy();

    worker1.join();
    worker2.join();
}
