// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
