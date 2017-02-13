// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <Ice/BuiltinSequences.ice>

[["java:package:test.Ice.ami"]]
module Test
{

exception TestIntfException
{
};

enum CloseMode
{
    Forcefully,
    Gracefully,
    GracefullyWithWait
};

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
};

interface TestIntfController
{
    void holdAdapter();
    void resumeAdapter();
};

};
