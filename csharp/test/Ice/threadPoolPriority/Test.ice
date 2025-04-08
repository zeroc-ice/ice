// Copyright (c) ZeroC, Inc.

#pragma once

["cs:identifier:Ice.threadPoolPriority.Test"]
module Test
{
    interface Priority
    {
        void shutdown();
        string getPriority();
    }
}
