// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_ICE_UTILS_H
#define ICE_SSL_ICE_UTILS_H

#include <IceUtil/Config.h>
#include <Ice/BuiltinSequences.h>

namespace IceSecurity
{

namespace Ssl
{

void ucharToByteSeq(unsigned char*, int, Ice::ByteSeq&);

unsigned char* byteSeqToUChar(const Ice::ByteSeq&);

}

}

#endif

