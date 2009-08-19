// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef COMPRESSION_ICE
#define COMPRESSION_ICE

module Compression
{

sequence<byte> ByteSeq;

interface Receiver
{
    void sendData(ByteSeq bytes);
};

};

#endif
