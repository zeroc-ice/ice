// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.IO.Compression;
using System.Threading.Tasks;

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
        /// <summary>The encoding of the frame payload.</summary>
        public abstract Encoding Encoding { get; }

        /// <summary>True for a sealed frame, false otherwise, a sealed frame does not change its contents.</summary>
        public bool IsSealed { get; private protected set; }

        /// <summary>Returns a list of array segments with the contents of the frame payload.</summary>
        /// <remarks>Treat this list as if it was read-only, like an IReadOnlyList{ReadOnlyMemory{byte}}. It is not
        /// read-only for compatibility with the Socket APIs.</remarks>
        public IList<ArraySegment<byte>> Payload
        {
            get
            {
                _payload ??= Data.Slice(PayloadStart, PayloadEnd);
                return _payload;
            }
        }

        /// <summary>The Ice protocol of this frame.</summary>
        public Protocol Protocol { get; }

        /// <summary>The frame byte count.</summary>
        public int Size { get; private protected set; }

        // True if Ice1 frames should use protocol compression, false otherwise.
        internal bool Compress { get; }

        internal List<ArraySegment<byte>> Data { get; }

        /// <summary>The stream data writer if the request or response has an outgoing stream param. The writer is
        /// called after the request or response frame is sent over a socket stream.</summary>
        internal Action<SocketStream>? StreamDataWriter { get; set; }

        // Position of the end of the payload. With ice1, this is always the end of the frame.
        private protected OutputStream.Position PayloadEnd { get; set; }

        // Position of the start of the payload.
        private protected OutputStream.Position PayloadStart { get; set; }

        private HashSet<int>? _binaryContextKeys;

        // OutputStream used to write the binary context.
        private OutputStream? _binaryContextOstr;

        private readonly CompressionLevel _compressionLevel;
        private readonly int _compressionMinSize;

        // Cached computed payload.
        private IList<ArraySegment<byte>>? _payload;

        /// <summary>Writes a binary context entry to the frame with the given key and value.</summary>
        /// <param name="key">The binary context entry key.</param>
        /// <param name="value">The binary context entry value.</param>
        /// <exception cref="NotSupportedException">If the frame protocol doesn't support binary context.</exception>
        /// <exception cref="ArgumentException">If the key is already in use.</exception>
        public void AddBinaryContextEntry(int key, ReadOnlySpan<byte> value)
        {
            OutputStream ostr = StartBinaryContext();
            if (AddKey(key))
            {
                ostr.WriteBinaryContextEntry(key, value);
            }
            else
            {
                throw new ArgumentException($"key `{key}' is already in use", nameof(key));
            }
        }

        /// <summary>Writes a binary context entry to the frame with the given key and value.</summary>
        /// <param name="key">The binary context entry key.</param>
        /// <param name="value">The value to marshal as the binary context entry value.</param>
        /// <param name="writer">The writer used to marshal the value.</param>
        /// <exception cref="NotSupportedException">If the frame protocol doesn't support binary context.</exception>
        /// <exception cref="ArgumentException">If the key is already in use.</exception>
        public void AddBinaryContextEntry<T>(int key, T value, OutputStreamWriter<T> writer)
        {
            OutputStream ostr = StartBinaryContext();
            if (AddKey(key))
            {
                ostr.WriteBinaryContextEntry(key, value, writer);
            }
            else
            {
                throw new ArgumentException($"key `{key}' is already in use", nameof(key));
            }
        }

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
                IList<ArraySegment<byte>> payload = Payload;
                int encapsulationOffset = this is OutgoingResponseFrame ? 1 : 0;

                // The encapsulation always starts in the first segment of the payload (at position 0 or 1).
                Debug.Assert(encapsulationOffset < payload[0].Count);

                int sizeLength = Protocol == Protocol.Ice2 ? payload[0][encapsulationOffset].ReadSizeLength20() : 4;
                byte compressionStatus = payload.GetByte(encapsulationOffset + sizeLength + 2);

                if (compressionStatus != 0)
                {
                    throw new InvalidOperationException("payload is already compressed");
                }

                int encapsulationSize = payload.GetByteCount() - encapsulationOffset; // this includes the size length
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
                    compressedData[0] = payload[0][0];
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
                    foreach (ArraySegment<byte> segment in payload.Slice(encapsulationOffset + sizeLength + 3))
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

                int binaryContextLastSegmentOffset = -1;

                if (_binaryContextOstr is OutputStream ostr)
                {
                    // If there is a binary context, we make sure it uses its own segment(s).
                    OutputStream.Position binaryContextEnd = ostr.Tail;
                    binaryContextLastSegmentOffset = binaryContextEnd.Offset;

                    // When we have a _binaryContextOstr, we wrote at least the size placeholder for the binary context
                    // dictionary.
                    Debug.Assert(binaryContextEnd.Segment > PayloadEnd.Segment ||
                        binaryContextEnd.Offset > PayloadEnd.Offset);

                    // The first segment of the binary context is immediately after the payload
                    ArraySegment<byte> segment = Data[PayloadEnd.Segment].Slice(PayloadEnd.Offset);
                    if (segment.Count > 0)
                    {
                        Data.Insert(PayloadEnd.Segment + 1, segment);
                        if (binaryContextEnd.Segment == PayloadEnd.Segment)
                        {
                            binaryContextLastSegmentOffset -= PayloadEnd.Offset;
                        }
                    }
                    // else the binary context already starts with its own segment
                }

                int start = PayloadStart.Segment;

                if (PayloadStart.Offset > 0)
                {
                    // There is non payload bytes in the first payload segment: we move them to their own segment.

                    ArraySegment<byte> segment = Data[PayloadStart.Segment];
                    Data[PayloadStart.Segment] = segment.Slice(0, PayloadStart.Offset);
                    start += 1;
                }

                Data.RemoveRange(start, PayloadEnd.Segment - start + 1);
                offset += (int)memoryStream.Position;
                Data.Insert(start, new ArraySegment<byte>(compressedData, 0, offset));

                PayloadStart = new OutputStream.Position(start, 0);
                PayloadEnd = new OutputStream.Position(start, offset);
                Size = Data.GetByteCount();

                if (_binaryContextOstr != null)
                {
                    // Recreate binary context OutputStream
                    _binaryContextOstr =
                        new OutputStream(_binaryContextOstr.Encoding,
                                         Data,
                                         new OutputStream.Position(Data.Count - 1, binaryContextLastSegmentOffset));
                }

                // Rewrite the encapsulation size
                compressedData.AsSpan(encapsulationOffset, sizeLength).WriteEncapsulationSize(
                    offset - sizeLength - encapsulationOffset,
                    Protocol.GetEncoding());

                _payload = null; // reset cache

                return CompressionResult.Success;
            }
        }

        private protected OutgoingFrame(
            Protocol protocol,
            bool compress,
            CompressionLevel compressionLevel,
            int compressionMinSize,
            List<ArraySegment<byte>> data)
        {
            Protocol = protocol;
            Protocol.CheckSupported();
            Data = data;
            Compress = compress;
            _compressionLevel = compressionLevel;
            _compressionMinSize = compressionMinSize;
        }

        private protected abstract ArraySegment<byte> GetDefaultBinaryContext();

        private OutputStream StartBinaryContext()
        {
            if (Protocol == Protocol.Ice1)
            {
                throw new NotSupportedException("binary context is not supported with ice1 protocol");
            }

            if (IsSealed)
            {
                throw new InvalidOperationException("cannot modify a sealed frame");
            }

            if (_binaryContextOstr == null)
            {
                _binaryContextOstr = new OutputStream(Encoding.V20, Data, PayloadEnd);
                _binaryContextOstr.WriteByteSpan(stackalloc byte[2]); // 2-bytes size place holder
            }
            return _binaryContextOstr;
        }

        // Finish prepares the frame for sending and adjusts the last written segment to match the offset of the written
        // data. If the frame contains a binary context, Finish appends the entries from defaultBinaryContext (if any)
        // and rewrites the binary context dictionary size.
        internal virtual void Finish()
        {
            if (!IsSealed)
            {
                ArraySegment<byte> defaultBinaryContext = GetDefaultBinaryContext();

                if (_binaryContextOstr is OutputStream ostr)
                {
                    Debug.Assert(_binaryContextKeys != null);
                    Data[^1] = Data[^1].Slice(0, ostr.Tail.Offset);
                    if (defaultBinaryContext.Count > 0)
                    {
                        // Add segment for each slot that was not written yet.
                        var istr = new InputStream(defaultBinaryContext, Encoding.V20);
                        int dictionarySize = istr.ReadSize();
                        for (int i = 0; i < dictionarySize; ++i)
                        {
                            int startPos = istr.Pos;
                            int key = istr.ReadVarInt();
                            int entrySize = istr.ReadSize();
                            istr.Skip(entrySize);
                            if (!ContainsKey(key))
                            {
                                Data.Add(defaultBinaryContext.Slice(startPos, istr.Pos - startPos));
                                AddKey(key);
                            }
                        }
                    }

                    ostr.RewriteFixedLengthSize20(_binaryContextKeys.Count, PayloadEnd, 2);
                }
                else
                {
                    Debug.Assert(_binaryContextKeys == null);

                    // Only when forwarding an ice2 request or response
                    if (defaultBinaryContext.Count > 0 && Data[^1].Array == defaultBinaryContext.Array)
                    {
                        // Just expand the last segment to include the binary context bytes as-is.
                        Data[^1] = new ArraySegment<byte>(Data[^1].Array!,
                                                          Data[^1].Offset,
                                                          Data[^1].Count + defaultBinaryContext.Count);
                    }
                    else
                    {
                        Data[^1] = Data[^1].Slice(0, PayloadEnd.Offset);
                        if (defaultBinaryContext.Count > 0)
                        {
                            // Can happen if an interceptor compresses the payload
                            Data.Add(defaultBinaryContext);
                        }
                    }
                }

                Size = Data.GetByteCount();
                IsSealed = true;
            }
        }

        private protected bool ContainsKey(int key) => _binaryContextKeys?.Contains(key) ?? false;

        private bool AddKey(int key)
        {
            _binaryContextKeys ??= new HashSet<int>();
            return _binaryContextKeys.Add(key);
        }
    }
}
