// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

["objc:prefix:TestACM"]
module Test
{

interface TestIntf
{
    void sleep(int seconds);
    void sleepAndHold(int seconds);
    void interruptSleep();
    void startHeartbeatCount();
    void waitForHeartbeatCount(int count);
}

interface RemoteObjectAdapter
{
    TestIntf* getTestIntf();
    void activate();
    void hold();
    void deactivate();
}

interface RemoteCommunicator
{
    RemoteObjectAdapter* createObjectAdapter(int acmTimeout, int close, int heartbeat);
    void shutdown();
}

}
