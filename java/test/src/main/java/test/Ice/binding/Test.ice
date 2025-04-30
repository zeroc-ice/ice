// Copyright (c) ZeroC, Inc.

#pragma once

["java:identifier:test.Ice.binding.Test"]
module Test
{
    interface TestIntf
    {
        string getAdapterName();
    }

    interface RemoteObjectAdapter
    {
        TestIntf* getTestIntf();

        void deactivate();
    }

    interface RemoteCommunicator
    {
        RemoteObjectAdapter* createObjectAdapter(string name, string endpoints);

        void deactivateObjectAdapter(RemoteObjectAdapter* adapter);

        void shutdown();
    }
}
