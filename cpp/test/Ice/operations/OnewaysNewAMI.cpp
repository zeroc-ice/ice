// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>
#include <StateChanger.h>

using namespace std;

namespace
{

class CallbackBase : public Ice::LocalObject
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
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_m);
        while(!_called)
        {
            _m.wait();
        }
        _called = false;
    }

protected:

    void called()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_m);
        assert(!_called);
        _called = true;
        _m.notify();
    }

private:

    IceUtil::Monitor<IceUtil::Mutex> _m;
    bool _called;
};

typedef IceUtil::Handle<CallbackBase> CallbackBasePtr;

class NoEndpointCallback : public CallbackBase
{
public:

    void unsafe(const Ice::AsyncResultPtr& r)
    {
        try
        {
            Test::MyClassPrx p = Test::MyClassPrx::uncheckedCast(r->getProxy());
            p->end_opVoid(r);
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
            called();
        }
    }

    void success()
    {
        test(false);
    }

    void exCB(const Ice::Exception& ex)
    {
        test(dynamic_cast<const Ice::NoEndpointException*>(&ex));
        called();
    }

    void sentCB()
    {
        test(false);
    }
};

typedef IceUtil::Handle<NoEndpointCallback> NoEndpointCallbackPtr;

struct Cookie : public Ice::LocalObject
{
    Cookie(int i) : val(i)
    {
    }
    int val;
};

typedef IceUtil::Handle<Cookie> CookiePtr;

class Callback : public CallbackBase
{
public:

    Callback()
    {
    }

    void opVoid(const CookiePtr& cookie)
    {
        test(cookie->val == 99);
        called();
    }

    void exCBNC(const Ice::Exception& ex)
    {
        test(false);
    }

    void exCB(const Ice::Exception& ex, const CookiePtr& cookie)
    {
        test(cookie->val == 99);
        called();
    }
};

typedef IceUtil::Handle<Callback> CallbackPtr;

enum ThrowType { LocalException, UserException, StandardException, OtherException };

class Thrower : public CallbackBase
{
public:

    Thrower(ThrowType t)
        : _t(t)
    {
    }

    void exCB(const Ice::Exception& ex)
    {
        called();
        throwEx();
    }

private:

    void throwEx()
    {
        switch(_t)
        {
            case LocalException:
            {
                throw Ice::ObjectNotExistException(__FILE__, __LINE__);
                break;
            }
            case UserException:
            {
                throw Test::SomeException();
                break;
            }
            case StandardException:
            {
                throw ::std::bad_alloc();
                break;
            }
            case OtherException:
            {
                    throw 99;
                break;
            }
            default:
            {
                assert(false);
                break;
            }
        }
    }

    ThrowType _t;
};

typedef IceUtil::Handle<Thrower> ThrowerPtr;

class SentCounter : public CallbackBase
{
public:

    SentCounter() : _queuedCount(0)
    {
    }

    void exCB(const Ice::Exception&)
    {
        test(false);
    }

    void sentCB()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_m);
        ++_queuedCount;
    }

    int queuedCount()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_m);
        return _queuedCount;
    }

    void check(int size)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_m);
        while(_queuedCount != size)
        {
             _m.wait();
        }
    }

private:

    int _queuedCount;
    IceUtil::ThreadControl::ID _id;
    IceUtil::Monitor<IceUtil::Mutex> _m;
};

typedef IceUtil::Handle<SentCounter> SentCounterPtr;

}

