//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[suppress-warning(reserved-identifier)]]

module ZeroC::Ice::Test::Threading
{

exception TestFailedException
{
    string reason;
}

interface TestIntf
{
    void pingSync();
    [amd] void ping();
    void concurrent(int level);
    void reset();
    void shutdown();
}

}
