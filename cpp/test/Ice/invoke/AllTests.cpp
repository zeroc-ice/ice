// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
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
            string s;
            in->read(s);
            test(s == cmp);
            in->read(s);
            test(s == cmp);
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
            try
            {
                in->throwException();
            }
            catch(const Test::MyException&)
            {
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
            string s;
            in->read(s);
            test(s == testString);
            in->read(s);
            test(s == testString);
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
            string s;
            in->read(s);
            test(s == cookie->getString());
            in->read(s);
            test(s == cookie->getString());
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
            try
            {
                in->throwException();
            }
            catch(const Test::MyException&)
            {
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
            try
            {
                in->throwException();
            }
            catch(const Test::MyException&)
            {
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

    void (Callback::*nullEx)(const Ice::Exception&) = 0;
    void (Callback::*nullExWC)(const Ice::Exception&, const CookiePtr&) = 0;
    
    cout << "testing ice_invoke... " << flush;

    {
        Ice::ByteSeq inParams, outParams;
        if(!oneway->ice_invoke("opOneway", Ice::Normal, inParams, outParams))
        {
            test(false);
        }

        Ice::OutputStreamPtr out = Ice::createOutputStream(communicator);
        out->write(testString);
        out->finished(inParams);

        // ice_invoke
        if(cl->ice_invoke("opString", Ice::Normal, inParams, outParams))
        {
            Ice::InputStreamPtr in = Ice::createInputStream(communicator, outParams);
            string s;
            in->read(s);
            test(s == testString);
            in->read(s);
            test(s == testString);
        }
        else
        {
            test(false);
        }

        // ice_invoke with array mapping
        pair<const ::Ice::Byte*, const ::Ice::Byte*> inPair(&inParams[0], &inParams[0] + inParams.size());
        if(cl->ice_invoke("opString", Ice::Normal, inPair, outParams))
        {
            Ice::InputStreamPtr in = Ice::createInputStream(communicator, outParams);
            string s;
            in->read(s);
            test(s == testString);
            in->read(s);
            test(s == testString);
        }
        else
        {
            test(false);
        }
    }

    {
        Ice::ByteSeq inParams, outParams;
        if(cl->ice_invoke("opException", Ice::Normal, inParams, outParams))
        {
            test(false);
        }
        else
        {
            Ice::InputStreamPtr in = Ice::createInputStream(communicator, outParams);
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
        }
    }

    cout << "ok" << endl;

    cout << "testing asynchronous ice_invoke... " << flush;

    {
        CookiePtr cookie = new Cookie();

        Ice::ByteSeq inParams, outParams;
        Ice::AsyncResultPtr result = oneway->begin_ice_invoke("opOneway", Ice::Normal, inParams);
        if(!oneway->end_ice_invoke(outParams, result))
        {
            test(false);
        }
        
        Ice::OutputStreamPtr out = Ice::createOutputStream(communicator);
        out->write(testString);
        out->finished(inParams);

        // begin_ice_invoke with no callback
        result = cl->begin_ice_invoke("opString", Ice::Normal, inParams);
        if(cl->end_ice_invoke(outParams, result))
        {
            Ice::InputStreamPtr in = Ice::createInputStream(communicator, outParams);
            string s;
            in->read(s);
            test(s == testString);
            in->read(s);
            test(s == testString);
        }
        else
        {
            test(false);
        };

        // begin_ice_invoke with no callback and array mapping
        pair<const ::Ice::Byte*, const ::Ice::Byte*> inPair(&inParams[0], &inParams[0] + inParams.size());
        result = cl->begin_ice_invoke("opString", Ice::Normal, inPair);
        if(cl->end_ice_invoke(outParams, result))
        {
            Ice::InputStreamPtr in = Ice::createInputStream(communicator, outParams);
            string s;
            in->read(s);
            test(s == testString);
            in->read(s);
            test(s == testString);
        }
        else
        {
            test(false);
        };

        // begin_ice_invoke with Callback
        CallbackPtr cb = new Callback(communicator, false);
        cl->begin_ice_invoke("opString", Ice::Normal, inParams, Ice::newCallback(cb, &Callback::opString));
        cb->check();

        // begin_ice_invoke with Callback and Cookie
        cb = new Callback(communicator, true);
        cl->begin_ice_invoke("opString", Ice::Normal, inParams, Ice::newCallback(cb, &Callback::opString), cookie);
        cb->check();

        // begin_ice_invoke with Callback_Object_ice_invoke 
        cb = new Callback(communicator, false);
        Ice::Callback_Object_ice_invokePtr d = Ice::newCallback_Object_ice_invoke(cb, &Callback::opStringNC, nullEx);
        cl->begin_ice_invoke("opString", Ice::Normal, inParams, d);
        cb->check();

        // begin_ice_invoke with Callback_Object_ice_invoke with Cookie
        cb = new Callback(communicator, false);
        d = Ice::newCallback_Object_ice_invoke(cb, &Callback::opStringWC, nullExWC);
        cl->begin_ice_invoke("opString", Ice::Normal, inParams, d, cookie);
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
        Ice::ByteSeq inParams, outParams;

        // begin_ice_invoke with no callback
        Ice::AsyncResultPtr result = cl->begin_ice_invoke("opException", Ice::Normal, inParams);
        if(cl->end_ice_invoke(outParams, result))
        {
            test(false);
        }
        else
        {
            Ice::InputStreamPtr in = Ice::createInputStream(communicator, outParams);
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
        }

        // begin_ice_invoke with Callback
        CallbackPtr cb = new Callback(communicator, false);
        cl->begin_ice_invoke("opException", Ice::Normal, inParams, Ice::newCallback(cb, &Callback::opException));
        cb->check();

        // begin_ice_invoke with Callback and Cookie
        cb = new Callback(communicator, true);
        cl->begin_ice_invoke("opException", Ice::Normal, inParams, Ice::newCallback(cb, &Callback::opException),
                             cookie);
        cb->check();

        // begin_ice_invoke with Callback_Object_ice_invoke
        cb = new Callback(communicator, false);
        Ice::Callback_Object_ice_invokePtr d = Ice::newCallback_Object_ice_invoke(cb, &Callback::opExceptionNC, nullEx);
        cl->begin_ice_invoke("opException", Ice::Normal, inParams, d);
        cb->check();

        // begin_ice_invoke with Callback_Object_ice_invoke with Cookie
        cb = new Callback(communicator, false);
        d = Ice::newCallback_Object_ice_invoke(cb, &Callback::opExceptionWC, nullExWC);
        cl->begin_ice_invoke("opException", Ice::Normal, inParams, d, cookie);
        cb->check();
    }

    cout << "ok" << endl;

    return cl;
}
