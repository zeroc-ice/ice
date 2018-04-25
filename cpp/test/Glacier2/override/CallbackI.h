// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
#include <vector>

class CallbackReceiverI : public ::Test::CallbackReceiver, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    CallbackReceiverI();

    virtual void callback(int token, const Ice::Current&);
    virtual void callbackWithPayload(const Ice::ByteSeq&, const ::Ice::Current&);

    int callbackOK(int, int);
    int callbackWithPayloadOK(int);

    void hold();
    void activate();

private:

    void checkForHold();

    bool _holding;

    int _lastToken;
    int _callback;
    int _callbackWithPayload;
};
typedef IceUtil::Handle<CallbackReceiverI> CallbackReceiverIPtr;

class CallbackI : public ::Test::Callback
{
public:

    CallbackI();

    virtual void initiateCallback_async(const ::Test::AMD_Callback_initiateCallbackPtr&,
                                        const ::Test::CallbackReceiverPrx&, int, const Ice::Current&);
    virtual void initiateCallbackWithPayload_async(const ::Test::AMD_Callback_initiateCallbackWithPayloadPtr&,
                                                   const ::Test::CallbackReceiverPrx&,
                                                   const ::Ice::Current&);
    virtual void shutdown(const Ice::Current&);
};

#endif
