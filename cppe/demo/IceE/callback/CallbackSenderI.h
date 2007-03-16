// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef CALLBACK_I_H
#define CALLBACK_I_H

#include <Callback.h>

class CallbackSenderI : public Demo::CallbackSender
{
public:

    virtual void initiateCallback(const Demo::CallbackReceiverPrx&, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);
};

#endif
