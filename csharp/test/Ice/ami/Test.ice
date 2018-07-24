// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <Ice/BuiltinSequences.ice>
#include <Ice/Identity.ice>

[["cs:namespace:Ice.ami"]]
module Test
{

exception TestIntfException
{
}

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

    ["amd"] void opAsyncDispatch();
    ["amd"] int opWithResultAsyncDispatch();
    ["amd"] void opWithUEAsyncDispatch()
        throws TestIntfException;

    void pingBiDir(Ice::Identity id);
}

interface TestIntfController
{
    void holdAdapter();
    void resumeAdapter();
}

module Outer
{

module Inner
{

interface TestIntf
{
    int op(int i, out int j);
}

}

}

}
