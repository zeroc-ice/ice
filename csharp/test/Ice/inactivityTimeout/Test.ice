//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

["cs:namespace:Ice.inactivityTimeout"]
module Test
{
    interface TestIntf
    {
        ["amd"]
        void sleep(int ms);

        void shutdown();
    }
}
