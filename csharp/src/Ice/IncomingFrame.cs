// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
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

        /// <summary>Returns true this payload holds a compressed encapsulation payload; otherwise, returns false.
        /// </summary>
        public bool HasCompressedPayload { get; private protected set; }

        /// <summary>The payload of this frame. The bytes inside the data should not be written to;
        /// they are writable because of the <see cref="System.Net.Sockets.Socket"/> methods for sending.</summary>
        public ArraySegment<byte> Payload { get; private protected set; }

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
        /// supported with 2.0 encoding.</summary>
        public void DecompressPayload()
        {
            if (!HasCompressedPayload)
            {
                throw new InvalidOperationException("the encapsulation's payload is not compressed");
            }
            else
            {
                int encapsulationOffset = this is IncomingResponseFrame ? 1 : 0;

                ReadOnlySpan<byte> buffer = Payload.Slice(encapsulationOffset);
                (int size, int sizeLength, Encoding _) = buffer.ReadEncapsulationHeader(Protocol.GetEncoding());

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
                byte[] decompressedPayload = new byte[Payload.Count - size + decompressedSize];

                // Index of the start of the GZip data in Payload
                int gzipIndex = encapsulationOffset + sizeLength + 3;

                // Copy the data before the encapsulation to the new buffer
                Payload.AsSpan(0, gzipIndex).CopyTo(decompressedPayload);

                // Set the compression status to '0' not-compressed
                decompressedPayload[gzipIndex - 1] = 0;

                using var decompressedStream = new MemoryStream(decompressedPayload,
                                                                gzipIndex,
                                                                decompressedPayload.Length - gzipIndex);
                Debug.Assert(Payload.Array != null);
                using var compressed = new GZipStream(
                    new MemoryStream(Payload.Array,
                                     Payload.Offset + gzipIndex + decompressedSizeLength,
                                     Payload.Count - gzipIndex - decompressedSizeLength),
                    CompressionMode.Decompress);
                compressed.CopyTo(decompressedStream);
                // +3 corresponds to (Encoding 2 bytes and Compression status 1 byte), that are part of the
                // decompressed size, but are not GZip compressed.
                if (decompressedStream.Position + 3 != decompressedSize)
                {
                    throw new InvalidDataException(
                        @$"received GZip compressed payload with a decompressed size of only {decompressedStream.
                        Position + 3} bytes; expected {decompressedSize} bytes");
                }

                Payload = decompressedPayload;

                // Rewrite the encapsulation size
                Payload.AsSpan(encapsulationOffset, sizeLength).WriteEncapsulationSize(decompressedSize,
                                                                                       Protocol.GetEncoding());
                HasCompressedPayload = false;
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
