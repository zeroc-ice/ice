// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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

class OpAMICallback : public Test::AMI_Background_op, public Ice::AMISentCallback
{
public:
    
    virtual void 
    ice_response()
    {
        _response.called();
    }

    virtual void
    ice_exception(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        test(false);
    }

    virtual void
    ice_sent()
    {
        _sent.called();
    }

    bool
    response(bool wait)
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
    responseAndSent()
    {
        _sent.check();
        _response.check();
    }

private:

    Callback _response;
    Callback _sent;
};
typedef IceUtil::Handle<OpAMICallback> OpAMICallbackPtr; 

class FlushBatchRequestsCallback : public Ice::AMI_Object_ice_flushBatchRequests
{
public:
    
    virtual void
    ice_exception(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        test(false);
    }
};
typedef IceUtil::Handle<FlushBatchRequestsCallback> FlushBatchRequestsCallbackPtr; 

class OpExAMICallback : public Test::AMI_Background_op, public Ice::AMISentCallback
{
public:
    
    virtual void 
    ice_response()
    {
        test(false);
    }

    virtual void
    ice_exception(const Ice::Exception& ex)
    {
        _exception.called();
    }
    
    virtual void
    ice_sent()
    {
        _sent.called();
    }

    void
    exception()
    {
        _exception.check();
    }

    bool
    sent(bool wait)
    {
        if(wait)
        {
            _sent.check();
            return true;
        }
        else
        {
            return _sent.isCalled();
        }
    }

private:

    Callback _exception;
    Callback _sent;
};
typedef IceUtil::Handle<OpExAMICallback> OpExAMICallbackPtr; 

class OpWithPayloadOnewayAMICallback : public Test::AMI_Background_opWithPayload
{
public:

    virtual void
    ice_response()
    {
        test(false);
    }

    virtual void
    ice_exception(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        test(false);
    }
};
typedef IceUtil::Handle<OpWithPayloadOnewayAMICallback> OpWithPayloadOnewayAMICallbackPtr; 

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
                _background->op_async(new OpExAMICallback());
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
        obj = communicator->stringToProxy("locator:default -p 12010 -t 500");
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
        OpAMICallbackPtr cb = new OpAMICallback();
        bg->op_async(cb);
        OpAMICallbackPtr cb2 = new OpAMICallback();
        bg->op_async(cb2);
        test(!cb->response(false));
        test(!cb2->response(false));
        backgroundController->resumeCall("findAdapterById");
        test(cb->response(true));
        test(cb2->response(true));
    }
    cout << "ok" << endl;

    cout << "testing router... " << flush;
    {
        Ice::RouterPrx router;

        obj = communicator->stringToProxy("router:default -p 12010 -t 500");
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
        OpAMICallbackPtr cb = new OpAMICallback();
        bg->op_async(cb);
        OpAMICallbackPtr cb2 = new OpAMICallback();
        bg->op_async(cb2);
        test(!cb->response(false));
        test(!cb2->response(false));
        backgroundController->resumeCall("getClientProxy");
        test(cb->response(true));
        test(cb2->response(true));
    }
    cout << "ok" << endl;

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
        
    try
    {
        configuration->connectorsException(new Ice::DNSException(__FILE__, __LINE__));
        background->op();
        test(false);
    }
    catch(const Ice::DNSException&)
    {
        configuration->connectorsException(0);
    }

    OpExAMICallbackPtr cbEx = new OpExAMICallback();

    configuration->connectorsException(new Ice::DNSException(__FILE__, __LINE__));
    test(!background->op_async(cbEx));
    cbEx->exception();
    configuration->connectorsException(0);

    configuration->connectorsException(new Ice::DNSException(__FILE__, __LINE__));
    test(!background->ice_oneway()->op_async(cbEx));
    cbEx->exception();
    configuration->connectorsException(0);

    try
    {
        configuration->connectException(new Ice::SocketException(__FILE__, __LINE__));
        background->op();
        test(false);
    }
    catch(const Ice::SocketException&)
    {
        configuration->connectException(0);
    }

    configuration->connectException(new Ice::SocketException(__FILE__, __LINE__));
    test(!background->op_async(cbEx));
    cbEx->exception();
    configuration->connectException(0);

    configuration->connectException(new Ice::SocketException(__FILE__, __LINE__));
    test(!background->ice_oneway()->op_async(cbEx));
    cbEx->exception();
    configuration->connectException(0);

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
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    background->ice_getConnection()->close(false);

    try
    {
        configuration->initializeException(new Ice::SocketException(__FILE__, __LINE__));
        background->op();
        test(false);
    }
    catch(const Ice::SocketException&)
    {
        configuration->initializeException(0);
    }

    OpExAMICallbackPtr cbEx = new OpExAMICallback();

    configuration->initializeException(new Ice::SocketException(__FILE__, __LINE__));
    test(!background->op_async(cbEx));
    cbEx->exception();
    configuration->initializeException(0);

    configuration->initializeException(new Ice::SocketException(__FILE__, __LINE__));
    test(!background->ice_oneway()->op_async(cbEx));
    cbEx->exception();
    configuration->initializeException(0);

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

    try
    {
        configuration->initializeSocketOperation(IceInternal::SocketOperationWrite);
        configuration->initializeException(new Ice::SocketException(__FILE__, __LINE__));
        background->op();
        test(false);
    }
    catch(const Ice::SocketException&)
    {
        configuration->initializeException(0);
        configuration->initializeSocketOperation(IceInternal::SocketOperationNone);
    }

    configuration->initializeSocketOperation(IceInternal::SocketOperationWrite);
    configuration->initializeException(new Ice::SocketException(__FILE__, __LINE__));
    test(!background->op_async(cbEx));
    cbEx->exception();
    configuration->initializeException(0);
    configuration->initializeSocketOperation(IceInternal::SocketOperationNone);

    configuration->initializeSocketOperation(IceInternal::SocketOperationWrite);
    configuration->initializeException(new Ice::SocketException(__FILE__, __LINE__));
    test(!background->ice_oneway()->op_async(cbEx));
    cbEx->exception();
    configuration->initializeException(0);
    configuration->initializeSocketOperation(IceInternal::SocketOperationNone);
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

    OpExAMICallbackPtr cbEx = new OpExAMICallback();

    configuration->readException(new Ice::SocketException(__FILE__, __LINE__));
    test(!background->op_async(cbEx));
    cbEx->exception();
    configuration->readException(0);

    configuration->readException(new Ice::SocketException(__FILE__, __LINE__));
    test(!background->ice_oneway()->op_async(cbEx));
    cbEx->exception();
    configuration->readException(0);

    if(background->ice_getCommunicator()->getProperties()->getProperty("Ice.Default.Protocol") != "test-ssl")
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

        configuration->readReady(false);
        configuration->readException(new Ice::SocketException(__FILE__, __LINE__));
        test(!background->op_async(cbEx));
        cbEx->exception();
        configuration->readException(0);
        configuration->readReady(true);

        configuration->readReady(false);
        configuration->readException(new Ice::SocketException(__FILE__, __LINE__));
        test(!background->ice_oneway()->op_async(cbEx));
        cbEx->exception();
        configuration->readException(0);
        configuration->readReady(true);
    }

    ctl->holdAdapter(); // Hold to block in connection validation
    OpAMICallbackPtr cb = new OpAMICallback();
    test(!background->op_async(cb));
    OpAMICallbackPtr cb2 = new OpAMICallback();
    test(!background->op_async(cb2));
    test(!cb->response(false));
    test(!cb2->response(false));
    ctl->resumeAdapter();
    cb->responseAndSent();
    cb2->responseAndSent();

