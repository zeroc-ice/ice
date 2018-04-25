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

    virtual void callback(const Ice::Current&);
    virtual void callbackEx(const Ice::Current&);
    virtual void concurrentCallback_async(const ::Test::AMD_CallbackReceiver_concurrentCallbackPtr&,
                                      Ice::Int,
                                      const ::Ice::Current&);

    virtual void waitCallback(const ::Ice::Current&);
    virtual void callbackWithPayload(const Ice::ByteSeq&, const ::Ice::Current&);

    void callbackOK(int = 1);
    void waitCallbackOK();
    void callbackWithPayloadOK();
    void notifyWaitCallback();
    void answerConcurrentCallbacks(unsigned int);

private:

    int _callback;
    bool _waitCallback;
    bool _callbackWithPayload;
    bool _finishWaitCallback;
    std::vector<std::pair< ::Test::AMD_CallbackReceiver_concurrentCallbackPtr, Ice::Int> > _callbacks;
};
typedef IceUtil::Handle<CallbackReceiverI> CallbackReceiverIPtr;

class CallbackI : public ::Test::Callback
{
public:

    CallbackI();

    virtual void initiateCallback_async(const ::Test::AMD_Callback_initiateCallbackPtr&,
                                        const ::Test::CallbackReceiverPrx&, const Ice::Current&);
    virtual void initiateCallbackEx_async(const ::Test::AMD_Callback_initiateCallbackExPtr&,
                                          const ::Test::CallbackReceiverPrx&, const Ice::Current&);
    virtual void initiateConcurrentCallback_async(const ::Test::AMD_Callback_initiateConcurrentCallbackPtr&,
                                                  Ice::Int,
                                                  const ::Test::CallbackReceiverPrx&,
                                                  const ::Ice::Current&);
    virtual void initiateWaitCallback_async(const ::Test::AMD_Callback_initiateWaitCallbackPtr&,
                                            const ::Test::CallbackReceiverPrx&,
                                            const ::Ice::Current&);
    virtual void initiateCallbackWithPayload_async(const ::Test::AMD_Callback_initiateCallbackWithPayloadPtr&,
                                                   const ::Test::CallbackReceiverPrx&,
                                                   const ::Ice::Current&);

    virtual void shutdown(const Ice::Current&);
};

#endif
