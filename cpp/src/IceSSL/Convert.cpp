// **********************************************************************
//
// Copyright (c) 2002
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

#include <IceUtil/Config.h>
#include <IceSSL/Convert.h>
#include <iterator>

void
IceSSL::ucharToByteSeq(unsigned char* ucharBuffer, int length, Ice::ByteSeq& destBuffer)
{
    assert(ucharBuffer != 0);
    destBuffer.reserve(length);
    std::copy(ucharBuffer, (ucharBuffer + length), std::back_inserter(destBuffer));
}

unsigned char*
IceSSL::byteSeqToUChar(const Ice::ByteSeq& sequence)
{
    int seqSize = sequence.size();

    assert(seqSize > 0);

    unsigned char* ucharSeq = new unsigned char[seqSize];

    assert(ucharSeq != 0);

    unsigned char* ucharPtr = ucharSeq;
    std::copy(sequence.begin(), sequence.end(), ucharPtr);

    return ucharSeq;
}
