//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

module Test
{
    interface TestIntf
    {
        void init();
        void sleep(int ms);
        int getHeartbeatCount();
        void shutdown();
    }
}
