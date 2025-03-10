// Copyright (c) ZeroC, Inc.

#pragma once

module Test
{
    interface TestIntf
    {
        ["amd"]
        void sleep(int ms);

        void shutdown();
    }
}
