// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


#ifndef CALLBACK_I_H
#define CALLBACK_I_H

#include <Callback.h>

class CallbackReceiverI : public CallbackReceiver
{
public:

    virtual void callback(const Ice::Current&);
};

class CallbackI : public Callback
{
public:

    virtual void initiateCallback(const CallbackReceiverPrx&, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

};

#endif
