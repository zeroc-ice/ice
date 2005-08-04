// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef CALLBACK_I_H
#define CALLBACK_I_H

#include <IceE/Thread.h>
#include <IceE/Monitor.h>
#include <Callback.h>
#include <set>

class CallbackSenderI;
typedef IceUtil::Handle<CallbackSenderI> CallbackSenderIPtr;

class CallbackSenderI : public Demo::CallbackSender, public IceUtil::Monitor<IceUtil::Mutex>
{
public:
    
    CallbackSenderI();

    void destroy();

    virtual void addClient(const Ice::Identity&, const Ice::Current&);

    void start();
    void run();

private:

    bool _destroy;
    Ice::Int _num;
    std::set<Demo::CallbackReceiverPrx> _clients;

    //
    // We cannot derive CallbackSenderI from IceUtil::Thread, because
    // CallbackSenderI uses Ice::GCShared as base, and
    // IceUtil::Thread uses IceUtil::Shared as base. These two base
    // classes are not compatible. Therefore we use this helper class.
    //
    class CallbackSenderThread : public IceUtil::Thread
    {
    public:

	CallbackSenderThread(const CallbackSenderIPtr& callbackSender) :
	    _callbackSender(callbackSender)
	{
	}

	virtual void run()
	{
	    _callbackSender->run();
	}

    private:

	CallbackSenderIPtr _callbackSender;
    };

    IceUtil::ThreadPtr _callbackSenderThread;

};

#endif
