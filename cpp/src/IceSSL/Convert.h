// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

