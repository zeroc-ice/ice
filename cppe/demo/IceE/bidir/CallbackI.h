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

#include <IceE/Thread.h>
#include <IceE/Monitor.h>
#include <Callback.h>
#include <set>

class CallbackSenderI;
typedef IceE::Handle<CallbackSenderI> CallbackSenderIPtr;

class CallbackSenderI : public Demo::CallbackSender, public IceE::Monitor<IceE::Mutex>
{
public:
    
    CallbackSenderI();

    void destroy();

    virtual void addClient(const IceE::Identity&, const IceE::Current&);

    void start();
    void run();

private:

    bool _destroy;
    IceE::Int _num;
    std::set<Demo::CallbackReceiverPrx> _clients;

    //
    // We cannot derive CallbackSenderI from IceE::Thread, because
    // CallbackSenderI uses IceE::GCShared as base, and
    // IceE::Thread uses IceE::Shared as base. These two base
    // classes are not compatible. Therefore we use this helper class.
    //
    class CallbackSenderThread : public IceE::Thread
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

    IceE::ThreadPtr _callbackSenderThread;

};

#endif