#if defined(ICE_USE_IOCP) || defined(ICE_USE_CFSTREAM)
    if(background->ice_getCommunicator()->getProperties()->getProperty("Ice.Default.Protocol") != "test-ssl")
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
    backgroundBatchOneway->ice_ping();
    backgroundBatchOneway->ice_getConnection()->close(false);
    try
    {
        backgroundBatchOneway->ice_ping();
        test(false);
    }
    catch(const Ice::CloseConnectionException&)
    {
    }
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
    backgroundBatchOneway->ice_ping();
    backgroundBatchOneway->ice_getConnection()->close(false);
    try
    {
        backgroundBatchOneway->ice_ping();
        test(false);
    }
    catch(const Ice::CloseConnectionException&)
    {
    }
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

    backgroundBatchOneway->ice_ping();
    backgroundBatchOneway->ice_getConnection()->close(false);
    try
    {
        backgroundBatchOneway->ice_ping();
        test(false);
    }
    catch(const Ice::CloseConnectionException&)
    {
    }
    ctl->holdAdapter();
    backgroundBatchOneway->op();
    backgroundBatchOneway->op();
    backgroundBatchOneway->op();
    backgroundBatchOneway->op();
    ctl->resumeAdapter();
    FlushBatchRequestsCallbackPtr fcb = new FlushBatchRequestsCallback();
    backgroundBatchOneway->ice_flushBatchRequests_async(fcb);
    backgroundBatchOneway->ice_getConnection()->close(false);

    backgroundBatchOneway->ice_ping();
    backgroundBatchOneway->ice_getConnection()->close(false);
    try
    {
        backgroundBatchOneway->ice_ping();
        test(false);
    }
    catch(const Ice::CloseConnectionException&)
    {
    }
    ctl->holdAdapter();
    backgroundBatchOneway->opWithPayload(seq);
    backgroundBatchOneway->opWithPayload(seq);
    backgroundBatchOneway->opWithPayload(seq);
    backgroundBatchOneway->opWithPayload(seq);
    ctl->resumeAdapter();
    fcb = new FlushBatchRequestsCallback();
    backgroundBatchOneway->ice_flushBatchRequests_async(fcb);
    //
    // We can't close the connection before ensuring all the batches have been sent since
    // with auto-flushing the close connection message might be sent once the first call
    // opWithPayload is sent and before the flushBatchRequests (this would therefore result
    // in the flush to report a CloseConnectionException). Instead we flush a second time 
    // with the same callback to wait for the first flush to complete.
    // 
    //backgroundBatchOneway->ice_getConnection()->close(false);
    backgroundBatchOneway->ice_flushBatchRequests_async(fcb);
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
        
    try
    {
        background->ice_ping();
        configuration->writeException(new Ice::SocketException(__FILE__, __LINE__)); 
        background->op();
        test(false);
    }
    catch(const Ice::SocketException&)
    {
        configuration->writeException(0);
    }

    OpExAMICallbackPtr cbEx = new OpExAMICallback();

    background->ice_ping();
    configuration->writeException(new Ice::SocketException(__FILE__, __LINE__));
    test(!background->op_async(cbEx));
    cbEx->exception();
    test(!cbEx->sent(false));
    configuration->writeException(0);

    background->ice_ping();
    configuration->writeException(new Ice::SocketException(__FILE__, __LINE__));
    test(!background->ice_oneway()->op_async(cbEx));
    cbEx->exception();
    test(!cbEx->sent(false));
    configuration->writeException(0);

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
    if(!background->op_async(cbEx))
    {
        // The read exception might propagate before the message send is seen as completed on IOCP.
#ifndef ICE_USE_IOCP        
        test(cbEx->sent(true));
#endif
    }
    cbEx->exception();
    configuration->readException(0);
    configuration->readReady(true);

