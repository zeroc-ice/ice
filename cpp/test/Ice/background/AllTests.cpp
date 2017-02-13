// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <IceUtil/Random.h>
#include <Ice/Ice.h>
#include <TestCommon.h>
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

    OpThread(const BackgroundPrx& background) :
        _destroyed(false),
        _background(BackgroundPrx::uncheckedCast(background->ice_oneway()))
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
                _background->begin_op();
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
    BackgroundPrx _background;
};
typedef IceUtil::Handle<OpThread> OpThreadPtr;

void connectTests(const ConfigurationPtr&, const Test::BackgroundPrx&);
void initializeTests(const ConfigurationPtr&, const Test::BackgroundPrx&, const Test::BackgroundControllerPrx&);
void validationTests(const ConfigurationPtr&, const Test::BackgroundPrx&, const Test::BackgroundControllerPrx&);
void readWriteTests(const ConfigurationPtr&, const Test::BackgroundPrx&, const Test::BackgroundControllerPrx&);

BackgroundPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    string sref = "background:default -p 12010";
    Ice::ObjectPrx obj = communicator->stringToProxy(sref);
    test(obj);

    BackgroundPrx background = BackgroundPrx::uncheckedCast(obj);

    sref = "backgroundController:tcp -p 12011";
    obj = communicator->stringToProxy(sref);
    test(obj);

    BackgroundControllerPrx backgroundController = BackgroundControllerPrx::uncheckedCast(obj);

    PluginI* plugin = dynamic_cast<PluginI*>(communicator->getPluginManager()->getPlugin("Test").get());
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
        Ice::LocatorPrx locator;
        obj = communicator->stringToProxy("locator:default -p 12010")->ice_invocationTimeout(250);
        locator = Ice::LocatorPrx::uncheckedCast(obj);
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

        obj = communicator->stringToProxy("locator:default -p 12010");
        locator = Ice::LocatorPrx::uncheckedCast(obj);
        obj = obj->ice_locator(locator);
        obj->ice_ping();

        obj = communicator->stringToProxy("background@Test")->ice_locator(locator);
        BackgroundPrx bg = BackgroundPrx::uncheckedCast(obj);

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
    }
    cout << "ok" << endl;

    cout << "testing router... " << flush;
    {
        Ice::RouterPrx router;

        obj = communicator->stringToProxy("router:default -p 12010")->ice_invocationTimeout(250);
        router = Ice::RouterPrx::uncheckedCast(obj);
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

        obj = communicator->stringToProxy("router:default -p 12010");
        router = Ice::RouterPrx::uncheckedCast(obj);
        obj = communicator->stringToProxy("background@Test")->ice_router(router);
        BackgroundPrx bg = BackgroundPrx::uncheckedCast(obj);
        test(bg->ice_getRouter());

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
    }
    cout << "ok" << endl;

    const bool ws = communicator->getProperties()->getProperty("Ice.Default.Protocol") == "test-ws";
    const bool wss = communicator->getProperties()->getProperty("Ice.Default.Protocol") == "test-wss";
    if(!ws && !wss)
    {
        cout << "testing buffered transport... " << flush;

        configuration->buffered(true);
        backgroundController->buffered(true);
        background->begin_op();
        background->ice_getCachedConnection()->close(true);
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

        cout << "ok" << endl;
    }

    return background;
}

