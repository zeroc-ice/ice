//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    // Definitions for the ice1 protocol.
    internal static class Ice1Definitions
    {
        // The encoding of the header for ice1 frames. It is nominally 1.0, but in practice it is identical to 1.1
        // for the subset of the encoding used by the ice1 headers.
        internal static readonly Encoding Encoding = new Encoding(1, 1);

        // Size of an ice1 frame or message header:
        // Magic number (4 bytes)
        // Post magic (4 bytes)
        // Message type (Byte)
        // Compression status (Byte)
        // Message size (Int - 4 bytes)
        internal const int HeaderSize = 14;

        // The magic number at the front of each message
        internal static readonly byte[] Magic = new byte[] { 0x49, 0x63, 0x65, 0x50 }; // 'I', 'c', 'e', 'P'

        // 4-bytes after magic that provide the protocol version (always 1.0 for an ice1 frame) and the
        // encoding of the frame header (always 1.0 with the an ice1 frame).
        internal static readonly byte[] PostMagic = new byte[] { 1, 0, 1, 0 };

        // The Ice protocol message types
        internal const byte RequestMessage = 0;
        internal const byte RequestBatchMessage = 1;
        internal const byte ReplyMessage = 2;
        internal const byte ValidateConnectionMessage = 3;
        internal const byte CloseConnectionMessage = 4;

        internal static readonly byte[] RequestHeader = new byte[]
        {
            Magic[0], Magic[1], Magic[2], Magic[3],
            PostMagic[0], PostMagic[1], PostMagic[2], PostMagic[3],
            RequestMessage,
            0, // Compression status.
            0, 0, 0, 0, // Message size (placeholder).
            0, 0, 0, 0 // Request ID (placeholder).
        };

        internal static readonly byte[] RequestBatchHeader = new byte[]
        {
            Magic[0], Magic[1], Magic[2], Magic[3],
            PostMagic[0], PostMagic[1], PostMagic[2], PostMagic[3],
            RequestBatchMessage,
            0, // Compression status.
            0, 0, 0, 0, // Message size (placeholder).
            0, 0, 0, 0 // Number of requests in batch (placeholder).
        };

        internal static readonly byte[] ReplyHeader = new byte[]
        {
            Magic[0], Magic[1], Magic[2], Magic[3],
            PostMagic[0], PostMagic[1], PostMagic[2], PostMagic[3],
            ReplyMessage,
            0, // Compression status.
            0, 0, 0, 0 // Message size (placeholder).
        };
    }
}
