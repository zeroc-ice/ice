// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef CALLBACK_I_H
#define CALLBACK_I_H

#include <IceUtil/IceUtil.h>
#include <Callback.h>
#include <set>

class CallbackSenderI;
typedef IceUtil::Handle<CallbackSenderI> CallbackSenderIPtr;

class CallbackSenderI : public Demo::CallbackSender, public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:
    
    CallbackSenderI(const Ice::CommunicatorPtr&);

    void destroy();

    virtual void addClient(const Ice::Identity&, const Ice::Current&);

    virtual void run();

private:

    Ice::CommunicatorPtr _communicator;
    bool _destroy;
    std::set<Demo::CallbackReceiverPrx> _clients;
};

#endif
