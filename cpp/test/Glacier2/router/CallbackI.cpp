// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <CallbackI.h>

using namespace std;
using namespace Ice;
using namespace Test;

class AMI_CallbackReceiver_nestedCallbackI : public AMI_CallbackReceiver_nestedCallback
{
public:

    AMI_CallbackReceiver_nestedCallbackI(const AMD_Callback_initiateNestedCallbackPtr cb) :
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

    const AMD_Callback_initiateNestedCallbackPtr _cb;
};

CallbackReceiverI::CallbackReceiverI() :
    _callback(false)
{
}

void
CallbackReceiverI::callback(const Current&)
{
    Lock sync(*this);

    assert(!_callback);
    _callback = true;
    notify();
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
CallbackReceiverI::nestedCallback_async(const AMD_CallbackReceiver_nestedCallbackPtr& cb,
					Int number,
					const Current&)
{
    Lock sync(*this);

    pair<AMD_CallbackReceiver_nestedCallbackPtr, Int> p;
    p.first = cb;
    p.second = number;
    _callbacks.push_back(p);
    notify();
}

bool
CallbackReceiverI::callbackOK()
{
    Lock sync(*this);

    while(!_callback)
    {
	if(!timedWait(IceUtil::Time::milliSeconds(5000)))
	{
	    return false;
	}
    }

    _callback = false;
    return true;
}

bool
CallbackReceiverI::answerNestedCallbacks(unsigned int num)
{
    Lock sync(*this);

    while(_callbacks.size() != num)
    {
	if(!timedWait(IceUtil::Time::milliSeconds(5000)))
	{
	    return false;
	}
    }

    for(vector<pair<AMD_CallbackReceiver_nestedCallbackPtr, Int> >::const_iterator p = _callbacks.begin();
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
CallbackI::initiateCallback(const CallbackReceiverPrx& proxy, const Current& current)
{
    proxy->callback(current.ctx);
}

void
CallbackI::initiateCallbackEx(const CallbackReceiverPrx& proxy, const Current& current)
{
    proxy->callbackEx(current.ctx);
}

void
CallbackI::initiateNestedCallback_async(const AMD_Callback_initiateNestedCallbackPtr& cb,
					Int number,
					const CallbackReceiverPrx& proxy,
					const Current& current)
{
    proxy->nestedCallback_async(new AMI_CallbackReceiver_nestedCallbackI(cb), number, current.ctx);
}

void
CallbackI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
