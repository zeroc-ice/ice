// Copyright (c) ZeroC, Inc.

#pragma once

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
