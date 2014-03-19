// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

module Demo
{

sequence<byte> ByteSeq;
const int ByteSeqSize = 500000;

sequence<string> StringSeq;
const int StringSeqSize = 50000;

struct StringDouble
{
    string s;
    double d;
};
sequence<StringDouble> StringDoubleSeq;
const int StringDoubleSeqSize = 50000;

struct Fixed
{
    int i;
    int j;
    double d;
};
sequence<Fixed> FixedSeq;
const int FixedSeqSize = 50000;

interface Throughput
{
    bool needsWarmup();
    void startWarmup();
    void endWarmup();

    void sendByteSeq(["cpp:array"] ByteSeq seq);
    ["amd", "cpp:array"] ByteSeq recvByteSeq();
    ["amd", "cpp:array"] ByteSeq echoByteSeq(["cpp:array"] ByteSeq seq);

    void sendStringSeq(StringSeq seq);
    StringSeq recvStringSeq();
    StringSeq echoStringSeq(StringSeq seq);

    void sendStructSeq(StringDoubleSeq seq);
    StringDoubleSeq recvStructSeq();
    StringDoubleSeq echoStructSeq(StringDoubleSeq seq);

    void sendFixedSeq(FixedSeq seq);
    FixedSeq recvFixedSeq();
    FixedSeq echoFixedSeq(FixedSeq seq);

    void shutdown();
};

};
