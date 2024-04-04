//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

module Test
{
    interface TestIntf
    {
        void init();
        int getHeartbeatCount();
        void shutdown();
    }
}
