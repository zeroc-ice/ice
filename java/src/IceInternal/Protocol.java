// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final class Protocol
{
    //
    // Size of the Ice protocol header
    //
    // Magic number (4 bytes)
    // Protocol version major (Byte)
    // Protocol version minor (Byte)
    // Encoding version major (Byte)
    // Encoding version minor (Byte)
    // Message type (Byte)
    // Compression status (Byte)
    // Message size (Int)
    //
    final static int headerSize = 14;

    //
    // The magic number at the front of each message
    //
    final static byte magic[] = { 0x49, 0x63, 0x65, 0x50 };	// 'I', 'c', 'e', 'P'

    //
    // The current Ice protocol and encoding version
    //
    final static byte protocolMajor = 1;
    final static byte protocolMinor = 0;
    final static byte encodingMajor = 1;
    final static byte encodingMinor = 0;

    //
    // The Ice protocol message types
    //
    final static byte requestMsg = 0;
    final static byte requestBatchMsg = 1;
    final static byte replyMsg = 2;
    final static byte validateConnectionMsg = 3;
    final static byte closeConnectionMsg = 4;
}
