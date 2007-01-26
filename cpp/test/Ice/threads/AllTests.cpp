// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/Thread.h>
#include <TestCommon.h>
#include <Test.h>
#include <set>

using namespace std;

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

class AMI_Adapter_waitForWakeupI : public Test::AMI_Adapter_waitForWakeup, public CallbackBase
{
public:

    virtual void ice_response(Ice::Int, bool);
    virtual void ice_exception(const Ice::Exception&);

    Ice::Int id() const;
    bool notified() const;

private:

    Ice::Int _id;
    bool _notified;
};
typedef IceUtil::Handle<AMI_Adapter_waitForWakeupI> AMI_Adapter_waitForWakeupIPtr;

void
AMI_Adapter_waitForWakeupI::ice_response(Ice::Int id, bool notified)
{
    _id = id;
    _notified = notified;
    called();
}

void
AMI_Adapter_waitForWakeupI::ice_exception(const Ice::Exception&)
{
    test(false);
}

Ice::Int
AMI_Adapter_waitForWakeupI::id() const
{
    return _id;
}

bool
AMI_Adapter_waitForWakeupI::notified() const
{
    return _notified;
}

class Counter : public IceUtil::Shared, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    Counter(int);

    bool bump(int);

private:

    int _target;
    int _count;
};
typedef IceUtil::Handle<Counter> CounterPtr;

Counter::Counter(int target) :
    _target(target),
    _count(0)
{
}

bool
Counter::bump(int timeout)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(_count < _target);

    _count++;
    notifyAll();

    IceUtil::Time now = IceUtil::Time::now();
    IceUtil::Time end;
    if(timeout > 0)
    {
        end = now + IceUtil::Time::milliSeconds(timeout);
    }

    while(_count < _target)
    {
        if(timeout > 0)
        {
            timedWait(end - now);
            now = IceUtil::Time::now();
            if(now >= end)
            {
                break;
            }
        }
        else
        {
            wait();
        }
    }
    return _count == _target;
}

class AMI_Server_pingI : public Test::AMI_Server_ping, public CallbackBase
{
public:

    AMI_Server_pingI(const CounterPtr&, int);

    virtual void ice_response();
    virtual void ice_exception(const Ice::Exception&);

    IceUtil::ThreadControl::ID id() const;
    bool ok() const;

private:

    CounterPtr _counter;
    int _timeout;
    IceUtil::ThreadControl::ID _id;
    bool _ok;
};
typedef IceUtil::Handle<AMI_Server_pingI> AMI_Server_pingIPtr;

AMI_Server_pingI::AMI_Server_pingI(const CounterPtr& counter, int timeout) :
    _counter(counter), _timeout(timeout), _ok(false)
{
}

void
AMI_Server_pingI::ice_response()
{
    _id = IceUtil::ThreadControl().id();
    _ok = _counter->bump(_timeout);
    called();
}

void
AMI_Server_pingI::ice_exception(const Ice::Exception&)
{
    test(false);
}

IceUtil::ThreadControl::ID
AMI_Server_pingI::id() const
{
    return _id;
}

bool
AMI_Server_pingI::ok() const
{
    return _ok;
}

