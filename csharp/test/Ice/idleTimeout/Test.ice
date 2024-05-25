//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

["cs:namespace:Ice.idleTimeout"]
module Test
{
    sequence<byte> ByteSeq;

    interface TestIntf
    {
        void sleep(int ms);
        void shutdown();
    }
}
