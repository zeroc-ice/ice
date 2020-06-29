//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace ZeroC.Ice
{
    // Definitions for the ice2 protocol.

    // TODO: for now they are identical to the ice1 protocol definitions, except for Encoding, ProtocolBytes and some
    // sizes.
    internal static class Ice2Definitions
    {
        internal static readonly Encoding Encoding = Encoding.V2_0;

        // Size of an ice2 frame header for now:
        // Magic number (4 bytes)
        // Protocol bytes (4 bytes)
        // Frame type (Byte)
        // Compression status (Byte) (not used)
        // Frame size (size = varulong on 4 bytes)
        internal const int HeaderSize = 14;

        // The magic number at the front of each frame.
        internal static readonly byte[] Magic = new byte[] { 0x49, 0x63, 0x65, 0x50 }; // 'I', 'c', 'e', 'P'

        // 4-bytes after magic that provide the protocol version (always 2.0 for an ice2 frame) and the
        // encoding of the frame header (always set to 2.0 with the an ice2 frame).
        internal static readonly byte[] ProtocolBytes = new byte[] { 2, 0, 2, 0 };

        // ice2 frame types:
        internal enum FrameType : byte
        {
            Request = 0,
            Reply = 2,
            ValidateConnection = 3,
            CloseConnection = 4
        }

        internal static readonly byte[] RequestHeader = new byte[]
        {
            Magic[0], Magic[1], Magic[2], Magic[3],
            ProtocolBytes[0], ProtocolBytes[1], ProtocolBytes[2], ProtocolBytes[3],
            (byte) FrameType.Request,
            0, // Compression status.
            0, 0, 0, 0, // Frame size (placeholder).
            0, 0, 0, 0 // Request ID (placeholder).
        };

        internal static readonly byte[] ReplyHeader = new byte[]
        {
            Magic[0], Magic[1], Magic[2], Magic[3],
            ProtocolBytes[0], ProtocolBytes[1], ProtocolBytes[2], ProtocolBytes[3],
            (byte)FrameType.Reply,
            0, // Compression status.
            0, 0, 0, 0 // Frame size (placeholder).
        };

        internal static readonly byte[] ValidateConnectionFrame = new byte[]
        {
            Magic[0], Magic[1], Magic[2], Magic[3],
            ProtocolBytes[0], ProtocolBytes[1], ProtocolBytes[2], ProtocolBytes[3],
            (byte)FrameType.ValidateConnection,
            0, // Compression status.
            (HeaderSize << 2) | 2, 0, 0, 0 // Frame size on 2^2 = 4 bytes
        };

        internal static readonly byte[] CloseConnectionFrame = new byte[]
        {
            Magic[0], Magic[1], Magic[2], Magic[3],
            ProtocolBytes[0], ProtocolBytes[1], ProtocolBytes[2], ProtocolBytes[3],
            (byte)FrameType.CloseConnection,
            0, // Compression status.
            (HeaderSize << 2) | 2, 0, 0, 0 // Frame size on 2^2 = 4 bytes
        };

        internal static readonly byte[] EmptyResponsePayload = new byte[]
        {
            (byte) ReplyStatus.OK,
            2 << 2,                  // Encapsulation size
            Encoding.V2_0.Major,
            Encoding.V2_0.Minor
        };

        // Verify that the first 8 bytes correspond to Magic + ProtocolBytes
        internal static void CheckHeader(Span<byte> header)
        {
            Debug.Assert(header.Length >= 8);
            if (header[0] != Magic[0] || header[1] != Magic[1] || header[2] != Magic[2] || header[3] != Magic[3])
            {
                throw new InvalidDataException(
                    $"received incorrect magic bytes in header of ice2 frame: {BytesToString(header.Slice(0, 4))}");
            }

            header = header.Slice(4);

            if (header[0] != ProtocolBytes[0] || header[1] != ProtocolBytes[1])
            {
                throw new InvalidDataException(
                    $"received ice2 protocol frame with protocol set to {header[0]}.{header[1]}");
            }

            if (header[2] != ProtocolBytes[2] || header[3] != ProtocolBytes[3])
            {
                throw new InvalidDataException(
                    $"received ice2 protocol frame with protocol encoding set to {header[2]}.{header[3]}");
            }
        }

        internal static List<ArraySegment<byte>> GetRequestData(OutgoingRequestFrame frame, int requestId)
        {
            byte[] headerData = new byte[HeaderSize + 4];
            RequestHeader.CopyTo(headerData.AsSpan());

            OutputStream.WriteSize20(frame.Size + HeaderSize + 4, headerData.AsSpan(10, 4));
            OutputStream.WriteInt(requestId, headerData.AsSpan(HeaderSize, 4));

            var data = new List<ArraySegment<byte>>() { headerData };
            data.AddRange(frame.Data);
            return data;
        }

        internal static List<ArraySegment<byte>> GetResponseData(OutgoingResponseFrame frame, int requestId)
        {
            byte[] headerData = new byte[HeaderSize + 4];
            ReplyHeader.CopyTo(headerData.AsSpan());

            OutputStream.WriteSize20(frame.Size + HeaderSize + 4, headerData.AsSpan(10, 4));
            OutputStream.WriteInt(requestId, headerData.AsSpan(HeaderSize, 4));

            var data = new List<ArraySegment<byte>>() { headerData };
            data.AddRange(frame.Data);
            return data;
        }

        private static string BytesToString(Span<byte> bytes) => BitConverter.ToString(bytes.ToArray());
    }
}