void
onewaysNewAMI(const Ice::CommunicatorPtr& communicator, const Test::MyClassPrx& proxy)
{
    Test::MyClassPrx p = Test::MyClassPrx::uncheckedCast(proxy->ice_oneway());

    {
        //
        // Check that a call to a void operation raises NoEndpointException
        // in the end_ method instead of at the point of call.
        //
        Test::MyClassPrx indirect = Test::MyClassPrx::uncheckedCast(p->ice_adapterId("dummy")->ice_oneway());
        Ice::AsyncResultPtr r;

        r = indirect->begin_opVoid();
        try
        {
            indirect->end_opVoid(r);
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
        }

        //
        // Check that a second call to the end_ method throws IllegalArgumentException.
        //
        try
        {
            indirect->end_opVoid(r);
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }
    }

    {
        //
        // Check that a call to a void operation raises NoEndpointException in the callback.
        // Also test that the sent callback is not called in this case.
        //
        Test::MyClassPrx indirect = Test::MyClassPrx::uncheckedCast(p->ice_adapterId("dummy")->ice_oneway());

        NoEndpointCallbackPtr cb = new NoEndpointCallback;

        Ice::CallbackPtr callback = Ice::newCallback(cb, &NoEndpointCallback::exCB, &NoEndpointCallback::sentCB);
        indirect->begin_opVoid(callback);
        cb->check();
    }

    {
        //
        // Check that calling the end_ method with a different proxy or for a different operation than the begin_
        // method throws IllegalArgumentException. If the test throws as expected, we never call the end_ method,
        // so this also tests that it is safe to throw the AsyncResult away without calling the end_ method.
        //
        Test::MyClassPrx indirect1 = Test::MyClassPrx::uncheckedCast(p->ice_adapterId("dummy")->ice_oneway());
        Test::MyClassPrx indirect2 = Test::MyClassPrx::uncheckedCast(p->ice_adapterId("dummy2"))->ice_oneway();

        Ice::AsyncResultPtr r1 = indirect1->begin_opVoid();
        Ice::AsyncResultPtr r2 = indirect2->begin_opVoid();

        try
        {
            indirect1->end_opVoid(r2); // Wrong proxy
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }

        try
        {
            indirect1->end_shutdown(r1); // Wrong operation
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }
    }

    {
        //
        // Check that calling the end_ method with a null result throws IllegalArgumentException.
        //
        try
        {
            p->end_opVoid(0);
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }
    }

    {
        //
        // Check that passing a null callback Instance throws IllegalArgumentException.
        //
        CallbackPtr cb;
        try
        {
            Test::newCallback_MyClass_opVoid(cb, &Callback::opVoid, &Callback::exCB);
            test(false);
        }
        catch(IceUtil::IllegalArgumentException&)
        {
        }
    }

    {
        //
        // Check that throwing an exception from the exception callback doesn't cause problems.
        //
        Test::MyClassPrx indirect = Test::MyClassPrx::uncheckedCast(p->ice_adapterId("dummy"));

        {
            ThrowerPtr cb = new Thrower(LocalException);
            Ice::CallbackPtr callback = Ice::newCallback(cb, &Thrower::exCB);
            indirect->begin_opVoid(callback);
            cb->check();
        }

        {
            ThrowerPtr cb = new Thrower(UserException);
            Ice::CallbackPtr callback = Ice::newCallback(cb, &Thrower::exCB);
            indirect->begin_opVoid(callback);
            cb->check();
        }

        {
            ThrowerPtr cb = new Thrower(StandardException);
            Ice::CallbackPtr callback = Ice::newCallback(cb, &Thrower::exCB);
            indirect->begin_opVoid(callback);
            cb->check();
        }

        {
            ThrowerPtr cb = new Thrower(OtherException);
            Ice::CallbackPtr callback = Ice::newCallback(cb, &Thrower::exCB);
            indirect->begin_opVoid(callback);
            cb->check();
        }
    }

    {
        //
        // Check that CommunicatorDestroyedException is raised directly.
        //
        Ice::InitializationData initData;
        initData.properties = communicator->getProperties()->clone();
        Ice::CommunicatorPtr ic = Ice::initialize(initData);
        Ice::ObjectPrx obj = ic->stringToProxy(p->ice_toString())->ice_oneway();
        Test::MyClassPrx p2 = Test::MyClassPrx::uncheckedCast(obj);

        ic->destroy();

        try
        {
            p2->begin_opVoid();
            test(false);
        }
        catch(const Ice::CommunicatorDestroyedException&)
        {
            // Expected.
        }
    }

    {
        //
        // Check that we can call operations on Object asynchronously.
        //
        {
            Ice::AsyncResultPtr r = p->begin_ice_ping();
            try
            {
                p->end_ice_ping(r);
            }
            catch(...)
            {
                test(false);
            }
        }

        {
            CallbackPtr cb = new Callback;
            Ice::CallbackPtr callback = Ice::newCallback(cb, &Callback::exCBNC);
            p->begin_ice_ping(callback);
        }
    }

    //
    // Test that marshaling works as expected, and that the callbacks for each type of callback work.
    //
    {
        {
            Ice::AsyncResultPtr r = p->begin_opVoid();
            p->end_opVoid(r);
        }
    }

    //
    // Test that calling a twoway operation with a oneway proxy raises TwowayOnlyException.
    //
    {
        Ice::AsyncResultPtr r = p->begin_opByte(Ice::Byte(0xff), Ice::Byte(0x0f));
        try
        {
            Ice::Byte p3;
            p->end_opByte(p3, r);
            test(false);
        }
        catch(const Ice::TwowayOnlyException&)
        {
        }
    }

    {
        Ice::Double d = 1278312346.0 / 13.0;
        Test::DoubleS ds(5, d);
        CallbackPtr cb = new Callback;
        Ice::CallbackPtr callback = Ice::newCallback(cb, &Callback::exCBNC);
        p->begin_opDoubleMarshaling(d, ds, callback);
    }

    {
        Test::MyDerivedClassPrx derived = Test::MyDerivedClassPrx::uncheckedCast(p);
        test(derived);
        CallbackPtr cb = new Callback;
        Ice::CallbackPtr callback = Ice::newCallback(cb, &Callback::exCBNC);
        derived->begin_opDerived(callback);
    }

    //
    // Test that cookies work. Because the same generated template is instantiated each time, it is not necessary
    // to test all possible operations. Instead, we only need to test once for each of the three begin_ methods
    // that accept a cookie for a oneway and a twoway operation.
    //

    {
        Test::MyClassPrx indirect = Test::MyClassPrx::uncheckedCast(p->ice_adapterId("dummy")->ice_oneway());
        CallbackPtr cb = new Callback;
        CookiePtr cookie = new Cookie(99);
        Ice::CallbackPtr callback = Ice::newCallback(cb, &Callback::exCB);
        indirect->begin_opVoid(callback, cookie);
        cb->check();
    }

    {
        //
        // Test that not passing a cookie when one is expected throws IlllegalArgumentException.
        // (The opposite, passing a cookie when none is expected, causes a compile-time error.)
        //
//         CallbackPtr cb = new Callback;
//         Test::Callback_MyClass_opVoidPtr callback = Test::newCallback_MyClass_opVoid(cb, 
//                                                                                      &Callback::opVoid,
//                                                                                      &Callback::exCB);
//         try
//         {
//             p->begin_opVoid(callback);
//             test(false);
//         }
//         catch(const IceUtil::IllegalArgumentException&)
//         {
//         }
    }

    //
    // Test that queuing indication works.
    //
    {
        Ice::AsyncResultPtr r = p->begin_delay(100);
        r->waitForSent();
        test(r->isCompleted() && r->isSent());
        p->end_delay(r);
        test(r->isCompleted());
    }

    //
    // Put the server's adapter into the holding state and pump out requests until one is queued.
    // Then activate the adapter again and pump out more until one isn't queued again.
    // Check that all the callbacks arrive after calling the end_ method for each request.
    // We fill a context with a few kB of data to make sure we don't queue up too many requests.
    //
    {
        const int contextSize = 10; // in Kilobytes
        string s(1024, 'a');        // One kilobyte of a's

        Ice::Context ctx;
        for(int i = 0; i < contextSize; ++i)
        {
            ostringstream ss;
            ss << "i" << i;
            ctx[ss.str()] = s;
        }

        //
        // Keep all the AsyncResults we get from the begin_ calls, so we can call end_ for each of them.
        //
        vector<Ice::AsyncResultPtr> results;

        int queuedCount = 0;

        SentCounterPtr cb = new SentCounter;
        Ice::CallbackPtr callback = Ice::newCallback(cb, &SentCounter::exCB, &SentCounter::sentCB);
        Ice::AsyncResultPtr r;

        Test::StateChangerPrx state = Test::StateChangerPrx::checkedCast(
                                        communicator->stringToProxy("hold:default -p 12011"));
        state->hold(3);

        do
        {
            r = p->begin_opVoid(ctx, callback);
            results.push_back(r);
            if(!r->sentSynchronously())
            {
                ++queuedCount;
            }
        }
        while(r->sentSynchronously());

        vector<Ice::AsyncResultPtr>::size_type numRequests = results.size();
        test(numRequests > 1); // Something is wrong if we didn't get something out without queueing.

        //
        // Re-enable the adapter.
        //
        state->activate(3);

        //
        // Fire off a bunch more requests until we get one that wasn't queued.
        // We sleep in between calls to allow the queued requests to drain.
        //
        do
        {
            r = p->begin_opVoid(callback);
            results.push_back(r);
            if(!r->sentSynchronously())
            {
                ++queuedCount;
            }
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(1));
        }
        while(!r->sentSynchronously());
        test(results.size() > numRequests); // Something is wrong if we didn't queue additional requests.

        //
        // Now make all the outstanding calls to the end_ method.
        //
        for(vector<Ice::AsyncResultPtr>::iterator q = results.begin(); q != results.end(); ++q)
        {
            p->end_opVoid(*q);
        }

        //
        // Check that that we got a sent callback for each queued request.
        //
        cb->check(queuedCount);
    }
}
