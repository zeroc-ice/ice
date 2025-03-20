// Copyright (c) ZeroC, Inc.

#pragma once

[["java:package:test.Ice.threadPoolPriority"]]
module Test
{
    interface Priority
    {
        void shutdown();
        int getPriority();
    }
}
