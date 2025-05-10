// Copyright (c) ZeroC, Inc.

#pragma once

["java:identifier:test.Ice.inactivityTimeout.Test"]
module Test
{
    interface TestIntf
    {
        ["amd"]
        void sleep(int ms);

        void shutdown();
    }
}
