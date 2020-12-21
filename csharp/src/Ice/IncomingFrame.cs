// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.IO.Compression;

namespace ZeroC.Ice
{
    /// <summary>Base class for incoming frames.</summary>
    public abstract class IncomingFrame
    {
        /// <summary>Returns the binary context of this frame.</summary>
        public abstract IReadOnlyDictionary<int, ReadOnlyMemory<byte>> BinaryContext { get; }

        /// <summary>Returns true when the payload is compressed; otherwise, returns false.</summary>
        public bool HasCompressedPayload => PayloadCompressionFormat != CompressionFormat.Decompressed;

        /// <summary>The payload of this frame. The bytes inside the data should not be written to;
        /// they are writable because of the <see cref="System.Net.Sockets.Socket"/> methods for sending.</summary>
        public ArraySegment<byte> Payload { get; private protected set; }

        /// <summary>Returns the payload's compression format.</summary>
        public CompressionFormat PayloadCompressionFormat { get; private protected set; }

        /// <summary>Returns the number of bytes in the payload.</summary>
        /// <remarks>Provided for consistency with <see cref="OutgoingFrame.PayloadSize"/>.</remarks>
        public int PayloadSize => Payload.Count;

        /// <summary>Returns the encoding of the payload of this frame.</summary>
        /// <remarks>The header of the frame is always encoded using the frame protocol's encoding.</remarks>
        public abstract Encoding PayloadEncoding { get; }

        /// <summary>The Ice protocol of this frame.</summary>
        public Protocol Protocol { get; }

        private readonly int _maxSize;

        /// <summary>Decompresses the encapsulation payload if it is compressed. Compressed encapsulations are only
        /// supported with the 2.0 encoding.</summary>
        public void DecompressPayload()
        {
            if (PayloadCompressionFormat == CompressionFormat.Decompressed)
            {
                throw new InvalidOperationException("the encapsulation's payload is not compressed");
            }
            else if (PayloadCompressionFormat != CompressionFormat.GZip)
            {
                throw new NotSupportedException($"cannot decompress compression format `{PayloadCompressionFormat}'");
            }
            else
            {
                int encapsulationOffset = this is IncomingResponseFrame ? 1 : 0;

                ReadOnlySpan<byte> buffer = Payload.Slice(encapsulationOffset);
                int sizeLength = Protocol == Protocol.Ice2 ? buffer[0].ReadSizeLength20() : 4;

                // Read the decompressed size that is written after the compression status byte when the payload is
                // compressed +3 corresponds to (Encoding 2 bytes, Compression status 1 byte)
                (int decompressedSize, int decompressedSizeLength) = buffer.Slice(sizeLength + 3).ReadSize20();

                if (decompressedSize > _maxSize)
                {
                    throw new InvalidDataException(
                        @$"decompressed size of {decompressedSize
                        } bytes is greater than the configured IncomingFrameMaxSize value ({_maxSize} bytes)");
                }

                // We are going to replace the Payload segment with a new Payload segment/array that contains a
                // decompressed encapsulation.
                byte[] decompressedPayload = new byte[encapsulationOffset + decompressedSizeLength + decompressedSize];

                // Write the result type and the encapsulation header "by hand" in decompressedPayload.
                if (encapsulationOffset == 1)
                {
                    decompressedPayload[0] = Payload[0]; // copy the result type.
                }

                decompressedPayload.AsSpan(encapsulationOffset, decompressedSizeLength).WriteEncapsulationSize(
                    decompressedSize,
                    Protocol.GetEncoding());

                int compressedIndex = encapsulationOffset + sizeLength;
                int decompressedIndex = encapsulationOffset + decompressedSizeLength;

                // Keep same encoding
                decompressedPayload[decompressedIndex++] = Payload[compressedIndex++];
                decompressedPayload[decompressedIndex++] = Payload[compressedIndex++];

                // Set the payload's compression format to Decompressed.
                decompressedPayload[decompressedIndex++] = (byte)CompressionFormat.Decompressed;

                // Verify PayloadCompressionFormat was set correctly.
                Debug.Assert(Payload[compressedIndex] == (byte)CompressionFormat.GZip);

                using var decompressedStream = new MemoryStream(decompressedPayload,
                                                                decompressedIndex,
                                                                decompressedPayload.Length - decompressedIndex);

                // Skip compression status and decompressed size in compressed payload.
                compressedIndex += 1 + decompressedSizeLength;

                Debug.Assert(Payload.Array != null);
                using var compressed = new GZipStream(
                    new MemoryStream(Payload.Array, Payload.Offset + compressedIndex, Payload.Count - compressedIndex),
                    CompressionMode.Decompress);
                compressed.CopyTo(decompressedStream);

                // "3" corresponds to (Encoding 2 bytes and Compression status 1 byte), that are part of the
                // decompressedSize but are not GZip compressed.
                if (decompressedStream.Position + 3 != decompressedSize)
                {
                    throw new InvalidDataException(
                        @$"received GZip compressed payload with a decompressed size of only {decompressedStream.
                        Position + 3} bytes; expected {decompressedSize} bytes");
                }

                Payload = decompressedPayload;
                PayloadCompressionFormat = CompressionFormat.Decompressed;
            }
        }

        /// <summary>Constructs a new <see cref="IncomingFrame"/>.</summary>
        /// <param name="protocol">The frame protocol.</param>
        /// <param name="maxSize">The maximum payload size, checked during decompression.</param>
        protected IncomingFrame(Protocol protocol, int maxSize)
        {
            Protocol = protocol;
            _maxSize = maxSize;
        }
    }
}
