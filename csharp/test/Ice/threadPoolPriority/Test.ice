// Copyright (c) ZeroC, Inc.

#pragma once

["cs:namespace:Ice.threadPoolPriority"]
module Test
{
    interface Priority
    {
        void shutdown();
        string getPriority();
    }
}
