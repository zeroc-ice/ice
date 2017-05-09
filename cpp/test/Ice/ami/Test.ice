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
#include <Ice/Identity.ice>

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
    int opWithResultAndUE()
        throws TestIntfException;
    void opBatch();

    void opWithArgs(out int one, out int two, out int three, out int four, out int five, out int six, out int seven,
                    out int eight, out int nine, out int ten, out int eleven);
    int opBatchCount();
    bool waitForBatch(int count);
    void close(CloseMode mode);
    void sleep(int ms);
    ["amd"] void startDispatch();
    void finishDispatch();
    void shutdown();

    bool supportsAMD();
    bool supportsFunctionalTests();

    void pingBiDir(Ice::Identity id);
}

interface TestIntfController
{
    void holdAdapter();
    void resumeAdapter();
}

}
