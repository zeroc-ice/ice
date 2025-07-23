// Copyright (c) ZeroC, Inc.

#pragma once

["python:identifier:generated.test.Ice.faultTolerance.Test"]
module Test
{
    interface TestIntf
    {
        void shutdown();
        void abort();
        idempotent void idempotentAbort();
        idempotent int pid();
    }
}
