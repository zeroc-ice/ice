//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

module Test
{
    interface TestIntf
    {
        ["amd"]
        void sleep(int ms);

        void shutdown();
    }
}
