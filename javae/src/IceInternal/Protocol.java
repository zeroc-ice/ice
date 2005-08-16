// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final public class Protocol
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
    public final static int headerSize = 14;

    //
    // The magic number at the front of each message
    //
    public final static byte magic[] = { 0x49, 0x63, 0x65, 0x50 };	// 'I', 'c', 'e', 'P'

    //
    // The current Ice protocol and encoding version
    //
    public final static byte protocolMajor = 1;
    public final static byte protocolMinor = 0;
    public final static byte encodingMajor = 1;
    public final static byte encodingMinor = 0;

    //
    // The Ice protocol message types
    //
    public final static byte requestMsg = 0;
    public final static byte requestBatchMsg = 1;
    public final static byte replyMsg = 2;
    public final static byte validateConnectionMsg = 3;
    public final static byte closeConnectionMsg = 4;
}
