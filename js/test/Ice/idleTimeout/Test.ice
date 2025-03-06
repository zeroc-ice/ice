// Copyright (c) ZeroC, Inc.

#pragma once

module Test
{
    interface DelayedTestIntf
    {
        void sleep(int ms);
    }

    interface TestIntf extends DelayedTestIntf
    {
        void shutdown();
    }

    interface TestIntfBidir
    {
        void makeSleep(bool aborted, int ms, DelayedTestIntf* target);
    }
}
