//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include "Ice/BuiltinSequences.ice"
#include "Ice/Identity.ice"

[["java:package:test.Ice.ami"]]
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

    bool opBool(bool b);
    byte opByte(byte b);
    short opShort(short s);
    int opInt(int i);
    long opLong(long l);
    float opFloat(float f);
    double opDouble(double d);

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
