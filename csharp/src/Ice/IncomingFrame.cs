//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
        public IReadOnlyDictionary<int, ReadOnlyMemory<byte>> BinaryContext
        {
            get
            {
                if (Protocol == Protocol.Ice1)
                {
                    throw new NotSupportedException("binary context is not supported with Ice1 protocol");
                }

                if (_binaryContext == null)
                {
                    int binaryContextSize = Payload.Count - Encapsulation.Count - (Encapsulation.Offset - Payload.Offset);
                    if (binaryContextSize > 0)
                    {
                        int offset = Payload.Count - binaryContextSize;
                        var istr = new InputStream(Payload.Slice(offset), Encoding.V2_0);
                        int entriesSize = istr.ReadSize();
                        var binaryContext = new Dictionary<int, ReadOnlyMemory<byte>>();
                        for (int i = 0; i < entriesSize; ++i)
                        {
                            int key = istr.ReadInt();
                            int entrySize = istr.ReadSize();
                            binaryContext[key] = Payload.AsReadOnlyMemory(offset + istr.Pos, entrySize);
                            istr.Skip(entrySize);
                        }
                        _binaryContext = binaryContext.ToImmutableDictionary();
                    }
                }
                return _binaryContext ?? ImmutableDictionary<int, ReadOnlyMemory<byte>>.Empty;
            }
        }
        /// <summary>The encoding of the frame payload.</summary>
        public abstract Encoding Encoding { get; }
        /// <summary>True if the encapsulation has a compressed payload, false otherwise.</summary>
        public bool HasCompressedPayload { get; protected set; }

        /// <summary>The data of this frame. The bytes inside the data should not be written to;
        /// they are writable because of the <see cref="System.Net.Sockets.Socket"/> methods for sending.</summary>
        // TODO: describe how long this payload remains valid once we add memory pooling.
        public ArraySegment<byte> Payload { get; protected set; }

        /// <summary>The Ice protocol of this frame.</summary>
        public Protocol Protocol { get; }

        /// <summary>The frame byte count</summary>
        public int Size => Payload.Count;

        // If the frame payload contains an encapsulation, this segment corresponds to the frame encapsulation
        // otherwise is an empty segment. This is always an Slice of the Payload, both must use the same array.
        private protected ArraySegment<byte> Encapsulation { get; set; }

        private IReadOnlyDictionary<int, ReadOnlyMemory<byte>>? _binaryContext;

        private readonly int _sizeMax;

        /// <summary>Decompress the encapsulation payload if it is compressed. Compressed encapsulations are
        /// only supported with 2.0 encoding.</summary>
        public void DecompressPayload()
        {
            if (!HasCompressedPayload)
            {
                throw new InvalidOperationException("the payload is not compressed");
            }
            else
            {
                ReadOnlySpan<byte> buffer = Encapsulation.AsReadOnlySpan();
                (int size, int sizeLength, Encoding _) = buffer.ReadEncapsulationHeader(Protocol.GetEncoding());
                // Offset of the start of the GZip decompressed data +3 corresponds to (Encoding 2 bytes, Compression
                // status 1 byte)
                int offset = sizeLength + 3;

                // Read the decompressed size that is written after the compression status byte when the payload is
                // compressed.
                (int decompressedSize, int decompressedSizeLength) = buffer.Slice(offset).ReadSize20();
                if (decompressedSize > _sizeMax)
                {
                    throw new InvalidDataException(@$"decompressed size of {decompressedSize
                                                   } bytes is greater than the configured IncomingFrameSizeMax value");
                }

                Debug.Assert(Payload.Array == Encapsulation.Array);

                byte[] decompressedData = new byte[Payload.Count - size + decompressedSize];
                int encapsulationOffset = Encapsulation.Offset - Payload.Offset;
                Payload.AsSpan(0, encapsulationOffset + offset).CopyTo(decompressedData);
                // Set the compression status to '0' not-compressed
                decompressedData[encapsulationOffset + sizeLength + 2] = 0;

                using var decompressedStream = new MemoryStream(decompressedData,
                                                                encapsulationOffset + offset,
                                                                decompressedData.Length - encapsulationOffset - offset);
                Debug.Assert(Encapsulation.Array != null);
                var compressed = new GZipStream(
                    new MemoryStream(Encapsulation.Array,
                                     Encapsulation.Offset + offset + decompressedSizeLength,
                                     Encapsulation.Count - offset - decompressedSizeLength),
                    CompressionMode.Decompress);
                compressed.CopyTo(decompressedStream);
                // +3 corresponds to (Encoding 2 bytes and Compression status 1 byte), that are part of the
                // decompressed size, but are not GZip compressed.
                if (decompressedStream.Position + 3 != decompressedSize)
                {
                    throw new InvalidDataException(
                        @$"received GZip compressed payload with a decompressed size of only {
                        decompressedStream.Position} bytes {decompressedSize}");
                }

                // Copy the data after the encapsulation and invalidate the cached binary context dictionary
                Payload.AsSpan(encapsulationOffset + Encapsulation.Count).CopyTo(
                    decompressedData.AsSpan(encapsulationOffset + decompressedSize + sizeLength));
                _binaryContext = null;

                Payload = new ArraySegment<byte>(decompressedData,
                                                 0,
                                                 Payload.Count - size + decompressedSize);

                Encapsulation = new ArraySegment<byte>(decompressedData,
                                                       encapsulationOffset,
                                                       decompressedSize + sizeLength);
                // Rewrite the encapsulation size
                OutputStream.WriteEncapsulationSize(decompressedSize,
                                                    Encapsulation.AsSpan(0, sizeLength),
                                                    Protocol.GetEncoding());
                HasCompressedPayload = false;
            }
        }

        protected IncomingFrame(Protocol protocol, ArraySegment<byte> payload, int sizeMax)
        {
            Protocol = protocol;
            Payload = payload;
            _sizeMax = sizeMax;
        }
    }
}
