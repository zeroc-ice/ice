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
const ::Ice::Byte magic[] = { 0x49, 0x63, 0x65, 0x50 };	// 'I', 'c', 'e', 'P'

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

}

#endif
