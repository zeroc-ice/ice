// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Ice;

static string testString = "This is a test string";

#ifndef ICE_CPP11_MAPPING
class Cookie : public Ice::LocalObject
{
public:

    string getString()
    {
        return testString;
    }
};
typedef IceUtil::Handle<Cookie> CookiePtr;

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

    Callback(const Ice::CommunicatorPtr& communicator, bool useCookie) :
        _communicator(communicator),
        _useCookie(useCookie)
    {
    }

    void opString(const Ice::AsyncResultPtr& result)
    {
        string cmp = testString;
        if(_useCookie)
        {
            CookiePtr cookie = CookiePtr::dynamicCast(result->getCookie());
            cmp = cookie->getString();
        }

        Ice::ByteSeq outParams;
        if(result->getProxy()->end_ice_invoke(outParams, result))
        {
            Ice::InputStream in(_communicator, result->getProxy()->ice_getEncodingVersion(), outParams);
            in.startEncapsulation();
            string s;
            in.read(s);
            test(s == cmp);
            in.read(s);
            test(s == cmp);
            in.endEncapsulation();
            called();
        }
        else
        {
            test(false);
        }
    }

    void opException(const Ice::AsyncResultPtr& result)
    {
        if(_useCookie)
        {
            CookiePtr cookie = CookiePtr::dynamicCast(result->getCookie());
            test(cookie->getString() == testString);
        }

        Ice::ByteSeq outParams;
        if(result->getProxy()->end_ice_invoke(outParams, result))
        {
            test(false);
        }
        else
        {
            Ice::InputStream in(_communicator, result->getProxy()->ice_getEncodingVersion(), outParams);
            in.startEncapsulation();
            try
            {
                in.throwException();
            }
            catch(const Test::MyException&)
            {
                in.endEncapsulation();
                called();
            }
            catch(...)
            {
                test(false);
            }
        }
    }

    void opStringNC(bool ok, const Ice::ByteSeq& outParams)
    {
        if(ok)
        {
            Ice::InputStream in(_communicator, outParams);
            in.startEncapsulation();
            string s;
            in.read(s);
            test(s == testString);
            in.read(s);
            test(s == testString);
            in.endEncapsulation();
            called();
        }
        else
        {
            test(false);
        }
    }

    void opStringPairNC(bool ok, const pair<const Ice::Byte*, const Ice::Byte*>& outParams)
    {
        if(ok)
        {
            Ice::InputStream in(_communicator, outParams);
            in.startEncapsulation();
            string s;
            in.read(s);
            test(s == testString);
            in.read(s);
            test(s == testString);
            in.endEncapsulation();
            called();
        }
        else
        {
            test(false);
        }
    }

    void opStringWC(bool ok, const Ice::ByteSeq& outParams, const CookiePtr& cookie)
    {
        if(ok)
        {
            Ice::InputStream in(_communicator, outParams);
            in.startEncapsulation();
            string s;
            in.read(s);
            test(s == cookie->getString());
            in.read(s);
            test(s == cookie->getString());
            in.endEncapsulation();
            called();
        }
        else
        {
            test(false);
        }
    }

    void opExceptionNC(bool ok, const Ice::ByteSeq& outParams)
    {
        if(ok)
        {
            test(false);
        }
        else
        {
            Ice::InputStream in(_communicator, outParams);
            in.startEncapsulation();
            try
            {
                in.throwException();
            }
            catch(const Test::MyException&)
            {
                in.endEncapsulation();
                called();
            }
            catch(...)
            {
                test(false);
            }
        }
    }

    void opExceptionPairNC(bool ok, const pair<const Ice::Byte*, const Ice::Byte*>& outParams)
    {
        if(ok)
        {
            test(false);
        }
        else
        {
            Ice::InputStream in(_communicator, outParams);
            in.startEncapsulation();
            try
            {
                in.throwException();
            }
            catch(const Test::MyException&)
            {
                in.endEncapsulation();
                called();
            }
            catch(...)
            {
                test(false);
            }
        }
    }

    void opExceptionWC(bool ok, const Ice::ByteSeq& outParams, const CookiePtr& cookie)
    {
        test(cookie->getString() == testString);
        if(ok)
        {
            test(false);
        }
        else
        {
            Ice::InputStream in(_communicator, outParams);
            in.startEncapsulation();
            try
            {
                in.throwException();
            }
            catch(const Test::MyException&)
            {
                in.endEncapsulation();
                called();
            }
            catch(...)
            {
                test(false);
            }
        }
    }