#if defined(ICE_USE_IOCP) || defined(ICE_USE_CFSTREAM)
    if(background->ice_getCommunicator()->getProperties()->getProperty("Ice.Default.Protocol") != "test-ssl")
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

    background->ice_ping();
    configuration->writeReady(false);
    configuration->writeException(new Ice::SocketException(__FILE__, __LINE__));
    test(!background->op_async(cbEx));
    cbEx->exception();
    test(!cbEx->sent(false));
    configuration->writeReady(true);
    configuration->writeException(0);

    background->ice_ping();
    configuration->writeReady(false);
    configuration->writeException(new Ice::SocketException(__FILE__, __LINE__));
    test(!background->ice_oneway()->op_async(cbEx));
    cbEx->exception();
    test(!cbEx->sent(false));
    configuration->writeReady(true);
    configuration->writeException(0);

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

    background->ice_ping();
    configuration->readReady(false);
    configuration->readException(new Ice::SocketException(__FILE__, __LINE__));
    if(!background->op_async(cbEx))
    {
        // The read exception might propagate before the message send is seen as completed on IOCP.
#ifndef ICE_USE_IOCP        
        test(cbEx->sent(true));
#endif
    }
    cbEx->exception();
    configuration->readReady(true);
    configuration->readException(0);

    background->ice_ping();
    configuration->readReady(false);
    configuration->writeReady(false);
    configuration->readException(new Ice::SocketException(__FILE__, __LINE__));
    test(!background->op_async(cbEx));
    // The read exception might propagate before the message send is seen as completed on IOCP.
#ifndef ICE_USE_IOCP        
    test(cbEx->sent(true));
#endif
    cbEx->exception();
    configuration->writeReady(true);
    configuration->readReady(true);
    configuration->readException(0);
#if defined(ICE_USE_IOCP) || defined(ICE_USE_CFSTREAM)
    }
#endif

    background->ice_ping(); // Establish the connection
    
    BackgroundPrx backgroundOneway = BackgroundPrx::uncheckedCast(background->ice_oneway());
    test(backgroundOneway->ice_getConnection() == background->ice_getConnection());
    
    ctl->holdAdapter(); // Hold to block in request send.
    
    Ice::ByteSeq seq;
    seq.resize(512 * 1024); // Make sure the request doesn't compress too well.
    for(Ice::ByteSeq::iterator p = seq.begin(); p != seq.end(); ++p)
    {
        *p = static_cast<Ice::Byte>(IceUtilInternal::random(255));
    }
    while(backgroundOneway->opWithPayload_async(new OpWithPayloadOnewayAMICallback(), seq))
    {
    }
    OpAMICallbackPtr cb = new OpAMICallback();
    test(!background->op_async(cb));
    OpAMICallbackPtr cb2 = new OpAMICallback();
    test(!background->op_async(cb2));
    test(!backgroundOneway->opWithPayload_async(new OpWithPayloadOnewayAMICallback(), seq));
    test(!backgroundOneway->opWithPayload_async(new OpWithPayloadOnewayAMICallback(), seq));
    test(!cb->response(false));
    test(!cb2->response(false));
    ctl->resumeAdapter();
    cb->responseAndSent();
    cb2->responseAndSent();
    
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
        background->ice_getCachedConnection()->close(true);
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(10));

        background->ice_getCachedConnection()->close(true);
    }

    thread1->destroy();
    thread2->destroy();

    thread1->getThreadControl().join();
    thread2->getThreadControl().join();
}