Test::ServerPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    const string ref = "server:default -p 12010 -t 10000";
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);
    Test::ServerPrx server = Test::ServerPrx::checkedCast(base);

    cout << "testing proxy configurations... " << flush;
    {
        Ice::InitializationData id;
        id.properties = communicator->getProperties()->clone();
        id.properties->setProperty("Ice.ThreadPerConnection", "0");
        Ice::CommunicatorPtr comm = Ice::initialize(id);

        Ice::ObjectPrx proxy = comm->stringToProxy(ref);
        test(!proxy->ice_isThreadPerConnection());

        proxy = proxy->ice_oneway();
        test(!proxy->ice_isThreadPerConnection());

        proxy = proxy->ice_threadPerConnection(true);
        test(proxy->ice_isThreadPerConnection());

        proxy = proxy->ice_twoway();
        test(proxy->ice_isThreadPerConnection());

        proxy = proxy->ice_threadPerConnection(false);
        test(!proxy->ice_isThreadPerConnection());

        comm->destroy();
    }
    cout << "ok" << endl;

    cout << "testing connections... " << flush;
    {
        //
        // Proxies with different settings for thread-per-connection should use different connections.
        //
        Ice::InitializationData id;
        id.properties = communicator->getProperties()->clone();
        id.properties->setProperty("Ice.ThreadPerConnection", "0");
        Ice::CommunicatorPtr comm = Ice::initialize(id);

        Ice::ObjectPrx proxy1 = comm->stringToProxy(ref);
        test(!proxy1->ice_isThreadPerConnection());
        Ice::ConnectionPtr conn1 = proxy1->ice_getConnection();
        proxy1->ice_ping();

        Ice::ObjectPrx proxy2 = proxy1->ice_threadPerConnection(true);
        test(proxy2->ice_isThreadPerConnection());
        Ice::ConnectionPtr conn2 = proxy2->ice_getConnection();
        test(conn2 != conn1);
        proxy2->ice_ping();

        //
        // Verify connection re-use.
        //
        Ice::ObjectPrx proxy3 = comm->stringToProxy(ref);
        test(!proxy3->ice_isThreadPerConnection());
        Ice::ConnectionPtr conn3 = proxy3->ice_getConnection();
        test(conn3 == conn1);
        proxy3->ice_ping();

        Ice::ObjectPrx proxy4 = proxy3->ice_threadPerConnection(true);
        test(proxy4->ice_isThreadPerConnection());
        Ice::ConnectionPtr conn4 = proxy4->ice_getConnection();
        test(conn4 == conn2);
        proxy4->ice_ping();

        comm->destroy();
    }
    cout << "ok" << endl;

    cout << "testing object adapters... " << flush;
    {
        Test::AdapterSeq adapters = server->getAdapters();
        set<Ice::Int> threadPerConnectionThreads;
        set<Ice::Int> threadPoolThreads;

        for(Test::AdapterSeq::iterator p = adapters.begin(); p != adapters.end(); ++p)
        {
            Test::AdapterPrx adapter = *p;
            if(adapter->isThreadPerConnection())
            {
                //
                // For thread-per-connection, only one request can be dispatched at a time.
                // Since the adapters that use a thread pool are configured to allow up to
                // two threads, we can verify that the adapter is using thread-per-connection
                // by attempting to make two concurrent requests.
                //
                AMI_Adapter_waitForWakeupIPtr cb = new AMI_Adapter_waitForWakeupI;
                adapter->reset();
                adapter->waitForWakeup_async(cb, 250);
                Ice::Int tid = adapter->wakeup();
                test(cb->check());
                test(!cb->notified());
                test(tid == cb->id()); // The thread id must be the same for both requests.
                test(threadPerConnectionThreads.count(tid) == 0);
                threadPerConnectionThreads.insert(tid);

                //
                // Closing the connection and creating a new one should start a new thread
                // in the server.
                //
                adapter->ice_getConnection()->close(false);
                Ice::Int tid2 = adapter->getThreadId();
                test(tid != tid2);
                test(threadPerConnectionThreads.count(tid2) == 0);
                threadPerConnectionThreads.insert(tid2);
            }
            else
            {
                AMI_Adapter_waitForWakeupIPtr cb = new AMI_Adapter_waitForWakeupI;
                adapter->reset();
                adapter->waitForWakeup_async(cb, 250);
                Ice::Int tid = adapter->wakeup();
                test(cb->check());
                test(cb->notified());
                test(tid != cb->id()); // The thread ids must be different for the requests.
                test(threadPoolThreads.count(tid) == 0);
                threadPoolThreads.insert(tid);
                test(threadPoolThreads.count(cb->id()) == 0);
                threadPoolThreads.insert(cb->id());
            }
        }
    }
    cout << "ok" << endl;

    cout << "testing client thread pool... " << flush;
    {
        //
        // With two threads in the client-side thread pool, the AMI responses
        // should be dispatched concurrently.
        //
        Ice::InitializationData id;
        id.properties = communicator->getProperties()->clone();
        id.properties->setProperty("Ice.ThreadPerConnection", "0");
        id.properties->setProperty("Ice.ThreadPool.Client.SizeMax", "2");
        id.properties->setProperty("Ice.ThreadPool.Client.SizeWarn", "0");
        Ice::CommunicatorPtr comm = Ice::initialize(id);

        Test::ServerPrx srv = Test::ServerPrx::checkedCast(comm->stringToProxy(ref));
        test(!srv->ice_isThreadPerConnection());
        CounterPtr counter = new Counter(2);
        AMI_Server_pingIPtr cb1 = new AMI_Server_pingI(counter, 0);
        srv->ping_async(cb1);
        AMI_Server_pingIPtr cb2 = new AMI_Server_pingI(counter, 0);
        srv->ping_async(cb2);
        cb1->check();
        cb2->check();
        test(cb1->ok());
        test(cb2->ok());
#ifdef _WIN32
        test(cb1->id() != cb2->id());
#else
        test(pthread_equal(cb1->id(), cb2->id()) == 0);
#endif

        comm->destroy();
    }
    cout << "ok" << endl;

    cout << "testing client thread-per-connection... " << flush;
    {
        //
        // With thread-per-connection, AMI responses are dispatched synchronously by
        // the same thread.
        //
        Ice::InitializationData id;
        id.properties = communicator->getProperties()->clone();
        id.properties->setProperty("Ice.ThreadPerConnection", "1");
        Ice::CommunicatorPtr comm = Ice::initialize(id);

        Test::ServerPrx srv = Test::ServerPrx::checkedCast(comm->stringToProxy(ref));
        test(srv->ice_isThreadPerConnection());
        CounterPtr counter = new Counter(2);
        AMI_Server_pingIPtr cb1 = new AMI_Server_pingI(counter, 250);
        srv->ping_async(cb1);
        AMI_Server_pingIPtr cb2 = new AMI_Server_pingI(counter, 250);
        srv->ping_async(cb2);
        cb1->check();
        cb2->check();
        //
        // One of the callbacks should have timed out.
        //
        test((cb1->ok() && !cb2->ok()) || (!cb1->ok() && cb2->ok()));
#ifdef _WIN32
        test(cb1->id() == cb2->id());
#else
        test(pthread_equal(cb1->id(), cb2->id()) != 0);
#endif

        comm->destroy();
    }
    cout << "ok" << endl;

    cout << "testing collocated invocations... " << flush;
    {
        Test::AdapterSeq adapters = server->getAdapters();
        for(Test::AdapterSeq::iterator p = adapters.begin(); p != adapters.end(); ++p)
        {
            (*p)->callSelf(*p);
        }
    }
    cout << "ok" << endl;

    return server;
}
