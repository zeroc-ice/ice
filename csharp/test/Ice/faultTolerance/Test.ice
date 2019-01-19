//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

module Test
{

interface TestIntf
{
    void shutdown();
    void abort();
    idempotent void idempotentAbort();
    idempotent int pid();
}

interface Cleaner
{
    void cleanup();
}

}
