// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef THROUGHPUT_ICE
#define THROUGHPUT_ICE

module Demo
{

sequence<byte> ByteSeq;

sequence<string> StringSeq;

struct StringDouble
{
    string s;
    double d;
};
sequence<StringDouble> StringDoubleSeq;

interface Throughput
{
    void sendByteSeq(ByteSeq seq);
    ByteSeq recvByteSeq();
    ByteSeq echoByteSeq(ByteSeq seq);

    void sendStringSeq(StringSeq seq);
    StringSeq recvStringSeq();
    StringSeq echoStringSeq(StringSeq seq);

    void sendStructSeq(StringDoubleSeq seq);
    StringDoubleSeq recvStructSeq();
    StringDoubleSeq echoStructSeq(StringDoubleSeq seq);

    idempotent void shutdown();
};

};

#endif
