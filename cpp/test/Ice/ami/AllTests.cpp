// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/Random.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;

namespace
{

enum ThrowType { LocalException, UserException, StandardException, OtherException };

#ifdef ICE_CPP11_MAPPING
ThrowType throwEx[] = { LocalException, UserException, StandardException, OtherException };
auto thrower = [](ThrowType t)
    {
        switch(t)
        {
            case LocalException:
            {
                throw Ice::ObjectNotExistException(__FILE__, __LINE__);
                break;
            }
            case UserException:
            {
                throw Test::TestIntfException();
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
    };
#else
struct Cookie : public Ice::LocalObject
{
    Cookie(int i) : val(i)
    {
    }
    int val;
};
typedef IceUtil::Handle<Cookie> CookiePtr;

class CallbackBase : public virtual Ice::LocalObject
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

class AsyncCallback : public CallbackBase
{
public:

    AsyncCallback()
    {
    }

    AsyncCallback(const CookiePtr& cookie) : _cookie(cookie)
    {
    }

    void isA(const Ice::AsyncResultPtr& result)
    {
        test(result->getCookie() == _cookie);
        test(result->getProxy()->end_ice_isA(result));
        called();
    }

    void ping(const Ice::AsyncResultPtr& result)
    {
        test(result->getCookie() == _cookie);
        result->getProxy()->end_ice_ping(result);
        called();
    }

    void id(const Ice::AsyncResultPtr& result)
    {
        test(result->getCookie() == _cookie);
        test(result->getProxy()->end_ice_id(result) == Test::TestIntf::ice_staticId());
        called();
    }

    void ids(const Ice::AsyncResultPtr& result)
    {
        test(result->getCookie() == _cookie);
        test(result->getProxy()->end_ice_ids(result).size() == 2);
        called();
    }

    void connection(const Ice::AsyncResultPtr& result)
    {
        test(result->getCookie() == _cookie);
        test(result->getProxy()->end_ice_getConnection(result));
        called();
    }

    void op(const Ice::AsyncResultPtr& result)
    {
        test(result->getCookie() == _cookie);
        Test::TestIntfPrx::uncheckedCast(result->getProxy())->end_op(result);
        called();
    }

    void opWithResult(const Ice::AsyncResultPtr& result)
    {
        test(result->getCookie() == _cookie);
        test(Test::TestIntfPrx::uncheckedCast(result->getProxy())->end_opWithResult(result) == 15);
        called();
    }

    void opWithUE(const Ice::AsyncResultPtr& result)
    {
        test(result->getCookie() == _cookie);
        try
        {
            Test::TestIntfPrx::uncheckedCast(result->getProxy())->end_opWithUE(result);
            test(false);
        }
        catch(const Test::TestIntfException&)
        {
            called();
        }
        catch(const Ice::Exception&)
        {
            test(false);
        }
    }

    void isAEx(const Ice::AsyncResultPtr& result)
    {
        test(result->getCookie() == _cookie);
        try
        {
            result->getProxy()->end_ice_isA(result);
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
            called();
        }
        catch(const Ice::Exception&)
        {
            test(false);
        }
    }

    void pingEx(const Ice::AsyncResultPtr& result)
    {
        test(result->getCookie() == _cookie);
        try
        {
            result->getProxy()->end_ice_ping(result);
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
            called();
        }
        catch(const Ice::Exception&)
        {
            test(false);
        }
    }

    void idEx(const Ice::AsyncResultPtr& result)
    {
        test(result->getCookie() == _cookie);
        try
        {
            result->getProxy()->end_ice_id(result);
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
            called();
        }
        catch(const Ice::Exception&)
        {
            test(false);
        }
    }

    void idsEx(const Ice::AsyncResultPtr& result)
    {
        test(result->getCookie() == _cookie);
        try
        {
            result->getProxy()->end_ice_ids(result);
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
            called();
        }
        catch(const Ice::Exception&)
        {
            test(false);
        }
    }

    void connectionEx(const Ice::AsyncResultPtr& result)
    {
        test(result->getCookie() == _cookie);
        try
        {
            result->getProxy()->end_ice_getConnection(result);
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
            called();
        }
        catch(const Ice::Exception&)
        {
            test(false);
        }
    }

    void opEx(const Ice::AsyncResultPtr& result)
    {
        test(result->getCookie() == _cookie);
        try
        {
            Test::TestIntfPrx::uncheckedCast(result->getProxy())->end_op(result);
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
            called();
        }
        catch(const Ice::Exception&)
        {
            test(false);
        }
    }

private:

    CookiePtr _cookie;
};
typedef IceUtil::Handle<AsyncCallback> AsyncCallbackPtr;

class ResponseCallback : public CallbackBase
{
public:

    ResponseCallback()
    {
    }

    void isA(bool r)
    {
        test(r);
        called();
    }

    void ping()
    {
        called();
    }

    void id(const string& id)
    {
        test(id == Test::TestIntf::ice_staticId());
        called();
    }

    void ids(const Ice::StringSeq& ids)
    {
        test(ids.size() == 2);
        called();
    }

    void connection(const Ice::ConnectionPtr& conn)
    {
        test(conn);
        called();
    }

    void op()
    {
        called();
    }

    void opWithResult(int r)
    {
        test(r == 15);
        called();
    }

    void opWithUE(const Ice::Exception& ex)
    {
        try
        {
            ex.ice_throw();
            test(false);
        }
        catch(const Test::TestIntfException&)
        {
            called();
        }
    }

    void ex(const Ice::Exception&)
    {
    }
};
typedef IceUtil::Handle<ResponseCallback> ResponseCallbackPtr;

class ResponseCallbackWC : public CallbackBase
{
public:

    ResponseCallbackWC(const CookiePtr& cookie) : _cookie(cookie)
    {
    }

    void isA(bool r, const CookiePtr& cookie)
    {
        test(cookie == _cookie);
        test(r);
        called();
    }

    void ping(const CookiePtr& cookie)
    {
        test(cookie == _cookie);
        called();
    }

    void id(const string& id, const CookiePtr& cookie)
    {
        test(cookie == _cookie);
        test(id == Test::TestIntf::ice_staticId());
        called();
    }

    void ids(const Ice::StringSeq& ids, const CookiePtr& cookie)
    {
        test(cookie == _cookie);
        test(ids.size() == 2);
        called();
    }

    void connection(const Ice::ConnectionPtr& conn, const CookiePtr& cookie)
    {
        test(conn);
        test(cookie == _cookie);
        called();
    }

    void op(const CookiePtr& cookie)
    {
        test(cookie == _cookie);
        called();
    }

    void opWithResult(int r, const CookiePtr& cookie)
    {
        test(cookie == _cookie);
        test(r == 15);
        called();
    }

    void opWithUE(const Ice::Exception& ex, const CookiePtr& cookie)
    {
        test(cookie == _cookie);
        try
        {
            ex.ice_throw();
            test(false);
        }
        catch(const Test::TestIntfException&)
        {
            called();
        }
    }

    void ex(const Ice::Exception&, const CookiePtr&)
    {
    }

private:

    CookiePtr _cookie;
};
typedef IceUtil::Handle<ResponseCallbackWC> ResponseCallbackWCPtr;

class ExceptionCallback : public CallbackBase
{
public:

    ExceptionCallback()
    {
    }

    void isA(bool)
    {
        test(false);
    }

    void ping()
    {
        test(false);
    }

    void id(const string&)
    {
        test(false);
    }

    void ids(const Ice::StringSeq&)
    {
        test(false);
    }

    void connection(const Ice::ConnectionPtr&)
    {
        test(false);
    }

    void op()
    {
        test(false);
    }

    void opWithUE(const Ice::Exception& ex)
    {
        test(dynamic_cast<const Test::TestIntfException*>(&ex));
        called();
    }

    void ex(const Ice::Exception& ex)
    {
        test(dynamic_cast<const Ice::NoEndpointException*>(&ex));
        called();
    }

    void noEx(const Ice::Exception& ex)
    {
        cout << ex << endl;
        cout << ex.ice_stackTrace() << endl;
        test(false);
    }
};
typedef IceUtil::Handle<ExceptionCallback> ExceptionCallbackPtr;

class ExceptionCallbackWC : public CallbackBase
{
public:

    ExceptionCallbackWC(const CookiePtr& cookie) : _cookie(cookie)
    {
    }

    void isA(bool, const CookiePtr&)
    {
        test(false);
    }

    void ping(const CookiePtr&)
    {
        test(false);
    }

    void id(const string&, const CookiePtr&)
    {
        test(false);
    }

    void ids(const Ice::StringSeq&, const CookiePtr&)
    {
        test(false);
    }

    void op(const CookiePtr&)
    {
        test(false);
    }

    void opWithUE(const Ice::Exception& ex, const CookiePtr& cookie)
    {
        test(cookie == _cookie);
        test(dynamic_cast<const Test::TestIntfException*>(&ex));
        called();
    }

    void ex(const Ice::Exception& ex, const CookiePtr& cookie)
    {
        test(cookie == _cookie);
        test(dynamic_cast<const Ice::NoEndpointException*>(&ex));
        called();
    }

    void noEx(const Ice::Exception&, const CookiePtr&)
    {
        test(false);
    }

private:

    CookiePtr _cookie;
};
typedef IceUtil::Handle<ExceptionCallbackWC> ExceptionCallbackWCPtr;

class SentCallback : public CallbackBase
{
public:

    SentCallback()
    {
    }

    SentCallback(const CookiePtr& cookie) : _cookie(cookie)
    {
    }

    void isA(bool)
    {
    }

    void isAWC(bool, const CookiePtr&)
    {
    }

    void ping()
    {
    }

    void pingWC(const CookiePtr&)
    {
    }

    void id(const string&)
    {
    }

    void idWC(const string&, const CookiePtr&)
    {
    }

    void ids(const Ice::StringSeq&)
    {
    }

    void idsWC(const Ice::StringSeq&, const CookiePtr&)
    {
    }

    void opAsync(const Ice::AsyncResultPtr&)
    {
    }

    void op()
    {
    }

    void opWC(const CookiePtr&)
    {
    }

    void ex(const Ice::Exception&)
    {
    }

    void exWC(const Ice::Exception&, const CookiePtr&)
    {
    }

    void sentAsync(const Ice::AsyncResultPtr& result)
    {
        test((result->sentSynchronously() && _thread == IceUtil::ThreadControl()) ||
             (!result->sentSynchronously() && _thread != IceUtil::ThreadControl()));
        called();
    }

    void sent(bool sentSynchronously)
    {
        test((sentSynchronously && _thread == IceUtil::ThreadControl()) ||
             (!sentSynchronously && _thread != IceUtil::ThreadControl()));
        called();
    }

    void sentWC(bool sentSynchronously, const CookiePtr& cookie)
    {
        test((sentSynchronously && _thread == IceUtil::ThreadControl()) ||
             (!sentSynchronously && _thread != IceUtil::ThreadControl()));
        test(cookie == _cookie);
        called();
    }

private:

    CookiePtr _cookie;
    IceUtil::ThreadControl _thread;
};
typedef IceUtil::Handle<SentCallback> SentCallbackPtr;

class FlushCallback : public CallbackBase
{
public:

    FlushCallback()
    {
    }

    FlushCallback(const CookiePtr& cookie) : _cookie(cookie)
    {
    }

    void completedAsync(const Ice::AsyncResultPtr&)
    {
        test(false);
    }

    void exception(const Ice::Exception&)
    {
        test(false);
    }

    void exceptionWC(const Ice::Exception&, const CookiePtr&)
    {
        test(false);
    }

    void sentAsync(const Ice::AsyncResultPtr& result)
    {
        test((result->sentSynchronously() && _thread == IceUtil::ThreadControl()) ||
             (!result->sentSynchronously() && _thread != IceUtil::ThreadControl()));
        called();
    }

    void sent(bool sentSynchronously)
    {
        test((sentSynchronously && _thread == IceUtil::ThreadControl()) ||
             (!sentSynchronously && _thread != IceUtil::ThreadControl()));
        called();
    }

    void sentWC(bool sentSynchronously, const CookiePtr& cookie)
    {
        test((sentSynchronously && _thread == IceUtil::ThreadControl()) ||
             (!sentSynchronously && _thread != IceUtil::ThreadControl()));
        test(cookie == _cookie);
        called();
    }

private:

    CookiePtr _cookie;
    IceUtil::ThreadControl _thread;
};
typedef IceUtil::Handle<FlushCallback> FlushCallbackPtr;

class FlushExCallback : public CallbackBase
{
public:

    FlushExCallback()
    {
    }

    FlushExCallback(const CookiePtr& cookie) : _cookie(cookie)
    {
    }

    void completedAsync(const Ice::AsyncResultPtr& r)
    {
        test(r->getCookie() == _cookie);
        try
        {
            if(r->getConnection())
            {
                r->getConnection()->end_flushBatchRequests(r);
            }
            else
            {
                r->getProxy()->end_ice_flushBatchRequests(r);
            }
            test(false);
        }
        catch(const Ice::LocalException&)
        {
            called();
        }
    }

    void exception(const Ice::Exception&)
    {
        called();
    }

    void exceptionWC(const Ice::Exception&, const CookiePtr& cookie)
    {
        test(cookie == _cookie);
        called();
    }

    void sentAsync(const Ice::AsyncResultPtr&)
    {
        test(false);
    }

    void sent(bool)
    {
        test(false);
    }

    void sentWC(bool, const CookiePtr&)
    {
        test(false);
    }

private:

    CookiePtr _cookie;
    IceUtil::ThreadControl _thread;
};
typedef IceUtil::Handle<FlushExCallback> FlushExCallbackPtr;

class CloseCallback : public virtual CallbackBase, public virtual Ice::CloseCallback
{
public:

    CloseCallback()
    {
    }

    virtual void closed(const Ice::ConnectionPtr&)
    {
        called();
    }
};
typedef IceUtil::Handle<CloseCallback> CloseCallbackPtr;

class Thrower : public CallbackBase
{
public:

    Thrower(ThrowType t)
        : _t(t)
    {
    }

    void opAsync(const Ice::AsyncResultPtr&)
    {
        called();
        throwEx();
    }

    void op()
    {
        called();
        throwEx();
    }

    void opWC(const CookiePtr&)
    {
        called();
        throwEx();
    }

    void noOp()
    {
    }

    void noOpWC(const CookiePtr&)
    {
    }

    void ex(const Ice::Exception&)
    {
        called();
        throwEx();
    }

    void exWC(const Ice::Exception&, const CookiePtr&)
    {
        called();
        throwEx();
    }

    void noEx(const Ice::Exception&)
    {
        test(false);
    }

    void noExWC(const Ice::Exception&, const CookiePtr&)
    {
        test(false);
    }

    void sent(bool)
    {
        called();
        throwEx();
    }

    void sentWC(bool, const CookiePtr&)
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
                throw Test::TestIntfException();
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
#endif

}

void
allTests(Test::TestHelper* helper, bool collocated)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    const string protocol = communicator->getProperties()->getProperty("Ice.Default.Protocol");

#ifdef ICE_CPP11_MAPPING
    string sref = "test:" + helper->getTestEndpoint();
    auto prx = communicator->stringToProxy(sref);
    test(prx);

    auto p = Ice::uncheckedCast<Test::TestIntfPrx>(prx);

    sref = "testController:" + helper->getTestEndpoint(1);
    prx = communicator->stringToProxy(sref);
    test(prx);

    auto testController = Ice::uncheckedCast<Test::TestIntfControllerPrx>(prx);

    Ice::Context ctx;
    cout << "testing lambda API... " << flush;
    {
        {
            p->ice_isAAsync(Test::TestIntf::ice_staticId(), nullptr);
        }
        {
            promise<bool> promise;
            p->ice_isAAsync(Test::TestIntf::ice_staticId(),
                [&](bool value)
                {
                    promise.set_value(value);
                });
            test(promise.get_future().get());
        }
        {
            promise<bool> promise;
            p->ice_isAAsync(Test::TestIntf::ice_staticId(),
                [&](bool value)
                {
                    promise.set_value(value);
                },
                [&](const exception_ptr& ex)
                {
                    promise.set_exception(ex);
                });
            test(promise.get_future().get());
        }

        {
            promise<bool> promise;
            p->ice_isAAsync(Test::TestIntf::ice_staticId(),
                [&](bool value)
                {
                    promise.set_value(value);
                },
                nullptr, nullptr, ctx);
            test(promise.get_future().get());
        }

        {
            p->ice_pingAsync(nullptr);
        }
        {
            promise<void> promise;
            p->ice_pingAsync(
                [&]()
                {
                    promise.set_value();
                });
            promise.get_future().get();
        }
        {
            promise<void> promise;
            p->ice_pingAsync(
                [&]()
                {
                    promise.set_value();
                },
                [&](const exception_ptr& ex)
                {
                    promise.set_exception(ex);
                });
            promise.get_future().get();
        }
        {
            promise<void> promise;
            p->ice_pingAsync(
                [&]()
                {
                    promise.set_value();
                },
                nullptr, nullptr, ctx);
            promise.get_future().get();
        }

        {
            p->ice_idAsync(nullptr);
        }
        {
            promise<string> promise;
            p->ice_idAsync(
                [&](const string& id)
                {
                    promise.set_value(id);
                });
            test(promise.get_future().get() == Test::TestIntf::ice_staticId());
        }
        {
            promise<string> promise;
            p->ice_idAsync(
                [&](const string& id)
                {
                    promise.set_value(id);
                },
                [&](const exception_ptr& ex)
                {
                    promise.set_exception(ex);
                });
            test(promise.get_future().get() == Test::TestIntf::ice_staticId());
        }
        {
            promise<string> promise;
            p->ice_idAsync(
                [&](const string& id)
                {
                    promise.set_value(id);
                },
                nullptr, nullptr, ctx);
            test(promise.get_future().get() == Test::TestIntf::ice_staticId());
        }

        {
            p->ice_idsAsync(nullptr);
        }
        {
            promise<vector<string>> promise;
            p->ice_idsAsync(
                [&](const vector<string>& ids)
                {
                    promise.set_value(ids);
                });
            test(promise.get_future().get().size() == 2);
        }
        {
            promise<vector<string>> promise;
            p->ice_idsAsync(
                [&](const vector<string>& ids)
                {
                    promise.set_value(ids);
                },
                [&](const exception_ptr& ex)
                {
                    promise.set_exception(ex);
                });
            test(promise.get_future().get().size() == 2);
        }
        {
            promise<vector<string>> promise;
            p->ice_idsAsync(
                [&](const vector<string>& ids)
                {
                    promise.set_value(ids);
                },
                nullptr, nullptr, ctx);
            test(promise.get_future().get().size() == 2);
        }

        if(!collocated)
        {
            {
                p->ice_getConnectionAsync(nullptr);
            }
            {
                promise<Ice::ConnectionPtr> promise;
                p->ice_getConnectionAsync(
                    [&](const Ice::ConnectionPtr& connection)
                    {
                        promise.set_value(connection);
                    });
                test(promise.get_future().get());
            }
            {
                promise<Ice::ConnectionPtr> promise;
                p->ice_getConnectionAsync(
                    [&](const Ice::ConnectionPtr& connection)
                    {
                        promise.set_value(connection);
                    },
                    [&](const exception_ptr& ex)
                    {
                        promise.set_exception(ex);
                    });
                test(promise.get_future().get());
            }
        }

        {
            p->opAsync(nullptr);
        }
        {
            promise<void> promise;
            p->opAsync(
                [&]()
                {
                    promise.set_value();
                });
            promise.get_future().get();
        }
        {
            promise<void> promise;
            p->opAsync(
                [&]()
                {
                    promise.set_value();
                },
                [&](const exception_ptr& ex)
                {
                    promise.set_exception(ex);
                });
            promise.get_future().get();
        }
        {
            promise<void> promise;
            p->opAsync(
                [&]()
                {
                    promise.set_value();
                },
                nullptr, nullptr, ctx);
            promise.get_future().get();
        }

        {
            p->opWithResultAsync(nullptr);
        }
        {
            promise<int> promise;
            p->opWithResultAsync(
                [&](int result)
                {
                    promise.set_value(result);
                });
            test(promise.get_future().get() == 15);
        }
        {
            promise<int> promise;
            p->opWithResultAsync(
                [&](int result)
                {
                    promise.set_value(result);
                },
                [&](const exception_ptr& ex)
                {
                    promise.set_exception(ex);
                });
            test(promise.get_future().get() == 15);
        }
        {
            promise<int> promise;
            p->opWithResultAsync(
                [&](int result)
                {
                    promise.set_value(result);
                },
                nullptr, nullptr, ctx);
            test(promise.get_future().get() == 15);
        }

        {
            p->opWithUEAsync(nullptr);
        }
        {
            promise<void> promise;
            p->opWithUEAsync(
                nullptr,
                [&](const exception_ptr& ex)
                {
                    promise.set_exception(ex);
                });

            try
            {
                promise.get_future().get();
                test(false);
            }
            catch(const Test::TestIntfException&)
            {
            }
        }
        {
            promise<void> promise;
            p->opWithUEAsync(
                [&]()
                {
                    promise.set_value();
                },
                [&](const exception_ptr& ex)
                {
                    promise.set_exception(ex);
                });

            try
            {
                promise.get_future().get();
                test(false);
            }
            catch(const Test::TestIntfException&)
            {
            }
        }
        {
            promise<void> promise;
            p->opWithUEAsync(
                nullptr,
                [&](const exception_ptr& ex)
                {
                    promise.set_exception(ex);
                },
                nullptr, ctx);

            try
            {
                promise.get_future().get();
                test(false);
            }
            catch(const Test::TestIntfException&)
            {
            }
        }

        {
            p->opWithResultAndUEAsync(nullptr);
        }
        {
            promise<void> promise;
            p->opWithResultAndUEAsync(
                nullptr,
                [&](const exception_ptr& ex)
                {
                    promise.set_exception(ex);
                });

            try
            {
                promise.get_future().get();
                test(false);
            }
            catch(const Test::TestIntfException&)
            {
            }
            catch(const Ice::OperationNotExistException&)
            {
            }
        }
        {
            promise<void> promise;
            p->opWithResultAndUEAsync(
                [&](int)
                {
                    promise.set_value();
                },
                [&](const exception_ptr& ex)
                {
                    promise.set_exception(ex);
                });

            try
            {
                promise.get_future().get();
                test(false);
            }
            catch(const Test::TestIntfException&)
            {
            }
            catch(const Ice::OperationNotExistException&)
            {
            }
        }
        {
            promise<void> promise;
            p->opWithResultAndUEAsync(
                nullptr,
                [&](const exception_ptr& ex)
                {
                    promise.set_exception(ex);
                },
                nullptr, ctx);

            try
            {
                promise.get_future().get();
                test(false);
            }
            catch(const Test::TestIntfException&)
            {
            }
            catch(const Ice::OperationNotExistException&)
            {
            }
        }
    }
    cout << "ok" << endl;

    cout << "testing future API... " << flush;
    {

        test(p->ice_isAAsync(Test::TestIntf::ice_staticId()).get());
        test(p->ice_isAAsync(Test::TestIntf::ice_staticId(), ctx).get());

        p->ice_pingAsync().get();
        p->ice_pingAsync(ctx).get();

        test(p->ice_idsAsync().get().size() == 2);
        test(p->ice_idsAsync(ctx).get().size() == 2);

        if(!collocated)
        {
            test(p->ice_getConnectionAsync().get());
        }

        p->opAsync().get();
        p->opAsync(ctx).get();

        test(p->opWithResultAsync().get() == 15);
        test(p->opWithResultAsync(ctx).get() == 15);

        try
        {
            p->opWithUEAsync().get();
            test(false);
        }
        catch(const Test::TestIntfException&)
        {
        }

        try
        {
            p->opWithUEAsync(ctx).get();
            test(false);
        }
        catch(const Test::TestIntfException&)
        {
        }

        try
        {
            p->opWithResultAndUEAsync().get();
            test(false);
        }
        catch(const Test::TestIntfException&)
        {
        }
        catch(const Ice::OperationNotExistException&)
        {
        }

        try
        {
            p->opWithResultAndUEAsync(ctx).get();
            test(false);
        }
        catch(const Test::TestIntfException&)
        {
        }
        catch(const Ice::OperationNotExistException&)
        {
        }
    }
    cout << "ok" << endl;

    cout << "testing local exceptions with lambda API... " << flush;
    {
        auto indirect = Ice::uncheckedCast<Test::TestIntfPrx>(p->ice_adapterId("dummy"));

        {
            promise<void> promise;
            indirect->opAsync(
                [&]()
                {
                    promise.set_value();
                },
                [&](const exception_ptr& ex)
                {
                    promise.set_exception(ex);
                });
            try
            {
                promise.get_future().get();
                test(false);
            }
            catch(const Ice::NoEndpointException&)
            {
            }
        }

        {
            try
            {
                promise<int> promise;
                p->ice_oneway()->opWithResultAsync(
                    [&](int value)
                    {
                        promise.set_value(value);
                    },
                    [&](const exception_ptr& ex)
                    {
                        promise.set_exception(ex);
                    });
                test(false);
            }
            catch(const Ice::TwowayOnlyException&)
            {
            }
        }

        //
        // Check that CommunicatorDestroyedException is raised directly.
        //
        if(p->ice_getConnection() && protocol != "bt")
        {
            Ice::InitializationData initData;
            initData.properties = communicator->getProperties()->clone();
            Ice::CommunicatorPtr ic = Ice::initialize(initData);
            auto obj = ic->stringToProxy(p->ice_toString());
            auto p2 = Ice::checkedCast<Test::TestIntfPrx>(obj);
            ic->destroy();

            try
            {
                promise<void> promise;
                p2->opAsync(
                    [&]()
                    {
                        promise.set_value();
                    },
                    [&](const exception_ptr& ex)
                    {
                        promise.set_exception(ex);
                    });
                test(false);
            }
            catch(const Ice::CommunicatorDestroyedException&)
            {
                // Expected.
            }
        }
    }
    cout << "ok" << endl;

    cout << "testing local exceptions with future API... " << flush;
    {
        auto indirect = Ice::uncheckedCast<Test::TestIntfPrx>(p->ice_adapterId("dummy"));
        auto r = indirect->opAsync();
        try
        {
            r.get();
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
        }

        try
        {
            p->ice_oneway()->opWithResultAsync().get();
            test(false);
        }
        catch(const Ice::TwowayOnlyException&)
        {
        }

        //
        // Check that CommunicatorDestroyedException is raised directly.
        //
        if(p->ice_getConnection())
        {
            Ice::InitializationData initData;
            initData.properties = communicator->getProperties()->clone();
            Ice::CommunicatorPtr ic = Ice::initialize(initData);
            auto obj = ic->stringToProxy(p->ice_toString());
            auto p2 = Ice::checkedCast<Test::TestIntfPrx>(obj);
            ic->destroy();

            try
            {
                p2->opAsync();
                test(false);
            }
            catch(const Ice::CommunicatorDestroyedException&)
            {
                // Expected.
            }
        }
    }
    cout << "ok" << endl;

    cout << "testing exception callback with lambda API... " << flush;
    {
        auto i = Ice::uncheckedCast<Test::TestIntfPrx>(p->ice_adapterId("dummy"));

        {
            promise<bool> promise;
            i->ice_isAAsync(Test::TestIntf::ice_staticId(),
                [&](bool value)
                {
                    promise.set_value(value);
                },
                [&](const exception_ptr& ex)
                {
                    promise.set_exception(ex);
                });
            try
            {
                promise.get_future().get();
                test(false);
            }
            catch(const Ice::NoEndpointException&)
            {
            }
        }

        {
            promise<void> promise;
            i->opAsync(
                [&]()
                {
                    promise.set_value();
                },
                [&](const exception_ptr& ex)
                {
                    promise.set_exception(ex);
                });

            try
            {
                promise.get_future().get();
                test(false);
            }
            catch(const Ice::NoEndpointException&)
            {
            }
        }

        {
            promise<void> promise;
            i->opWithUEAsync(
                [&]()
                {
                    promise.set_value();
                },
                [&](const exception_ptr& ex)
                {
                    promise.set_exception(ex);
                });

            try
            {
                promise.get_future().get();
                test(false);
            }
            catch(const Ice::NoEndpointException&)
            {
            }
        }

        // Ensures no exception is called when response is received
        {
            promise<bool> promise;
            p->ice_isAAsync(
                Test::TestIntf::ice_staticId(),
                [&](bool value)
                {
                    promise.set_value(value);
                },
                [&](const exception_ptr& ex)
                {
                    promise.set_exception(ex);
                });
            try
            {
                test(promise.get_future().get());
            }
            catch(...)
            {
                test(false);
            }
        }

        {
            promise<void> promise;
            p->opAsync(
                [&]()
                {
                    promise.set_value();
                },
                [&](const exception_ptr& ex)
                {
                    promise.set_exception(ex);
                });
            try
            {
                promise.get_future().get();
            }
            catch(...)
            {
                test(false);
            }
        }

        // If response is a user exception, it should be received.
        {
            promise<void> promise;
            p->opWithUEAsync(
                [&]()
                {
                    promise.set_value();
                },
                [&](const exception_ptr& ex)
                {
                    promise.set_exception(ex);
                });

            try
            {
                promise.get_future().get();
                test(false);
            }
            catch(const Test::TestIntfException&)
            {
            }
        }
    }
    cout << "ok" << endl;

    cout << "testing sent callback with lambda API... " << flush;
    {
        {
            promise<bool> response;
            promise<bool> sent;

            p->ice_isAAsync("",
                [&](bool value)
                {
                    response.set_value(value);
                },
                [&](const exception_ptr& ex)
                {
                    response.set_exception(ex);
                },
                [&](bool sentAsync)
                {
                    sent.set_value(sentAsync);
                });

            sent.get_future().get();
            response.get_future().get();
        }

        {
            promise<void> response;
            promise<bool> sent;

            p->ice_pingAsync(
                [&]()
                {
                    response.set_value();
                },
                [&](const exception_ptr& ex)
                {
                    response.set_exception(ex);
                },
                [&](bool sentAsync)
                {
                    sent.set_value(sentAsync);
                });

            sent.get_future().get();
            response.get_future().get();
        }

        {
            promise<string> response;
            promise<bool> sent;

            p->ice_idAsync(
                [&](string value)
                {
                    response.set_value(value);
                },
                [&](const exception_ptr& ex)
                {
                    response.set_exception(ex);
                },
                [&](bool sentAsync)
                {
                    sent.set_value(sentAsync);
                });

            sent.get_future().get();
            response.get_future().get();
        }

        {
            promise<vector<string>> response;
            promise<bool> sent;

            p->ice_idsAsync(
                [&](vector<string> value)
                {
                    response.set_value(value);
                },
                [&](const exception_ptr& ex)
                {
                    response.set_exception(ex);
                },
                [&](bool sentAsync)
                {
                    sent.set_value(sentAsync);
                });

            sent.get_future().get();
            response.get_future().get();
        }

        {
            promise<void> response;
            promise<bool> sent;

            p->opAsync(
                [&]()
                {
                    response.set_value();
                },
                [&](const exception_ptr& ex)
                {
                    response.set_exception(ex);
                },
                [&](bool sentAsync)
                {
                    sent.set_value(sentAsync);
                });

            sent.get_future().get();
            response.get_future().get();
        }

        vector<future<bool>> futures;
        Ice::ByteSeq seq;
        seq.resize(1024);
        testController->holdAdapter();
        try
        {
            while(true)
            {
                auto s = make_shared<promise<bool>>();
                auto f = s->get_future();
                p->opWithPayloadAsync(
                    seq,
                    [](){},
                    [s](const exception_ptr& ex)
                    {
                        s->set_exception(ex);
                    },
                    [s](bool value)
                    {
                        s->set_value(value);
                    });

                if(f.wait_for(chrono::seconds(0)) != future_status::ready || !f.get())
                {
                    break;
                }
                futures.push_back(move(f));
            }
        }
        catch(...)
        {
            testController->resumeAdapter();
            throw;
        }
        testController->resumeAdapter();
    }
    cout << "ok" << endl;

    cout << "testing unexpected exceptions from callback... " << flush;
    {
        auto q = Ice::uncheckedCast<Test::TestIntfPrx>(p->ice_adapterId("dummy"));

        for(int i = 0; i < 4; ++i)
        {
            {
                promise<void> promise;
                p->opAsync(
                    [&, i]()
                    {
                        promise.set_value();
                        thrower(throwEx[i]);
                    },
                    [&](const exception_ptr&)
                    {
                        test(false);
                    });

                try
                {
                    promise.get_future().get();
                }
                catch(const exception& ex)
                {
                    cerr << ex.what() << endl;
                    test(false);
                }
            }

            {
                promise<void> promise;
                p->opAsync(nullptr, nullptr,
                    [&, i](bool)
                    {
                        promise.set_value();
                        thrower(throwEx[i]);
                    });

                try
                {
                    promise.get_future().get();
                }
                catch(const exception&)
                {
                    test(false);
                }
            }
        }
    }
    cout << "ok" << endl;

    cout << "testing batch requests with proxy... " << flush;
    {
        {
            test(p->opBatchCount() == 0);
            auto b1 = p->ice_batchOneway();
            b1->opBatchAsync().get();
            b1->opBatch();
            auto id = this_thread::get_id();
            promise<void> promise;
            b1->ice_flushBatchRequestsAsync(
                [&](const exception_ptr& ex)
                {
                    promise.set_exception(ex);
                },
                [&](bool sentSynchronously)
                {
                    test((sentSynchronously && id == this_thread::get_id()) ||
                         (!sentSynchronously && id != this_thread::get_id()));
                    promise.set_value();
                });
            promise.get_future().get();
            test(p->waitForBatch(2));
        }

        if(p->ice_getConnection() && protocol != "bt")
        {
            test(p->opBatchCount() == 0);
            auto b1 = p->ice_batchOneway();
            b1->opBatch();
            b1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));

            auto id = this_thread::get_id();
            promise<void> promise;
            b1->ice_flushBatchRequestsAsync(
                [&](const exception_ptr& ex)
                {
                    promise.set_exception(ex);
                },
                [&](bool sentSynchronously)
                {
                    test((sentSynchronously && id == this_thread::get_id()) ||
                         (!sentSynchronously && id != this_thread::get_id()));
                    promise.set_value();
                });
            promise.get_future().get();
            test(p->waitForBatch(1));
        }
    }
    cout << "ok" << endl;

