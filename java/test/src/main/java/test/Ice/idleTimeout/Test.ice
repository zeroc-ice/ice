// Copyright (c) ZeroC, Inc.

#pragma once

[["java:package:test.Ice.idleTimeout"]]

module Test
{
    interface TestIntf
    {
        void sleep(int ms);
        void shutdown();
    }
}
