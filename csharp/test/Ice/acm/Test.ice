// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

module Test
{

interface TestIntf
{
    void sleep(int seconds);
    void sleepAndHold(int seconds);
    void interruptSleep();
    void waitForHeartbeat(int count);
};

interface RemoteObjectAdapter
{
    TestIntf* getTestIntf();
    void activate();
    void hold();
    void deactivate();
};

interface RemoteCommunicator
{
    RemoteObjectAdapter* createObjectAdapter(int acmTimeout, int close, int heartbeat);
    void shutdown();
};

};

