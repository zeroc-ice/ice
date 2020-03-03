//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    // Definitions for the ice1 protocol.
    internal static class Ice1Definitions
    {
        //
        // Size of the ice1 protocol header
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
        internal const int HeaderSize = 14;

        //
        // The magic number at the front of each message
        //
        internal static readonly byte[] Magic = new byte[] { 0x49, 0x63, 0x65, 0x50 }; // 'I', 'c', 'e', 'P'

        //
        // The current Ice protocol and encoding version
        //
        internal const byte ProtocolMajor = 1;
        internal const byte ProtocolMinor = 0;
        internal const byte ProtocolEncodingMajor = 1;
        internal const byte ProtocolEncodingMinor = 0;

        //
        // The Ice protocol message types
        //
        internal const byte RequestMessage = 0;
        internal const byte RequestBatchMessage = 1;
        internal const byte ReplyMessage = 2;
        internal const byte ValidateConnectionMessage = 3;
        internal const byte CloseConnectionMessage = 4;

        internal static readonly byte[] RequestHeader = new byte[]
        {
            Magic[0], Magic[1], Magic[2], Magic[3],
            ProtocolMajor, ProtocolMinor,
            ProtocolEncodingMajor, ProtocolEncodingMinor,
            RequestMessage,
            0, // Compression status.
            0, 0, 0, 0, // Message size (placeholder).
            0, 0, 0, 0 // Request ID (placeholder).
        };

        internal static readonly byte[] RequestBatchHeader = new byte[]
        {
            Magic[0], Magic[1], Magic[2], Magic[3],
            ProtocolMajor, ProtocolMinor,
            ProtocolEncodingMajor, ProtocolEncodingMinor,
            RequestBatchMessage,
            0, // Compression status.
            0, 0, 0, 0, // Message size (placeholder).
            0, 0, 0, 0 // Number of requests in batch (placeholder).
        };

        internal static readonly byte[] ReplyHeader = new byte[]
        {
            Magic[0], Magic[1], Magic[2], Magic[3],
            ProtocolMajor, ProtocolMinor,
            ProtocolEncodingMajor, ProtocolEncodingMinor,
            ReplyMessage,
            0, // Compression status.
            0, 0, 0, 0 // Message size (placeholder).
        };
    }
}
