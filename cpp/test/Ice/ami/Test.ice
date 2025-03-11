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
        int opWithResultAndUE()
            throws TestIntfException;
        void opBatch();

        void opWithArgs(out int one, out int two, out int three, out int four, out int five, out int six, out int seven,
            out int eight, out int nine, out int ten, out int eleven);
        int opBatchCount();
        bool waitForBatch(int count);
        void closeConnection();
        void abortConnection();
        void sleep(int ms);
        ["amd"] void startDispatch();
        void finishDispatch();
        void shutdown();

        bool supportsFunctionalTests();
        bool supportsBackPressureTests();

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
