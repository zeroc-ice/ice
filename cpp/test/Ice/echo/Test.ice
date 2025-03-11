// Copyright (c) ZeroC, Inc.

#pragma once

module Test
{
    //
    // This object is available with the identity "__echo".
    //
    interface Echo
    {
        void setConnection();
        void startBatch();
        void flushBatch();
        void shutdown();
    }
}