private:

    Ice::CommunicatorPtr _communicator;
    bool _useCookie;
};
typedef IceUtil::Handle<Callback> CallbackPtr;
#endif

Test::MyClassPrxPtr
allTests(const Ice::CommunicatorPtr& communicator)
{
    string ref = "test:" + getTestEndpoint(communicator, 0);
    Ice::ObjectPrxPtr base = communicator->stringToProxy(ref);
    test(base);

    Test::MyClassPrxPtr cl = ICE_CHECKED_CAST(Test::MyClassPrx, base);
    test(cl);

    Test::MyClassPrxPtr oneway = cl->ice_oneway();
    Test::MyClassPrxPtr batchOneway = cl->ice_batchOneway();

    cout << "testing ice_invoke... " << flush;

    {
        Ice::ByteSeq inEncaps, outEncaps;
        if(!oneway->ice_invoke("opOneway", ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps))
        {
            test(false);
        }

        test(batchOneway->ice_invoke("opOneway", ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps));
        test(batchOneway->ice_invoke("opOneway", ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps));
        test(batchOneway->ice_invoke("opOneway", ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps));
        test(batchOneway->ice_invoke("opOneway", ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps));
        batchOneway->ice_flushBatchRequests();

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(testString);
        out.endEncapsulation();
        out.finished(inEncaps);

        // ice_invoke
        if(cl->ice_invoke("opString", ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps))
        {
            Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
            in.startEncapsulation();
            string s;
            in.read(s);
            test(s == testString);
            in.read(s);
            test(s == testString);
            in.endEncapsulation();
        }
        else
        {
            test(false);
        }

        // ice_invoke with array mapping
        pair<const ::Ice::Byte*, const ::Ice::Byte*> inPair(&inEncaps[0], &inEncaps[0] + inEncaps.size());
        if(cl->ice_invoke("opString", ICE_ENUM(OperationMode, Normal), inPair, outEncaps))
        {
            Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
            in.startEncapsulation();
            string s;
            in.read(s);
            test(s == testString);
            in.read(s);
            test(s == testString);
            in.endEncapsulation();
        }
        else
        {
            test(false);
        }
    }

    {
        Ice::ByteSeq inEncaps, outEncaps;
        if(cl->ice_invoke("opException", ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps))
        {
            test(false);
        }
        else
        {
            Ice::InputStream in(communicator, cl->ice_getEncodingVersion(), outEncaps);
            in.startEncapsulation();
            try
            {
                in.throwException();
            }
            catch(const Test::MyException&)
            {
            }
            catch(...)
            {
                test(false);
            }
            in.endEncapsulation();
        }
    }

    cout << "ok" << endl;

    cout << "testing asynchronous ice_invoke... " << flush;
#ifdef ICE_CPP11_MAPPING

    {
        Ice::ByteSeq inEncaps;
        batchOneway->ice_invokeAsync("opOneway", ICE_ENUM(OperationMode, Normal), inEncaps,
            [](bool, const vector<Ice::Byte>)
            {
                test(false);
            },
            [](exception_ptr)
            {
                test(false);
            },
            [](bool)
            {
                test(false);
            });
        batchOneway->ice_flushBatchRequests();
    }
    //
    // repeat with the future API.
    //
    {
        Ice::ByteSeq inEncaps;
        test(batchOneway->ice_invokeAsync("opOneway", ICE_ENUM(OperationMode, Normal), inEncaps).get().returnValue);
        test(batchOneway->ice_invokeAsync("opOneway", ICE_ENUM(OperationMode, Normal), inEncaps).get().returnValue);
        test(batchOneway->ice_invokeAsync("opOneway", ICE_ENUM(OperationMode, Normal), inEncaps).get().returnValue);
        test(batchOneway->ice_invokeAsync("opOneway", ICE_ENUM(OperationMode, Normal), inEncaps).get().returnValue);
        batchOneway->ice_flushBatchRequests();
    }

    {
        promise<bool> completed;
        Ice::ByteSeq inEncaps, outEncaps;
        oneway->ice_invokeAsync(
            "opOneway",
            OperationMode::Normal,
            inEncaps,
            nullptr,
            [&](exception_ptr ex)
            {
                completed.set_exception(ex);
            },
            [&](bool)
            {
                completed.set_value(true);
            });

        test(completed.get_future().get());
    }

    //
    // repeat with the future API.
    //

    {
        Ice::ByteSeq inEncaps, outEncaps;
        auto completed = oneway->ice_invokeAsync("opOneway", OperationMode::Normal, inEncaps);
        test(completed.get().returnValue);
    }

    {
        promise<bool> completed;
        Ice::ByteSeq inEncaps, outEncaps;
        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(testString);
        out.endEncapsulation();
        out.finished(inEncaps);

        cl->ice_invokeAsync("opString", OperationMode::Normal, inEncaps,
            [&](bool ok, vector<Ice::Byte> outParams)
            {
                outEncaps = move(outParams);
                completed.set_value(ok);
            },
            [&](exception_ptr ex)
            {
                completed.set_exception(ex);
            });
        test(completed.get_future().get());

        Ice::InputStream in(communicator, outEncaps);
        in.startEncapsulation();
        string s;
        in.read(s);
        test(s == testString);
        in.read(s);
        test(s == testString);
        in.endEncapsulation();
    }
    //
    // repeat with the future API.
    //
    {
        Ice::ByteSeq inEncaps, outEncaps;
        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(testString);
        out.endEncapsulation();
        out.finished(inEncaps);

        auto result = cl->ice_invokeAsync("opString", OperationMode::Normal, inEncaps).get();
        test(result.returnValue);

        Ice::InputStream in(communicator, result.outParams);
        in.startEncapsulation();
        string s;
        in.read(s);
        test(s == testString);
        in.read(s);
        test(s == testString);
        in.endEncapsulation();
    }


    {
        promise<bool> completed;
        promise<void> sent;
        Ice::ByteSeq inEncaps, outEncaps;
        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(testString);
        out.endEncapsulation();
        out.finished(inEncaps);

        auto inPair = make_pair(inEncaps.data(), inEncaps.data() + inEncaps.size());

        cl->ice_invokeAsync("opString", OperationMode::Normal, inPair,
            [&](bool ok, pair<const Ice::Byte*, const Ice::Byte*> outParams)
            {
                vector<Ice::Byte>(outParams.first, outParams.second).swap(outEncaps);
                completed.set_value(ok);
            },
            [&](exception_ptr ex)
            {
                completed.set_exception(ex);
            },
            [&](bool)
            {
                sent.set_value();
            });
        sent.get_future().get(); // Ensure sent callback was called
        test(completed.get_future().get());

        Ice::InputStream in(communicator, outEncaps);
        in.startEncapsulation();
        string s;
        in.read(s);
        test(s == testString);
        in.read(s);
        test(s == testString);
        in.endEncapsulation();
    }
    //
    // repeat with the future API.
    //

    {
        Ice::ByteSeq inEncaps, outEncaps;
        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(testString);
        out.endEncapsulation();
        out.finished(inEncaps);

        auto inPair = make_pair(inEncaps.data(), inEncaps.data() + inEncaps.size());

        auto result = cl->ice_invokeAsync("opString", OperationMode::Normal, inPair).get();
        test(result.returnValue);

        Ice::InputStream in(communicator, result.outParams);
        in.startEncapsulation();
        string s;
        in.read(s);
        test(s == testString);
        in.read(s);
        test(s == testString);
        in.endEncapsulation();
    }

    {
        promise<bool> completed;
        promise<void> sent;
        Ice::ByteSeq inEncaps, outEncaps;

        cl->ice_invokeAsync("opException", OperationMode::Normal, inEncaps,
            [&](bool ok, vector<Ice::Byte> outParams)
            {
                outEncaps = move(outParams);
                completed.set_value(ok);
            },
            [&](exception_ptr ex)
            {
                completed.set_exception(ex);
            },
            [&](bool)
            {
                sent.set_value();
            });
        sent.get_future().get(); // Ensure sent callback was called
        test(!completed.get_future().get());

        Ice::InputStream in(communicator, outEncaps);
        in.startEncapsulation();
        try
        {
            in.throwException();
            test(false);
        }
        catch(const Test::MyException&)
        {
        }
        catch(...)
        {
            test(false);
        }
    }
    //
    // repeat with the future API.
    //
    {
        Ice::ByteSeq inEncaps;
        auto result = cl->ice_invokeAsync("opException", OperationMode::Normal, inEncaps).get();
        test(!result.returnValue);

        Ice::InputStream in(communicator, result.outParams);
        in.startEncapsulation();
        try
        {
            in.throwException();
            test(false);
        }
        catch(const Test::MyException&)
        {
        }
        catch(...)
        {
            test(false);
        }
    }

#else
    void (::Callback::*nullEx)(const Ice::Exception&) = 0;
    void (::Callback::*nullExWC)(const Ice::Exception&, const CookiePtr&) = 0;

    {
        Ice::ByteSeq inEncaps, outEncaps;
        test(batchOneway->end_ice_invoke(outEncaps, batchOneway->begin_ice_invoke("opOneway", ICE_ENUM(OperationMode, Normal), inEncaps)));
        test(batchOneway->end_ice_invoke(outEncaps, batchOneway->begin_ice_invoke("opOneway", ICE_ENUM(OperationMode, Normal), inEncaps)));
        test(batchOneway->end_ice_invoke(outEncaps, batchOneway->begin_ice_invoke("opOneway", ICE_ENUM(OperationMode, Normal), inEncaps)));
        test(batchOneway->end_ice_invoke(outEncaps, batchOneway->begin_ice_invoke("opOneway", ICE_ENUM(OperationMode, Normal), inEncaps)));
        batchOneway->ice_flushBatchRequests();
    }

    {
        CookiePtr cookie = new Cookie();

        Ice::ByteSeq inEncaps, outEncaps;
        Ice::AsyncResultPtr result = oneway->begin_ice_invoke("opOneway", Ice::Normal, inEncaps);
        if(!oneway->end_ice_invoke(outEncaps, result))
        {
            test(false);
        }

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(testString);
        out.endEncapsulation();
        out.finished(inEncaps);

        // begin_ice_invoke with no callback
        result = cl->begin_ice_invoke("opString", Ice::Normal, inEncaps);
        if(cl->end_ice_invoke(outEncaps, result))
        {
            Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
            in.startEncapsulation();
            string s;
            in.read(s);
            test(s == testString);
            in.read(s);
            test(s == testString);
            in.endEncapsulation();
        }
        else
        {
            test(false);
        }

        // begin_ice_invoke with no callback and array mapping
        pair<const ::Ice::Byte*, const ::Ice::Byte*> inPair(&inEncaps[0], &inEncaps[0] + inEncaps.size());
        result = cl->begin_ice_invoke("opString", Ice::Normal, inPair);
        if(cl->end_ice_invoke(outEncaps, result))
        {
            Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
            in.startEncapsulation();
            string s;
            in.read(s);
            test(s == testString);
            in.read(s);
            test(s == testString);
            in.endEncapsulation();
        }
        else
        {
            test(false);
        }

        // begin_ice_invoke with Callback
        ::CallbackPtr cb = new ::Callback(communicator, false);
        cl->begin_ice_invoke("opString", Ice::Normal, inEncaps, Ice::newCallback(cb, &Callback::opString));
        cb->check();

        // begin_ice_invoke with Callback and Cookie
        cb = new ::Callback(communicator, true);
        cl->begin_ice_invoke("opString", Ice::Normal, inEncaps, Ice::newCallback(cb, &Callback::opString), cookie);
        cb->check();

        // begin_ice_invoke with Callback_Object_ice_invoke
        cb = new ::Callback(communicator, false);
        Ice::Callback_Object_ice_invokePtr d = Ice::newCallback_Object_ice_invoke(cb, &Callback::opStringNC, nullEx);
        cl->begin_ice_invoke("opString", Ice::Normal, inEncaps, d);
        cb->check();

        // begin_ice_invoke with Callback_Object_ice_invoke with Cookie
        cb = new ::Callback(communicator, false);
        d = Ice::newCallback_Object_ice_invoke(cb, &Callback::opStringWC, nullExWC);
        cl->begin_ice_invoke("opString", Ice::Normal, inEncaps, d, cookie);
        cb->check();

        // begin_ice_invoke with Callback_Object_ice_invoke and array mapping
        cb = new ::Callback(communicator, false);
        d = Ice::newCallback_Object_ice_invoke(cb, &Callback::opStringNC, nullEx);
        cl->begin_ice_invoke("opString", Ice::Normal, inPair, d);
        cb->check();

        // begin_ice_invoke with Callback_Object_ice_invoke and array mapping with Cookie
        cb = new ::Callback(communicator, false);
        d = Ice::newCallback_Object_ice_invoke(cb, &Callback::opStringWC, nullExWC);
        cl->begin_ice_invoke("opString", Ice::Normal, inPair, d, cookie);
        cb->check();
    }

    {
        CookiePtr cookie = new Cookie();
        Ice::ByteSeq inEncaps, outEncaps;

        // begin_ice_invoke with no callback
        Ice::AsyncResultPtr result = cl->begin_ice_invoke("opException", Ice::Normal, inEncaps);
        if(cl->end_ice_invoke(outEncaps, result))
        {
            test(false);
        }
        else
        {
            Ice::InputStream in(communicator, cl->ice_getEncodingVersion(), outEncaps);
            in.startEncapsulation();
            try
            {
                in.throwException();
            }
            catch(const Test::MyException&)
            {
            }
            catch(...)
            {
                test(false);
            }
            in.endEncapsulation();
        }

        // begin_ice_invoke with Callback
        ::CallbackPtr cb = new ::Callback(communicator, false);
        cl->begin_ice_invoke("opException", Ice::Normal, inEncaps, Ice::newCallback(cb, &Callback::opException));
        cb->check();

        // begin_ice_invoke with Callback and Cookie
        cb = new ::Callback(communicator, true);
        cl->begin_ice_invoke("opException", Ice::Normal, inEncaps, Ice::newCallback(cb, &Callback::opException),
                             cookie);
        cb->check();

        // begin_ice_invoke with Callback_Object_ice_invoke
        cb = new ::Callback(communicator, false);
        Ice::Callback_Object_ice_invokePtr d = Ice::newCallback_Object_ice_invoke(cb, &Callback::opExceptionNC, nullEx);
        cl->begin_ice_invoke("opException", Ice::Normal, inEncaps, d);
        cb->check();

        // begin_ice_invoke with Callback_Object_ice_invoke with Cookie
        cb = new ::Callback(communicator, false);
        d = Ice::newCallback_Object_ice_invoke(cb, &Callback::opExceptionWC, nullExWC);
        cl->begin_ice_invoke("opException", Ice::Normal, inEncaps, d, cookie);
        cb->check();
    }
#endif
    cout << "ok" << endl;
    return cl;
}
