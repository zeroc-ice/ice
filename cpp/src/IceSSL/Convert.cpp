// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
    size_t seqSize = sequence.size();

    assert(seqSize > 0);

    unsigned char* ucharSeq = new unsigned char[seqSize];

    assert(ucharSeq != 0);

    unsigned char* ucharPtr = ucharSeq;
    std::copy(sequence.begin(), sequence.end(), ucharPtr);

    return ucharSeq;
}
