// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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

    CallbackI(const Ice::CommunicatorPtr&);

    virtual void initiateCallback(const CallbackReceiverPrx&, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

private:

    Ice::CommunicatorPtr _communicator;
};

#endif
