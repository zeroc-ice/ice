//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.IO;
using System.IO.Compression;

namespace ZeroC.Ice
{
    /// <summary>Base class for incoming frames.</summary>
    public abstract class IncomingFrame
    {
        /// <summary>The encoding of the frame payload.</summary>
        public abstract Encoding Encoding { get; }
        /// <summary>True if the encapsulation has a compressed payload, false otherwise.</summary>
        public bool HasCompressedPayload { get; protected set; }

        /// <summary>The data of this frame. The bytes inside the data should not be written to;
        /// they are writable because of the <see cref="System.Net.Sockets.Socket"/> methods for sending.</summary>
        // TODO: describe how long this payload remains valid once we add memory pooling.
        public ArraySegment<byte> Data { get; protected set; }

        // If the frame payload contains an encaps, this segment corresponds to the frame encaps otherwise is an
        // empty segment. This is always an Slice of the Payload, both must use the same array.
        private protected ArraySegment<byte> Payload { get; set; }

        /// <summary>The Ice protocol of this frame.</summary>
        public Protocol Protocol { get; }

        /// <summary>The frame byte count</summary>
        public int Size => Data.Count;

        private readonly int _sizeMax;

        /// <summary>Decompress the encapsulation payload if it is compressed. Compressed encapsulations are
        /// only supported with 2.0 encoding.</summary>
        public void DecompressPayload()
        {
            if (!HasCompressedPayload)
            {
                throw new InvalidOperationException("the encaps payload is not compressed");
            }
            else
            {
                ReadOnlySpan<byte> buffer = Payload.AsReadOnlySpan();
                (int _, int sizeLength) = buffer.ReadSize(Protocol.GetEncoding());
                // Offset of the start of the GZip decompressed data +3 corresponds to (Encoding 2 bytes, Compression
                // status 1 byte)
                int offset = sizeLength + 3;

                // Read the decompressed size that is written after the compression status byte when the payload is
                // compressed.
                (int decompressedSize, int decompressedSizeLength) = buffer.Slice(offset).ReadSize20();
                if (decompressedSize > _sizeMax)
                {
                    throw new InvalidDataException(@$"decompressed size of {decompressedSize
                                                   } bytes is greater than Ice.IncomingFrameSizeMax value");
                }

                Debug.Assert(Data.Array == Payload.Array);

                byte[] decompressedData = new byte[Payload.Offset + sizeLength + decompressedSize];
                Payload.Array.AsSpan(0, Payload.Offset + offset).CopyTo(decompressedData);
                // Set the compression status to '0' not-compressed
                decompressedData[Payload.Offset + sizeLength + 2] = 0;

                using var decompressedStream = new MemoryStream(decompressedData,
                                                                Payload.Offset + offset,
                                                                decompressedData.Length - Payload.Offset - offset);
                Debug.Assert(Payload.Array != null);
                var compressed = new GZipStream(
                    new MemoryStream(Payload.Array,
                                     Payload.Offset + offset + decompressedSizeLength,
                                     Payload.Count - offset - decompressedSizeLength),
                    CompressionMode.Decompress);
                compressed.CopyTo(decompressedStream);
                // +3 corresponds to (Encoding 2 bytes and Compression status 1 byte), that are part of the
                // decompressed size, but are not GZip compressed.
                if (decompressedStream.Position + 3 != decompressedSize)
                {
                    throw new InvalidDataException(
                        @$"received gzip compressed encapsulation with a decompressed size of only {
                        decompressedStream.Position} bytes {decompressedSize}");
                }
                Data = new ArraySegment<byte>(decompressedData, Data.Offset, Data.Count);
                Payload = new ArraySegment<byte>(decompressedData, Payload.Offset, decompressedSize + sizeLength);
                // Rewrite the encapsulation size
                OutputStream.WriteEncapsSize(decompressedSize, Payload.AsSpan(0, sizeLength), Protocol.GetEncoding());
                HasCompressedPayload = false;
            }
        }

        protected IncomingFrame(Protocol protocol, ArraySegment<byte> data, int sizeMax)
        {
            Protocol = protocol;
            Data = data;
            _sizeMax = sizeMax;
        }
    }
}
