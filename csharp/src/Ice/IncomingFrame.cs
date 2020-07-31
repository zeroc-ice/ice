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
        /// <summary>The encoding of the frame payload</summary>
        public abstract Encoding Encoding { get; }
        /// <summary>The payload of this request frame. The bytes inside the payload should not be written to;
        /// they are writable because of the <see cref="System.Net.Sockets.Socket"/> methods for sending.</summary>
        // TODO: describe how long this payload remains valid once we add memory pooling.
        public abstract ArraySegment<byte> Payload { get; internal set; }

        /// <summary>The Ice protocol of this frame.</summary>
        public Protocol Protocol { get; }

        /// <summary>The frame byte count</summary>
        public int Size { get; private set; }

        // Ice2 compression status, is the first byte of the encaps payload
        internal byte CompressionStatus
        {
            get
            {
                Debug.Assert(Encoding == Encoding.V2_0);
                int sizeLength = 1 << (Payload[0] & 0x03);
                return Payload[sizeLength + 2];
            }
        }

        private readonly int _sizeMax;

        /// <summary>Decompress the encapsulation payload if it is compressed. Compressed encapsulation are
        /// only supported with 2.0 encoding.</summary>
        public void DecompressPayload()
        {
            if (Encoding == Encoding.V2_0 && CompressionStatus == 1)
            {
                // TODO should this throw if encoding is not V2_0 or the frame is not compressed?
                ReadOnlySpan<byte> buffer = Payload.AsReadOnlySpan();
                (int size, int sizeLength) = buffer.ReadSize20();
                (int decompressedSize, int decompressedSizeLength) = buffer.Slice(sizeLength + 3).ReadSize20();
                if (decompressedSize > _sizeMax)
                {
                    throw new InvalidDataException(@$"decompressed size of {decompressedSize
                                                   } bytes is greater than Ice.IncomingFrameSizeMax value");
                }

                // Offset of the start of the GZip decompressed data
                int offset = sizeLength + 3;
                byte[] decompressedData = new byte[offset + decompressedSize];
                buffer.Slice(0, offset).CopyTo(decompressedData);
                // Set the compression status to '0' not-compressed
                decompressedData[sizeLength + 2] = 0;

                using var decompressedStream = new MemoryStream(decompressedData,
                                                                offset,
                                                                decompressedData.Length - offset);
                offset += decompressedSizeLength;

                Debug.Assert(Payload.Array != null);
                var compressed = new GZipStream(new MemoryStream(Payload.Array,
                                                                 Payload.Offset + offset,
                                                                 Payload.Count - offset),
                                                CompressionMode.Decompress);
                compressed.CopyTo(decompressedStream);
                if (decompressedStream.Position != decompressedSize)
                {
                    throw new InvalidDataException(
                        @$"received gzip compressed encapsulation with a decompressed size of only {
                        decompressedStream.Position} bytes");
                }
                Payload = decompressedData;
                // Rewrite the encapsulation size and adjust the frame size
                int newSize = Payload.Count - sizeLength;
                OutputStream.WriteSize20(newSize, decompressedData.AsSpan(0, sizeLength));
                Size += newSize - size;
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
