//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

module Test
{

interface TestIntf
{
    string getReplicaId();
    string getReplicaIdAndShutdown();
}

}
