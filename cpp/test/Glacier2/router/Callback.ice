// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

#include <Ice/BuiltinSequences.ice>

module Test
{

exception CallbackException
{
    double someValue;
    string someString;
}

interface CallbackReceiver
{
    void callback();

    void callbackEx()
        throws CallbackException;

    ["amd"] int concurrentCallback(int number);

    void waitCallback();

    void callbackWithPayload(Ice::ByteSeq payload);
}

interface Callback
{
    ["amd"] void initiateCallback(CallbackReceiver* proxy);

    ["amd"] void initiateCallbackEx(CallbackReceiver* proxy)
        throws CallbackException;

    ["amd"] int initiateConcurrentCallback(int number, CallbackReceiver* proxy);

    ["amd"] void initiateWaitCallback(CallbackReceiver* proxy);

    ["amd"] void initiateCallbackWithPayload(CallbackReceiver* proxy);

    void shutdown();
}

}
