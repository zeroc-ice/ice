// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <CallbackI.h>
#include <TestHelper.h>

using namespace std;
using namespace Ice;
using namespace Test;

class Cookie : public Ice::LocalObject
{
};
typedef IceUtil::Handle<Cookie> CookiePtr;

template<class T>
class CookieT : public Cookie
{
public:

    CookieT(const T& v) : cb(v)
    {
    }

    T cb;
};

template<typename T> CookiePtr newCookie(const T& cb)
{
    return new CookieT<T>(cb);
}

template<typename T> const T& getCookie(const CookiePtr& cookie)
{
    return dynamic_cast<CookieT<T>* >(cookie.get())->cb;
}

class AsyncCB : public IceUtil::Shared
{
public:

    void
    responseCallback(const CookiePtr& cookie)
    {
        getCookie<AMD_Callback_initiateCallbackPtr>(cookie)->ice_response();
    }

    void
    exceptionCallback(const Ice::Exception& ex, const CookiePtr& cookie)
    {
        getCookie<AMD_Callback_initiateCallbackPtr>(cookie)->ice_exception(ex);
    }

    void
    responseCallbackWithPayload(const CookiePtr& cookie)
    {
        getCookie<AMD_Callback_initiateCallbackWithPayloadPtr>(cookie)->ice_response();
    }

    void
    exceptionCallbackWithPayload(const Ice::Exception& ex, const CookiePtr& cookie)
    {
        getCookie<AMD_Callback_initiateCallbackWithPayloadPtr>(cookie)->ice_exception(ex);
    }
};
typedef IceUtil::Handle<AsyncCB> AsyncCBPtr;

CallbackReceiverI::CallbackReceiverI() :
    _holding(false),
    _lastToken(-1),
    _callback(0),
    _callbackWithPayload(0)
{
}

void
CallbackReceiverI::callback(int token, const Current&)
{
    Lock sync(*this);
    checkForHold();

    if(token != _lastToken)
    {
        _callback = 0;
        _lastToken = token;
    }
    ++_callback;
    notifyAll();
}

void
CallbackReceiverI::callbackWithPayload(const Ice::ByteSeq&, const Current&)
{
    Lock sync(*this);
    checkForHold();
    ++_callbackWithPayload;
    notifyAll();
}

int
CallbackReceiverI::callbackOK(int count, int token)
{
    Lock sync(*this);

    while(_lastToken != token || _callback < count)
    {
        wait();
    }

    _callback -= count;
    return _callback;
}

int
CallbackReceiverI::callbackWithPayloadOK(int count)
{
    Lock sync(*this);

    while(_callbackWithPayload < count)
    {
        wait();
    }

    _callbackWithPayload -= count;
    return _callbackWithPayload;
}

void
CallbackReceiverI::hold()
{
    Lock sync(*this);
    _holding = true;
}

void
CallbackReceiverI::activate()
{
    Lock sync(*this);
    _holding = false;
    notifyAll();
}

void
CallbackReceiverI::checkForHold()
{
    while(_holding)
    {
        wait();
    }
}

CallbackI::CallbackI()
{
}

void
CallbackI::initiateCallback_async(const AMD_Callback_initiateCallbackPtr& cb,
                                  const CallbackReceiverPrx& proxy, int token, const Current& current)
{
    Ice::Context::const_iterator p = current.ctx.find("serverOvrd");
    Ice::Context ctx = current.ctx;
    if(p != current.ctx.end())
    {
        ctx["_ovrd"] = p->second;
    }

    if(proxy->ice_isTwoway())
    {
        AsyncCBPtr acb = new AsyncCB();
        proxy->begin_callback(token, ctx,
            newCallback_CallbackReceiver_callback(acb, &AsyncCB::responseCallback, &AsyncCB::exceptionCallback),
            newCookie(cb));
    }
    else
    {
        proxy->callback(token, ctx);
        cb->ice_response();
    }
}

void
CallbackI::initiateCallbackWithPayload_async(const AMD_Callback_initiateCallbackWithPayloadPtr& cb,
                                             const CallbackReceiverPrx& proxy,
                                             const Current& current)
{
    Ice::Context::const_iterator p = current.ctx.find("serverOvrd");
    Ice::Context ctx = current.ctx;
    if(p != current.ctx.end())
    {
        ctx["_ovrd"] = p->second;
    }

    Ice::ByteSeq seq(1000 * 1024, 0);
    if(proxy->ice_isTwoway())
    {
        AsyncCBPtr acb = new AsyncCB();
        proxy->begin_callbackWithPayload(seq, ctx,
                                         newCallback_CallbackReceiver_callbackWithPayload(
                                             acb,
                                             &AsyncCB::responseCallbackWithPayload,
                                             &AsyncCB::exceptionCallbackWithPayload),
                                         newCookie(cb));
    }
    else
    {
        proxy->callbackWithPayload(seq, ctx);
        cb->ice_response();
    }
}

void
CallbackI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
