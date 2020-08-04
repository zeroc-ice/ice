//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;

namespace ZeroC.Ice
{
    /// <summary>Indicates the result of the <see cref="OutgoingFrame.CompressPayload"/> operation.</summary>
    public enum CompressionResult
    {
        /// <summary>The payload was successfully compressed.</summary>
        Success,
        /// <summary>The payload size is smaller than the configured compression threshold.</summary>
        PayloadTooSmall,
        /// <summary>The payload was not compressed, compressing it would increase its size.</summary>
        PayloadNotCompressible
    }

    /// <summary>Base class for outgoing frames.</summary>
    public abstract class OutgoingFrame
    {
        /// <summary>Returns a list of array segments with the contents of the frame encapsulation, if the frame
        /// doesn't contain an encapsulation it returns an empty list.</summary>
        public IList<ArraySegment<byte>> Encapsulation
        {
            get
            {
                if (_encapsulation == null && EncapsulationEnd is OutputStream.Position encapsulationEnd)
                {
                    var encapsulation = new List<ArraySegment<byte>>();
                    if (EncapsulationStart.Segment == encapsulationEnd.Segment)
                    {
                        encapsulation.Add(Payload[EncapsulationStart.Segment].Slice(
                            EncapsulationStart.Offset,
                            encapsulationEnd.Offset - EncapsulationStart.Offset));
                    }
                    else
                    {
                        ArraySegment<byte> segment = Payload[EncapsulationStart.Segment].Slice(
                            EncapsulationStart.Offset);
                        if (segment.Count > 0)
                        {
                            encapsulation.Add(segment);
                        }
                        for (int i = EncapsulationStart.Segment + 1; i < encapsulationEnd.Segment; i++)
                        {
                            encapsulation.Add(Payload[i]);
                        }

                        segment = Payload[encapsulationEnd.Segment].Slice(0, encapsulationEnd.Offset);
                        if (segment.Count > 0)
                        {
                            encapsulation.Add(segment);
                        }
                    }
                    _encapsulation = encapsulation;
                }
                return _encapsulation ?? Array.Empty<ArraySegment<byte>>();
            }
        }
        /// <summary>The encoding of the frame payload.</summary>
        public Encoding Encoding { get; protected set; }
        /// <summary>True for a sealed frame, false otherwise, a sealed frame does not change its contents.</summary>
        public bool IsSealed { get; private protected set; }
        /// <summary>Returns a list of array segments with the contents of the frame payload.</summary>
        public List<ArraySegment<byte>> Payload { get; }
        /// <summary>The Ice protocol of this frame.</summary>
        public Protocol Protocol { get; }

        /// <summary>The frame byte count.</summary>
        public int Size { get; private protected set; }

        // True if Ice1 frames should use protocol compression, false otherwise.
        internal bool Compress { get; }

        // Position of the end of the encapsulation, for ice1 this is always the frame end.
        internal OutputStream.Position? EncapsulationEnd;
        // Position of the start of the encapsulation.
        internal OutputStream.Position EncapsulationStart;

        private readonly CompressionLevel _compressionLevel;
        private readonly int _compressionMinSize;

        private IList<ArraySegment<byte>>? _encapsulation;

