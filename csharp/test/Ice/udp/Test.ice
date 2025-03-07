// Copyright (c) ZeroC, Inc.

#pragma once

#include "Ice/Identity.ice"

["cs:namespace:Ice.udp"]
module Test
{
    interface PingReply
    {
        void reply();
    }
<<<<<<< Updated upstream

    sequence<byte> ByteSeq;

=======

    sequence<byte> ByteSeq;

>>>>>>> Stashed changes
    interface TestIntf
    {
        void ping(PingReply* reply);
        void sendByteSeq(ByteSeq seq, PingReply* reply);
        void pingBiDir(Ice::Identity id);
        void shutdown();
    }
}