    if(p->ice_getConnection()) // No collocation optimization
    {
        cout << "testing batch requests with connection... " << flush;
        {
            {
                test(p->opBatchCount() == 0);
                auto b1 = Ice::uncheckedCast<Test::TestIntfPrx>(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                b1->opBatch();
                b1->opBatch();

                auto id = this_thread::get_id();
                promise<void> promise;

                b1->ice_getConnection()->flushBatchRequestsAsync(
                    Ice::CompressBatch::BasedOnProxy,
                    [&](const exception_ptr& ex)
                    {
                        promise.set_exception(ex);
                    },
                    [&](bool sentSynchronously)
                    {
                        test((sentSynchronously && id == this_thread::get_id()) ||
                            (!sentSynchronously && id != this_thread::get_id()));
                        promise.set_value();
                    });
                promise.get_future().get();
                test(p->waitForBatch(2));
            }

            {
                // Ensure it also works with a twoway proxy
                auto id = this_thread::get_id();
                promise<void> promise;
                p->ice_getConnection()->flushBatchRequestsAsync(
                    Ice::CompressBatch::BasedOnProxy,
                    [&](const exception_ptr& ex)
                    {
                        promise.set_exception(ex);
                    },
                    [&](bool sentSynchronously)
                    {
                        test((sentSynchronously && id == this_thread::get_id()) ||
                            (!sentSynchronously && id != this_thread::get_id()));
                        promise.set_value();
                    });
                promise.get_future().get();
            }

            if(protocol != "bt")
            {
                test(p->opBatchCount() == 0);
                auto b1 = Ice::uncheckedCast<Test::TestIntfPrx>(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                b1->opBatch();
                b1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));

                promise<void> promise;
                b1->ice_getConnection()->flushBatchRequestsAsync(
                    Ice::CompressBatch::BasedOnProxy,
                    [&](exception_ptr ex)
                    {
                        promise.set_exception(move(ex));
                    },
                    [&](bool)
                    {
                        promise.set_value();
                    });

                try
                {
                    promise.get_future().get();
                    test(false);
                }
                catch(const Ice::LocalException&)
                {
                }
                test(p->opBatchCount() == 0);
            }
        }
        cout << "ok" << endl;

        cout << "testing batch requests with communicator... " << flush;
        {
            {
                //
                // 1 connection. Test future
                //
                test(p->opBatchCount() == 0);
                auto b1 = Ice::uncheckedCast<Test::TestIntfPrx>(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                b1->opBatch();
                b1->opBatch();

                communicator->flushBatchRequestsAsync(Ice::CompressBatch::BasedOnProxy).get();
                test(p->waitForBatch(2));
            }

            {
                //
                // 1 connection.
                //
                test(p->opBatchCount() == 0);
                auto b1 = Ice::uncheckedCast<Test::TestIntfPrx>(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                b1->opBatch();
                b1->opBatch();

                promise<void> promise;
                auto id = this_thread::get_id();
                communicator->flushBatchRequestsAsync(
                    Ice::CompressBatch::BasedOnProxy,
                    [&](exception_ptr ex)
                    {
                        promise.set_exception(move(ex));
                    },
                    [&](bool sentSynchronously)
                    {
                        test((sentSynchronously && id == this_thread::get_id()) ||
                             (!sentSynchronously && id != this_thread::get_id()));
                        promise.set_value();
                    });
                promise.get_future().get();
                test(p->waitForBatch(2));
            }

            if(protocol != "bt")
            {
                //
                // Exception - 1 connection.
                //
                test(p->opBatchCount() == 0);
                auto b1 = Ice::uncheckedCast<Test::TestIntfPrx>(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                b1->opBatch();
                b1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));

                promise<void> promise;
                auto id = this_thread::get_id();
                communicator->flushBatchRequestsAsync(
                    Ice::CompressBatch::BasedOnProxy,
                    [&](exception_ptr ex)
                    {
                        promise.set_exception(move(ex));
                    },
                    [&](bool sentSynchronously)
                    {
                        test((sentSynchronously && id == this_thread::get_id()) ||
                             (!sentSynchronously && id != this_thread::get_id()));
                        promise.set_value();
                    });
                promise.get_future().get(); // Exceptions are ignored!
                test(p->opBatchCount() == 0);
            }

            if(protocol != "bt")
            {
                //
                // 2 connections.
                //
                test(p->opBatchCount() == 0);
                auto b1 = Ice::uncheckedCast<Test::TestIntfPrx>(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                auto b2 = Ice::uncheckedCast<Test::TestIntfPrx>(
                    p->ice_connectionId("2")->ice_getConnection()->createProxy(
                        p->ice_getIdentity())->ice_batchOneway());
                b2->ice_getConnection(); // Ensure connection is established.
                b1->opBatch();
                b1->opBatch();
                b2->opBatch();
                b2->opBatch();

                promise<void> promise;
                auto id = this_thread::get_id();
                communicator->flushBatchRequestsAsync(
                    Ice::CompressBatch::BasedOnProxy,
                    [&](exception_ptr ex)
                    {
                        promise.set_exception(move(ex));
                    },
                    [&](bool sentSynchronously)
                    {
                        test((sentSynchronously && id == this_thread::get_id()) ||
                             (!sentSynchronously && id != this_thread::get_id()));
                        promise.set_value();
                    });
                promise.get_future().get();
                test(p->waitForBatch(4));
            }

            if(protocol != "bt")
            {
                //
                // 2 connections - 2 failures.
                //
                // The sent callback should be invoked even if all connections fail.
                //
                test(p->opBatchCount() == 0);
                auto b1 = Ice::uncheckedCast<Test::TestIntfPrx>(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                auto b2 = Ice::uncheckedCast<Test::TestIntfPrx>(
                    p->ice_connectionId("2")->ice_getConnection()->createProxy(
                        p->ice_getIdentity())->ice_batchOneway());

                b2->ice_getConnection(); // Ensure connection is established.
                b1->opBatch();
                b2->opBatch();
                b1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
                b2->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));

                promise<void> promise;
                auto id = this_thread::get_id();
                communicator->flushBatchRequestsAsync(
                    Ice::CompressBatch::BasedOnProxy,
                    [&](exception_ptr ex)
                    {
                        promise.set_exception(move(ex));
                    },
                    [&](bool sentSynchronously)
                    {
                        test((sentSynchronously && id == this_thread::get_id()) ||
                             (!sentSynchronously && id != this_thread::get_id()));
                        promise.set_value();
                    });
                promise.get_future().get(); // Exceptions are ignored!
                test(p->opBatchCount() == 0);
            }
        }
        cout << "ok" << endl;

        cout << "testing cancel operations... " << flush;
        {
            if(p->ice_getConnection())
            {
                testController->holdAdapter();
                {
                    promise<void> promise;
                    auto cancel = p->ice_pingAsync(
                        [&]()
                        {
                            promise.set_value();
                        },
                        [&](exception_ptr ex)
                        {
                            promise.set_exception(ex);
                        });
                    cancel();

                    try
                    {
                        promise.get_future().get();
                        test(false);
                    }
                    catch(const Ice::InvocationCanceledException&)
                    {
                    }
                    catch(...)
                    {
                        testController->resumeAdapter();
                        throw;
                    }
                }

                {
                    promise<void> promise;
                    auto cancel = p->ice_idAsync(
                        [&](string)
                        {
                            promise.set_value();
                        },
                        [&](exception_ptr ex)
                        {
                            promise.set_exception(ex);
                        });
                    cancel();

                    try
                    {
                        promise.get_future().get();
                        test(false);
                    }
                    catch(const Ice::InvocationCanceledException&)
                    {
                    }
                    catch(...)
                    {
                        testController->resumeAdapter();
                        throw;
                    }
                }
                testController->resumeAdapter();
            }
        }
        cout << "ok" << endl;

        if(p->ice_getConnection() && protocol != "bt" && p->supportsAMD())
        {
            cout << "testing graceful close connection with wait... " << flush;
            {
                //
                // Local case: begin a request, close the connection gracefully, and make sure it waits
                // for the request to complete.
                //
                auto con = p->ice_getConnection();
                auto sc = make_shared<promise<void>>();
                con->setCloseCallback(
                    [sc](Ice::ConnectionPtr)
                    {
                        sc->set_value();
                    });
                auto fc = sc->get_future();
                auto s = make_shared<promise<void>>();
                p->sleepAsync(100,
                              [s]() { s->set_value(); },
                              [s](exception_ptr ex) { s->set_exception(ex); });
                auto f = s->get_future();
                // Blocks until the request completes.
                con->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
                f.get(); // Should complete successfully.
                fc.get();
            }
            {
                //
                // Remote case.
                //
                Ice::ByteSeq seq;
                seq.resize(1024 * 10);
                for(Ice::ByteSeq::iterator q = seq.begin(); q != seq.end(); ++q)
                {
                    *q = static_cast<Ice::Byte>(IceUtilInternal::random(255));
                }

                //
                // Send multiple opWithPayload, followed by a close and followed by multiple opWithPaylod.
                // The goal is to make sure that none of the opWithPayload fail even if the server closes
                // the connection gracefully in between.
                //

                int maxQueue = 2;
                bool done = false;
                while(!done && maxQueue < 5000)
                {
                    done = true;
                    p->ice_ping();
                    vector<future<void>> results;
                    for(int i = 0; i < maxQueue; ++i)
                    {
                        auto s = make_shared<promise<void>>();
                        p->opWithPayloadAsync(seq,
                                              [s]() { s->set_value(); },
                                              [s](exception_ptr ex) { s->set_exception(ex); });
                        results.push_back(s->get_future());
                    }
                    atomic_flag sent = ATOMIC_FLAG_INIT;
                    p->closeAsync(Test::CloseMode::GracefullyWithWait, nullptr, nullptr,
                                  [&sent](bool) { sent.test_and_set(); });
                    if(!sent.test_and_set())
                    {
                        for(int i = 0; i < maxQueue; i++)
                        {
                            auto s = make_shared<promise<void>>();
                            atomic_flag sent2 = ATOMIC_FLAG_INIT;
                            p->opWithPayloadAsync(seq,
                                                  [s]() { s->set_value(); },
                                                  [s](exception_ptr ex) { s->set_exception(ex); },
                                                  [&sent2](bool) { sent2.test_and_set(); });
                            results.push_back(s->get_future());
                            if(sent2.test_and_set())
                            {
                                done = false;
                                maxQueue *= 2;
                                break;
                            }
                        }
                    }
                    else
                    {
                        maxQueue *= 2;
                        done = false;
                    }

                    for(vector<future<void>>::iterator r = results.begin(); r != results.end(); ++r)
                    {
                        try
                        {
                            r->get();
                        }
                        catch(const Ice::LocalException&)
                        {
                            test(false);
                        }
                    }
                }
            }
            cout << "ok" << endl;

            cout << "testing graceful close connection without wait... " << flush;
            {
                //
                // Local case: start an operation and then close the connection gracefully on the client side
                // without waiting for the pending invocation to complete. There will be no retry and we expect the
                // invocation to fail with ConnectionManuallyClosedException.
                //
                p = p->ice_connectionId("CloseGracefully"); // Start with a new connection.
                auto con = p->ice_getConnection();
                auto s = make_shared<promise<void>>();
                auto sent = make_shared<promise<void>>();
                p->startDispatchAsync([s]() { s->set_value(); },
                                      [s](exception_ptr ex) { s->set_exception(ex); },
                                      [sent](bool) { sent->set_value(); });
                auto f = s->get_future();
                sent->get_future().get(); // Ensure the request was sent before we close the connection.
                con->close(Ice::ConnectionClose::Gracefully);
                try
                {
                    f.get();
                    test(false);
                }
                catch(const Ice::ConnectionManuallyClosedException& ex)
                {
                    test(ex.graceful);
                }
                p->finishDispatch();

                //
                // Remote case: the server closes the connection gracefully, which means the connection
                // will not be closed until all pending dispatched requests have completed.
                //
                con = p->ice_getConnection();
                auto sc = make_shared<promise<void>>();
                con->setCloseCallback(
                    [sc](Ice::ConnectionPtr)
                    {
                        sc->set_value();
                    });
                auto fc = sc->get_future();
                s = make_shared<promise<void>>();
                p->sleepAsync(100,
                              [s]() { s->set_value(); },
                              [s](exception_ptr ex) { s->set_exception(ex); });
                f = s->get_future();
                p->close(Test::CloseMode::Gracefully); // Close is delayed until sleep completes.
                fc.get(); // Ensure connection was closed.
                try
                {
                    f.get();
                }
                catch(const Ice::LocalException&)
                {
                    test(false);
                }
            }
            cout << "ok" << endl;

            cout << "testing forceful close connection... " << flush;
            {
                //
                // Local case: start a lengthy operation and then close the connection forcefully on the client side.
                // There will be no retry and we expect the invocation to fail with ConnectionManuallyClosedException.
                //
                p->ice_ping();
                auto con = p->ice_getConnection();
                auto s = make_shared<promise<void>>();
                auto sent = make_shared<promise<void>>();
                p->startDispatchAsync([s]() { s->set_value(); },
                                      [s](exception_ptr ex) { s->set_exception(ex); },
                                      [sent](bool) { sent->set_value(); });
                auto f = s->get_future();
                sent->get_future().get(); // Ensure the request was sent before we close the connection.
                con->close(Ice::ConnectionClose::Forcefully);
                try
                {
                    f.get();
                    test(false);
                }
                catch(const Ice::ConnectionManuallyClosedException& ex)
                {
                    test(!ex.graceful);
                }
                p->finishDispatch();

                //
                // Remote case: the server closes the connection forcefully. This causes the request to fail
                // with a ConnectionLostException. Since the close() operation is not idempotent, the client
                // will not retry.
                //
                try
                {
                    p->close(Test::CloseMode::Forcefully);
                    test(false);
                }
                catch(const Ice::ConnectionLostException&)
                {
                    // Expected.
                }
            }
            cout << "ok" << endl;
        }
    }

    {
        cout << "testing result struct... " << flush;

        auto q = Ice::uncheckedCast<Test::Outer::Inner::TestIntfPrx>(
            communicator->stringToProxy("test2:" + helper->getTestEndpoint()));

        promise<void> promise;
        q->opAsync(1,
            [&promise](int i, int j)
                {
                    test(i == j);
                    promise.set_value();
                },
                [](const exception_ptr& ex)
                {
                    try
                    {
                        rethrow_exception(ex);
                    }
                    catch(const std::exception& exc)
                    {
                        cerr << exc.what() << endl;
                    }
                    test(false);
                });
        promise.get_future().get();

        auto f = q->opAsync(1);
        auto r = f.get();
        test(r.returnValue == r.j);
        test(r.returnValue == 1);
        cout << "ok" << endl;
    }

    p->shutdown();

#else
    string sref = "test:" + helper->getTestEndpoint();
    Ice::ObjectPrx obj = communicator->stringToProxy(sref);
    test(obj);

    Test::TestIntfPrx p = Test::TestIntfPrx::uncheckedCast(obj);

    sref = "testController:" + helper->getTestEndpoint(1);
    obj = communicator->stringToProxy(sref);
    test(obj);

    Test::TestIntfControllerPrx testController = Test::TestIntfControllerPrx::uncheckedCast(obj);

    cout << "testing begin/end invocation... " << flush;
    {
        Ice::AsyncResultPtr result;
        Ice::Context ctx;

        result = p->begin_ice_isA(Test::TestIntf::ice_staticId());
        test(p->end_ice_isA(result));
        result = p->begin_ice_isA(Test::TestIntf::ice_staticId(), ctx);
        test(p->end_ice_isA(result));

        result = p->begin_ice_ping();
        p->end_ice_ping(result);
        result = p->begin_ice_ping(ctx);
        p->end_ice_ping(result);

        result = p->begin_ice_id();
        test(p->end_ice_id(result) == Test::TestIntf::ice_staticId());
        result = p->begin_ice_id(ctx);
        test(p->end_ice_id(result) == Test::TestIntf::ice_staticId());

        result = p->begin_ice_ids();
        test(p->end_ice_ids(result).size() == 2);
        result = p->begin_ice_ids(ctx);
        test(p->end_ice_ids(result).size() == 2);

        if(!collocated)
        {
            result = p->begin_ice_getConnection();
            test(p->end_ice_getConnection(result));
        }

        result = p->begin_op();
        p->end_op(result);
        result = p->begin_op(ctx);
        p->end_op(result);

        result = p->begin_opWithResult();
        test(p->end_opWithResult(result) == 15);
        result = p->begin_opWithResult(ctx);
        test(p->end_opWithResult(result) == 15);

        result = p->begin_opWithUE();
        try
        {
            p->end_opWithUE(result);
            test(false);
        }
        catch(const Test::TestIntfException&)
        {
        }
        result = p->begin_opWithUE(ctx);
        try
        {
            p->end_opWithUE(result);
            test(false);
        }
        catch(const Test::TestIntfException&)
        {
        }
    }
    cout << "ok" << endl;

    cout << "testing async callback... " << flush;
    {
        AsyncCallbackPtr cb = new AsyncCallback();
        Ice::Context ctx;
        CookiePtr cookie = new Cookie(5);
        AsyncCallbackPtr cbWC = new AsyncCallback(cookie);

        p->begin_ice_isA(Test::TestIntf::ice_staticId(), Ice::newCallback(cb, &AsyncCallback::isA));
        cb->check();
        p->begin_ice_isA(Test::TestIntf::ice_staticId(), Ice::newCallback(cbWC, &AsyncCallback::isA), cookie);
        cbWC->check();
        p->begin_ice_isA(Test::TestIntf::ice_staticId(), ctx, Ice::newCallback(cb, &AsyncCallback::isA));
        cb->check();
        p->begin_ice_isA(Test::TestIntf::ice_staticId(), ctx, Ice::newCallback(cbWC, &AsyncCallback::isA), cookie);
        cbWC->check();

        p->begin_ice_ping(Ice::newCallback(cb, &AsyncCallback::ping));
        cb->check();
        p->begin_ice_ping(Ice::newCallback(cbWC, &AsyncCallback::ping), cookie);
        cbWC->check();
        p->begin_ice_ping(ctx, Ice::newCallback(cb, &AsyncCallback::ping));
        cb->check();
        p->begin_ice_ping(ctx, Ice::newCallback(cbWC, &AsyncCallback::ping), cookie);
        cbWC->check();

        p->begin_ice_id(Ice::newCallback(cb, &AsyncCallback::id));
        cb->check();
        p->begin_ice_id(Ice::newCallback(cbWC, &AsyncCallback::id), cookie);
        cbWC->check();
        p->begin_ice_id(ctx, Ice::newCallback(cb, &AsyncCallback::id));
        cb->check();
        p->begin_ice_id(ctx, Ice::newCallback(cbWC, &AsyncCallback::id), cookie);
        cbWC->check();

        p->begin_ice_ids(Ice::newCallback(cb, &AsyncCallback::ids));
        cb->check();
        p->begin_ice_ids(Ice::newCallback(cbWC, &AsyncCallback::ids), cookie);
        cbWC->check();
        p->begin_ice_ids(ctx, Ice::newCallback(cb, &AsyncCallback::ids));
        cb->check();
        p->begin_ice_ids(ctx, Ice::newCallback(cbWC, &AsyncCallback::ids), cookie);
        cbWC->check();

        if(!collocated)
        {
            p->begin_ice_getConnection(Ice::newCallback(cb, &AsyncCallback::connection));
            cb->check();
            p->begin_ice_getConnection(Ice::newCallback(cbWC, &AsyncCallback::connection), cookie);
            cbWC->check();
        }

        p->begin_op(Ice::newCallback(cb, &AsyncCallback::op));
        cb->check();
        p->begin_op(Ice::newCallback(cbWC, &AsyncCallback::op), cookie);
        cbWC->check();
        p->begin_op(ctx, Ice::newCallback(cb, &AsyncCallback::op));
        cb->check();
        p->begin_op(ctx, Ice::newCallback(cbWC, &AsyncCallback::op), cookie);
        cbWC->check();

        p->begin_opWithResult(Ice::newCallback(cb, &AsyncCallback::opWithResult));
        cb->check();
        p->begin_opWithResult(Ice::newCallback(cbWC, &AsyncCallback::opWithResult), cookie);
        cbWC->check();
        p->begin_opWithResult(ctx, Ice::newCallback(cb, &AsyncCallback::opWithResult));
        cb->check();
        p->begin_opWithResult(ctx, Ice::newCallback(cbWC, &AsyncCallback::opWithResult), cookie);
        cbWC->check();

        p->begin_opWithUE(Ice::newCallback(cb, &AsyncCallback::opWithUE));
        cb->check();
        p->begin_opWithUE(Ice::newCallback(cbWC, &AsyncCallback::opWithUE), cookie);
        cbWC->check();
        p->begin_opWithUE(ctx, Ice::newCallback(cb, &AsyncCallback::opWithUE));
        cb->check();
        p->begin_opWithUE(ctx, Ice::newCallback(cbWC, &AsyncCallback::opWithUE), cookie);
        cbWC->check();
    }
    cout << "ok" << endl;

    cout << "testing response callback... " << flush;
    {
        ResponseCallbackPtr cb = new ResponseCallback();
        Ice::Context ctx;
        CookiePtr cookie = new Cookie(5);
        ResponseCallbackWCPtr cbWC = new ResponseCallbackWC(cookie);

        Ice::CallbackNC_Object_ice_isA<ResponseCallback>::Exception nullEx = 0;
        Ice::Callback_Object_ice_isA<ResponseCallbackWC, CookiePtr>::Exception nullExWC = 0;

        p->begin_ice_isA(Test::TestIntf::ice_staticId(), Ice::newCallback_Object_ice_isA(cb, &ResponseCallback::isA,
                                                                                         nullEx));
        cb->check();
        p->begin_ice_isA(Test::TestIntf::ice_staticId(),
                         Ice::newCallback_Object_ice_isA(cbWC, &ResponseCallbackWC::isA, nullExWC), cookie);
        cbWC->check();
        p->begin_ice_isA(Test::TestIntf::ice_staticId(), ctx,
                         Ice::newCallback_Object_ice_isA(cb, &ResponseCallback::isA, nullEx));
        cb->check();
        p->begin_ice_isA(Test::TestIntf::ice_staticId(), ctx,
                         Ice::newCallback_Object_ice_isA(cbWC, &ResponseCallbackWC::isA, nullExWC), cookie);
        cbWC->check();

        p->begin_ice_ping(Ice::newCallback_Object_ice_ping(cb, &ResponseCallback::ping, nullEx));
        cb->check();
        p->begin_ice_ping(Ice::newCallback_Object_ice_ping(cbWC, &ResponseCallbackWC::ping, nullExWC), cookie);
        cbWC->check();
        p->begin_ice_ping(ctx, Ice::newCallback_Object_ice_ping(cb, &ResponseCallback::ping, nullEx));
        cb->check();
        p->begin_ice_ping(ctx, Ice::newCallback_Object_ice_ping(cbWC, &ResponseCallbackWC::ping, nullExWC), cookie);
        cbWC->check();

        p->begin_ice_id(Ice::newCallback_Object_ice_id(cb, &ResponseCallback::id, nullEx));
        cb->check();
        p->begin_ice_id(Ice::newCallback_Object_ice_id(cbWC, &ResponseCallbackWC::id, nullExWC), cookie);
        cbWC->check();
        p->begin_ice_id(ctx, Ice::newCallback_Object_ice_id(cb, &ResponseCallback::id, nullEx));
        cb->check();
        p->begin_ice_id(ctx, Ice::newCallback_Object_ice_id(cbWC, &ResponseCallbackWC::id, nullExWC), cookie);
        cbWC->check();

        p->begin_ice_ids(Ice::newCallback_Object_ice_ids(cb, &ResponseCallback::ids, nullEx));
        cb->check();
        p->begin_ice_ids(Ice::newCallback_Object_ice_ids(cbWC, &ResponseCallbackWC::ids, nullExWC), cookie);
        cbWC->check();
        p->begin_ice_ids(ctx, Ice::newCallback_Object_ice_ids(cb, &ResponseCallback::ids, nullEx));
        cb->check();
        p->begin_ice_ids(ctx, Ice::newCallback_Object_ice_ids(cbWC, &ResponseCallbackWC::ids, nullExWC), cookie);
        cbWC->check();

        if(!collocated)
        {
            p->begin_ice_getConnection(Ice::newCallback_Object_ice_getConnection(cb, &ResponseCallback::connection,
                                       nullEx));
            cb->check();
            p->begin_ice_getConnection(Ice::newCallback_Object_ice_getConnection(cbWC, &ResponseCallbackWC::connection,
                                       nullExWC), cookie);
            cbWC->check();
        }

        p->begin_op(Test::newCallback_TestIntf_op(cb, &ResponseCallback::op, nullEx));
        cb->check();
        p->begin_op(Test::newCallback_TestIntf_op(cbWC, &ResponseCallbackWC::op, nullExWC), cookie);
        cbWC->check();
        p->begin_op(ctx, Test::newCallback_TestIntf_op(cb, &ResponseCallback::op, nullEx));
        cb->check();
        p->begin_op(ctx, Test::newCallback_TestIntf_op(cbWC, &ResponseCallbackWC::op, nullExWC), cookie);
        cbWC->check();

        p->begin_opWithResult(Test::newCallback_TestIntf_opWithResult(cb, &ResponseCallback::opWithResult, nullEx));
        cb->check();
        p->begin_opWithResult(Test::newCallback_TestIntf_opWithResult(cbWC, &ResponseCallbackWC::opWithResult,
                                                                      nullExWC), cookie);
        cbWC->check();
        p->begin_opWithResult(ctx, Test::newCallback_TestIntf_opWithResult(cb, &ResponseCallback::opWithResult,
                                                                           nullEx));
        cb->check();
        p->begin_opWithResult(ctx, Test::newCallback_TestIntf_opWithResult(cbWC, &ResponseCallbackWC::opWithResult,
                                                                           nullExWC), cookie);
        cbWC->check();

        p->begin_opWithUE(Test::newCallback_TestIntf_opWithUE(cb, &ResponseCallback::op, &ResponseCallback::opWithUE));
        cb->check();
        p->begin_opWithUE(Test::newCallback_TestIntf_opWithUE(cbWC, &ResponseCallbackWC::op,
                                                              &ResponseCallbackWC::opWithUE), cookie);
        cbWC->check();
        p->begin_opWithUE(ctx, Test::newCallback_TestIntf_opWithUE(cb, &ResponseCallback::op,
                                                                   &ResponseCallback::opWithUE));
        cb->check();
        p->begin_opWithUE(ctx, Test::newCallback_TestIntf_opWithUE(cbWC, &ResponseCallbackWC::op,
                                                                   &ResponseCallbackWC::opWithUE), cookie);
        cbWC->check();
    }
    cout << "ok" << endl;

    cout << "testing local exceptions... " << flush;
    {
        Test::TestIntfPrx indirect = Test::TestIntfPrx::uncheckedCast(p->ice_adapterId("dummy"));
        Ice::AsyncResultPtr r;

        r = indirect->begin_op();
        try
        {
            indirect->end_op(r);
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
        }

        try
        {
            r = p->ice_oneway()->begin_opWithResult();
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }

        //
        // Check that CommunicatorDestroyedException is raised directly.
        //
        if(p->ice_getConnection() && protocol != "bt")
        {
            Ice::InitializationData initData;
            initData.properties = communicator->getProperties()->clone();
            Ice::CommunicatorPtr ic = Ice::initialize(initData);
            obj = ic->stringToProxy(p->ice_toString());
            Test::TestIntfPrx p2 = Test::TestIntfPrx::checkedCast(obj);
            ic->destroy();

            try
            {
                p2->begin_op();
                test(false);
            }
            catch(const Ice::CommunicatorDestroyedException&)
            {
                // Expected.
            }
        }
    }
    cout << "ok" << endl;

    cout << "testing local exceptions with async callback... " << flush;
    {
        Test::TestIntfPrx i = Test::TestIntfPrx::uncheckedCast(p->ice_adapterId("dummy"));
        AsyncCallbackPtr cb = new AsyncCallback();
        CookiePtr cookie = new Cookie(5);
        AsyncCallbackPtr cbWC = new AsyncCallback(cookie);

        i->begin_ice_isA(Test::TestIntf::ice_staticId(), Ice::newCallback(cb, &AsyncCallback::isAEx));
        cb->check();
        i->begin_ice_isA(Test::TestIntf::ice_staticId(), Ice::newCallback(cbWC, &AsyncCallback::isAEx), cookie);
        cbWC->check();

        i->begin_ice_ping(Ice::newCallback(cb, &AsyncCallback::pingEx));
        cb->check();
        i->begin_ice_ping(Ice::newCallback(cbWC, &AsyncCallback::pingEx), cookie);
        cbWC->check();

        i->begin_ice_id(Ice::newCallback(cb, &AsyncCallback::idEx));
        cb->check();
        i->begin_ice_id(Ice::newCallback(cbWC, &AsyncCallback::idEx), cookie);
        cbWC->check();

        i->begin_ice_ids(Ice::newCallback(cb, &AsyncCallback::idsEx));
        cb->check();
        i->begin_ice_ids(Ice::newCallback(cbWC, &AsyncCallback::idsEx), cookie);
        cbWC->check();

        if(!collocated)
        {
            i->begin_ice_getConnection(Ice::newCallback(cb, &AsyncCallback::connectionEx));
            cb->check();
        }

        i->begin_op(Ice::newCallback(cb, &AsyncCallback::opEx));
        cb->check();
        i->begin_op(Ice::newCallback(cbWC, &AsyncCallback::opEx), cookie);
        cbWC->check();
    }
    cout << "ok" << endl;

    cout << "testing local exceptions with response callback... " << flush;
    {
        Test::TestIntfPrx i = Test::TestIntfPrx::uncheckedCast(p->ice_adapterId("dummy"));
        ExceptionCallbackPtr cb = new ExceptionCallback();
        CookiePtr cookie = new Cookie(5);
        ExceptionCallbackWCPtr cbWC = new ExceptionCallbackWC(cookie);

        i->begin_ice_isA(Test::TestIntf::ice_staticId(),
                         Ice::newCallback_Object_ice_isA(cb, &ExceptionCallback::isA, &ExceptionCallback::ex));
        cb->check();
        i->begin_ice_isA(Test::TestIntf::ice_staticId(),
                         Ice::newCallback_Object_ice_isA(cbWC, &ExceptionCallbackWC::isA, &ExceptionCallbackWC::ex),
                         cookie);
        cbWC->check();

        i->begin_ice_ping(Ice::newCallback_Object_ice_ping(cb, &ExceptionCallback::ping, &ExceptionCallback::ex));
        cb->check();
        i->begin_ice_ping(Ice::newCallback_Object_ice_ping(cbWC, &ExceptionCallbackWC::ping,
                                                           &ExceptionCallbackWC::ex), cookie);
        cbWC->check();

        i->begin_ice_id(Ice::newCallback_Object_ice_id(cb, &ExceptionCallback::id, &ExceptionCallback::ex));
        cb->check();
        i->begin_ice_id(Ice::newCallback_Object_ice_id(cbWC, &ExceptionCallbackWC::id,
                                                       &ExceptionCallbackWC::ex), cookie);
        cbWC->check();

        i->begin_ice_ids(Ice::newCallback_Object_ice_ids(cb, &ExceptionCallback::ids, &ExceptionCallback::ex));
        cb->check();
        i->begin_ice_ids(Ice::newCallback_Object_ice_ids(cbWC, &ExceptionCallbackWC::ids,
                                                         &ExceptionCallbackWC::ex), cookie);
        cbWC->check();

        if(!collocated)
        {
            i->begin_ice_getConnection(
                Ice::newCallback_Object_ice_getConnection(cb, &ExceptionCallback::connection, &ExceptionCallback::ex));
            cb->check();
        }

        i->begin_op(Test::newCallback_TestIntf_op(cb, &ExceptionCallback::op, &ExceptionCallback::ex));
        cb->check();
        i->begin_op(Test::newCallback_TestIntf_op(cbWC, &ExceptionCallbackWC::op, &ExceptionCallbackWC::ex), cookie);
        cbWC->check();
    }
    cout << "ok" << endl;

    cout << "testing exception callback... " << flush;
    {
        Test::TestIntfPrx i = Test::TestIntfPrx::uncheckedCast(p->ice_adapterId("dummy"));
        ExceptionCallbackPtr cb = new ExceptionCallback();
        CookiePtr cookie = new Cookie(5);
        ExceptionCallbackWCPtr cbWC = new ExceptionCallbackWC(cookie);

        i->begin_ice_isA(Test::TestIntf::ice_staticId(),
                         Ice::newCallback_Object_ice_isA(cb, &ExceptionCallback::ex));
        cb->check();
        i->begin_ice_isA(Test::TestIntf::ice_staticId(),
                         Ice::newCallback_Object_ice_isA(cbWC, &ExceptionCallbackWC::ex), cookie);
        cbWC->check();

        i->begin_op(Test::newCallback_TestIntf_op(cb, &ExceptionCallback::ex));
        cb->check();
        i->begin_op(Test::newCallback_TestIntf_op(cbWC, &ExceptionCallbackWC::ex), cookie);
        cbWC->check();

        // Operations that return a result must provide the response callback
        //i->begin_opWithResult(Test::newCallback_TestIntf_opWithResult(cb, &ExceptionCallback::ex));
        //cb->check();
        //i->begin_opWithResult(Test::newCallback_TestIntf_opWithResult(cbWC, &ExceptionCallbackWC::ex), cookie);
        //cbWC->check();

        i->begin_opWithUE(Test::newCallback_TestIntf_opWithUE(cb, &ExceptionCallback::ex));
        cb->check();
        i->begin_opWithUE(Test::newCallback_TestIntf_opWithUE(cbWC, &ExceptionCallbackWC::ex), cookie);
        cbWC->check();

        // Ensures no exception is called when response is received
        p->begin_ice_isA(Test::TestIntf::ice_staticId(),
                         Ice::newCallback_Object_ice_isA(cb, &ExceptionCallback::noEx));
        p->begin_ice_isA(Test::TestIntf::ice_staticId(),
                         Ice::newCallback_Object_ice_isA(cbWC, &ExceptionCallbackWC::noEx), cookie);
        p->begin_op(Test::newCallback_TestIntf_op(cb, &ExceptionCallback::noEx));
        p->begin_op(Test::newCallback_TestIntf_op(cbWC, &ExceptionCallbackWC::noEx), cookie);
        //p->begin_opWithResult(Test::newCallback_TestIntf_opWithResult(cb, &ExceptionCallback::noEx));
        //p->begin_opWithResult(Test::newCallback_TestIntf_opWithResult(cbWC, &ExceptionCallbackWC::noEx), cookie);

        // If response is a user exception, it should be received.
        p->begin_opWithUE(Test::newCallback_TestIntf_opWithUE(cb, &ExceptionCallback::opWithUE));
        cb->check();

        p->begin_opWithUE(Test::newCallback_TestIntf_opWithUE(cbWC, &ExceptionCallbackWC::opWithUE), cookie);
        cbWC->check();
    }
    cout << "ok" << endl;

    cout << "testing sent callback... " << flush;
    {
        SentCallbackPtr cb = new SentCallback;
        CookiePtr cookie = new Cookie(4);
        SentCallbackPtr cbWC = new SentCallback(cookie);

        p->begin_ice_isA("", Ice::newCallback_Object_ice_isA(cb, &SentCallback::isA, &SentCallback::ex,
                                                             &SentCallback::sent));
        cb->check();
        p->begin_ice_isA("", Ice::newCallback_Object_ice_isA(cbWC, &SentCallback::isAWC, &SentCallback::exWC,
                                                             &SentCallback::sentWC), cookie);
        cbWC->check();

        p->begin_ice_ping(Ice::newCallback_Object_ice_ping(cb, &SentCallback::ping, &SentCallback::ex,
                                                           &SentCallback::sent));
        cb->check();
        p->begin_ice_ping(Ice::newCallback_Object_ice_ping(cbWC, &SentCallback::pingWC, &SentCallback::exWC,
                                                           &SentCallback::sentWC), cookie);
        cbWC->check();

        p->begin_ice_id(Ice::newCallback_Object_ice_id(cb, &SentCallback::id, &SentCallback::ex, &SentCallback::sent));
        cb->check();
        p->begin_ice_id(Ice::newCallback_Object_ice_id(cbWC, &SentCallback::idWC, &SentCallback::exWC,
                                                       &SentCallback::sentWC), cookie);
        cbWC->check();

        p->begin_ice_ids(Ice::newCallback_Object_ice_ids(cb, &SentCallback::ids, &SentCallback::ex,
                                                         &SentCallback::sent));
        cb->check();
        p->begin_ice_ids(Ice::newCallback_Object_ice_ids(cbWC, &SentCallback::idsWC, &SentCallback::exWC,
                                                         &SentCallback::sentWC), cookie);
        cbWC->check();

        p->begin_op(Test::newCallback_TestIntf_op(cb, &SentCallback::op, &SentCallback::ex, &SentCallback::sent));
        cb->check();
        p->begin_op(Test::newCallback_TestIntf_op(cbWC, &SentCallback::opWC, &SentCallback::exWC,
                                                  &SentCallback::sentWC), cookie);
        cbWC->check();

        p->begin_op(Ice::newCallback(cb, &SentCallback::opAsync, &SentCallback::sentAsync));
        cb->check();
        p->begin_op(Ice::newCallback(cbWC, &SentCallback::opAsync, &SentCallback::sentAsync), cookie);
        cbWC->check();

        p->begin_op(Test::newCallback_TestIntf_op(cb, &SentCallback::ex, &SentCallback::sent));
        cb->check();
        p->begin_op(Test::newCallback_TestIntf_op(cbWC, &SentCallback::exWC, &SentCallback::sentWC), cookie);
        cbWC->check();

        vector<SentCallbackPtr> cbs;
        Ice::ByteSeq seq;
        seq.resize(1024);
        testController->holdAdapter();
        try
        {
            cb = new SentCallback();
            while(p->begin_opWithPayload(seq, Test::newCallback_TestIntf_opWithPayload(
                                             cb, &SentCallback::ex, &SentCallback::sent))->sentSynchronously())
            {
                cbs.push_back(cb);
                cb = new SentCallback();
            }
        }
        catch(...)
        {
            testController->resumeAdapter();
            throw;
        }
        testController->resumeAdapter();
        for(vector<SentCallbackPtr>::const_iterator r = cbs.begin(); r != cbs.end(); r++)
        {
            (*r)->check();
        }
    }
    cout << "ok" << endl;

    cout << "testing illegal arguments... " << flush;
    {
        Ice::AsyncResultPtr result;

        result = p->begin_op();
        p->end_op(result);
        try
        {
            p->end_op(result);
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }

        result = p->begin_op();
        try
        {
            p->end_opWithResult(result);
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }

        try
        {
            p->end_op(0);
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }

        try
        {
            p->begin_op(Ice::newCallback(AsyncCallbackPtr(), &AsyncCallback::op));
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }

        try
        {
            p->begin_op(Test::newCallback_TestIntf_op(ResponseCallbackPtr(), &ResponseCallback::op,
                                                      &ResponseCallback::ex));
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }

        try
        {
            p->begin_op(Test::newCallback_TestIntf_op(ResponseCallbackWCPtr(), &ResponseCallbackWC::op,
                                                      &ResponseCallbackWC::ex));
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }

        try
        {
            void (AsyncCallback::*nullCallback)(const Ice::AsyncResultPtr&) = 0;
            p->begin_op(Ice::newCallback(new AsyncCallback, nullCallback));
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }

        try
        {
            void (ResponseCallback::*nullCallback)() = 0;
            void (ResponseCallback::*nullExCallback)(const Ice::Exception&) = 0;
            p->begin_op(Test::newCallback_TestIntf_op(new ResponseCallback, nullCallback, nullExCallback));
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }

        try
        {
            ResponseCallbackPtr cb = new ResponseCallback();
            p->begin_op(Test::newCallback_TestIntf_op(cb, &ResponseCallback::op, &ResponseCallback::ex), new Cookie(3));
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }

        try
        {
            ResponseCallbackWCPtr cb = new ResponseCallbackWC(new Cookie(3));
            p->begin_op(Test::newCallback_TestIntf_op(cb, &ResponseCallbackWC::op, &ResponseCallbackWC::ex),
                        new Ice::LocalObject());
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }
    }
    cout << "ok" << endl;

    cout << "testing unexpected exceptions from callback... " << flush;
    {
        Test::TestIntfPrx q = Test::TestIntfPrx::uncheckedCast(p->ice_adapterId("dummy"));
        ThrowType throwEx[] = { LocalException, UserException, StandardException, OtherException };
        CookiePtr cookie = new Cookie(5);

        for(int i = 0; i < 4; ++i)
        {
            ThrowerPtr cb = new Thrower(throwEx[i]);

            p->begin_op(Ice::newCallback(cb, &Thrower::opAsync));
            cb->check();

            p->begin_op(Test::newCallback_TestIntf_op(cb, &Thrower::op, &Thrower::noEx));
            cb->check();

            p->begin_op(Test::newCallback_TestIntf_op(cb, &Thrower::opWC, &Thrower::noExWC), cookie);
            cb->check();

            q->begin_op(Test::newCallback_TestIntf_op(cb, &Thrower::op, &Thrower::ex));
            cb->check();

            q->begin_op(Test::newCallback_TestIntf_op(cb, &Thrower::opWC, &Thrower::exWC), cookie);
            cb->check();

            p->begin_op(Test::newCallback_TestIntf_op(cb, &Thrower::noOp, &Thrower::ex, &Thrower::sent));
            cb->check();

            p->begin_op(Test::newCallback_TestIntf_op(cb, &Thrower::noOpWC, &Thrower::exWC, &Thrower::sentWC), cookie);
            cb->check();

            q->begin_op(Test::newCallback_TestIntf_op(cb, &Thrower::ex));
            cb->check();

            q->begin_op(Test::newCallback_TestIntf_op(cb, &Thrower::exWC), cookie);
            cb->check();
        }
    }
    cout << "ok" << endl;

    cout << "testing batch requests with proxy... " << flush;
    {
        test(p->ice_batchOneway()->begin_ice_flushBatchRequests()->sentSynchronously());

        CookiePtr cookie = new Cookie(5);

        {
            //
            // AsyncResult without cookie.
            //
            test(p->opBatchCount() == 0);
            Test::TestIntfPrx b1 = p->ice_batchOneway();
            b1->opBatch();
            b1->opBatch();
            FlushCallbackPtr cb = new FlushCallback();
            Ice::AsyncResultPtr r = b1->begin_ice_flushBatchRequests(
                Ice::newCallback(cb, &FlushCallback::completedAsync, &FlushCallback::sentAsync));
            cb->check();
            test(r->isSent());
            test(r->isCompleted());
            test(p->waitForBatch(2));
        }

        {
            //
            // AsyncResult with cookie.
            //
            test(p->opBatchCount() == 0);
            Test::TestIntfPrx b1 = p->ice_batchOneway();
            b1->opBatch();
            b1->opBatch();
            FlushCallbackPtr cb = new FlushCallback(cookie);
            b1->begin_ice_flushBatchRequests(
                Ice::newCallback(cb, &FlushCallback::completedAsync, &FlushCallback::sentAsync), cookie);
            cb->check();
            test(p->waitForBatch(2));
        }

        if(p->ice_getConnection())
        {
            if(protocol != "bt")
            {
                //
                // AsyncResult exception without cookie.
                //
                test(p->opBatchCount() == 0);
                Test::TestIntfPrx b1 = p->ice_batchOneway();
                b1->opBatch();
                b1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
                FlushCallbackPtr cb = new FlushCallback();
                Ice::AsyncResultPtr r = b1->begin_ice_flushBatchRequests(
                    Ice::newCallback(cb, &FlushCallback::completedAsync, &FlushCallback::sentAsync));
                cb->check();
                test(r->isSent());
                test(r->isCompleted());
                test(p->waitForBatch(1));
            }

            if(protocol != "bt")
            {
                //
                // AsyncResult exception with cookie.
                //
                test(p->opBatchCount() == 0);
                Test::TestIntfPrx b1 = p->ice_batchOneway();
                b1->opBatch();
                b1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
                FlushCallbackPtr cb = new FlushCallback(cookie);
                b1->begin_ice_flushBatchRequests(
                    Ice::newCallback(cb, &FlushCallback::completedAsync, &FlushCallback::sentAsync), cookie);
                cb->check();
                test(p->waitForBatch(1));
            }
        }

        {
            //
            // Without cookie.
            //
            test(p->opBatchCount() == 0);
            Test::TestIntfPrx b1 = p->ice_batchOneway();
            b1->opBatch();
            b1->opBatch();
            FlushCallbackPtr cb = new FlushCallback();
            Ice::AsyncResultPtr r = b1->begin_ice_flushBatchRequests(
                Ice::newCallback_Object_ice_flushBatchRequests(cb, &FlushCallback::exception, &FlushCallback::sent));
            cb->check();
            test(r->isSent());
            test(r->isCompleted());
            test(p->waitForBatch(2));
        }

        {
            //
            // With cookie.
            //
            test(p->opBatchCount() == 0);
            Test::TestIntfPrx b1 = p->ice_batchOneway();
            b1->opBatch();
            b1->opBatch();
            FlushCallbackPtr cb = new FlushCallback(cookie);
            b1->begin_ice_flushBatchRequests(
                Ice::newCallback_Object_ice_flushBatchRequests(cb, &FlushCallback::exceptionWC,
                                                               &FlushCallback::sentWC), cookie);
            cb->check();
            test(p->waitForBatch(2));
        }

        if(p->ice_getConnection() && protocol != "bt")
        {
            {
                //
                // Exception without cookie.
                //
                test(p->opBatchCount() == 0);
                Test::TestIntfPrx b1 = p->ice_batchOneway();
                b1->opBatch();
                b1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
                FlushCallbackPtr cb = new FlushCallback();
                Ice::AsyncResultPtr r = b1->begin_ice_flushBatchRequests(
                    Ice::newCallback_Object_ice_flushBatchRequests(cb, &FlushCallback::exception,
                                                                   &FlushCallback::sent));
                cb->check();
                test(r->isSent());
                test(r->isCompleted());
                test(p->waitForBatch(1));
            }

            {
                //
                // Exception with cookie.
                //
                test(p->opBatchCount() == 0);
                Test::TestIntfPrx b1 = p->ice_batchOneway();
                b1->opBatch();
                b1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
                FlushCallbackPtr cb = new FlushCallback(cookie);
                b1->begin_ice_flushBatchRequests(
                    Ice::newCallback_Object_ice_flushBatchRequests(cb, &FlushCallback::exceptionWC,
                                                                   &FlushCallback::sentWC), cookie);
                cb->check();
                test(p->waitForBatch(1));
            }
        }
    }
    cout << "ok" << endl;

    if(p->ice_getConnection()) // No collocation optimization
    {
        cout << "testing batch requests with connection... " << flush;
        {
            CookiePtr cookie = new Cookie(5);

            {
                //
                // AsyncResult without cookie.
                //
                test(p->opBatchCount() == 0);
                Test::TestIntfPrx b1 = Test::TestIntfPrx::uncheckedCast(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                b1->opBatch();
                b1->opBatch();
                FlushCallbackPtr cb = new FlushCallback();
                Ice::AsyncResultPtr r = b1->ice_getConnection()->begin_flushBatchRequests(
                    Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy),
                    Ice::newCallback(cb, &FlushCallback::completedAsync, &FlushCallback::sentAsync));
                cb->check();
                test(r->isSent());
                test(r->isCompleted());
                test(p->waitForBatch(2));

                // Ensure it also works with a twoway proxy
                cb = new FlushCallback();
                r = p->ice_getConnection()->begin_flushBatchRequests(Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy),
                    Ice::newCallback(cb, &FlushCallback::completedAsync, &FlushCallback::sentAsync));
                cb->check();
                test(r->isSent());
                test(r->isCompleted());
            }

            {
                //
                // AsyncResult with cookie.
                //
                test(p->opBatchCount() == 0);
                Test::TestIntfPrx b1 = Test::TestIntfPrx::uncheckedCast(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                b1->opBatch();
                b1->opBatch();
                FlushCallbackPtr cb = new FlushCallback(cookie);
                b1->ice_getConnection()->begin_flushBatchRequests(Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy),
                    Ice::newCallback(cb, &FlushCallback::completedAsync, &FlushCallback::sentAsync), cookie);
                cb->check();
                test(p->waitForBatch(2));
            }

            if(protocol != "bt")
            {
                //
                // AsyncResult exception without cookie.
                //
                test(p->opBatchCount() == 0);
                Test::TestIntfPrx b1 = Test::TestIntfPrx::uncheckedCast(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                b1->opBatch();
                b1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
                FlushExCallbackPtr cb = new FlushExCallback();
                Ice::AsyncResultPtr r = b1->ice_getConnection()->begin_flushBatchRequests(
                    Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy),
                    Ice::newCallback(cb, &FlushExCallback::completedAsync, &FlushExCallback::sentAsync));
                cb->check();
                test(!r->isSent());
                test(r->isCompleted());
                test(p->opBatchCount() == 0);
            }

            if(protocol != "bt")
            {
                //
                // AsyncResult exception with cookie.
                //
                test(p->opBatchCount() == 0);
                Test::TestIntfPrx b1 = Test::TestIntfPrx::uncheckedCast(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                b1->opBatch();
                b1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
                FlushExCallbackPtr cb = new FlushExCallback(cookie);
                b1->ice_getConnection()->begin_flushBatchRequests(Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy),
                    Ice::newCallback(cb, &FlushExCallback::completedAsync, &FlushExCallback::sentAsync), cookie);
                cb->check();
                test(p->opBatchCount() == 0);
            }

            {
                //
                // Without cookie.
                //
                test(p->opBatchCount() == 0);
                Test::TestIntfPrx b1 = Test::TestIntfPrx::uncheckedCast(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                b1->opBatch();
                b1->opBatch();
                FlushCallbackPtr cb = new FlushCallback();
                Ice::AsyncResultPtr r = b1->ice_getConnection()->begin_flushBatchRequests(
                    Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy),
                    Ice::newCallback_Connection_flushBatchRequests(cb, &FlushCallback::exception,
                                                                   &FlushCallback::sent));
                cb->check();
                test(r->isSent());
                test(r->isCompleted());
                test(p->waitForBatch(2));
            }

            {
                //
                // With cookie.
                //
                test(p->opBatchCount() == 0);
                Test::TestIntfPrx b1 = Test::TestIntfPrx::uncheckedCast(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                b1->opBatch();
                b1->opBatch();
                FlushCallbackPtr cb = new FlushCallback(cookie);
                b1->ice_getConnection()->begin_flushBatchRequests(Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy),
                    Ice::newCallback_Connection_flushBatchRequests(cb, &FlushCallback::exceptionWC,
                                                                   &FlushCallback::sentWC), cookie);
                cb->check();
                test(p->waitForBatch(2));
            }

            if(protocol != "bt")
            {
                //
                // Exception without cookie.
                //
                test(p->opBatchCount() == 0);
                Test::TestIntfPrx b1 = Test::TestIntfPrx::uncheckedCast(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                b1->opBatch();
                b1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
                FlushExCallbackPtr cb = new FlushExCallback();
                Ice::AsyncResultPtr r = b1->ice_getConnection()->begin_flushBatchRequests(
                    Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy),
                    Ice::newCallback_Connection_flushBatchRequests(cb, &FlushExCallback::exception,
                                                                   &FlushExCallback::sent));
                cb->check();
                test(!r->isSent());
                test(r->isCompleted());
                test(p->opBatchCount() == 0);
            }

            if(protocol != "bt")
            {
                //
                // Exception with cookie.
                //
                test(p->opBatchCount() == 0);
                Test::TestIntfPrx b1 = Test::TestIntfPrx::uncheckedCast(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                b1->opBatch();
                b1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
                FlushExCallbackPtr cb = new FlushExCallback(cookie);
                b1->ice_getConnection()->begin_flushBatchRequests(Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy),
                    Ice::newCallback_Connection_flushBatchRequests(cb, &FlushExCallback::exceptionWC,
                                                                   &FlushExCallback::sentWC), cookie);
                cb->check();
                test(p->opBatchCount() == 0);
            }
        }
        cout << "ok" << endl;

        cout << "testing batch requests with communicator... " << flush;
        {
            CookiePtr cookie = new Cookie(5);

            {
                //
                // AsyncResult without cookie - 1 connection.
                //
                test(p->opBatchCount() == 0);
                Test::TestIntfPrx b1 = Test::TestIntfPrx::uncheckedCast(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                b1->opBatch();
                b1->opBatch();
                FlushCallbackPtr cb = new FlushCallback();
                Ice::AsyncResultPtr r = communicator->begin_flushBatchRequests(
                    Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy),
                    Ice::newCallback(cb, &FlushCallback::completedAsync, &FlushCallback::sentAsync));
                cb->check();
                test(r->isSent());
                test(r->isCompleted());
                test(p->waitForBatch(2));
            }

            {
                //
                // AsyncResult with cookie - 1 connection.
                //
                test(p->opBatchCount() == 0);
                Test::TestIntfPrx b1 = Test::TestIntfPrx::uncheckedCast(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                b1->opBatch();
                b1->opBatch();
                FlushCallbackPtr cb = new FlushCallback(cookie);
                communicator->begin_flushBatchRequests(Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy),
                    Ice::newCallback(cb, &FlushCallback::completedAsync, &FlushCallback::sentAsync), cookie);
                cb->check();
                test(p->waitForBatch(2));
            }

            if(protocol != "bt")
            {
                //
                // AsyncResult exception without cookie - 1 connection.
                //
                test(p->opBatchCount() == 0);
                Test::TestIntfPrx b1 = Test::TestIntfPrx::uncheckedCast(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                b1->opBatch();
                b1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
                FlushCallbackPtr cb = new FlushCallback();
                Ice::AsyncResultPtr r = communicator->begin_flushBatchRequests(
                    Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy),
                    Ice::newCallback(cb, &FlushCallback::completedAsync, &FlushCallback::sentAsync));
                cb->check();
                test(r->isSent()); // Exceptions are ignored!
                test(r->isCompleted());
                test(p->opBatchCount() == 0);
            }

            if(protocol != "bt")
            {
                //
                // AsyncResult exception with cookie - 1 connection.
                //
                test(p->opBatchCount() == 0);
                Test::TestIntfPrx b1 = Test::TestIntfPrx::uncheckedCast(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                b1->opBatch();
                b1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
                FlushCallbackPtr cb = new FlushCallback(cookie);
                communicator->begin_flushBatchRequests(Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy),
                    Ice::newCallback(cb, &FlushCallback::completedAsync, &FlushCallback::sentAsync), cookie);
                cb->check();
                test(p->opBatchCount() == 0);
            }

            if(protocol != "bt")
            {
                //
                // AsyncResult - 2 connections.
                //
                test(p->opBatchCount() == 0);
                Test::TestIntfPrx b1 = Test::TestIntfPrx::uncheckedCast(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                Test::TestIntfPrx b2 = Test::TestIntfPrx::uncheckedCast(
                    p->ice_connectionId("2")->ice_getConnection()->createProxy(
                        p->ice_getIdentity())->ice_batchOneway());
                b2->ice_getConnection(); // Ensure connection is established.
                b1->opBatch();
                b1->opBatch();
                b2->opBatch();
                b2->opBatch();
                FlushCallbackPtr cb = new FlushCallback();
                Ice::AsyncResultPtr r = communicator->begin_flushBatchRequests(
                    Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy),
                    Ice::newCallback(cb, &FlushCallback::completedAsync, &FlushCallback::sentAsync));
                cb->check();
                test(r->isSent());
                test(r->isCompleted());
                test(p->waitForBatch(4));
            }

            if(protocol != "bt")
            {
                //
                // AsyncResult exception - 2 connections - 1 failure.
                //
                // All connections should be flushed even if there are failures on some connections.
                // Exceptions should not be reported.
                //
                test(p->opBatchCount() == 0);
                Test::TestIntfPrx b1 = Test::TestIntfPrx::uncheckedCast(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                Test::TestIntfPrx b2 = Test::TestIntfPrx::uncheckedCast(
                    p->ice_connectionId("2")->ice_getConnection()->createProxy(
                        p->ice_getIdentity())->ice_batchOneway());

                b2->ice_getConnection(); // Ensure connection is established.
                b1->opBatch();
                b2->opBatch();
                b1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
                FlushCallbackPtr cb = new FlushCallback();
                Ice::AsyncResultPtr r = communicator->begin_flushBatchRequests(
                    Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy),
                    Ice::newCallback(cb, &FlushCallback::completedAsync, &FlushCallback::sentAsync));
                cb->check();
                test(r->isSent()); // Exceptions are ignored!
                test(r->isCompleted());
                test(p->waitForBatch(1));
            }

            if(protocol != "bt")
            {
                //
                // AsyncResult exception - 2 connections - 2 failures.
                //
                // The sent callback should be invoked even if all connections fail.
                //
                test(p->opBatchCount() == 0);
                Test::TestIntfPrx b1 = Test::TestIntfPrx::uncheckedCast(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                Test::TestIntfPrx b2 = Test::TestIntfPrx::uncheckedCast(
                    p->ice_connectionId("2")->ice_getConnection()->createProxy(
                        p->ice_getIdentity())->ice_batchOneway());

                b2->ice_getConnection(); // Ensure connection is established.
                b1->opBatch();
                b2->opBatch();
                b1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
                b2->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
                FlushCallbackPtr cb = new FlushCallback();
                Ice::AsyncResultPtr r = communicator->begin_flushBatchRequests(
                    Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy),
                    Ice::newCallback(cb, &FlushCallback::completedAsync, &FlushCallback::sentAsync));
                cb->check();
                test(r->isSent()); // Exceptions are ignored!
                test(r->isCompleted());
                test(p->opBatchCount() == 0);
            }

            {
                //
                // Without cookie - 1 connection.
                //
                test(p->opBatchCount() == 0);
                Test::TestIntfPrx b1 = Test::TestIntfPrx::uncheckedCast(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                b1->opBatch();
                b1->opBatch();
                FlushCallbackPtr cb = new FlushCallback();
                Ice::AsyncResultPtr r = communicator->begin_flushBatchRequests(
                    Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy),
                    Ice::newCallback_Communicator_flushBatchRequests(cb, &FlushCallback::exception,
                                                                     &FlushCallback::sent));
                cb->check();
                test(r->isSent());
                test(r->isCompleted());
                test(p->waitForBatch(2));
            }

            {
                //
                // With cookie - 1 connection.
                //
                test(p->opBatchCount() == 0);
                Test::TestIntfPrx b1 = Test::TestIntfPrx::uncheckedCast(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                b1->opBatch();
                b1->opBatch();
                FlushCallbackPtr cb = new FlushCallback(cookie);
                communicator->begin_flushBatchRequests(Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy),
                    Ice::newCallback_Communicator_flushBatchRequests(cb, &FlushCallback::exceptionWC,
                                                                     &FlushCallback::sentWC), cookie);
                cb->check();
                test(p->waitForBatch(2));
            }

            if(protocol != "bt")
            {
                //
                // Exception without cookie - 1 connection.
                //
                test(p->opBatchCount() == 0);
                Test::TestIntfPrx b1 = Test::TestIntfPrx::uncheckedCast(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                b1->opBatch();
                b1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
                FlushCallbackPtr cb = new FlushCallback();
                Ice::AsyncResultPtr r = communicator->begin_flushBatchRequests(
                    Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy),
                    Ice::newCallback_Communicator_flushBatchRequests(cb, &FlushCallback::exception,
                                                                     &FlushCallback::sent));
                cb->check();
                test(r->isSent()); // Exceptions are ignored!
                test(r->isCompleted());
                test(p->opBatchCount() == 0);
            }

            if(protocol != "bt")
            {
                //
                // Exception with cookie - 1 connection.
                //
                test(p->opBatchCount() == 0);
                Test::TestIntfPrx b1 = Test::TestIntfPrx::uncheckedCast(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                b1->opBatch();
                b1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
                FlushCallbackPtr cb = new FlushCallback(cookie);
                communicator->begin_flushBatchRequests(Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy),
                    Ice::newCallback_Communicator_flushBatchRequests(cb, &FlushCallback::exceptionWC,
                                                                     &FlushCallback::sentWC), cookie);
                cb->check();
                test(p->opBatchCount() == 0);
            }

            if(protocol != "bt")
            {
                //
                // 2 connections.
                //
                test(p->opBatchCount() == 0);
                Test::TestIntfPrx b1 = Test::TestIntfPrx::uncheckedCast(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                Test::TestIntfPrx b2 = Test::TestIntfPrx::uncheckedCast(
                    p->ice_connectionId("2")->ice_getConnection()->createProxy(
                        p->ice_getIdentity())->ice_batchOneway());

                b2->ice_getConnection(); // Ensure connection is established.
                b1->opBatch();
                b1->opBatch();
                b2->opBatch();
                b2->opBatch();
                FlushCallbackPtr cb = new FlushCallback();
                Ice::AsyncResultPtr r = communicator->begin_flushBatchRequests(
                    Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy),
                    Ice::newCallback_Communicator_flushBatchRequests(cb, &FlushCallback::exception,
                                                                     &FlushCallback::sent));
                cb->check();
                test(r->isSent());
                test(r->isCompleted());
                test(p->waitForBatch(4));
            }

            if(protocol != "bt")
            {
                //
                // Exception - 2 connections - 1 failure.
                //
                // All connections should be flushed even if there are failures on some connections.
                // Exceptions should not be reported.
                //
                test(p->opBatchCount() == 0);
                Test::TestIntfPrx b1 = Test::TestIntfPrx::uncheckedCast(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                Test::TestIntfPrx b2 = Test::TestIntfPrx::uncheckedCast(
                    p->ice_connectionId("2")->ice_getConnection()->createProxy(
                        p->ice_getIdentity())->ice_batchOneway());

                b2->ice_getConnection(); // Ensure connection is established.
                b1->opBatch();
                b2->opBatch();
                b1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
                FlushCallbackPtr cb = new FlushCallback();
                Ice::AsyncResultPtr r = communicator->begin_flushBatchRequests(
                    Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy),
                    Ice::newCallback_Communicator_flushBatchRequests(cb, &FlushCallback::exception,
                                                                     &FlushCallback::sent));
                cb->check();
                test(r->isSent()); // Exceptions are ignored!
                test(r->isCompleted());
                test(p->waitForBatch(1));
            }

            if(protocol != "bt")
            {
                //
                // Exception - 2 connections - 2 failures.
                //
                // The sent callback should be invoked even if all connections fail.
                //
                test(p->opBatchCount() == 0);
                Test::TestIntfPrx b1 = Test::TestIntfPrx::uncheckedCast(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                Test::TestIntfPrx b2 = Test::TestIntfPrx::uncheckedCast(
                    p->ice_connectionId("2")->ice_getConnection()->createProxy(
                        p->ice_getIdentity())->ice_batchOneway());

                b2->ice_getConnection(); // Ensure connection is established.
                b1->opBatch();
                b2->opBatch();
                b1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
                b2->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
                FlushCallbackPtr cb = new FlushCallback();
                Ice::AsyncResultPtr r = communicator->begin_flushBatchRequests(
                    Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy),
                    Ice::newCallback_Communicator_flushBatchRequests(cb, &FlushCallback::exception,
                                                                     &FlushCallback::sent));
                cb->check();
                test(r->isSent()); // Exceptions are ignored!
                test(r->isCompleted());
                test(p->opBatchCount() == 0);
            }
        }
        cout << "ok" << endl;
    }
    cout << "testing AsyncResult operations... " << flush;
    {
        {
            Test::TestIntfPrx indirect = Test::TestIntfPrx::uncheckedCast(p->ice_adapterId("dummy"));
            Ice::AsyncResultPtr r;
            r = indirect->begin_op();
            try
            {
                r->waitForCompleted();
                r->throwLocalException();
                test(false);
            }
            catch(const Ice::NoEndpointException&)
            {
            }

            testController->holdAdapter();
            Ice::AsyncResultPtr r1;
            Ice::AsyncResultPtr r2;
            try
            {
                r1 = p->begin_op();
                Ice::ByteSeq seq;
                seq.resize(1024);
                while((r2 = p->begin_opWithPayload(seq))->sentSynchronously());

                test(r1 == r1);
                test(r1 != r2);

                test(r1->getHash() == r1->getHash());
                test(r1->getHash() != r2->getHash());
                test(r1->getHash() < r2->getHash() || r2->getHash() < r1->getHash());

                if(p->ice_getConnection())
                {
                    test((r1->sentSynchronously() && r1->isSent() && !r1->isCompleted()) ||
                         (!r1->sentSynchronously() && !r1->isCompleted()));

                    test(!r2->sentSynchronously() && !r2->isCompleted());
                }
            }
            catch(...)
            {
                testController->resumeAdapter();
                throw;
            }
            testController->resumeAdapter();

            r1->waitForSent();
            test(r1->isSent());

            r2->waitForSent();
            test(r2->isSent());

            r1->waitForCompleted();
            test(r1->isCompleted());

            r2->waitForCompleted();
            test(r2->isCompleted());

            test(r1->getOperation() == "op");
            test(r2->getOperation() == "opWithPayload");
        }

        {
            Ice::AsyncResultPtr r;

            //
            // Twoway
            //
            r = p->begin_ice_ping();
            test(r->getOperation() == "ice_ping");
            test(!r->getConnection()); // Expected
            test(r->getCommunicator() == communicator);
            test(r->getProxy() == p);
            p->end_ice_ping(r);

            Test::TestIntfPrx p2;

            //
            // Oneway
            //
            p2 = p->ice_oneway();
            r = p2->begin_ice_ping();
            test(r->getOperation() == "ice_ping");
            test(!r->getConnection()); // Expected
            test(r->getCommunicator() == communicator);
            test(r->getProxy() == p2);

            //
            // Batch request via proxy
            //
            p2 = p->ice_batchOneway();
            p2->ice_ping();
            r = p2->begin_ice_flushBatchRequests();
            test(!r->getConnection()); // Expected
            test(r->getCommunicator() == communicator);
            test(r->getProxy() == p2);
            p2->end_ice_flushBatchRequests(r);

            if(p->ice_getConnection())
            {
                //
                // Batch request via connection
                //
                Ice::ConnectionPtr con = p->ice_getConnection();
                p2 = p->ice_batchOneway();
                p2->ice_ping();
                r = con->begin_flushBatchRequests(Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy));
                test(r->getConnection() == con);
                test(r->getCommunicator() == communicator);
                test(!r->getProxy()); // Expected
                con->end_flushBatchRequests(r);

                //
                // Batch request via communicator
                //
                p2 = p->ice_batchOneway();
                p2->ice_ping();
                r = communicator->begin_flushBatchRequests(Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy));
                test(!r->getConnection()); // Expected
                test(r->getCommunicator() == communicator);
                test(!r->getProxy()); // Expected
                communicator->end_flushBatchRequests(r);
            }
        }

        if(p->ice_getConnection())
        {
            testController->holdAdapter();

            Ice::AsyncResultPtr r;
            Ice::ByteSeq seq;
            seq.resize(10024);
            for(int i = 0; i < 200; ++i) // 2MB
            {
                r = p->begin_opWithPayload(seq);
            }

            test(!r->isSent());

            Ice::AsyncResultPtr r1 = p->begin_ice_ping();
            Ice::AsyncResultPtr r2 = p->begin_ice_id();
            r1->cancel();
            r2->cancel();
            try
            {
                p->end_ice_ping(r1);
                test(false);
            }
            catch(const Ice::InvocationCanceledException&)
            {
            }
            try
            {
                p->end_ice_id(r2);
                test(false);
            }
            catch(const Ice::InvocationCanceledException&)
            {
            }

            testController->resumeAdapter();
            p->ice_ping();
            test(!r1->isSent() && r1->isCompleted());
            test(!r2->isSent() && r2->isCompleted());

            testController->holdAdapter();
            r1 = p->begin_op();
            r2 = p->begin_ice_id();
            r1->waitForSent();
            r2->waitForSent();
            r1->cancel();
            r2->cancel();
            try
            {
                p->end_op(r1);
                test(false);
            }
            catch(const Ice::InvocationCanceledException&)
            {
            }
            try
            {
                p->end_ice_id(r2);
                test(false);
            }
            catch(const Ice::InvocationCanceledException&)
            {
            }
            testController->resumeAdapter();
        }
    }
    cout << "ok" << endl;

    if(p->ice_getConnection() && protocol != "bt")
    {
        cout << "testing graceful close connection with wait... " << flush;
        {
            //
            // Local case: begin a request, close the connection gracefully, and make sure it waits
            // for the request to complete.
            //
            Ice::ConnectionPtr con = p->ice_getConnection();
            CloseCallbackPtr cb = new CloseCallback;
            con->setCloseCallback(cb);
            Ice::AsyncResultPtr r = p->begin_sleep(100);
            // Blocks until the request completes.
            con->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
            r->waitForCompleted(); // Should complete successfully.
            try
            {
                r->throwLocalException();
            }
            catch(const Ice::LocalException&)
            {
                test(false);
            }
            cb->check();
        }
        {
            //
            // Remote case.
            //
            Ice::ByteSeq seq;
            seq.resize(1024 * 10);
            for(Ice::ByteSeq::iterator q = seq.begin(); q != seq.end(); ++q)
            {
                *q = static_cast<Ice::Byte>(IceUtilInternal::random(255));
            }

            //
            // Send multiple opWithPayload, followed by a close and followed by multiple opWithPaylod.
            // The goal is to make sure that none of the opWithPayload fail even if the server closes
            // the connection gracefully in between.
            //
            int maxQueue = 2;
            bool done = false;
            while(!done && maxQueue < 5000)
            {
                done = true;
                p->ice_ping();
                vector<Ice::AsyncResultPtr> results;
                for(int i = 0; i < maxQueue; ++i)
                {
                    results.push_back(p->begin_opWithPayload(seq));
                }
                if(!p->begin_close(Test::GracefullyWithWait)->isSent())
                {
                    for(int i = 0; i < maxQueue; i++)
                    {
                        Ice::AsyncResultPtr r = p->begin_opWithPayload(seq);
                        results.push_back(r);
                        if(r->isSent())
                        {
                            done = false;
                            maxQueue *= 2;
                            break;
                        }
                    }
                }
                else
                {
                    maxQueue *= 2;
                    done = false;
                }

                for(vector<Ice::AsyncResultPtr>::const_iterator r = results.begin(); r != results.end(); ++r)
                {
                    (*r)->waitForCompleted();
                    try
                    {
                        (*r)->throwLocalException();
                    }
                    catch(const Ice::LocalException&)
                    {
                        test(false);
                    }
                }
            }
        }
        cout << "ok" << endl;

        cout << "testing graceful close connection without wait... " << flush;
        {
            //
            // Local case: start an operation and then close the connection gracefully on the client side
            // without waiting for the pending invocation to complete. There will be no retry and we expect the
            // invocation to fail with ConnectionManuallyClosedException.
            //
            p = p->ice_connectionId("CloseGracefully"); // Start with a new connection.
            Ice::ConnectionPtr con = p->ice_getConnection();
            Ice::AsyncResultPtr r = p->begin_startDispatch();
            r->waitForSent(); // Ensure the request was sent before we close the connection.
            con->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, Gracefully));
            r->waitForCompleted();
            try
            {
                r->throwLocalException();
                test(false);
            }
            catch(const Ice::ConnectionManuallyClosedException& ex)
            {
                test(ex.graceful);
            }
            p->finishDispatch();

            //
            // Remote case: the server closes the connection gracefully, which means the connection
            // will not be closed until all pending dispatched requests have completed.
            //
            con = p->ice_getConnection();
            CloseCallbackPtr cb = new CloseCallback;
            con->setCloseCallback(cb);
            r = p->begin_sleep(100);
            p->close(Test::Gracefully); // Close is delayed until sleep completes.
            cb->check(); // Ensure connection was closed.
            r->waitForCompleted();
            try
            {
                r->throwLocalException();
            }
            catch(const Ice::LocalException&)
            {
                test(false);
            }
        }
        cout << "ok" << endl;

        cout << "testing forceful close connection... " << flush;
        {
            //
            // Local case: start an operation and then close the connection forcefully on the client side.
            // There will be no retry and we expect the invocation to fail with ConnectionManuallyClosedException.
            //
            p->ice_ping();
            Ice::ConnectionPtr con = p->ice_getConnection();
            Ice::AsyncResultPtr r = p->begin_startDispatch();
            r->waitForSent(); // Ensure the request was sent before we close the connection.
            con->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, Forcefully));
            r->waitForCompleted();
            try
            {
                r->throwLocalException();
                test(false);
            }
            catch(const Ice::ConnectionManuallyClosedException& ex)
            {
                test(!ex.graceful);
            }
            p->finishDispatch();

            //
            // Remote case: the server closes the connection forcefully. This causes the request to fail
            // with a ConnectionLostException. Since the close() operation is not idempotent, the client
            // will not retry.
            //
            try
            {
                p->close(Test::Forcefully);
                test(false);
            }
            catch(const Ice::ConnectionLostException&)
            {
                // Expected.
            }
        }
        cout << "ok" << endl;
    }

    p->shutdown();
#endif
}
