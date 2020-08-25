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

        // ice2 frame types:
        internal enum FrameType : byte
        {
            Request = 0,
            Reply = 2,
        }

        internal static readonly byte[] RequestHeader = new byte[]
        {
            (byte) FrameType.Request,
            0, 0, 0, 0, // Frame size (placeholder).
        };

        internal static readonly byte[] ReplyHeader = new byte[]
        {
            (byte)FrameType.Reply,
            0, 0, 0, 0 // Frame size (placeholder).
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

        internal static List<ArraySegment<byte>> GetRequestData(OutgoingRequestFrame frame, long streamId)
        {
            byte[] headerData = new byte[HeaderSize + 4];
            RequestHeader.CopyTo(headerData.AsSpan());

            OutputStream.WriteSize20(frame.Size + HeaderSize + 4, headerData.AsSpan(10, 4));
            // TODO: Fix to support long streamId
            OutputStream.WriteInt((int)streamId, headerData.AsSpan(HeaderSize, 4));

            var data = new List<ArraySegment<byte>>() { headerData };
            data.AddRange(frame.Data);
            return data;
        }

        internal static List<ArraySegment<byte>> GetResponseData(OutgoingResponseFrame frame, long streamId)
        {
            byte[] headerData = new byte[HeaderSize + 4];
            ReplyHeader.CopyTo(headerData.AsSpan());

            OutputStream.WriteSize20(frame.Size + HeaderSize + 4, headerData.AsSpan(10, 4));
            // TODO: Fix to support long streamId
            OutputStream.WriteInt((int)streamId, headerData.AsSpan(HeaderSize, 4));

            var data = new List<ArraySegment<byte>>() { headerData };
            data.AddRange(frame.Data);
            return data;
        }

        private static string BytesToString(Span<byte> bytes) => BitConverter.ToString(bytes.ToArray());
    }
}
