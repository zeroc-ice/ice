// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef THROUGHPUT_ICE
#define THROUGHPUT_ICE

const int seqSize = 500000;

["cs:array"] sequence<byte> ByteSeq;

interface Throughput
{
    void sendByteSeq(ByteSeq seq);
    ByteSeq recvByteSeq();
    ByteSeq echoByteSeq(ByteSeq seq);
};

#endif
