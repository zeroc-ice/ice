// Copyright (c) ZeroC, Inc.

#ifndef TEST_ICE
#define TEST_ICE

module Test
{
    interface TestIntf
    {
        void shutdown();

        string getProperty(string name);
    }
}

#endif
