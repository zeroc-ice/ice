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

sequence<byte> ByteSeq;

interface Throughput
{
    ByteSeq echoByteSeq(ByteSeq seq);
};

#endif
