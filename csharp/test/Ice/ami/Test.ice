//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include "Ice/BuiltinSequences.ice"
#include "Ice/Identity.ice"

["cs:namespace:Ice.ami"]
module Test
{

exception TestIntfException
{
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

    void closeConnection();
    void abortConnection();
    void sleep(int ms);
    ["amd"] void startDispatch();
    void finishDispatch();
    void shutdown();

    bool supportsAMD();
    bool supportsFunctionalTests();
    bool supportsBackPressureTests();

    ["amd"] void opAsyncDispatch();
    ["amd"] int opWithResultAsyncDispatch();
    ["amd"] void opWithUEAsyncDispatch()
        throws TestIntfException;

    ["amd"] void pingBiDir(PingReply* reply);
}

interface TestIntfController
{
    void holdAdapter();
    void resumeAdapter();
}

module Outer::Inner
{

interface TestIntf
{
    int op(int i, out int j);
}

}

}
