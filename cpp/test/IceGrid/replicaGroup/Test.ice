// Copyright (c) ZeroC, Inc.

#pragma once

module Test
{
    interface TestIntf
    {
        string getReplicaId();
        string getReplicaIdAndShutdown();
    }
}
