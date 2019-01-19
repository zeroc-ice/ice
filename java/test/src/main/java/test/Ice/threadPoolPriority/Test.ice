//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