void
connectTests(const ConfigurationPtr& configuration, const Test::BackgroundPrx& background)
{
    try
    {
        background->op();
    }
    catch(const Ice::LocalException&)
    {
        test(false);
    }
    background->ice_getConnection()->close(false);

    int i;
    for(i = 0; i < 4; ++i)
    {
        if(i == 0 || i == 2)
        {
            configuration->connectorsException(new Ice::DNSException(__FILE__, __LINE__));
        }
        else
        {
            configuration->connectException(new Ice::SocketException(__FILE__, __LINE__));
        }
        BackgroundPrx prx = (i == 1 || i == 3) ? background : background->ice_oneway();

        try
        {
            prx->op();
            test(false);
        }
        catch(const Ice::Exception&)
        {
        }

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

    for(i = 0; i < 5; i++)
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
        background->ice_getCachedConnection()->close(true);
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
                const Test::BackgroundPrx& background,
                const Test::BackgroundControllerPrx& ctl)
{
    try
    {
        background->op();
    }
    catch(const Ice::LocalException&)
    {
        test(false);
    }
    background->ice_getConnection()->close(false);

    int i;
    for(i = 0; i < 4; i++)
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
        BackgroundPrx prx = (i == 1 || i == 3) ? background : background->ice_oneway();

        try
        {
            prx->op();
            test(false);
        }
        catch(const Ice::SocketException&)
        {
        }

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
    catch(const Ice::LocalException&)
    {
        test(false);
    }
    background->ice_getConnection()->close(false);

    try
    {
        configuration->initializeSocketOperation(IceInternal::SocketOperationConnect);
        background->op();
        configuration->initializeSocketOperation(IceInternal::SocketOperationNone);
    }
    catch(const Ice::LocalException&)
    {
        test(false);
    }
    background->ice_getConnection()->close(false);
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
    catch(const Ice::LocalException&)
    {
        test(false);
    }
    background->ice_getConnection()->close(false);

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

    for(i = 0; i < 5; i++)
    {
        try
        {
            background->ice_ping();
        }
        catch(const Ice::LocalException&)
        {
            test(false);
        }

        configuration->initializeException(new Ice::SocketException(__FILE__, __LINE__));
        background->ice_getCachedConnection()->close(true);
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
        catch(const Ice::LocalException&)
        {
            test(false);
        }

        configuration->initializeSocketOperation(IceInternal::SocketOperationWrite);
        background->ice_getCachedConnection()->close(true);
        background->ice_ping();
        configuration->initializeSocketOperation(IceInternal::SocketOperationNone);

        ctl->initializeException(true);
        background->ice_getCachedConnection()->close(true);
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
        catch(const Ice::LocalException&)
        {
            test(false);
        }

        try
        {
#if !defined(ICE_USE_IOCP) && !defined(ICE_USE_CFSTREAM)
            ctl->initializeSocketOperation(IceInternal::SocketOperationWrite);
            background->ice_getCachedConnection()->close(true);
            background->op();
            ctl->initializeSocketOperation(IceInternal::SocketOperationNone);
#else
            background->ice_getCachedConnection()->close(true);
            background->op();
#endif
        }
        catch(const Ice::LocalException& ex)
        {
            cerr << ex << endl;
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
                const Test::BackgroundPrx& background,
                const Test::BackgroundControllerPrx& ctl)
{
    try
    {
        background->op();
    }
    catch(const Ice::LocalException&)
    {
        test(false);
    }
    background->ice_getConnection()->close(false);

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
        test(false);
    }

    int i;
    for(i = 0; i < 2; i++)
    {
        configuration->readException(new Ice::SocketException(__FILE__, __LINE__));
        BackgroundPrx prx = i == 0 ? background : background->ice_oneway();
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
            test(false);
        }
        background->ice_getConnection()->close(false);

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
            test(false);
        }

        for(int i = 0; i < 2; i++)
        {
            configuration->readReady(false);
            configuration->readException(new Ice::SocketException(__FILE__, __LINE__));
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
            configuration->readException(0);
            configuration->readReady(true);
        }
    }

    ctl->holdAdapter(); // Hold to block in connection validation
    Ice::AsyncResultPtr r = background->begin_op();
    Ice::AsyncResultPtr r2 = background->begin_op();
    test(!r->sentSynchronously() && !r2->sentSynchronously());
    test(!r->isCompleted() && !r2->isCompleted());
    ctl->resumeAdapter();
    background->end_op(r);
    background->end_op(r2);
    test(r->isCompleted() && r2->isCompleted());

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
        test(false);
    }
    background->ice_getConnection()->close(false);

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
        test(false);
    }
