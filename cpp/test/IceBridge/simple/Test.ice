// Copyright (c) ZeroC, Inc.

#pragma once

module Test
{
    interface Callback
    {
        void ping();
        int getCount();
<<<<<<< Updated upstream

        void datagram();
        int getDatagramCount();
    }

=======

        void datagram();
        int getDatagramCount();
    }

>>>>>>> Stashed changes
    interface MyClass
    {
        ["amd"] void callCallback();
        ["amd"] int getCallbackCount();
<<<<<<< Updated upstream

        void incCounter(int expected);
        void waitCounter(int value);

        int getConnectionCount();
        string getConnectionInfo();
        void closeConnection(bool force);

        void datagram();
        int getDatagramCount();

        void callDatagramCallback();
        ["amd"] int getCallbackDatagramCount();

=======

        void incCounter(int expected);
        void waitCounter(int value);

        int getConnectionCount();
        string getConnectionInfo();
        void closeConnection(bool force);

        void datagram();
        int getDatagramCount();

        void callDatagramCallback();
        ["amd"] int getCallbackDatagramCount();

>>>>>>> Stashed changes
        void shutdown();
    }
}
