// **********************************************************************
//
// Copyright (c) 2001
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

#ifndef CALLBACK_ICE
#define CALLBACK_ICE

exception CallbackException
{
    double someValue;
    string someString;
};

class CallbackReceiver
{
    void callback();

    void callbackEx()
	throws CallbackException;
};

class Callback
{
    void initiateCallback(CallbackReceiver* proxy);

    void initiateCallbackEx(CallbackReceiver* proxy)
	throws CallbackException;

    void shutdown();
};

#endif
