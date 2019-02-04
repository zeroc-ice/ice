//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

["cs:namespace:Ice.binding"]
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
