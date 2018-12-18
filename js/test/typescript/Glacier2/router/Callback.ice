// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

[["js:es6-module"]]

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
}

interface Callback
{
    void initiateCallback(CallbackReceiver* proxy);

    void initiateCallbackEx(CallbackReceiver* proxy)
        throws CallbackException;

    void shutdown();
}

}
