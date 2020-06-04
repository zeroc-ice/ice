//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[suppress-warning:reserved-identifier]]

module ZeroC::Ice::Test::FaultTolerance
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
