// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
