// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef CALLBACK_I_H
#define CALLBACK_I_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <Callback.h>

class CallbackReceiverI : public ::Test::CallbackReceiver, IceUtil::Monitor<IceUtil::Mutex>
{
public:

    CallbackReceiverI();

    virtual void callback(const Ice::Current&);
    virtual void callbackEx(const Ice::Current&);
    bool callbackOK();

private:

    bool _callback;
};

class CallbackI : public ::Test::Callback
{
public:

    CallbackI();

    virtual void initiateCallback(const ::Test::CallbackReceiverPrx&, const Ice::Current&);
    virtual void initiateCallbackEx(const ::Test::CallbackReceiverPrx&, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);
};

#endif
