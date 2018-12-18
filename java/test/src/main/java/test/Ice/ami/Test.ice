// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

#include <Ice/BuiltinSequences.ice>
#include <Ice/Identity.ice>

[["java:package:test.Ice.ami"]]
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
    bool opBool(bool b);
    byte opByte(byte b);
    short opShort(short s);
    int opInt(int i);
    long opLong(long l);
    float opFloat(float f);
    double opDouble(double d);

    void pingBiDir(PingReply* reply);
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
