// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Protocol.h>

namespace IceInternal
{

const Ice::Byte magic[] = { 0x49, 0x63, 0x65, 0x50 };   // 'I', 'c', 'e', 'P'

#ifdef __BCPLUSPLUS__
const Ice::Byte requestHdr[headerSize + sizeof(Ice::Int)] = 
#else
const Ice::Byte requestHdr[] = 
#endif
{
    magic[0],
    magic[1],
    magic[2],
    magic[3],
    protocolMajor,
    protocolMinor,
    encodingMajor,
    encodingMinor,
    requestMsg,
    0, // Compression status
    0, 0, 0, 0, // Message size (placeholder)
    0, 0, 0, 0 // Request id (placeholder)
};

#ifdef __BCPLUSPLUS__
const Ice::Byte requestBatchHdr[headerSize + sizeof(Ice::Int)] =
#else
const Ice::Byte requestBatchHdr[] =
#endif
{
    magic[0],
    magic[1],
    magic[2],
    magic[3],
    protocolMajor,
    protocolMinor,
    encodingMajor,
    encodingMinor,
    requestBatchMsg,
    0, // Compression status
    0, 0, 0, 0, // Message size (place holder)
    0, 0, 0, 0  // Number of requests in batch (placeholder)
};

const Ice::Byte replyHdr[] = 
{
    magic[0],
    magic[1],
    magic[2],
    magic[3],
    protocolMajor,
    protocolMinor,
    encodingMajor,
    encodingMinor,
    replyMsg,
    0, // Compression status
    0, 0, 0, 0 // Message size (placeholder)
};

}
