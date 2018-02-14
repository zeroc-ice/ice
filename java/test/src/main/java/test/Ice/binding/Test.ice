// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["java:package:test.Ice.binding"]]
module Test
{

interface TestIntf
{
    string getAdapterName();
};

interface RemoteObjectAdapter
{
    TestIntf* getTestIntf();
    
    void deactivate();
};

interface RemoteCommunicator
{
    RemoteObjectAdapter* createObjectAdapter(string name, string endpoints);

    void deactivateObjectAdapter(RemoteObjectAdapter* adapter);

    void shutdown();
};

};
