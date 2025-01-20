// Copyright (c) ZeroC, Inc.

#pragma once

#include "Ice/BuiltinSequences.ice"

[["java:package:test.Ice.executor"]]
module Test
{

interface TestIntf
{
    void op();
    void sleep(int to);
    void opWithPayload(Ice::ByteSeq seq);
    void shutdown();
}

interface TestIntfController
{
    void holdAdapter();
    void resumeAdapter();
}

}
