//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[cs:namespace:ZeroC.Ice.acm]
module Test
{

interface TestIntf
{
    void sleep(int seconds);
    void interruptSleep();
    void startHeartbeatCount();
    void waitForHeartbeatCount(int count);
}

interface RemoteObjectAdapter
{
    TestIntf* getTestIntf();
    void deactivate();
}

interface RemoteCommunicator
{
    RemoteObjectAdapter* createObjectAdapter(int acmTimeout, int close, int heartbeat);
    void shutdown();
}

}
