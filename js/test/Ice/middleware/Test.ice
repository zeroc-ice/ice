// Copyright (c) ZeroC, Inc.

#pragma once

module Test
{

interface Echo
{
    void setConnection();
    void startBatch();
    void flushBatch();
    void shutdown();
    bool supportsCompress();
}

interface MyObject
{
    string getName();
    void shutdown();
}

}
