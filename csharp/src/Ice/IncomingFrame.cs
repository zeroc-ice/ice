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
        /// <summary>The binary context is a collection of byte blobs with an associated int key, the dispatch and
        /// invocation interceptors can use the binary context to marshal arbitrary data within a request.</summary>
        public IReadOnlyDictionary<int, ReadOnlyMemory<byte>> BinaryContext
        {
            get
            {
                if (Protocol == Protocol.Ice1)
                {
                    return ImmutableDictionary<int, ReadOnlyMemory<byte>>.Empty;
                }

                if (_binaryContext == null)
                {
                    ArraySegment<byte> buffer = Data.Slice(Payload.Offset + Payload.Count - Data.Offset);
                    if (buffer.Count > 0)
                    {
                        var istr = new InputStream(buffer, Encoding.V20);
                        int dictionarySize = istr.ReadSize();
                        var binaryContext = new Dictionary<int, ReadOnlyMemory<byte>>(dictionarySize);
                        for (int i = 0; i < dictionarySize; ++i)
                        {
                            int key = istr.ReadVarInt();
                            int entrySize = istr.ReadSize();
                            binaryContext[key] = buffer.AsReadOnlyMemory(istr.Pos, entrySize);
                            istr.Skip(entrySize);
                        }

                        _binaryContext = binaryContext;
                    }
                    else
                    {
                        _binaryContext = ImmutableDictionary<int, ReadOnlyMemory<byte>>.Empty;
                    }
                }
                return _binaryContext;
            }
        }
        /// <summary>The encoding of the frame payload.</summary>
        public abstract Encoding Encoding { get; }
        /// <summary>True if the encapsulation has a compressed payload, false otherwise.</summary>
        public bool HasCompressedPayload { get; private protected set; }

        /// <summary>The payload of this frame. The bytes inside the data should not be written to;
        /// they are writable because of the <see cref="System.Net.Sockets.Socket"/> methods for sending.</summary>
        public ArraySegment<byte> Payload { get; private protected set; }

        /// <summary>The Ice protocol of this frame.</summary>
        public Protocol Protocol { get; }

        /// <summary>The frame byte count.</summary>
        public int Size => Data.Count;

        /// <summary>The frame data.</summary>
        internal ArraySegment<byte> Data { get; set; }

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
                ArraySegment<byte> encapsulation = GetEncapsulation();

                ReadOnlySpan<byte> buffer = encapsulation.AsReadOnlySpan();
                (int size, int sizeLength, Encoding _) = buffer.ReadEncapsulationHeader(Protocol.GetEncoding());

                // Read the decompressed size that is written after the compression status byte when the payload is
                // compressed +3 corresponds to (Encoding 2 bytes, Compression status 1 byte)
                (int decompressedSize, int decompressedSizeLength) = buffer.Slice(sizeLength + 3).ReadSize20();

                if (decompressedSize > _sizeMax)
                {
                    throw new InvalidDataException(@$"decompressed size of {decompressedSize
                                                   } bytes is greater than the configured IncomingFrameSizeMax value");
                }

                // We are going to replace the Data segment with a new Data segment/array that contains a decompressed
                // encapsulation.
                byte[] decompressedData = new byte[Data.Count - size + decompressedSize];

                // Index of the start of the GZip data in Data
                int gzipIndex = encapsulation.Offset - Data.Offset + sizeLength + 3;

                // Copy the data before the encapsulation to the new buffer
                Data.AsSpan(0, gzipIndex).CopyTo(decompressedData);

                // Copy the binary context (if any) after the encapsulation
                Data.AsSpan(Payload.Offset + Payload.Count - Data.Offset).CopyTo(
                    decompressedData.AsSpan(gzipIndex + decompressedSize - 3));

                // Set the compression status to '0' not-compressed
                decompressedData[gzipIndex - 1] = 0;

                using var decompressedStream = new MemoryStream(decompressedData,
                                                                gzipIndex,
                                                                decompressedData.Length - gzipIndex);
                Debug.Assert(encapsulation.Array != null);
                using var compressed = new GZipStream(
                    new MemoryStream(encapsulation.Array,
                                     encapsulation.Offset + sizeLength + 3 + decompressedSizeLength,
                                     encapsulation.Count - sizeLength - 3 - decompressedSizeLength),
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

                Payload = new ArraySegment<byte>(decompressedData,
                                                 Payload.Offset - Data.Offset,
                                                 Payload.Count - size + decompressedSize);

                Data = decompressedData;
                // Rewrite the encapsulation size
                GetEncapsulation().AsSpan(0, sizeLength).WriteEncapsulationSize(decompressedSize,
                                                                                Protocol.GetEncoding());
                HasCompressedPayload = false;
            }
        }

        /// <summary>Constructs a new <see cref="IncomingFrame"/>.</summary>
        /// <param name="data">The frame data.</param>
        /// <param name="protocol">The frame protocol.</param>
        /// <param name="sizeMax">The maximum payload size, checked during decompression.</param>
        protected IncomingFrame(ArraySegment<byte> data, Protocol protocol, int sizeMax)
        {
            Data = data;
            Protocol = protocol;
            _sizeMax = sizeMax;
        }

        // Returns the subset of Payload that represents the encapsulation.
        private protected abstract ArraySegment<byte> GetEncapsulation();
    }
}
