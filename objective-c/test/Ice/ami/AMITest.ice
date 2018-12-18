// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <Ice/BuiltinSequences.ice>
#include <Ice/Identity.ice>

["objc:prefix:TestAMI"]
module Test
{

exception TestIntfException
{
}

["objc:scoped"]
enum CloseMode
{
    Forcefully,
    Gracefully,
    GracefullyWithWait
}

interface PingReply
{
    void reply();
}

interface TestIntf
{
    void op();
    void opWithPayload(Ice::ByteSeq seq);
    int opWithResult();
    void opWithUE()
        throws TestIntfException;
    void opBatch();
    int opBatchCount();
    bool waitForBatch(int count);
    void close(CloseMode mode);
    void sleep(int ms);
    ["amd"] void startDispatch();
    void finishDispatch();
    void shutdown();

    bool supportsAMD();
    bool supportsFunctionalTests();

    void pingBiDir(PingReply* reply);
}

interface TestIntfController
{
    void holdAdapter();
    void resumeAdapter();
}

module Outer
{

["objc:prefix:TestAMITestOuterInner"]
module Inner
{

interface TestIntf
{
    int op(int i, out int j);
}

}

}

}
