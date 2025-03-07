// Copyright (c) ZeroC, Inc.

#pragma once

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
