// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
