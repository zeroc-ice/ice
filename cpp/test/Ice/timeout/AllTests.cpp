// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

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

    bool check()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        while(!_called)
        {
            if(!timedWait(IceUtil::Time::seconds(5)))
            {
                return false;
            }
        }
        _called = false;
        return true;
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

class AMISendData : public Test::AMI_Timeout_sendData, public CallbackBase
{
public:

    virtual void ice_response()
    {
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }
};
typedef IceUtil::Handle<AMISendData> AMISendDataPtr;

class AMISendDataEx : public Test::AMI_Timeout_sendData, public CallbackBase
{
public:

    virtual void ice_response()
    {
        test(false);
    }

    virtual void ice_exception(const ::Ice::Exception& ex)
    {
        test(dynamic_cast<const Ice::TimeoutException*>(&ex));
        called();
    }
};
typedef IceUtil::Handle<AMISendDataEx> AMISendDataExPtr;

class AMISleep : public Test::AMI_Timeout_sleep, public CallbackBase
{
public:

    virtual void ice_response()
    {
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }
};
typedef IceUtil::Handle<AMISleep> AMISleepPtr;

class AMISleepEx : public Test::AMI_Timeout_sleep, public CallbackBase
{
public:

    virtual void ice_response()
    {
        test(false);
    }

    virtual void ice_exception(const ::Ice::Exception& ex)
    {
        test(dynamic_cast<const Ice::TimeoutException*>(&ex));
        called();
    }
};
typedef IceUtil::Handle<AMISleepEx> AMISleepExPtr;

TimeoutPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    string sref = "timeout:default -p 12010 -t 10000";
    Ice::ObjectPrx obj = communicator->stringToProxy(sref);
    test(obj);

    TimeoutPrx timeout = TimeoutPrx::checkedCast(obj);
    test(timeout);

    cout << "testing connect timeout... " << flush;
    {
        //
        // Expect ConnectTimeoutException.
        //
        TimeoutPrx to = TimeoutPrx::uncheckedCast(obj->ice_timeout(500));
        to->holdAdapter(750);
        to->ice_getConnection()->close(true); // Force a reconnect.
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
        TimeoutPrx to = TimeoutPrx::uncheckedCast(obj->ice_timeout(1000));
        to->holdAdapter(500);
        to->ice_getConnection()->close(true); // Force a reconnect.
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

    cout << "testing read timeout... " << flush;
    {
        //
        // Expect TimeoutException.
        //
        TimeoutPrx to = TimeoutPrx::uncheckedCast(obj->ice_timeout(500));
        try
        {
            to->sleep(750);
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
        TimeoutPrx to = TimeoutPrx::uncheckedCast(obj->ice_timeout(1000));
        try
        {
            to->sleep(500);
        }
        catch(const Ice::TimeoutException&)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "testing write timeout... " << flush;
    {
        //
        // Expect TimeoutException.
        //
        TimeoutPrx to = TimeoutPrx::uncheckedCast(obj->ice_timeout(500));
        to->holdAdapter(750);
        try
        {
            ByteSeq seq(100000);
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
        TimeoutPrx to = TimeoutPrx::uncheckedCast(obj->ice_timeout(1000));
        to->holdAdapter(500);
        try
        {
            ByteSeq seq(100000);
            to->sendData(seq);
        }
        catch(const Ice::TimeoutException&)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "testing AMI read timeout... " << flush;
    {
        //
        // The resolution of AMI timeouts is limited by the connection monitor
        // thread. We set Ice.MonitorConnections=1 (one second) in main().
        //
        // Expect TimeoutException.
        //
        TimeoutPrx to = TimeoutPrx::uncheckedCast(obj->ice_timeout(500));
        AMISleepExPtr cb = new AMISleepEx;
        to->sleep_async(cb, 2000);
        test(cb->check());
    }
    {
        //
        // Expect success.
        //
        timeout->op(); // Ensure adapter is active.
        TimeoutPrx to = TimeoutPrx::uncheckedCast(obj->ice_timeout(1000));
        AMISleepPtr cb = new AMISleep;
        to->sleep_async(cb, 500);
        test(cb->check());
    }
    cout << "ok" << endl;

    cout << "testing AMI write timeout... " << flush;
    {
        //
        // The resolution of AMI timeouts is limited by the connection monitor
        // thread. We set Ice.MonitorConnections=1 (one second) in main().
        //
        // Expect TimeoutException.
        //
        TimeoutPrx to = TimeoutPrx::uncheckedCast(obj->ice_timeout(500));
        to->holdAdapter(2000);
        ByteSeq seq(100000);
        AMISendDataExPtr cb = new AMISendDataEx;
        to->sendData_async(cb, seq);
        test(cb->check());
    }
    {
        //
        // Expect success.
        //
        timeout->op(); // Ensure adapter is active.
        TimeoutPrx to = TimeoutPrx::uncheckedCast(obj->ice_timeout(1000));
        to->holdAdapter(500);
        ByteSeq seq(100000);
        AMISendDataPtr cb = new AMISendData;
        to->sendData_async(cb, seq);
        test(cb->check());
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
        initData.properties->setProperty("Ice.Override.Timeout", "500");
        Ice::CommunicatorPtr comm = Ice::initialize(initData);
        TimeoutPrx to = TimeoutPrx::checkedCast(comm->stringToProxy(sref));
        try
        {
            to->sleep(750);
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
        to = TimeoutPrx::checkedCast(to->ice_timeout(1000));
        try
        {
            to->sleep(750);
            test(false);
        }
        catch(const Ice::TimeoutException&)
        {
            // Expected.
        }
        comm->destroy();
    }
    {
        //
        // Test Ice.Override.ConnectTimeout.
        //
        Ice::InitializationData initData;
        initData.properties = communicator->getProperties()->clone();
        initData.properties->setProperty("Ice.Override.ConnectTimeout", "750");
        Ice::CommunicatorPtr comm = Ice::initialize(initData);
        timeout->holdAdapter(1000);
        TimeoutPrx to = TimeoutPrx::uncheckedCast(comm->stringToProxy(sref));
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
        timeout->holdAdapter(1000);
        to = TimeoutPrx::uncheckedCast(to->ice_timeout(1250));
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
        to->op(); // Force connection.
        try
        {
            to->sleep(1500);
            test(false);
        }
        catch(const Ice::TimeoutException&)
        {
            // Expected.
        }
        comm->destroy();
    }
    cout << "ok" << endl;

    return timeout;
}
