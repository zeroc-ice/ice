// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_PROTOCOL_H
#define ICEE_PROTOCOL_H

#include <IceE/Config.h>

namespace IceEInternal
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
const ::IceE::Int headerSize = 14;

//
// The magic number at the front of each message
//
const ::IceE::Byte magic[] = { 0x49, 0x63, 0x65, 0x50 };	// 'I', 'c', 'e', 'P'

//
// The current Ice protocol and encoding version
//
const ::IceE::Byte protocolMajor = 1;
const ::IceE::Byte protocolMinor = 0;
const ::IceE::Byte encodingMajor = 1;
const ::IceE::Byte encodingMinor = 0;

//
// The Ice protocol message types
//
const ::IceE::Byte requestMsg = 0;
const ::IceE::Byte requestBatchMsg = 1;
const ::IceE::Byte replyMsg = 2;
const ::IceE::Byte validateConnectionMsg = 3;
const ::IceE::Byte closeConnectionMsg = 4;

}

#endif
