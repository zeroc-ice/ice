// Copyright (c) ZeroC, Inc.

#pragma once

["java:identifier:test.Ice.idleTimeout.Test"]
module Test
{
    interface TestIntf
    {
        void sleep(int ms);
        void shutdown();
    }
}
