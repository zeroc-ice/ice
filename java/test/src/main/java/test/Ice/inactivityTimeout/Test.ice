// Copyright (c) ZeroC, Inc.

#pragma once

[["java:package:test.Ice.inactivityTimeout"]]

module Test
{
    interface TestIntf
    {
        void sleep(int ms);
        void shutdown();
    }
}
