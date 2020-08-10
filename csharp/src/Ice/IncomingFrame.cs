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
                    return ImmutableDictionary<int, ReadOnlyMemory<byte>>.Empty;
                }

                if (_binaryContext == null)
                {
                    ArraySegment<byte> buffer = Data.Slice(Payload.Count);
                    if (buffer.Count > 0)
                    {
                        var istr = new InputStream(buffer, Encoding.V2_0);
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
        public bool HasCompressedPayload { get; protected set; }

        /// <summary>The payload of this frame. The bytes inside the data should not be written to;
        /// they are writable because of the <see cref="System.Net.Sockets.Socket"/> methods for sending.</summary>
        // TODO: describe how long this payload remains valid once we add memory pooling.
        public ArraySegment<byte> Payload { get; private protected set; }

        /// <summary>The Ice protocol of this frame.</summary>
        public Protocol Protocol { get; }

        /// <summary>The frame byte count</summary>
        public int Size => Payload.Count;

        /// <summary>The frame data</summary>
        internal ArraySegment<byte> Data { get; set; }

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

                // Read the decompressed size that is written after the compression status byte when the payload is
                // compressed.
                (int decompressedSize, int decompressedSizeLength) = buffer.Slice(sizeLength + 3).ReadSize20();
                if (decompressedSize > _sizeMax)
                {
                    throw new InvalidDataException(@$"decompressed size of {decompressedSize
                                                   } bytes is greater than the configured IncomingFrameSizeMax value");
                }

                Debug.Assert(Payload.Array == Encapsulation.Array);
                byte[] decompressedData = new byte[Data.Count - size + decompressedSize];

                // Offset of the start of the GZip data
                int gzipOffset = sizeLength + 3 + (Encapsulation.Offset - Data.Offset);
                // Copy the uncompressed data before the encapsulation to the new buffer
                Data.AsSpan(0, gzipOffset).CopyTo(decompressedData);
                // Copy the binary context if any after the encapsulation
                Data.AsSpan(Payload.Count).CopyTo(decompressedData.AsSpan(gzipOffset + decompressedSize - 3));

                // Set the compression status to '0' not-compressed
                decompressedData[gzipOffset - 1] = 0;

                using var decompressedStream = new MemoryStream(decompressedData,
                                                                gzipOffset,
                                                                decompressedData.Length - gzipOffset);
                Debug.Assert(Encapsulation.Array != null);
                var compressed = new GZipStream(
                    new MemoryStream(Encapsulation.Array,
                                     Encapsulation.Offset + sizeLength + 3 + decompressedSizeLength,
                                     Encapsulation.Count - sizeLength - 3 - decompressedSizeLength),
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

                Payload = new ArraySegment<byte>(decompressedData, 0, gzipOffset + decompressedSize - 3);

                Encapsulation = new ArraySegment<byte>(decompressedData,
                                                       gzipOffset - sizeLength - 3,
                                                       decompressedSize + sizeLength);
                Data = decompressedData;
                // Rewrite the encapsulation size
                OutputStream.WriteEncapsulationSize(decompressedSize,
                                                    Encapsulation.AsSpan(0, sizeLength),
                                                    Protocol.GetEncoding());
                HasCompressedPayload = false;
            }
        }

        protected IncomingFrame(ArraySegment<byte> data, Protocol protocol, int sizeMax)
        {
            Data = data;
            Protocol = protocol;
            _sizeMax = sizeMax;
        }
    }
}
