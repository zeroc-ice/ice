// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
// Protocol version (Byte)
// Encoding version (Byte)
// Message type (Byte)
// Message size (Int)
//
const ::Ice::Int headerSize = 7;

//
// The current Ice protocol and encoding version
//
const ::Ice::Byte protocolVersion = 0;
const ::Ice::Byte encodingVersion = 0;

//
// The Ice protocol message types
//
const ::Ice::Byte requestMsg = 0;
const ::Ice::Byte requestBatchMsg = 1;
const ::Ice::Byte replyMsg = 2;
const ::Ice::Byte validateConnectionMsg = 3;
const ::Ice::Byte closeConnectionMsg = 4;
const ::Ice::Byte compressedRequestMsg = 5;
const ::Ice::Byte compressedRequestBatchMsg = 6;
const ::Ice::Byte compressedReplyMsg = 7;

}

#endif
