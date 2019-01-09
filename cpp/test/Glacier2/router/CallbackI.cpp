// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
    responseCallbackEx(const CookiePtr& cookie)
    {
        getCookie<AMD_Callback_initiateCallbackExPtr>(cookie)->ice_response();
    }

    void
    exceptionCallbackEx(const Ice::Exception& ex, const CookiePtr& cookie)
    {
        getCookie<AMD_Callback_initiateCallbackExPtr>(cookie)->ice_exception(ex);
    }

    void
    responseConcurrentCallback(Int number, const CookiePtr& cookie)
    {
        getCookie<AMD_Callback_initiateConcurrentCallbackPtr>(cookie)->ice_response(number);
    }

    void
    exceptionConcurrentCallback(const Ice::Exception& ex, const CookiePtr& cookie)
    {
        getCookie<AMD_Callback_initiateConcurrentCallbackPtr>(cookie)->ice_exception(ex);
    }

    void
    responseWaitCallback(const CookiePtr& cookie)
    {
        getCookie<AMD_Callback_initiateWaitCallbackPtr>(cookie)->ice_response();
    }

    void
    exceptionWaitCallback(const Ice::Exception& ex, const CookiePtr& cookie)
    {
        getCookie<AMD_Callback_initiateWaitCallbackPtr>(cookie)->ice_exception(ex);
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
    _callback(0),
    _waitCallback(false),
    _callbackWithPayload(false),
    _finishWaitCallback(false)
{
}

void
CallbackReceiverI::callback(const Current&)
{
    Lock sync(*this);
    ++_callback;
    notifyAll();
}

void
CallbackReceiverI::callbackEx(const Current& current)
{
    callback(current);
    CallbackException ex;
    ex.someValue = 3.14;
    ex.someString = "3.14";
    throw ex;
}

void
CallbackReceiverI::concurrentCallback_async(const AMD_CallbackReceiver_concurrentCallbackPtr& cb,
                                        Int number,
                                        const Current&)
{
    Lock sync(*this);

    pair<AMD_CallbackReceiver_concurrentCallbackPtr, Int> p;
    p.first = cb;
    p.second = number;
    _callbacks.push_back(p);
    notifyAll();
}

void
CallbackReceiverI::waitCallback(const Current&)
{
    {
        Lock sync(*this);
        assert(!_waitCallback);
        _waitCallback = true;
        notifyAll();
    }

    {
        Lock sync(*this);
        while(!_finishWaitCallback)
        {
            wait();
        }
        _finishWaitCallback = false;
    }
}

void
CallbackReceiverI::callbackWithPayload(const Ice::ByteSeq&, const Current&)
{
    Lock sync(*this);
    assert(!_callbackWithPayload);
    _callbackWithPayload = true;
    notifyAll();
}

void
CallbackReceiverI::callbackOK(int expected)
{
    Lock sync(*this);

    while(_callback != expected)
    {
        wait();
    }
    _callback = 0;
}

void
CallbackReceiverI::waitCallbackOK()
{
    Lock sync(*this);
    while(!_waitCallback)
    {
        wait();
    }

    _waitCallback = false;
}

void
CallbackReceiverI::callbackWithPayloadOK()
{
    Lock sync(*this);

    while(!_callbackWithPayload)
    {
        wait();
    }

    _callbackWithPayload = false;
}

void
CallbackReceiverI::notifyWaitCallback()
{
    Lock sync(*this);
    _finishWaitCallback = true;
    notifyAll();
};

void
CallbackReceiverI::answerConcurrentCallbacks(unsigned int num)
{
    Lock sync(*this);

    while(_callbacks.size() != num)
    {
        wait();
    }

    for(vector<pair<AMD_CallbackReceiver_concurrentCallbackPtr, Int> >::const_iterator p = _callbacks.begin();
        p != _callbacks.end();
        ++p)
    {
        p->first->ice_response(p->second);
    }
    _callbacks.clear();
}

CallbackI::CallbackI()
{
}

void
CallbackI::initiateCallback_async(const AMD_Callback_initiateCallbackPtr& cb,
                                  const CallbackReceiverPrx& proxy, const Current& current)
{
    if(proxy->ice_isTwoway())
    {
        AsyncCBPtr acb = new AsyncCB();
        proxy->begin_callback(current.ctx,
            newCallback_CallbackReceiver_callback(acb, &AsyncCB::responseCallback, &AsyncCB::exceptionCallback),
            newCookie(cb));
    }
    else
    {
        proxy->callback(current.ctx);
        cb->ice_response();
    }
}

void
CallbackI::initiateCallbackEx_async(const AMD_Callback_initiateCallbackExPtr& cb,
                                    const CallbackReceiverPrx& proxy, const Current& current)
{
    if(proxy->ice_isTwoway())
    {
        AsyncCBPtr acb = new AsyncCB();
        proxy->begin_callbackEx(current.ctx,
            newCallback_CallbackReceiver_callbackEx(acb, &AsyncCB::responseCallbackEx, &AsyncCB::exceptionCallbackEx),
            newCookie(cb));
    }
    else
    {
        proxy->callbackEx(current.ctx);
        cb->ice_response();
    }
}

void
CallbackI::initiateConcurrentCallback_async(const AMD_Callback_initiateConcurrentCallbackPtr& cb,
                                            Int number,
                                            const CallbackReceiverPrx& proxy,
                                            const Current& current)
{
    AsyncCBPtr acb = new AsyncCB();
    proxy->begin_concurrentCallback(number, current.ctx,
        newCallback_CallbackReceiver_concurrentCallback(acb, &AsyncCB::responseConcurrentCallback,
                                                        &AsyncCB::exceptionConcurrentCallback),
        newCookie(cb));
}

void
CallbackI::initiateWaitCallback_async(const AMD_Callback_initiateWaitCallbackPtr& cb,
                                      const CallbackReceiverPrx& proxy,
                                      const Current& current)
{
    AsyncCBPtr acb = new AsyncCB();
    proxy->begin_waitCallback(current.ctx,
        newCallback_CallbackReceiver_waitCallback(acb, &AsyncCB::responseWaitCallback, &AsyncCB::exceptionWaitCallback),
        newCookie(cb));
}

void
CallbackI::initiateCallbackWithPayload_async(const AMD_Callback_initiateCallbackWithPayloadPtr& cb,
                                             const CallbackReceiverPrx& proxy,
                                             const Current& current)
{
    Ice::ByteSeq seq(1000 * 1024, 0);
    AsyncCBPtr acb = new AsyncCB();
    proxy->begin_callbackWithPayload(seq, current.ctx,
        newCallback_CallbackReceiver_callbackWithPayload(acb, &AsyncCB::responseCallbackWithPayload,
                                                         &AsyncCB::exceptionCallbackWithPayload),
        newCookie(cb));
}

void
CallbackI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
