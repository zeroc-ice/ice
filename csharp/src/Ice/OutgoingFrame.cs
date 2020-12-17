// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
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
        public Dictionary<int, Action<OutputStream>> BinaryContextOverride
        {
            get
            {
                if (_binaryContextOverride == null)
                {
                    if (Protocol == Protocol.Ice1)
                    {
                        throw new NotSupportedException("ice1 does not support binary contexts");
                    }

                    _binaryContextOverride = new Dictionary<int, Action<OutputStream>>();
                }
                return _binaryContextOverride;
            }
        }

        /// <summary>Returns the encoding of the payload of this frame.</summary>
        /// <remarks>The header of the frame is always encoded using the frame protocol's encoding.</remarks>
        public abstract Encoding Encoding { get; }

        public bool HasCompressedPayload { get; private set; }

        public abstract IReadOnlyDictionary<int, ReadOnlyMemory<byte>> InitialBinaryContext { get; }

        /// <summary>True for a sealed frame, false otherwise, a sealed frame does not change its contents.</summary>
        public bool IsSealed { get; private protected set; }

        /// <summary>Returns the payload of this frame.</summary>
        public IList<ArraySegment<byte>> Payload { get; } = new List<ArraySegment<byte>>();

        /// <summary>The Ice protocol of this frame.</summary>
        public Protocol Protocol { get; }

        /// <summary>The frame byte count.</summary>
        public int Size
        {
            get
            {
                if (_payloadSize == -1)
                {
                    _payloadSize = Payload.GetByteCount();
                }
                return _payloadSize;
            }
        }

        // True if Ice1 frames should use protocol compression, false otherwise.
        internal bool Compress { get; }

        /// <summary>The stream data writer if the request or response has an outgoing stream param. The writer is
        /// called after the request or response frame is sent over a socket stream.</summary>
        internal Action<SocketStream>? StreamDataWriter { get; set; }

        private Dictionary<int, Action<OutputStream>>? _binaryContextOverride;

        private readonly CompressionLevel _compressionLevel;
        private readonly int _compressionMinSize;

        private int _payloadSize = -1; // -1 means not initialized

        /// <summary>Compresses the encapsulation payload using GZip compression. Compressed encapsulation payload is
        /// only supported with the 2.0 encoding.</summary>
        /// <returns>A <see cref="CompressionResult"/> value indicating the result of the compression operation.
        /// </returns>
        public CompressionResult CompressPayload()
        {
            if (IsSealed)
            {
                throw new InvalidOperationException("cannot modify a sealed frame");
            }

            if (Encoding != Encoding.V20)
            {
                throw new NotSupportedException("payload compression is only supported with 2.0 encoding");
            }
            else
            {
                int encapsulationOffset = this is OutgoingResponseFrame ? 1 : 0;

                // The encapsulation always starts in the first segment of the payload (at position 0 or 1).
                Debug.Assert(encapsulationOffset < Payload[0].Count);

                int sizeLength = Protocol == Protocol.Ice2 ? Payload[0][encapsulationOffset].ReadSizeLength20() : 4;
                byte compressionStatus = Payload.GetByte(encapsulationOffset + sizeLength + 2);

                if (compressionStatus != 0)
                {
                    throw new InvalidOperationException("payload is already compressed");
                }

                int encapsulationSize = Payload.GetByteCount() - encapsulationOffset; // this includes the size length
                if (encapsulationSize < _compressionMinSize)
                {
                    return CompressionResult.PayloadTooSmall;
                }
                // Reserve memory for the compressed data, this should never be greater than the uncompressed data
                // otherwise we will just send the uncompressed data.
                byte[] compressedData = new byte[encapsulationOffset + encapsulationSize];
                // Copy the byte before the encapsulation, if any
                if (encapsulationOffset == 1)
                {
                    compressedData[0] = Payload[0][0];
                }
                // Write the encapsulation header
                int offset = encapsulationOffset + sizeLength;
                compressedData[offset++] = Encoding.Major;
                compressedData[offset++] = Encoding.Minor;
                // Set the compression status to '1' GZip compressed
                compressedData[offset++] = 1;
                // Write the size of the uncompressed data
                compressedData.AsSpan(offset, sizeLength).WriteFixedLengthSize20(encapsulationSize - sizeLength);

                offset += sizeLength;
                using var memoryStream = new MemoryStream(compressedData, offset, compressedData.Length - offset);
                using var gzipStream = new GZipStream(
                    memoryStream,
                    _compressionLevel == CompressionLevel.Fastest ? System.IO.Compression.CompressionLevel.Fastest :
                                                                    System.IO.Compression.CompressionLevel.Optimal);
                try
                {
                    // The data to compress starts after the compression status byte, + 3 corresponds to (Encoding 2
                    // bytes, Compression status 1 byte)
                    foreach (ArraySegment<byte> segment in Payload.Slice(encapsulationOffset + sizeLength + 3))
                    {
                        gzipStream.Write(segment);
                    }
                    gzipStream.Flush();
                }
                catch (NotSupportedException)
                {
                    // If the data doesn't fit in the memory stream NotSupportedException is thrown when GZipStream
                    // try to expand the fixed size MemoryStream.
                    return CompressionResult.PayloadNotCompressible;
                }

                Payload.Clear();
                offset += (int)memoryStream.Position;
                Payload.Add(new ArraySegment<byte>(compressedData, 0, offset));
                _payloadSize = -1; // reset cached value

                // Rewrite the encapsulation size
                compressedData.AsSpan(encapsulationOffset, sizeLength).WriteEncapsulationSize(
                    offset - sizeLength - encapsulationOffset,
                    Protocol.GetEncoding());

                HasCompressedPayload = true;

                return CompressionResult.Success;
            }
        }

        /// <summary>Gets or builds a combined binary context using InitialBinaryContext and _binaryContextOverride.
        /// This method is used for colocated calls.</summary>
        internal IReadOnlyDictionary<int, ReadOnlyMemory<byte>> GetBinaryContext()
        {
            if (_binaryContextOverride == null)
            {
                return InitialBinaryContext;
            }
            else
            {
                // Need to marshal/unmarshal this binary context
                var buffer = new List<ArraySegment<byte>>();
                var ostr = new OutputStream(Encoding.V20, buffer);
                WriteBinaryContext(ostr);
                _ = ostr.Finish();
                return buffer.AsArraySegment().AsReadOnlyMemory().Read(istr => istr.ReadBinaryContext());
            }
        }

        /// <summary>Writes the header of a frame. This header does not include the frame's prologue.</summary>
        /// <param name="ostr">The output stream.</param>
        /// <remarks>The preferred public method is <see cref="OutgoingFrameHelper.WriteHeader"/>.</remarks>
        internal abstract void WriteHeader(OutputStream ostr);

        private protected OutgoingFrame(
            Protocol protocol,
            bool compress,
            CompressionLevel compressionLevel,
            int compressionMinSize)
        {
            Protocol = protocol;
            Protocol.CheckSupported();
            Compress = compress;
            _compressionLevel = compressionLevel;
            _compressionMinSize = compressionMinSize;
        }

        // Finish prepares the frame for sending and adjusts the last written segment to match the offset of the written
        // data. If the frame contains a binary context, Finish appends the entries from defaultBinaryContext (if any)
        // and rewrites the binary context dictionary size.
        internal virtual void Finish()
        {
            if (!IsSealed)
            {
                IsSealed = true;
            }
        }

        private protected void WriteBinaryContext(OutputStream ostr)
        {
            Debug.Assert(Protocol == Protocol.Ice2);
            Debug.Assert(ostr.Encoding == Encoding.V20);

            int sizeLength = InitialBinaryContext.Count + (_binaryContextOverride?.Count ?? 0) < 64 ? 1 : 2;

            int size = 0;

            OutputStream.Position start = ostr.StartFixedLengthSize(sizeLength);

            // First write the overrides, then the InitialBinaryContext entries that were not overridden.

            if (_binaryContextOverride is Dictionary<int, Action<OutputStream>> binaryContextOverride)
            {
                foreach ((int key, Action<OutputStream> action) in binaryContextOverride)
                {
                    ostr.WriteVarInt(key);
                    OutputStream.Position startValue = ostr.StartFixedLengthSize(2);
                    action(ostr);
                    ostr.EndFixedLengthSize(startValue, 2);
                    size++;
                }
            }
            foreach ((int key, ReadOnlyMemory<byte> value) in InitialBinaryContext)
            {
                if (_binaryContextOverride == null || !_binaryContextOverride.ContainsKey(key))
                {
                    ostr.WriteVarInt(key);
                    ostr.WriteSize(value.Length);
                    ostr.WriteByteSpan(value.Span);
                    size++;
                }
            }
            ostr.RewriteFixedLengthSize20(size, start, sizeLength);
        }
    }

    public static class OutgoingFrameHelper
    {
        /// <summary>Writes the header of a frame. This header does not include the frame's prologue.</summary>
        /// <param name="ostr">The output stream.</param>
        /// <param name="frame">The frame.</param>
        public static void WriteHeader(this OutputStream ostr, OutgoingFrame frame) =>
            frame.WriteHeader(ostr);
    }
}
