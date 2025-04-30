// Copyright (c) ZeroC, Inc.

#pragma once

#include "Ice/BuiltinSequences.ice"

["java:identifier:test.Ice.interrupt.Test"]
module Test
{
    exception InterruptedException
    {}

    interface TestIntf
    {
        void op();
        idempotent void opIdempotent();
        void sleep(int to)
            throws InterruptedException;
        void opWithPayload(Ice::ByteSeq seq);
        void shutdown();
    }

    exception CannotInterruptException
    {}

    interface TestIntfController
    {
        void holdAdapter();
        void resumeAdapter();
        void interrupt()
            throws CannotInterruptException;
    }
}
