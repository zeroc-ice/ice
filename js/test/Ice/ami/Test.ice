// Copyright (c) ZeroC, Inc.

#pragma once

#include "Ice/BuiltinSequences.ice"
#include "Ice/Identity.ice"

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

        bool supportsFunctionalTests();

        void pingBiDir(PingReply* reply);
    }

    interface TestIntfController
    {
        void holdAdapter();
        void resumeAdapter();
    }
}
