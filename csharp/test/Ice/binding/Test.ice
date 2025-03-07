// Copyright (c) ZeroC, Inc.

#pragma once

["cs:namespace:Ice.binding"]
module Test
{
    interface TestIntf
    {
        string getAdapterName();
    }
<<<<<<< Updated upstream

    interface RemoteObjectAdapter
    {
        TestIntf* getTestIntf();

        void deactivate();
    }

    interface RemoteCommunicator
    {
        RemoteObjectAdapter* createObjectAdapter(string name, string endpoints);

        void deactivateObjectAdapter(RemoteObjectAdapter* adapter);

=======

    interface RemoteObjectAdapter
    {
        TestIntf* getTestIntf();

        void deactivate();
    }

    interface RemoteCommunicator
    {
        RemoteObjectAdapter* createObjectAdapter(string name, string endpoints);

        void deactivateObjectAdapter(RemoteObjectAdapter* adapter);

>>>>>>> Stashed changes
        void shutdown();
    }
}
