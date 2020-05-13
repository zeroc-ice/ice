//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

["cs:namespace:Ice.threading"]
module Test
{

exception TestFailedException
{
    string reason;
}

interface TestIntf
{
    void pingSync();
    ["amd"] void ping();
    void concurrent(int level);
    void reset();
    void shutdown();
}

}
