// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <Ice/Identity.ice>

module Test
{

interface PingReply
{
    void reply();
};

sequence<byte> ByteSeq;

interface TestIntf
{
    void ping(PingReply* reply);
    void sendByteSeq(ByteSeq seq, PingReply* reply);
    void pingBiDir(Ice::Identity reply);
    void shutdown();
};

};
