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

#ifndef ICE_SSL_CONVERT_H
#define ICE_SSL_CONVERT_H

#include <Ice/BuiltinSequences.h>

namespace IceSSL
{

void ucharToByteSeq(unsigned char*, int, Ice::ByteSeq&);

unsigned char* byteSeqToUChar(const Ice::ByteSeq&);

}

#endif

