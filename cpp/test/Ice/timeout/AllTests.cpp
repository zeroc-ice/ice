// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>
#include <TestI.h>

using namespace std;
using namespace Test;

class CallbackBase : public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    CallbackBase() :
        _called(false)
    {
    }

    virtual ~CallbackBase()
    {
    }

    void check()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        while(!_called)
        {
            wait();
        }
        _called = false;
    }

protected:

    void called()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        assert(!_called);
        _called = true;
        notify();
    }

private:

    bool _called;
};

class Callback : public IceUtil::Shared, public CallbackBase
{
public:

    void response()
    {
        called();
    }

    void exception(const ::Ice::Exception&)
    {
        test(false);
    }

    void responseEx()
    {
        test(false);
    }

    void exceptionEx(const ::Ice::Exception& ex)
    {
        test(dynamic_cast<const Ice::InvocationTimeoutException*>(&ex));
        called();
    }
};
typedef IceUtil::Handle<Callback> CallbackPtr;

TimeoutPrxPtr
allTests(const Ice::CommunicatorPtr& communicator)
{
    string sref = "timeout:" + getTestEndpoint(communicator, 0);
    Ice::ObjectPrxPtr obj = communicator->stringToProxy(sref);
    test(obj);

    TimeoutPrxPtr timeout = ICE_CHECKED_CAST(TimeoutPrx, obj);
    test(timeout);

    cout << "testing connect timeout... " << flush;
    {
        //
        // Expect ConnectTimeoutException.
        //
        TimeoutPrxPtr to = ICE_UNCHECKED_CAST(TimeoutPrx, obj->ice_timeout(100));
        timeout->holdAdapter(500);
        try
        {
            to->op();
            test(false);
        }
        catch(const Ice::ConnectTimeoutException&)
        {
            // Expected.
        }
    }
    {
        //
        // Expect success.
        //
        timeout->op(); // Ensure adapter is active.
        TimeoutPrxPtr to = ICE_UNCHECKED_CAST(TimeoutPrx, obj->ice_timeout(1000));
        timeout->holdAdapter(500);
        try
        {
            to->op();
        }
        catch(const Ice::ConnectTimeoutException&)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    // The sequence needs to be large enough to fill the write/recv buffers
    ByteSeq seq(2000000);

    cout << "testing connection timeout... " << flush;
    {
        //
        // Expect TimeoutException.
        //
        TimeoutPrxPtr to = ICE_UNCHECKED_CAST(TimeoutPrx, obj->ice_timeout(100));
        timeout->holdAdapter(500);
        try
        {
            to->sendData(seq);
            test(false);
        }
        catch(const Ice::TimeoutException&)
        {
            // Expected.
        }
    }
    {
        //
        // Expect success.
        //
        timeout->op(); // Ensure adapter is active.
        TimeoutPrxPtr to = ICE_UNCHECKED_CAST(TimeoutPrx, obj->ice_timeout(1000));
        timeout->holdAdapter(500);
        try
        {
            ByteSeq seq(1000000);
            to->sendData(seq);
        }
        catch(const Ice::TimeoutException&)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "testing invocation timeout... " << flush;
    {
        Ice::ConnectionPtr connection = obj->ice_getConnection();
        TimeoutPrxPtr to = ICE_UNCHECKED_CAST(TimeoutPrx, obj->ice_invocationTimeout(100));
        test(connection == to->ice_getConnection());
        try
        {
            to->sleep(750);
            test(false);
        }
        catch(const Ice::InvocationTimeoutException&)
        {
        }
        obj->ice_ping();
        to = ICE_CHECKED_CAST(TimeoutPrx, obj->ice_invocationTimeout(500));
        test(connection == to->ice_getConnection());
        try
        {
            to->sleep(100);
        }
        catch(const Ice::InvocationTimeoutException&)
        {
            test(false);
        }
        test(connection == to->ice_getConnection());
    }
    {
        //
        // Expect InvocationTimeoutException.
        //
        TimeoutPrxPtr to = ICE_UNCHECKED_CAST(TimeoutPrx, obj->ice_invocationTimeout(100));

#ifdef ICE_CPP11_MAPPING
        auto f = to->sleepAsync(750);
        try
        {
            f.get();
            test(false);
        }
        catch(const Ice::InvocationTimeoutException&)
        {
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback();
        to->begin_sleep(750, newCallback_Timeout_sleep(cb, &Callback::responseEx, &Callback::exceptionEx));
        cb->check();
#endif
        obj->ice_ping();
    }
    {
        //
        // Expect success.
        //
        TimeoutPrxPtr to = ICE_UNCHECKED_CAST(TimeoutPrx, obj->ice_invocationTimeout(500));
#ifdef ICE_CPP11_MAPPING
        auto f = to->sleepAsync(100);
        try
        {
            f.get();
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback();
        to->begin_sleep(100, newCallback_Timeout_sleep(cb, &Callback::response, &Callback::exception));
        cb->check();
#endif
    }
    {
        //
        // Backward compatible connection timeouts
        //
        TimeoutPrxPtr to = ICE_UNCHECKED_CAST(TimeoutPrx, obj->ice_invocationTimeout(-2)->ice_timeout(250));
        Ice::ConnectionPtr con = to->ice_getConnection();
        try
        {
            to->sleep(750);
            test(false);
        }
        catch(const Ice::TimeoutException&)
        {
            try
            {
                con->getInfo();
                test(false);
            }
            catch(const Ice::TimeoutException&)
            {
                // Connection got closed as well.
            }
        }
        obj->ice_ping();
        try
        {
            con = to->ice_getConnection();
#ifdef ICE_CPP11_MAPPING
            to->sleepAsync(750).get();
#else
            to->end_sleep(to->begin_sleep(750));
#endif
            test(false);
        }
        catch(const Ice::TimeoutException&)
        {
            try
            {
                con->getInfo();
                test(false);
            }
            catch(const Ice::TimeoutException&)
            {
                // Connection got closed as well.
            }
        }
        obj->ice_ping();
    }
    cout << "ok" << endl;

    cout << "testing close timeout... " << flush;
    {
        TimeoutPrxPtr to = ICE_CHECKED_CAST(TimeoutPrx, obj->ice_timeout(250));
        Ice::ConnectionPtr connection = to->ice_getConnection();
        timeout->holdAdapter(600);
        connection->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
        try
        {
            connection->getInfo(); // getInfo() doesn't throw in the closing state.
        }
        catch(const Ice::LocalException&)
        {
            test(false);
        }
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(650));
        try
        {
            connection->getInfo();
            test(false);
        }
        catch(const Ice::ConnectionManuallyClosedException& ex)
        {
            // Expected.
            test(ex.graceful);
        }
        timeout->op(); // Ensure adapter is active.
    }
    cout << "ok" << endl;

    cout << "testing timeout overrides... " << flush;
    {
        //
        // Test Ice.Override.Timeout. This property overrides all
        // endpoint timeouts.
        //
        Ice::InitializationData initData;
        initData.properties = communicator->getProperties()->clone();
        initData.properties->setProperty("Ice.Override.Timeout", "250");
        Ice::CommunicatorHolder ich(initData);
        TimeoutPrxPtr to = ICE_CHECKED_CAST(TimeoutPrx, ich->stringToProxy(sref));
        timeout->holdAdapter(700);
        try
        {
            to->sendData(seq);
            test(false);
        }
        catch(const Ice::TimeoutException&)
        {
            // Expected.
        }

        //
        // Calling ice_timeout() should have no effect.
        //
        timeout->op(); // Ensure adapter is active.
        to = ICE_CHECKED_CAST(TimeoutPrx, to->ice_timeout(1000));
        timeout->holdAdapter(500);
        try
        {
            to->sendData(seq);
            test(false);
        }
        catch(const Ice::TimeoutException&)
        {
            // Expected.
        }
    }
    {
        //
        // Test Ice.Override.ConnectTimeout.
        //
        Ice::InitializationData initData;
        initData.properties = communicator->getProperties()->clone();
        initData.properties->setProperty("Ice.Override.ConnectTimeout", "250");
        Ice::CommunicatorHolder ich(initData);
        timeout->holdAdapter(750);
        TimeoutPrxPtr to = ICE_UNCHECKED_CAST(TimeoutPrx, ich->stringToProxy(sref));
        try
        {
            to->op();
            test(false);
        }
        catch(const Ice::ConnectTimeoutException&)
        {
            // Expected.
        }
        //
        // Calling ice_timeout() should have no effect on the connect timeout.
        //
        timeout->op(); // Ensure adapter is active.
        timeout->holdAdapter(750);
        to = ICE_UNCHECKED_CAST(TimeoutPrx, to->ice_timeout(1000));
        try
        {
            to->op();
            test(false);
        }
        catch(const Ice::ConnectTimeoutException&)
        {
            // Expected.
        }
        //
        // Verify that timeout set via ice_timeout() is still used for requests.
        //
        timeout->op(); // Ensure adapter is active.
        to = ICE_UNCHECKED_CAST(TimeoutPrx, to->ice_timeout(250));
        to->ice_getConnection(); // Establish connection
        timeout->holdAdapter(750);
        try
        {
            to->sendData(seq);
            test(false);
        }
        catch(const Ice::TimeoutException&)
        {
            // Expected.
        }
    }
    {
        //
        // Test Ice.Override.CloseTimeout.
        //
        Ice::InitializationData initData;
        initData.properties = communicator->getProperties()->clone();
        initData.properties->setProperty("Ice.Override.CloseTimeout", "100");
        Ice::CommunicatorHolder ich(initData);
        Ice::ConnectionPtr connection = ich->stringToProxy(sref)->ice_getConnection();
        timeout->holdAdapter(800);
        IceUtil::Time now = IceUtil::Time::now();
        ich.release()->destroy();
        test(IceUtil::Time::now() - now < IceUtil::Time::milliSeconds(700));
    }
    cout << "ok" << endl;

    cout << "testing invocation timeouts with collocated calls... " << flush;
    {
        communicator->getProperties()->setProperty("TimeoutCollocated.AdapterId", "timeoutAdapter");

        Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TimeoutCollocated");
        adapter->activate();

        TimeoutPrxPtr timeout = ICE_UNCHECKED_CAST(TimeoutPrx, adapter->addWithUUID(ICE_MAKE_SHARED(TimeoutI)));
        timeout = timeout->ice_invocationTimeout(100);
        try
        {
            timeout->sleep(500);
            test(false);
        }
        catch(const Ice::InvocationTimeoutException&)
        {
        }

        try
        {
#ifdef ICE_CPP11_MAPPING
            timeout->sleepAsync(500).get();
#else
            timeout->end_sleep(timeout->begin_sleep(500));
#endif
            test(false);
        }
        catch(const Ice::InvocationTimeoutException&)
        {
        }

        try
        {
            timeout->ice_invocationTimeout(-2)->ice_ping();
            #ifdef ICE_CPP11_MAPPING
            timeout->ice_invocationTimeout(-2)->ice_pingAsync().get();
            #else
            timeout->ice_invocationTimeout(-2)->begin_ice_ping()->waitForCompleted();
            #endif
        }
        catch(const Ice::Exception&)
        {
            test(false);
        }

        TimeoutPrxPtr batchTimeout = timeout->ice_batchOneway();
        batchTimeout->ice_ping();
        batchTimeout->ice_ping();
        batchTimeout->ice_ping();

        // Keep the server thread pool busy.
#ifdef ICE_CPP11_MAPPING
        timeout->ice_invocationTimeout(-1)->sleepAsync(300);
#else
        timeout->ice_invocationTimeout(-1)->begin_sleep(300);
#endif
        try
        {
            batchTimeout->ice_flushBatchRequests();
            test(false);
        }
        catch(const Ice::InvocationTimeoutException&)
        {
        }

        batchTimeout->ice_ping();
        batchTimeout->ice_ping();
        batchTimeout->ice_ping();

        // Keep the server thread pool busy.
#ifdef ICE_CPP11_MAPPING
        timeout->ice_invocationTimeout(-1)->sleepAsync(300);
#else
        timeout->ice_invocationTimeout(-1)->begin_sleep(300);
#endif
        try
        {
#ifdef ICE_CPP11_MAPPING
            batchTimeout->ice_flushBatchRequestsAsync().get();
#else
            batchTimeout->end_ice_flushBatchRequests(batchTimeout->begin_ice_flushBatchRequests());
#endif
            test(false);
        }
        catch(const Ice::InvocationTimeoutException&)
        {
        }

        adapter->destroy();
    }
    cout << "ok" << endl;

    return timeout;
}
