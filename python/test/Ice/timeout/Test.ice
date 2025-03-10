// Copyright (c) ZeroC, Inc.

#pragma once

module Test
{
    sequence<byte> ByteSeq;

    interface Timeout
    {
        void op();
        void sendData(ByteSeq seq);
        void sleep(int to);
    }

    interface Controller
    {
        void holdAdapter(int to);
        void resumeAdapter();
        void shutdown();
    }
}
