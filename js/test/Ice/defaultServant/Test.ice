// Copyright (c) ZeroC, Inc.

#pragma once

module Test
{
    interface MyObject
    {
        string getName();
    }

    interface Echo
    {
        void setConnection();
        void startBatch();
        void flushBatch();
        void shutdown();
        bool supportsCompress();
    }
}
