// **********************************************************************
//
// Copyright (c) 2001
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

package IceInternal;

final class Protocol
{
    //
    // Size of the Ice protocol header
    //
    // Protocol version (Byte)
    // Encoding version (Byte)
    // Message type (Byte)
    // Message size (Int)
    //
    final static int headerSize = 7;

    //
    // The current Ice protocol and encoding version
    //
    final static byte protocolVersion = 0;
    final static byte encodingVersion = 0;

    //
    // The Ice protocol message types
    //
    final static byte requestMsg = 0;
    final static byte requestBatchMsg = 1;
    final static byte replyMsg = 2;
    final static byte validateConnectionMsg = 3;
    final static byte closeConnectionMsg = 4;
    final static byte compressedRequestMsg = 5;
    final static byte compressedRequestBatchMsg = 6;
    final static byte compressedReplyMsg = 7;
}
