// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef CALLBACK_ICE
#define CALLBACK_ICE

#include <Ice/BuiltinSequences.ice>

module Test
{

exception CallbackException
{
    double someValue;
    string someString;
};

interface CallbackReceiver
{
    ["ami"] void callback();

    ["ami"] void callbackEx()
        throws CallbackException;

    ["amd", "ami"] int concurrentCallback(int number);

    ["ami"] void waitCallback();

    ["ami"] void callbackWithPayload(Ice::ByteSeq payload);
};

interface Callback
{
    ["amd"] void initiateCallback(CallbackReceiver* proxy);

    ["amd"] void initiateCallbackEx(CallbackReceiver* proxy)
        throws CallbackException;

    ["amd", "ami"] int initiateConcurrentCallback(int number, CallbackReceiver* proxy);

    ["amd"] void initiateWaitCallback(CallbackReceiver* proxy);

    ["amd"] void initiateCallbackWithPayload(CallbackReceiver* proxy);

    void shutdown();
};

};

#endif
