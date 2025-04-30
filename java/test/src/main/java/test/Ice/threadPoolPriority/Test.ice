// Copyright (c) ZeroC, Inc.

#pragma once

["java:identifier:test.Ice.threadPoolPriority.Test"]
module Test
{
    interface Priority
    {
        void shutdown();
        int getPriority();
    }
}
