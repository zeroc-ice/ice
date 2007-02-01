// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <CallbackI.h>
#include <TestCommon.h>

using namespace std;
using namespace Ice;
using namespace Test;

class AMI_CallbackReceiver_callbackI : public AMI_CallbackReceiver_callback
{
public:

    AMI_CallbackReceiver_callbackI(const AMD_Callback_initiateCallbackPtr cb) :
        _cb(cb)
    {
    }

    virtual void
    ice_response()
    {
        _cb->ice_response();
    }

    virtual void
    ice_exception(const Exception& e)
    {
        _cb->ice_exception(e);
    }

private:

    const AMD_Callback_initiateCallbackPtr _cb;
};

class AMI_CallbackReceiver_callbackExI : public AMI_CallbackReceiver_callbackEx
{
public:

    AMI_CallbackReceiver_callbackExI(const AMD_Callback_initiateCallbackExPtr cb) :
        _cb(cb)
    {
    }

    virtual void
    ice_response()
    {
        _cb->ice_response();
    }

    virtual void
    ice_exception(const Exception& e)
    {
        _cb->ice_exception(e);
    }

private:

    const AMD_Callback_initiateCallbackExPtr _cb;
};

class AMI_CallbackReceiver_concurrentCallbackI : public AMI_CallbackReceiver_concurrentCallback
{
public:

    AMI_CallbackReceiver_concurrentCallbackI(const AMD_Callback_initiateConcurrentCallbackPtr cb) :
        _cb(cb)
    {
    }

    virtual void
    ice_response(Int number)
    {
        _cb->ice_response(number);
    }

    virtual void
    ice_exception(const Exception& e)
    {
        _cb->ice_exception(e);
    }

private:

    const AMD_Callback_initiateConcurrentCallbackPtr _cb;
};

class AMI_CallbackReceiver_waitCallbackI : public AMI_CallbackReceiver_waitCallback
{
public:

    AMI_CallbackReceiver_waitCallbackI(const AMD_Callback_initiateWaitCallbackPtr cb) :
        _cb(cb)
    {
    }

    virtual void
    ice_response()
    {
        _cb->ice_response();
    }

    virtual void
    ice_exception(const Exception& e)
    {
        _cb->ice_exception(e);
    }

private:

    const AMD_Callback_initiateWaitCallbackPtr _cb;
};

class AMI_CallbackReceiver_callbackWithPayloadI : public AMI_CallbackReceiver_callbackWithPayload
{
public:

    AMI_CallbackReceiver_callbackWithPayloadI(const AMD_Callback_initiateCallbackWithPayloadPtr cb) :
        _cb(cb)
    {
    }

    virtual void
    ice_response()
    {
        _cb->ice_response();
    }

    virtual void
    ice_exception(const Exception& e)
    {
        _cb->ice_exception(e);
    }

private:

    const AMD_Callback_initiateCallbackWithPayloadPtr _cb;
};

CallbackReceiverI::CallbackReceiverI() :
    _callback(false),
    _waitCallback(false),
    _callbackWithPayload(false),
    _finishWaitCallback(false)
{
}

void
CallbackReceiverI::callback(const Current&)
{
    Lock sync(*this);
    assert(!_callback);
    _callback = true;
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
            test(timedWait(IceUtil::Time::milliSeconds(10000)));
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

bool
CallbackReceiverI::callbackOK()
{
    Lock sync(*this);

    while(!_callback)
    {
        if(!timedWait(IceUtil::Time::milliSeconds(10000)))
        {
            return false;
        }
    }

    _callback = false;
    return true;
}

bool
CallbackReceiverI::waitCallbackOK()
{
    Lock sync(*this);
    while(!_waitCallback)
    {
        if(!timedWait(IceUtil::Time::milliSeconds(10000)))
        {
            return false;
        }
    }

    _waitCallback = false;
    return true;
}

bool
CallbackReceiverI::callbackWithPayloadOK()
{
    Lock sync(*this);

    while(!_callbackWithPayload)
    {
        if(!timedWait(IceUtil::Time::milliSeconds(10000)))
        {
            return false;
        }
    }

    _callbackWithPayload = false;
    return true;
}

void
CallbackReceiverI::notifyWaitCallback()
{
    Lock sync(*this);
    _finishWaitCallback = true;
    notifyAll();
};

bool
CallbackReceiverI::answerConcurrentCallbacks(unsigned int num)
{
    Lock sync(*this);

    while(_callbacks.size() != num)
    {
        if(!timedWait(IceUtil::Time::milliSeconds(10000)))
        {
            return false;
        }
    }

    for(vector<pair<AMD_CallbackReceiver_concurrentCallbackPtr, Int> >::const_iterator p = _callbacks.begin();
        p != _callbacks.end();
        ++p)
    {
        p->first->ice_response(p->second);
    }
    _callbacks.clear();
    return true;
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
        proxy->callback_async(new AMI_CallbackReceiver_callbackI(cb), current.ctx);
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
        proxy->callbackEx_async(new AMI_CallbackReceiver_callbackExI(cb), current.ctx);
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
    proxy->concurrentCallback_async(new AMI_CallbackReceiver_concurrentCallbackI(cb), number, current.ctx);
}

void
CallbackI::initiateWaitCallback_async(const AMD_Callback_initiateWaitCallbackPtr& cb,
                                      const CallbackReceiverPrx& proxy, 
                                      const Current& current)
{
    proxy->waitCallback_async(new AMI_CallbackReceiver_waitCallbackI(cb));
}

void
CallbackI::initiateCallbackWithPayload_async(const AMD_Callback_initiateCallbackWithPayloadPtr& cb, 
                                             const CallbackReceiverPrx& proxy, 
                                             const Current& current)
{
    Ice::ByteSeq seq(1000 * 1024, 0);
    proxy->callbackWithPayload_async(new AMI_CallbackReceiver_callbackWithPayloadI(cb), seq);
}

void
CallbackI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
