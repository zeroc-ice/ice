// Copyright (c) ZeroC, Inc.

#pragma once

module Test
{
    interface Callback
    {
        void ping();
        int getCount();

        void datagram();
        int getDatagramCount();
    }

    interface MyClass
    {
        ["amd"] void callCallback();
        ["amd"] int getCallbackCount();

        int getConnectionCount();
        string getConnectionInfo();
        void closeConnection(bool force);

        void datagram();
        int getDatagramCount();

        void callDatagramCallback();
        ["amd"] int getCallbackDatagramCount();

        void shutdown();
    }
}
