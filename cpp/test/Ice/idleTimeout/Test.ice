//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
        void putAsSleep(bool aborted, int ms, DelayedTestIntf* target);
    }
}
