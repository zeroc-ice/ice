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

#include <IceUtil/Thread.h>
#include <IceUtil/Monitor.h>
#include <Callback.h>
#include <set>

class CallbackSenderI : public Demo::CallbackSender, public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:
    
    CallbackSenderI();

    void destroy();

    virtual void addClient(const Ice::Identity&, const Ice::Current&);

    virtual void run();

private:

    bool _destroy;
    Ice::Int _num;
    std::set<Demo::CallbackReceiverPrx> _clients;
};

typedef IceUtil::Handle<CallbackSenderI> CallbackSenderIPtr;

#endif
