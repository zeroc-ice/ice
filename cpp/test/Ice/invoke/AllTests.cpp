// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/Stream.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;

static string testString = "This is a test string";

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
            Ice::InputStreamPtr in = Ice::createInputStream(_communicator, outParams);
            in->startEncapsulation();
            string s;
            in->read(s);
            test(s == cmp);
            in->read(s);
            test(s == cmp);
            in->endEncapsulation();
            called();
        }
        else
        {
            test(false);
        };
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
            Ice::InputStreamPtr in = Ice::createInputStream(_communicator, outParams);
            in->startEncapsulation();
            try
            {
                in->throwException();
            }
            catch(const Test::MyException&)
            {
                in->endEncapsulation();
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
            Ice::InputStreamPtr in = Ice::createInputStream(_communicator, outParams);
            in->startEncapsulation();
            string s;
            in->read(s);
            test(s == testString);
            in->read(s);
            test(s == testString);
            in->endEncapsulation();
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
            Ice::InputStreamPtr in = Ice::createInputStream(_communicator, outParams);
            in->startEncapsulation();
            string s;
            in->read(s);
            test(s == testString);
            in->read(s);
            test(s == testString);
            in->endEncapsulation();
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
            Ice::InputStreamPtr in = Ice::createInputStream(_communicator, outParams);
            in->startEncapsulation();
            string s;
            in->read(s);
            test(s == cookie->getString());
            in->read(s);
            test(s == cookie->getString());
            in->endEncapsulation();
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
            Ice::InputStreamPtr in = Ice::createInputStream(_communicator, outParams);
            in->startEncapsulation();
            try
            {
                in->throwException();
            }
            catch(const Test::MyException&)
            {
                in->endEncapsulation();
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
            Ice::InputStreamPtr in = Ice::createInputStream(_communicator, outParams);
            in->startEncapsulation();
            try
            {
                in->throwException();
            }
            catch(const Test::MyException&)
            {
                in->endEncapsulation();
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
            Ice::InputStreamPtr in = Ice::createInputStream(_communicator, outParams);
            in->startEncapsulation();
            try
            {
                in->throwException();
            }
            catch(const Test::MyException&)
            {
                in->endEncapsulation();
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

Test::MyClassPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    string ref = "test:default -p 12010";
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);

    Test::MyClassPrx cl = Test::MyClassPrx::checkedCast(base);
    test(cl);

    Test::MyClassPrx oneway = cl->ice_oneway();
    Test::MyClassPrx batchOneway = cl->ice_batchOneway();

    void (Callback::*nullEx)(const Ice::Exception&) = 0;
    void (Callback::*nullExWC)(const Ice::Exception&, const CookiePtr&) = 0;

    cout << "testing ice_invoke... " << flush;

    {
        Ice::ByteSeq inEncaps, outEncaps;
        if(!oneway->ice_invoke("opOneway", Ice::Normal, inEncaps, outEncaps))
        {
            test(false);
        }

        test(batchOneway->ice_invoke("opOneway", Ice::Normal, inEncaps, outEncaps));
        test(batchOneway->ice_invoke("opOneway", Ice::Normal, inEncaps, outEncaps));
        test(batchOneway->ice_invoke("opOneway", Ice::Normal, inEncaps, outEncaps));
        test(batchOneway->ice_invoke("opOneway", Ice::Normal, inEncaps, outEncaps));
        batchOneway->ice_flushBatchRequests();

        Ice::OutputStreamPtr out = Ice::createOutputStream(communicator);
        out->startEncapsulation();
        out->write(testString);
        out->endEncapsulation();
        out->finished(inEncaps);

        // ice_invoke
        if(cl->ice_invoke("opString", Ice::Normal, inEncaps, outEncaps))
        {
            Ice::InputStreamPtr in = Ice::createInputStream(communicator, outEncaps);
            in->startEncapsulation();
            string s;
            in->read(s);
            test(s == testString);
            in->read(s);
            test(s == testString);
            in->endEncapsulation();
        }
        else
        {
            test(false);
        }

        // ice_invoke with array mapping
        pair<const ::Ice::Byte*, const ::Ice::Byte*> inPair(&inEncaps[0], &inEncaps[0] + inEncaps.size());
        if(cl->ice_invoke("opString", Ice::Normal, inPair, outEncaps))
        {
            Ice::InputStreamPtr in = Ice::createInputStream(communicator, outEncaps);
            in->startEncapsulation();
            string s;
            in->read(s);
            test(s == testString);
            in->read(s);
            test(s == testString);
            in->endEncapsulation();
        }
        else
        {
            test(false);
        }
    }

    {
        Ice::ByteSeq inEncaps, outEncaps;
        if(cl->ice_invoke("opException", Ice::Normal, inEncaps, outEncaps))
        {
            test(false);
        }
        else
        {
            Ice::InputStreamPtr in = Ice::createInputStream(communicator, outEncaps);
            in->startEncapsulation();
            try
            {
                in->throwException();
            }
            catch(const Test::MyException&)
            {
            }
            catch(...)
            {
                test(false);
            }
            in->endEncapsulation();
        }
    }

    cout << "ok" << endl;

    cout << "testing asynchronous ice_invoke... " << flush;

    {
        CookiePtr cookie = new Cookie();

        Ice::ByteSeq inEncaps, outEncaps;
        Ice::AsyncResultPtr result = oneway->begin_ice_invoke("opOneway", Ice::Normal, inEncaps);
        if(!oneway->end_ice_invoke(outEncaps, result))
        {
            test(false);
        }
        
        Ice::OutputStreamPtr out = Ice::createOutputStream(communicator);
        out->startEncapsulation();
        out->write(testString);
        out->endEncapsulation();
        out->finished(inEncaps);

        // begin_ice_invoke with no callback
        result = cl->begin_ice_invoke("opString", Ice::Normal, inEncaps);
        if(cl->end_ice_invoke(outEncaps, result))
        {
            Ice::InputStreamPtr in = Ice::createInputStream(communicator, outEncaps);
            in->startEncapsulation();
            string s;
            in->read(s);
            test(s == testString);
            in->read(s);
            test(s == testString);
            in->endEncapsulation();
        }
        else
        {
            test(false);
        };

        // begin_ice_invoke with no callback and array mapping
        pair<const ::Ice::Byte*, const ::Ice::Byte*> inPair(&inEncaps[0], &inEncaps[0] + inEncaps.size());
        result = cl->begin_ice_invoke("opString", Ice::Normal, inPair);
        if(cl->end_ice_invoke(outEncaps, result))
        {
            Ice::InputStreamPtr in = Ice::createInputStream(communicator, outEncaps);
            in->startEncapsulation();
            string s;
            in->read(s);
            test(s == testString);
            in->read(s);
            test(s == testString);
            in->endEncapsulation();
        }
        else
        {
            test(false);
        };

        // begin_ice_invoke with Callback
        CallbackPtr cb = new Callback(communicator, false);
        cl->begin_ice_invoke("opString", Ice::Normal, inEncaps, Ice::newCallback(cb, &Callback::opString));
        cb->check();

        // begin_ice_invoke with Callback and Cookie
        cb = new Callback(communicator, true);
        cl->begin_ice_invoke("opString", Ice::Normal, inEncaps, Ice::newCallback(cb, &Callback::opString), cookie);
        cb->check();

        // begin_ice_invoke with Callback_Object_ice_invoke 
        cb = new Callback(communicator, false);
        Ice::Callback_Object_ice_invokePtr d = Ice::newCallback_Object_ice_invoke(cb, &Callback::opStringNC, nullEx);
        cl->begin_ice_invoke("opString", Ice::Normal, inEncaps, d);
        cb->check();

        // begin_ice_invoke with Callback_Object_ice_invoke with Cookie
        cb = new Callback(communicator, false);
        d = Ice::newCallback_Object_ice_invoke(cb, &Callback::opStringWC, nullExWC);
        cl->begin_ice_invoke("opString", Ice::Normal, inEncaps, d, cookie);
        cb->check();

        // begin_ice_invoke with Callback_Object_ice_invoke and array mapping
        cb = new Callback(communicator, false);
        d = Ice::newCallback_Object_ice_invoke(cb, &Callback::opStringNC, nullEx);
        cl->begin_ice_invoke("opString", Ice::Normal, inPair, d);
        cb->check();

        // begin_ice_invoke with Callback_Object_ice_invoke and array mapping with Cookie
        cb = new Callback(communicator, false);
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
            Ice::InputStreamPtr in = Ice::createInputStream(communicator, outEncaps);
            in->startEncapsulation();
            try
            {
                in->throwException();
            }
            catch(const Test::MyException&)
            {
            }
            catch(...)
            {
                test(false);
            }
            in->endEncapsulation();
        }

        // begin_ice_invoke with Callback
        CallbackPtr cb = new Callback(communicator, false);
        cl->begin_ice_invoke("opException", Ice::Normal, inEncaps, Ice::newCallback(cb, &Callback::opException));
        cb->check();

        // begin_ice_invoke with Callback and Cookie
        cb = new Callback(communicator, true);
        cl->begin_ice_invoke("opException", Ice::Normal, inEncaps, Ice::newCallback(cb, &Callback::opException),
                             cookie);
        cb->check();

        // begin_ice_invoke with Callback_Object_ice_invoke
        cb = new Callback(communicator, false);
        Ice::Callback_Object_ice_invokePtr d = Ice::newCallback_Object_ice_invoke(cb, &Callback::opExceptionNC, nullEx);
        cl->begin_ice_invoke("opException", Ice::Normal, inEncaps, d);
        cb->check();

        // begin_ice_invoke with Callback_Object_ice_invoke with Cookie
        cb = new Callback(communicator, false);
        d = Ice::newCallback_Object_ice_invoke(cb, &Callback::opExceptionWC, nullExWC);
        cl->begin_ice_invoke("opException", Ice::Normal, inEncaps, d, cookie);
        cb->check();
    }

    cout << "ok" << endl;
#ifdef ICE_CPP11
        cout << "testing asynchronous ice_invoke with C++11 style callbacks... " << flush;

    {
        Ice::ByteSeq inEncaps, outEncaps;
        Ice::OutputStreamPtr out = Ice::createOutputStream(communicator);
        out->startEncapsulation();
        out->write(testString);
        out->endEncapsulation();
        out->finished(inEncaps);

        CallbackPtr cb = new Callback(communicator, false);
        Ice::Context ctx;
        cl->begin_ice_invoke("opString", Ice::Normal, inEncaps, ctx,
                             [=](bool ret, const vector< ::Ice::Byte>& args)
                                {
                                    cb->opStringNC(ret, args);
                                });
        cb->check();

        cb = new Callback(communicator, false);
        pair<const ::Ice::Byte*, const ::Ice::Byte*> inPair(&inEncaps[0], &inEncaps[0] + inEncaps.size());
        cl->begin_ice_invoke("opString", Ice::Normal, inPair, 
                             [=](bool ret, const pair<const Ice::Byte*, const Ice::Byte*>& args)
                                {
                                    cb->opStringPairNC(ret, args);
                                });
        cb->check();
    }

    {
        Ice::ByteSeq inEncaps, outEncaps;

        CallbackPtr cb = new Callback(communicator, false);
        cl->begin_ice_invoke("opException", Ice::Normal, inEncaps,
                             [=](bool ret, const vector< ::Ice::Byte>& args)
                                {
                                    cb->opExceptionNC(ret, args);
                                }
        );
        cb->check();

        pair<const ::Ice::Byte*, const ::Ice::Byte*> inPair(static_cast< ::Ice::Byte*>(0), static_cast< ::Ice::Byte*>(0));
        cb = new Callback(communicator, false);
        cl->begin_ice_invoke("opException", Ice::Normal, inPair,
                             [=](bool ret, const pair<const Ice::Byte*, const Ice::Byte*>& args)
                                {
                                    cb->opExceptionPairNC(ret, args);
                                });
        cb->check();
    }

    cout << "ok" << endl;
#endif
    return cl;
}
