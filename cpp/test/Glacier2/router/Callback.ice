// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef CALLBACK_ICE
#define CALLBACK_ICE

module Test
{

exception CallbackException
{
    double someValue;
    string someString;
};

interface CallbackReceiver
{
    void callback();

    void callbackEx()
	throws CallbackException;

    ["amd", "ami"] int nestedCallback(int number);
};

interface Callback
{
    void initiateCallback(CallbackReceiver* proxy);

    void initiateCallbackEx(CallbackReceiver* proxy)
	throws CallbackException;

    ["amd", "ami"] int initiateNestedCallback(int number, CallbackReceiver* proxy);

    void shutdown();
};

};

#endif