        /// <summary>Compress the encapsulation payload using GZip compression, compressed encapsulation payload is
        /// only supported with 2.0 encoding.</summary>
        /// <returns>A <see cref="CompressionResult"/> value indicating the result of the compression operation.
        /// </returns>
        public CompressionResult CompressPayload()
        {
            if (EncapsulationEnd == null)
            {
                throw new InvalidOperationException("payload has not been written");
            }

            if (Encoding != Encoding.V2_0)
            {
                throw new InvalidOperationException("compressed payload are only supported with 2.0 encoding");
            }
            else
            {
                IList<ArraySegment<byte>> payload = Encapsulation;
                int sizeLength = Protocol == Protocol.Ice2 ? 1 << (payload[0][0] & 0x03) : 4;
                byte compressionStatus = payload[0].Count > sizeLength + 2 ?
                    payload[0][sizeLength + 2] : payload[1][sizeLength + 2 - payload[0].Count];

                if (compressionStatus != 0)
                {
                    throw new InvalidOperationException("payload is already compressed");
                }

                int payloadSize = payload.GetByteCount();
                if (payloadSize < _compressionMinSize)
                {
                    return CompressionResult.PayloadTooSmall;
                }
                // Reserve memory for the compressed data, this should never be greater than the uncompressed data
                // otherwise we will just send the uncompressed data.
                byte[] compressedData = new byte[payloadSize];
                // Write the encapsulation header
                int offset = sizeLength;
                compressedData[offset++] = Encoding.Major;
                compressedData[offset++] = Encoding.Minor;
                // Set the compression status to '1' GZip compressed
                compressedData[offset++] = 1;
                // Write the size of the uncompressed data
                OutputStream.WriteSize20(payloadSize - sizeLength, compressedData.AsSpan(offset, sizeLength));
                offset += sizeLength;
                using var memoryStream = new MemoryStream(compressedData, offset, compressedData.Length - offset);
                var gzipStream = new GZipStream(memoryStream,
                    _compressionLevel == CompressionLevel.Fastest ? System.IO.Compression.CompressionLevel.Fastest :
                                                                    System.IO.Compression.CompressionLevel.Optimal);
                try
                {
                    // The data to compress starts after the compression status byte, +3 corresponds to (Encoding 2
                    // bytes, Compression status 1 byte)
                    gzipStream.Write(payload[0].Slice(sizeLength + 3));
                    for (int i = 1; i < payload.Count; ++i)
                    {
                        gzipStream.Write(payload[i]);
                    }
                    gzipStream.Flush();
                }
                catch (NotSupportedException)
                {
                    // If the data doesn't fit in the memory stream NotSupportedException is thrown when GZipStream
                    // try to expand the fixed size MemoryStream.
                    return CompressionResult.PayloadNotCompressible;
                }

                // Slice the payload start segment and remove all segments after it, the compressed payload will be
                // added as a new segment.
                int remove = EncapsulationStart.Segment;
                if (EncapsulationStart.Offset > 0)
                {
                    Payload[EncapsulationStart.Segment] = Payload[EncapsulationStart.Segment].Slice(0, EncapsulationStart.Offset);
                    remove++;
                }
                // TODO return the segments to the pool when we implement memory pool
                Payload.RemoveRange(remove, Payload.Count - remove);

                var payloadData = new ArraySegment<byte>(compressedData, 0, offset + (int)memoryStream.Position);
                Payload.Add(payloadData);
                EncapsulationStart = new OutputStream.Position(Payload.Count - 1, 0);
                EncapsulationEnd = new OutputStream.Position(Payload.Count - 1, payloadData.Count);
                Size = Payload.GetByteCount();

                // Rewrite the payload size
                OutputStream.WriteEncapsulationSize(payloadData.Count - sizeLength,
                                             payloadData.AsSpan(0, sizeLength),
                                             Protocol.GetEncoding());
                _encapsulation = null;
                return CompressionResult.Success;
            }
        }

        private protected OutgoingFrame(
            Protocol protocol,
            Encoding encoding,
            bool compress,
            CompressionLevel compressionLevel,
            int compressionMinSize,
            List<ArraySegment<byte>> payload)
        {
            Protocol = protocol;
            Protocol.CheckSupported();
            Encoding = encoding;
            Payload = payload;
            Compress = compress;
            _compressionLevel = compressionLevel;
            _compressionMinSize = compressionMinSize;
        }

        internal void Finish(OutputStream.Position encapsulationEnd)
        {
            Size = Payload.GetByteCount();
            EncapsulationEnd = encapsulationEnd;
        }
    }
}
