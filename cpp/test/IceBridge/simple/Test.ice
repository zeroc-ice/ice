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

    void incCounter(int expected);
    void waitCounter(int value);

    int getConnectionCount();
    string getConnectionInfo();
    void closeConnection(bool force);

    void datagram();
    int getDatagramCount();

    void callDatagramCallback();
    ["amd"] int getCallbackDatagramCount();

    int getHeartbeatCount();
    void enableHeartbeats();

    void shutdown();
}

}
