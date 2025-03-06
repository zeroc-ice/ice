// Copyright (c) ZeroC, Inc.

#pragma once

module Test
{
    interface TestIntf
    {
        void shutdown();

        string getProperty(string name);
    }
}
