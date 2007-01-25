// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_ICE
#define TEST_ICE

module Test
{

interface Adapter
{
    bool isThreadPerConnection();

    int getThreadId();
    void reset();
    ["ami"] int waitForWakeup(int timeout, out bool notified);
    int wakeup();

    void callSelf(Adapter* proxy);
};

sequence<Adapter*> AdapterSeq;

interface Server
{
    AdapterSeq getAdapters();

    ["ami"] void ping();

    void shutdown();
};

};

#endif
