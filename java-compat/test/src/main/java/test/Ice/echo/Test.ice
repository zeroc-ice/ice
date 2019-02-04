//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["java:package:test.Ice.echo"]]
module Test
{

//
// This object is available with the identity "__echo".
//
interface Echo
{
    void startBatch();
    void flushBatch();
    void shutdown();
}

}
