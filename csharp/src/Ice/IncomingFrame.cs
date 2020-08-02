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
        public bool HasCompressedPayload { get; internal set; }

        /// <summary>The payload of this request frame. The bytes inside the payload should not be written to;
        /// they are writable because of the <see cref="System.Net.Sockets.Socket"/> methods for sending.</summary>
        // TODO: describe how long this payload remains valid once we add memory pooling.
        public abstract ArraySegment<byte> Payload { get; internal set; }

        /// <summary>The Ice protocol of this frame.</summary>
        public Protocol Protocol { get; }

        /// <summary>The frame byte count</summary>
        public int Size { get; private set; }

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
                (int size, int sizeLength) = buffer.ReadSize(Protocol.GetEncoding());
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

                byte[] decompressedData = new byte[decompressedSize + sizeLength];
                buffer.Slice(0, offset).CopyTo(decompressedData);
                // Set the compression status to '0' not-compressed
                decompressedData[sizeLength + 2] = 0;

                using var decompressedStream = new MemoryStream(decompressedData,
                                                                offset,
                                                                decompressedData.Length - offset);
                Debug.Assert(Payload.Array != null);
                var compressed = new GZipStream(new MemoryStream(Payload.Array,
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
                        decompressedStream.Position} bytes");
                }
                Payload = decompressedData;
                // Rewrite the encapsulation size and adjust the frame size
                int newSize = Payload.Count - sizeLength;
                OutputStream.WriteEncapsSize(newSize, decompressedData.AsSpan(0, sizeLength), Protocol.GetEncoding());
                Size += newSize - size;
                HasCompressedPayload = false;
            }
        }

        protected IncomingFrame(Protocol protocol, ArraySegment<byte> data, int sizeMax)
        {
            Protocol = protocol;
            Size = data.Count;
            _sizeMax = sizeMax;
        }
    }
}
