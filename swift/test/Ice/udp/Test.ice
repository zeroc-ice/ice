// Copyright (c) ZeroC, Inc.
#pragma once

#include "Ice/Identity.ice"

module Test
{
    interface PingReply
    {
        void reply();
    }

    sequence<byte> ByteSeq;

    interface TestIntf
    {
        void ping(PingReply* reply);
        void sendByteSeq(ByteSeq seq, PingReply* reply);
        void pingBiDir(Ice::Identity reply);
        void shutdown();
    }
}
