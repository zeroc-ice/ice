//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
        /// <summary>The encoding of the frame payload.</summary>
        public abstract Encoding Encoding { get; }
        /// <summary>True for a sealed frame, false otherwise, a sealed frame does not change its contents.</summary>
        public bool IsSealed { get; private protected set; }

        /// <summary>Returns a list of array segments with the contents of the frame payload.</summary>
        public IList<ArraySegment<byte>> Payload
        {
            get
            {
                if (Protocol == Protocol.Ice1)
                {
                    return Data;
                }
                else if (_payload == null && _encapsulationEnd is OutputStream.Position encapsulationEnd)
                {
                    _payload = Data.Slice(default, encapsulationEnd);
                }
                return _payload ?? Array.Empty<ArraySegment<byte>>();
            }
        }
        /// <summary>The Ice protocol of this frame.</summary>
        public Protocol Protocol { get; }

        /// <summary>The frame byte count.</summary>
        public int Size { get; private protected set; }

        // True if Ice1 frames should use protocol compression, false otherwise.
        internal bool Compress { get; }

        internal List<ArraySegment<byte>> Data { get; }

        // OutputStream used to write the binary context
        private protected OutputStream? _binaryContextOstr;
        private protected ArraySegment<byte> _defaultBinaryContext;

        // Position of the end of the encapsulation, for ice1 this is always the frame end.
        private protected OutputStream.Position? _encapsulationEnd;
        // Position of the start of the encapsulation.
        private protected OutputStream.Position _encapsulationStart;

        private readonly CompressionLevel _compressionLevel;
        private readonly int _compressionMinSize;

        private HashSet<int>? _binaryContextKeys;

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

        /// <summary>Writes a binary context entry to the frame with the given key and value.</summary>
        /// <param name="key">The binary context entry key.</param>
        /// <param name="value">The value to marshal as the binary context entry value.</param>
        /// <param name="writer">The writer used to marshal the value.</param>
        /// <exception cref="NotSupportedException">If the frame protocol doesn't support binary context.</exception>
        /// <exception cref="ArgumentException">If the key is already in use.</exception>
        public void AddBinaryContextEntry<T>(int key, in T value, OutputStreamValueWriter<T> writer) where T : struct
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

        /// <summary>Compresses the encapsulation payload using GZip compression, compressed encapsulation payload is
        /// only supported with 2.0 encoding.</summary>
        /// <returns>A <see cref="CompressionResult"/> value indicating the result of the compression operation.
        /// </returns>
        public CompressionResult CompressPayload()
        {
            if (IsSealed)
            {
                throw new InvalidOperationException("cannot modify a sealed frame");
            }

            if (Encoding != Encoding.V2_0)
            {
                throw new NotSupportedException("payload compression is only supported with 2.0 encoding");
            }
            else
            {
                Debug.Assert(_encapsulationEnd != null); // i.e. we have an encapsulation
                IList<ArraySegment<byte>> encapsulation = Data.Slice(_encapsulationStart, _encapsulationEnd.Value);

                int sizeLength = Protocol == Protocol.Ice2 ? 1 << (encapsulation[0][0] & 0x03) : 4;
                byte compressionStatus = encapsulation[0].Count > sizeLength + 2 ?
                    encapsulation[0][sizeLength + 2] : encapsulation[1][sizeLength + 2 - encapsulation[0].Count];

                if (compressionStatus != 0)
                {
                    throw new InvalidOperationException("payload is already compressed");
                }

                int encapsulationSize = encapsulation.GetByteCount();
                if (encapsulationSize < _compressionMinSize)
                {
                    return CompressionResult.PayloadTooSmall;
                }
                // Reserve memory for the compressed data, this should never be greater than the uncompressed data
                // otherwise we will just send the uncompressed data.
                byte[] compressedData = new byte[encapsulation.GetByteCount()];

                // Write the encapsulation header
                int offset = sizeLength;
                compressedData[offset++] = Encoding.Major;
                compressedData[offset++] = Encoding.Minor;
                // Set the compression status to '1' GZip compressed
                compressedData[offset++] = 1;
                // Write the size of the uncompressed data
                OutputStream.WriteFixedLengthSize20(encapsulationSize - sizeLength,
                                                    compressedData.AsSpan(offset, sizeLength));
                offset += sizeLength;
                using var memoryStream = new MemoryStream(compressedData, offset, compressedData.Length - offset);
                var gzipStream = new GZipStream(memoryStream,
                    _compressionLevel == CompressionLevel.Fastest ? System.IO.Compression.CompressionLevel.Fastest :
                                                                    System.IO.Compression.CompressionLevel.Optimal);
                try
                {
                    // The data to compress starts after the compression status byte, +3 corresponds to (Encoding 2
                    // bytes, Compression status 1 byte)
                    gzipStream.Write(encapsulation[0].Slice(sizeLength + 3));
                    for (int i = 1; i < encapsulation.Count; ++i)
                    {
                        gzipStream.Write(encapsulation[i]);
                    }
                    gzipStream.Flush();
                }
                catch (NotSupportedException)
                {
                    // If the data doesn't fit in the memory stream NotSupportedException is thrown when GZipStream
                    // try to expand the fixed size MemoryStream.
                    return CompressionResult.PayloadNotCompressible;
                }

                OutputStream.Position encapsulationEnd = _encapsulationEnd.Value;

                int start = _encapsulationStart.Segment;

                if (_binaryContextOstr is OutputStream ostr)
                {
                    ArraySegment<byte> segment = Data[encapsulationEnd.Segment];
                    OutputStream.Position binaryContextEnd = ostr.Tail;
                    if (binaryContextEnd.Segment == encapsulationEnd.Segment)
                    {
                        segment = segment.Slice(encapsulationEnd.Offset,
                                                binaryContextEnd.Offset - encapsulationEnd.Offset);
                    }
                    else
                    {
                        segment = segment.Slice(encapsulationEnd.Offset);
                    }

                    if (segment.Count > 0)
                    {
                        Data.Insert(encapsulationEnd.Segment + 1, segment);
                    }
                }

                if (_encapsulationStart.Offset > 0)
                {
                    ArraySegment<byte> segment = Data[_encapsulationStart.Segment];
                    Data[_encapsulationStart.Segment] = segment.Slice(0, _encapsulationStart.Offset);
                    start += 1;
                }

                Data.RemoveRange(start, encapsulationEnd.Segment - start + 1);
                offset += (int)memoryStream.Position;
                Data.Insert(start, new ArraySegment<byte>(compressedData, 0, offset));

                _encapsulationStart = new OutputStream.Position(start, 0);
                _encapsulationEnd = new OutputStream.Position(start, offset);
                Size = Data.GetByteCount();

                if (_binaryContextOstr != null)
                {
                    OutputStream.Position binaryContextEnd = _binaryContextOstr.Tail;
                    if (binaryContextEnd.Segment == encapsulationEnd.Segment)
                    {
                        binaryContextEnd.Offset -= encapsulationEnd.Offset;
                    }
                    binaryContextEnd.Segment = Data.Count - 1;
                    _binaryContextOstr = new OutputStream(_binaryContextOstr.Encoding, Data, binaryContextEnd);
                }

                // Rewrite the payload size
                OutputStream.WriteEncapsulationSize(offset - sizeLength,
                                                    compressedData.AsSpan(0, sizeLength),
                                                    Protocol.GetEncoding());
                _payload = null;

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
                Debug.Assert(_encapsulationEnd != null);
                _binaryContextOstr = new OutputStream(Encoding.V2_0, Data, _encapsulationEnd.Value);
                _binaryContextOstr.WriteByteSpan(stackalloc byte[2]); // 2-bytes size place holder
            }
            return _binaryContextOstr;
        }

        // Once we finish writing the encapsulation we adjust the _encapsulationEnd position.
        private protected void FinishEncapsulation(OutputStream.Position encapsulationEnd) =>
            _encapsulationEnd = encapsulationEnd;

        // Finish prepares the frame for sending, adjusts the last written segment to match the offset of the written
        // data. If the frame contains a binary context, Finish appends the entries from _defaultBinaryContext (if any)
        // and rewrites the binary context dictionary size.
        internal virtual void Finish()
        {
            if (!IsSealed)
            {
                if (_binaryContextOstr is OutputStream ostr)
                {
                    Debug.Assert(_binaryContextKeys != null);
                    Data[^1] = Data[^1].Slice(0, ostr.Tail.Offset);
                    if (_defaultBinaryContext.Count > 0)
                    {
                        // Add segment for each slot that was not written yet.
                        var istr = new InputStream(_defaultBinaryContext, Encoding.V2_0);
                        int dictionarySize = istr.ReadSize();
                        for (int i = 0; i < dictionarySize; ++i)
                        {
                            int key = istr.ReadVarInt();
                            int entrySize = istr.ReadSize();
                            if (ContainsKey(key))
                            {
                                istr.Skip(entrySize);
                            }
                            else
                            {
                                Data.Add(_defaultBinaryContext.Slice(istr.Pos, entrySize));
                                AddKey(key);
                            }
                        }
                    }

                    Debug.Assert(_encapsulationEnd != null);
                    ostr.RewriteFixedLengthSize20(_binaryContextKeys.Count, _encapsulationEnd.Value, 2);
                }
                else
                {
                    Debug.Assert(_binaryContextKeys == null);
                    if (_defaultBinaryContext.Count > 0) // only when forwarding an ice2 request or response
                    {
                        Debug.Assert(Data[^1].Array == _defaultBinaryContext.Array);

                        // Just expand the last segment to include the binary context bytes as-is.
                        Data[^1] = new ArraySegment<byte>(Data[^1].Array!,
                                                          Data[^1].Offset,
                                                          Data[^1].Count + _defaultBinaryContext.Count);
                    }
                    else
                    {
                        Debug.Assert(_encapsulationEnd != null);
                        Data[^1] = Data[^1].Slice(0, _encapsulationEnd.Value.Offset);
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
