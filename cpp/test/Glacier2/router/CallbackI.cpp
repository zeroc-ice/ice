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

CallbackReceiverI::CallbackReceiverI() :
    _callback(false)
{
}

void
CallbackReceiverI::callback(const Current&)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
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

bool
CallbackReceiverI::callbackOK()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

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
CallbackI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
