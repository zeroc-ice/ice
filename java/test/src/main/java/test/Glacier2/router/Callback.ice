// Copyright (c) ZeroC, Inc.

#pragma once

["java:identifier:test.Glacier2.router.Test"]
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
