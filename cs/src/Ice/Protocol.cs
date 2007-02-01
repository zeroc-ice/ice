// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    sealed class Protocol
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
        internal const int headerSize = 14;
        
        //
        // The magic number at the front of each message
        //
        internal static readonly byte[] magic
            = new byte[] { (byte)0x49, (byte)0x63, (byte)0x65, (byte)0x50 }; // 'I', 'c', 'e', 'P'
        
        //
        // The current Ice protocol and encoding version
        //
        internal const byte protocolMajor = 1;
        internal const byte protocolMinor = 0;
        internal const byte encodingMajor = 1;
        internal const byte encodingMinor = 0;
        
        //
        // The Ice protocol message types
        //
        internal const byte requestMsg = 0;
        internal const byte requestBatchMsg = 1;
        internal const byte replyMsg = 2;
        internal const byte validateConnectionMsg = 3;
        internal const byte closeConnectionMsg = 4;

        internal static readonly byte[] requestHdr = new byte[]
        {
            IceInternal.Protocol.magic[0], IceInternal.Protocol.magic[1], IceInternal.Protocol.magic[2], 
            IceInternal.Protocol.magic[3],
            IceInternal.Protocol.protocolMajor, IceInternal.Protocol.protocolMinor,
            IceInternal.Protocol.encodingMajor, IceInternal.Protocol.encodingMinor,
            IceInternal.Protocol.requestMsg,
            (byte)0, // Compression status.
            (byte)0, (byte)0, (byte)0, (byte)0, // Message size (placeholder).
            (byte)0, (byte)0, (byte)0, (byte)0  // Request ID (placeholder).
        };

        internal static readonly byte[] requestBatchHdr = new byte[]
        {
            IceInternal.Protocol.magic[0], IceInternal.Protocol.magic[1], IceInternal.Protocol.magic[2],
            IceInternal.Protocol.magic[3],
            IceInternal.Protocol.protocolMajor, IceInternal.Protocol.protocolMinor,
            IceInternal.Protocol.encodingMajor, IceInternal.Protocol.encodingMinor,
            IceInternal.Protocol.requestBatchMsg,
            (byte)0, // Compression status.
            (byte)0, (byte)0, (byte)0, (byte)0, // Message size (placeholder).
            (byte)0, (byte)0, (byte)0, (byte)0  // Number of requests in batch (placeholder).
        };
        
        internal static readonly byte[] replyHdr = new byte[]
        {
            IceInternal.Protocol.magic[0], IceInternal.Protocol.magic[1], IceInternal.Protocol.magic[2], 
            IceInternal.Protocol.magic[3],
            IceInternal.Protocol.protocolMajor, IceInternal.Protocol.protocolMinor,
            IceInternal.Protocol.encodingMajor, IceInternal.Protocol.encodingMinor,
            IceInternal.Protocol.replyMsg,
            (byte)0, // Compression status.
            (byte)0, (byte)0, (byte)0, (byte)0 // Message size (placeholder).
        };
        
    }

}
