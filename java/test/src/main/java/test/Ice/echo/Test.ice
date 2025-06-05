// Copyright (c) ZeroC, Inc.

#pragma once

["java:identifier:test.Ice.echo.Test"]
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
