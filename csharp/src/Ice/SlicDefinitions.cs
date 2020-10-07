// Copyright (c) ZeroC, Inc. All rights reserved.

namespace ZeroC.Ice
{
    internal static class SlicDefinitions
    {
        internal enum FrameType : byte
        {
            Initialize = 0x01,
            InitializeAck = 0x02,
            Version = 0x03,
            Ping = 0x04,
            Pong = 0x05,
            Stream = 0x06,
            StreamLast = 0x07,
            StreamReset = 0x08,
            StreamUnidirectionalFin = 0x09,
        }

        // The header below is a sentinel header used to reserve space in the protocol frame to avoid
        // allocating again a byte buffer for the Slic header.
        internal static byte[] FrameHeader = new byte[]
        {
            0x05, // Frame type
            0x02, 0x04, 0x06, 0x08, // FrameSize (varint)
            0x03, 0x05, 0x07, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, // Stream ID (varlong)
        };

        internal static readonly Encoding Encoding = Encoding.V20;
    }
}
