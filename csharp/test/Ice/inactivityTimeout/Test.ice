// Copyright (c) ZeroC, Inc.

#pragma once

["cs:identifier:Ice.inactivityTimeout.Test"]
module Test
{
    interface TestIntf
    {
        ["amd"]
        void sleep(int ms);

        void shutdown();
    }
}
