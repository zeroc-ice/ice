// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PROTOCOL_H
#define ICE_PROTOCOL_H

#include <Ice/Config.h>

namespace IceInternal
{

//
// Size of the Ice protocol header
//
// Magic number (4 Bytes)
// Protocol version major (Byte)
// Protocol version minor (Byte)
// Encoding version major (Byte)
// Encoding version minor (Byte)
// Message type (Byte)
// Compression status (Byte)
// Message size (Int)
//
const ::Ice::Int headerSize = 14;

//
// The magic number at the front of each message
//
extern const ::Ice::Byte magic[4];

//
// The current Ice protocol and encoding version
//
const ::Ice::Byte protocolMajor = 1;
const ::Ice::Byte protocolMinor = 0;
const ::Ice::Byte encodingMajor = 1;
const ::Ice::Byte encodingMinor = 0;

//
// The Ice protocol message types
//
const ::Ice::Byte requestMsg = 0;
const ::Ice::Byte requestBatchMsg = 1;
const ::Ice::Byte replyMsg = 2;
const ::Ice::Byte validateConnectionMsg = 3;
const ::Ice::Byte closeConnectionMsg = 4;

//
// The request header, batch request header and reply header.
//
extern const ::Ice::Byte requestHdr[headerSize + sizeof(Ice::Int)];
extern const ::Ice::Byte requestBatchHdr[headerSize + sizeof(Ice::Int)];
extern const ::Ice::Byte replyHdr[headerSize];

}

#endif
