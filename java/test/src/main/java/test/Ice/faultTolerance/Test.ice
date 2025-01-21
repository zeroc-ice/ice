// Copyright (c) ZeroC, Inc.

#pragma once

[["java:package:test.Ice.faultTolerance"]]
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
