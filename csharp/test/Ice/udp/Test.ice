//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Ice/Identity.ice>

[[suppress-warning(reserved-identifier)]]

module ZeroC::Ice::Test::UDP
{

interface PingReply
{
    void reply();
}

sequence<byte> ByteSeq;

interface TestIntf
{
    int getValue();
    void ping(PingReply reply);
    void sendByteSeq(ByteSeq seq, PingReply? reply);
    void pingBiDir(Ice::Identity id);
    void shutdown();
}

}
