// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceUtil/Config.h>
#include <Ice/SslIceUtils.h>
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