#if defined(ICE_USE_IOCP) || defined(ICE_USE_CFSTREAM)
    }
#endif

    Ice::ByteSeq seq;
    seq.resize(512 * 1024);

    BackgroundPrx backgroundBatchOneway = BackgroundPrx::uncheckedCast(background->ice_batchOneway());

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
    backgroundBatchOneway->begin_ice_flushBatchRequests();
    backgroundBatchOneway->ice_getConnection()->close(false);

    ctl->holdAdapter();
    backgroundBatchOneway->opWithPayload(seq);
    backgroundBatchOneway->opWithPayload(seq);
    backgroundBatchOneway->opWithPayload(seq);
    backgroundBatchOneway->opWithPayload(seq);
    ctl->resumeAdapter();
    r = backgroundBatchOneway->begin_ice_flushBatchRequests();
    //
    // We can't close the connection before ensuring all the batches
    // have been sent since with auto-flushing the close connection
    // message might be sent once the first call opWithPayload is sent
    // and before the flushBatchRequests (this would therefore result
    // in the flush to report a CloseConnectionException). Instead we
    // wait for the first flush to complete.
    //
    //backgroundBatchOneway->ice_getConnection()->close(false);
    backgroundBatchOneway->end_ice_flushBatchRequests(r);
    backgroundBatchOneway->ice_getConnection()->close(false);
}

void
readWriteTests(const ConfigurationPtr& configuration,
               const Test::BackgroundPrx& background,
               const Test::BackgroundControllerPrx& ctl)
{
    try
    {
        background->op();
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }

    int i;
    for(i = 0; i < 2; i++)
    {
        BackgroundPrx prx = i == 0 ? background : background->ice_oneway();

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

        for(i = 0; i < 2; ++i)
        {
            BackgroundPrx prx = i == 0 ? background : background->ice_oneway();

            background->ice_ping();
            configuration->writeReady(false);
            configuration->writeException(new Ice::SocketException(__FILE__, __LINE__));
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
            configuration->readReady(true);
            configuration->readException(0);
        }

        {
            background->ice_ping();
            configuration->readReady(false);
            configuration->writeReady(false);
            configuration->readException(new Ice::SocketException(__FILE__, __LINE__));
            Ice::AsyncResultPtr r = background->begin_op();
            // The read exception might propagate before the message send is seen as completed on IOCP.
#ifndef ICE_USE_IOCP
            r->waitForSent();
#endif
            try
            {
                background->end_op(r);
                test(false);
            }
            catch(const Ice::SocketException&)
            {
            }
            test(r->isCompleted());
            configuration->writeReady(true);
            configuration->readReady(true);
            configuration->readException(0);
        }
#if defined(ICE_USE_IOCP) || defined(ICE_USE_CFSTREAM)
    }
#endif

    background->ice_ping(); // Establish the connection

    BackgroundPrx backgroundOneway = BackgroundPrx::uncheckedCast(background->ice_oneway());
    test(backgroundOneway->ice_getConnection() == background->ice_getConnection());

    ctl->holdAdapter(); // Hold to block in request send.

    Ice::ByteSeq seq;
    seq.resize(10024); // Make sure the request doesn't compress too well.
    for(Ice::ByteSeq::iterator p = seq.begin(); p != seq.end(); ++p)
    {
        *p = static_cast<Ice::Byte>(IceUtilInternal::random(255));
    }
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

    for(i = 0; i < 5; i++)
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
        background->ice_getCachedConnection()->close(true);
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(10));

        background->ice_getCachedConnection()->close(true);
    }

    thread1->destroy();
    thread2->destroy();

    thread1->getThreadControl().join();
    thread2->getThreadControl().join();
}
