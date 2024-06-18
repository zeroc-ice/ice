// Copyright (c) ZeroC, Inc.

#pragma once

[["java:package:test.Ice.idleTimeout"]]

module Test
{
    sequence<byte> ByteSeq;

    interface TestIntf
    {
        void sleep(int ms);
        void shutdown();
    }
}
