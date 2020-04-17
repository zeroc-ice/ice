//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Binary;

namespace Ice
{
    public interface IStreamableStruct
    {
        public void IceWrite(OutputStream ostr);
    }

    public delegate void OutputStreamWriter<in T>(OutputStream os, T value);
    public delegate void OutputStreamStructWriter<T>(OutputStream ostr, in T value) where T : struct;

    /// <summary>
    /// Interface for output streams used to write Slice types to a sequence of bytes.
    /// </summary>
    public sealed class OutputStream
    {
        /// <summary>Represents a position in a list of array segments, the position
        /// consists of the index of the segment in the list and the offset into
        /// the array.</summary>
        public struct Position
        {
            /// <summary>Creates a new position from the segment and offset values.</summary>
            /// <param name="segment">The zero based index of the segment.</param>
            /// <param name="offset">The offset into the segment.</param>
            public Position(int segment, int offset)
            {
                Segment = segment;
                Offset = offset;
            }

            /// <summary>The zero based index of the segment.</summary>
            public int Segment;
            /// <summary>The offset into the segment.</summary>
            public int Offset;
        }

        public static readonly OutputStreamWriter<bool> IceWriterFromBool = (ostr, value) => ostr.WriteBool(value);
        public static readonly OutputStreamWriter<byte> IceWriterFromByte = (ostr, value) => ostr.WriteByte(value);
        public static readonly OutputStreamWriter<short> IceWriterFromShort = (ostr, value) => ostr.WriteShort(value);
        public static readonly OutputStreamWriter<int> IceWriterFromInt = (ostr, value) => ostr.WriteInt(value);
        public static readonly OutputStreamWriter<long> IceWriterFromLong = (ostr, value) => ostr.WriteLong(value);
        public static readonly OutputStreamWriter<float> IceWriterFromFloat = (ostr, value) => ostr.WriteFloat(value);
        public static readonly OutputStreamWriter<double> IceWriterFromDouble =
            (ostr, value) => ostr.WriteDouble(value);

        public static readonly OutputStreamWriter<string> IceWriterFromString =
            (ostr, value) => ostr.WriteString(value);

        public static readonly OutputStreamWriter<bool[]> IceWriterFromBoolArray =
            (ostr, value) => ostr.WriteBoolSeq(value);

        public static readonly OutputStreamWriter<byte[]> IceWriterFromByteArray =
            (ostr, value) => ostr.WriteByteSeq(value);

        public static readonly OutputStreamWriter<short[]> IceWriterFromShortArray =
            (ostr, value) => ostr.WriteShortSeq(value);

        public static readonly OutputStreamWriter<int[]> IceWriterFromIntArray =
            (ostr, value) => ostr.WriteIntSeq(value);

        public static readonly OutputStreamWriter<long[]> IceWriterFromLongArray =
            (ostr, value) => ostr.WriteLongSeq(value);

        public static readonly OutputStreamWriter<float[]> IceWriterFromFloatArray =
            (ostr, value) => ostr.WriteFloatSeq(value);

        public static readonly OutputStreamWriter<double[]> IceWriterFromDoubleArray =
            (ostr, value) => ostr.WriteDoubleSeq(value);

        /// <summary>The encoding used when writing to this stream.</summary>
        /// <value>The encoding.</value>
        public Encoding Encoding { get; private set; }

        /// <summary>Determines the current size of the stream. This corresponds to the number of bytes already written
        /// to the stream.</summary>
        /// <value>The current size.</value>
        internal int Size { get; private set; }

        internal Position Tail => _tail;

        private const int DefaultSegmentSize = 256;

        // The number of bytes that the stream can hold.
        private int _capacity;

        // Data for the class or exception instance that is currently getting marshaled.
        private InstanceData? _current;

        // The segment currently used by write operations, this is usually the last segment of
        // the segment list but it can occasionally be one before last after expanding the list.
        // The tail Position always points to this segment, and the tail offset indicates how much
        // of the segment has been used.
        private ArraySegment<byte> _currentSegment;

        // When set, we are writing an endpoint encapsulation. An endpoint encapsulation is a lightweight
        // encapsulation that cannot contain classes, exceptions, tagged members/parameters, or another
        // endpoint. It is often but not always set when _mainEncaps is set (so nested inside _mainEncaps).
        private Encaps? _endpointEncaps;

        // The current class/exception format, can be either Compact or Sliced.
        private readonly FormatType _format;

        // Map of class instance to instance ID, where the instance IDs start at 2.
        // When writing a top-level encapsulation:
        //  - Instance ID = 0 means null.
        //  - Instance ID = 1 means the instance is encoded inline afterwards.
        //  - Instance ID > 1 means a reference to a previously encoded instance, found in this map.
        private Dictionary<AnyClass, int>? _instanceMap;

        // When set, we are writing to a top-level encapsulation.
        private readonly Encaps? _mainEncaps;

        // all segments before the tail segment are fully used
        private readonly List<ArraySegment<byte>> _segmentList;

        // This is the position for the next write operation, it holds the index of the current
        // segment and the offset into it.
        private Position _tail;

        // Map of type ID string to type ID index.
        // When writing into a top-level encapsulation, we assign a type ID index (starting with 1) to each type ID we
        // write, in order.
        private Dictionary<string, int>? _typeIdMap;

        // Constructor for header and other non-encapsulated data
        internal OutputStream(Encoding encoding, List<ArraySegment<byte>> data, Position? startAt = null)
        {
            Encoding = encoding;
            Encoding.CheckSupported();
            _format = default; // not used
            _segmentList = data;
            if (_segmentList.Count == 0)
            {
                _currentSegment = ArraySegment<byte>.Empty;
                _capacity = 0;
                Size = 0;
                _tail = new Position(0, 0);
            }
            else
            {
                _tail = startAt ?? new Position(0, 0);
                _currentSegment = _segmentList[_tail.Segment];
                Size = Distance(new Position(0, 0));
                _capacity = 0;
                foreach (ArraySegment<byte> segment in _segmentList)
                {
                    _capacity += segment.Count;
                }
            }
        }

        // Constructor that starts an encapsulation
        internal OutputStream(Encoding encoding, List<ArraySegment<byte>> data, Position startAt, FormatType format)
            : this(encoding, data, startAt)
        {
            _mainEncaps = new Encaps(Encoding, _tail);
            _format = format;
            WriteEncapsulationHeader(0, Encoding);
        }

        /// <summary>
        /// Finish writing the contents of the stream, and inform the frame that
        /// create the stream the payload is ready. The stream should not longer
        /// be used.
        /// </summary>
        /// <returns>The tail position that marks the end of the stream.</returns>
        /// TODO: The stream should not longer be used, how can we enforce it.
        internal Position Save()
        {
            _segmentList[_tail.Segment] = _segmentList[_tail.Segment].Slice(0, _tail.Offset);

            if (_mainEncaps.HasValue)
            {
                Debug.Assert(_endpointEncaps == null);

                Encaps encaps = _mainEncaps.Value;

                // Size includes size and version.
                RewriteInt(Distance(encaps.StartPos), encaps.StartPos);
            }
            return _tail;
        }

        // Start writing a slice of a class or exception instance.
        // This is an Ice-internal method marked public because it's called by the generated code.
        // typeId is the type ID of this slice.
        // firstSlice is true when writing the first (most derived) slice of an instance.
        // slicedData is the preserved sliced-off slices, if any. Can only be provided when firstSlice is true.
        // compactId is the compact type ID of this slice, if specified.
        [EditorBrowsable(EditorBrowsableState.Never)]
        public void IceStartSlice(string typeId, bool firstSlice, SlicedData? slicedData = null, int? compactId = null)
        {
            Debug.Assert(_mainEncaps != null && _current != null);
            if (slicedData.HasValue)
            {
                Debug.Assert(firstSlice);
                try
                {
                    WriteSlicedData(slicedData.Value);
                    firstSlice = false;
                }
                catch (NotSupportedException)
                {
                    // Ignored: for some reason we could not remarshal the sliced data and firstSlice remains true
                }
            }

            _current.SliceFlags = default;

            if (_format == FormatType.Sliced)
            {
                // Encode the slice size if using the sliced format.
                _current.SliceFlags |= EncodingDefinitions.SliceFlags.HasSliceSize;
            }

            _current.SliceFlagsPos = _tail;
            WriteByte(0); // Placeholder for the slice flags

            // For instance slices, encode the flag and the type ID either as a string or index. For exception slices,
            // always encode the type ID a string.
            if (_current.InstanceType == InstanceType.Class)
            {
                // Encode the type ID (only in the first slice for the compact
                // encoding).
                // This  also shows that the firtSlice is currently useful/used only for class instances in
                // compact format.
                if (_format == FormatType.Sliced || firstSlice)
                {
                    if (compactId.HasValue)
                    {
                        _current.SliceFlags |= EncodingDefinitions.SliceFlags.HasTypeIdCompact;
                        WriteSize(compactId.Value);
                    }
                    else
                    {
                        int index = RegisterTypeId(typeId);
                        if (index < 0)
                        {
                            _current.SliceFlags |= EncodingDefinitions.SliceFlags.HasTypeIdString;
                            WriteString(typeId);
                        }
                        else
                        {
                            _current.SliceFlags |= EncodingDefinitions.SliceFlags.HasTypeIdIndex;
                            WriteSize(index);
                        }
                    }
                }
            }
            else
            {
                WriteString(typeId);
            }

            if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasSliceSize) != 0)
            {
                _current.SliceSizePos = _tail;
                WriteInt(0); // Placeholder for the slice length.
            }
            _current.SliceFirstMemberPos = _tail;
        }

        // Marks the end of a slice for a class instance or user exception.
        // This is an Ice-internal method marked public because it's called by the generated code.
        // lastSlice is true when it's the last (least derived) slice of the instance.
        [EditorBrowsable(EditorBrowsableState.Never)]
        public void IceEndSlice(bool lastSlice)
        {
            Debug.Assert(_mainEncaps != null && _current != null);

            if (lastSlice)
            {
                _current.SliceFlags |= EncodingDefinitions.SliceFlags.IsLastSlice;
            }

            // Write the tagged member end marker if some tagged members were encoded. Note that the optional members
            // are encoded before the indirection table and are included in the slice size.
            if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasTaggedMembers) != 0)
            {
                WriteByte(EncodingDefinitions.TaggedEndMarker);
            }

            // Write the slice size if necessary.
            if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasSliceSize) != 0)
            {
                RewriteInt(Distance(_current.SliceSizePos), _current.SliceSizePos);
            }

            if (_current.IndirectionTable?.Count > 0)
            {
                Debug.Assert(_format == FormatType.Sliced);
                _current.SliceFlags |= EncodingDefinitions.SliceFlags.HasIndirectionTable;

                WriteSize(_current.IndirectionTable.Count);
                foreach (AnyClass v in _current.IndirectionTable)
                {
                    WriteInstance(v);
                }
                _current.IndirectionTable.Clear();
                _current.IndirectionMap?.Clear(); // IndirectionMap is null when writing SlicedData.
            }

            // Update SliceFlags in case they were updated.
            RewriteByte((byte)_current.SliceFlags, _current.SliceFlagsPos);
        }

        /// <summary>Writes a size to the stream.</summary>
        /// <param name="v">The size to write.</param>
        public void WriteSize(int v)
        {
            if (v > 254)
            {
                WriteByte(255);
                WriteInt(v);
            }
            else
            {
                WriteByte((byte)v);
            }
        }

        /// <summary>Write the header information for a tagged value.</summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <param name="format">The optional format of the value.</param>
        public bool WriteOptional(int tag, OptionalFormat format)
        {
            // TODO: eliminate return value, which was used for 1.0 encoding.
            Debug.Assert(_mainEncaps != null && _endpointEncaps == null);

            int v = (int)format;
            if (tag < 30)
            {
                v |= tag << 3;
                WriteByte((byte)v);
            }
            else
            {
                v |= 0x0F0; // tag = 30
                WriteByte((byte)v);
                WriteSize(tag);
            }
            if (_current != null)
            {
                _current.SliceFlags |= EncodingDefinitions.SliceFlags.HasTaggedMembers;
            }
            return true;
        }

        /// <summary>Writes a serializable object to the stream.</summary>
        /// <param name="o">The serializable object to write.</param>
        public void WriteSerializable(object o)
        {
            var w = new OutputStreamWrapper(this);
            IFormatter f = new BinaryFormatter();
            f.Serialize(w, o);
            w.Close();
        }

        /// <summary>Writes a tagged serializable object to the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="o">The serializable object to write.</param>
        public void WriteSerializable(int tag, object? o)
        {
            if (o !=null && WriteOptional(tag, OptionalFormat.VSize))
            {
                var w = new OutputStreamWrapper(this);
                IFormatter f = new BinaryFormatter();
                f.Serialize(w, o);
                w.Close();
            }
        }

        /// <summary>Writes a byte to the stream.</summary>
        /// <param name="v">The byte to write.</param>
        public void WriteByte(byte v)
        {
            Expand(1);
            int offset = _tail.Offset;
            if (offset < _currentSegment.Count)
            {
                _currentSegment[offset] = v;
                _tail.Offset++;
            }
            else
            {
                _currentSegment = _segmentList[++_tail.Segment];
                _currentSegment[0] = v;
                _tail.Offset = 1;
            }
            Size++;
        }

        /// <summary>Writes a tagged byte to the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The byte to write to the stream.</param>
        public void WriteByte(int tag, byte? v)
        {
            if (v is byte value && WriteOptional(tag, OptionalFormat.F1))
            {
                WriteByte(value);
            }
        }

        /// <summary>Writes a sequence of bytes to the stream.</summary>
        /// <param name="v">The sequence of bytes to write to the stream.</param>
        public void WriteByteSeq(ReadOnlySpan<byte> v) => WriteFixedSizeNumericSeq(v);

        /// <summary>Writes a tagged sequence of byte to the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The sequence of byte to write to the stream.</param>
        public void WriteByteSeq(int tag, ReadOnlySpan<byte> v) => WriteFixedSizeNumericSeq(tag, v, sizeof(byte));

        /// <summary>Writes a boolean to the stream.</summary>
        /// <param name="v">The boolean to write to the stream.</param>
        public void WriteBool(bool v) => WriteByte(v ? (byte)1 : (byte)0);

        /// <summary>Writes a tagged boolean to the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The boolean to write to the stream.</param>
        public void WriteBool(int tag, bool? v)
        {
            if (v is bool value && WriteOptional(tag, OptionalFormat.F1))
            {
                WriteBool(value);
            }
        }

        /// <summary>Writes a sequence of boolean to the stream.</summary>
        /// <param name="v">The sequence of boolean.</param>
        // Note: bool is treated as is if it was a fixed-size numeric type.
        public void WriteBoolSeq(ReadOnlySpan<bool> v) => WriteFixedSizeNumericSeq(v);

        /// <summary>Writes a tagged sequence of boolean to the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The sequence of boolean to write to the stream.</param>
        public void WriteBoolSeq(int tag, ReadOnlySpan<bool> v) => WriteFixedSizeNumericSeq(tag, v, sizeof(bool));

        /// <summary>Writes a short to the stream.</summary>
        /// <param name="v">The short to write to the stream.</param>
        public void WriteShort(short v) => WriteFixedSizeNumeric(v, sizeof(short));

        /// <summary>Writes a tagged short to the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The short to write to the stream.</param>
        public void WriteShort(int tag, short? v)
        {
            if (v is short value && WriteOptional(tag, OptionalFormat.F2))
            {
                WriteShort(value);
            }
        }

        /// <summary>Writes a sequence of short to the stream.</summary>
        /// <param name="v">The sequence of short to write to the stream.</param>
        public void WriteShortSeq(ReadOnlySpan<short> v) => WriteFixedSizeNumericSeq(v);

        /// <summary>Writes a tagged sequence of short to the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The sequence of short to  write to the stream.</param>
        public void WriteShortSeq(int tag, ReadOnlySpan<short> v) => WriteFixedSizeNumericSeq(tag, v, sizeof(short));

        /// <summary>Writes an int to the stream.</summary>
        /// <param name="v">The int to write to the stream.</param>
        public void WriteInt(int v) => WriteFixedSizeNumeric(v, sizeof(int));

        /// <summary>Writes a tagged int to the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The int to write to the stream.</param>
        public void WriteInt(int tag, int? v)
        {
            if (v is int value && WriteOptional(tag, OptionalFormat.F4))
            {
                WriteInt(value);
            }
        }

        /// <summary>Writes a sequence of int to the stream.</summary>
        /// <param name="v">The sequence of int to write to the stream.</param>
        public void WriteIntSeq(ReadOnlySpan<int> v) => WriteFixedSizeNumericSeq(v);

        /// <summary>Writes a tagged sequence of int to the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The sequence of int to write to the stream.</param>
        public void WriteIntSeq(int tag, ReadOnlySpan<int> v) => WriteFixedSizeNumericSeq(tag, v, sizeof(int));

        /// <summary>Writes a long to the stream.</summary>
        /// <param name="v">The long to write to the stream.</param>
        public void WriteLong(long v) => WriteFixedSizeNumeric(v, sizeof(long));

        /// <summary>Writes a tagged long to the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The long to write to the stream.</param>
        public void WriteLong(int tag, long? v)
        {
            if (v is long value && WriteOptional(tag, OptionalFormat.F8))
            {
                WriteLong(value);
            }
        }

        /// <summary>Writes a sequence of long to the stream.</summary>
        /// <param name="v">The sequence of long to write to the stream.</param>
        public void WriteLongSeq(ReadOnlySpan<long> v) => WriteFixedSizeNumericSeq(v);

        /// <summary>Writes a tagged sequence of long to the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The sequence of long to write to the stream.</param>
        public void WriteLongSeq(int tag, ReadOnlySpan<long> v) => WriteFixedSizeNumericSeq(tag, v, sizeof(long));

        /// <summary>Writes a float to the stream.</summary>
        /// <param name="v">The float to write to the stream.</param>
        public void WriteFloat(float v) => WriteFixedSizeNumeric(v, sizeof(float));

        /// <summary>Writes a tagged float to the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The float to write to the stream.</param>
        public void WriteFloat(int tag, float? v)
        {
            if (v is float value && WriteOptional(tag, OptionalFormat.F4))
            {
                WriteFloat(value);
            }
        }

        /// <summary>Writes a sequence of float to the stream.</summary>
        /// <param name="v">The sequence of float to write to the stream.</param>
        public void WriteFloatSeq(ReadOnlySpan<float> v) => WriteFixedSizeNumericSeq(v);

        /// <summary>Writes a tagged sequence of float to the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The sequence of float to write to the stream.</param>
        public void WriteFloatSeq(int tag, ReadOnlySpan<float> v) => WriteFixedSizeNumericSeq(tag, v, sizeof(float));

        /// <summary>Writes a double to the stream.</summary>
        /// <param name="v">The double to write to the stream.</param>
        public void WriteDouble(double v) => WriteFixedSizeNumeric(v, sizeof(double));

        /// <summary>Writes a tagged double to the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The double to write to the stream.</param>
        public void WriteDouble(int tag, double? v)
        {
            if (v is double value && WriteOptional(tag, OptionalFormat.F8))
            {
                WriteDouble(value);
            }
        }

        /// <summary>Writes a sequence of double to the stream.</summary>
        /// <param name="v">The sequence of double to  write to the stream.</param>
        public void WriteDoubleSeq(ReadOnlySpan<double> v) => WriteFixedSizeNumericSeq(v);

        /// <summary>Writes a tagged sequence of double to the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The sequence of double to write to the stream.</param>
        public void WriteDoubleSeq(int tag, ReadOnlySpan<double> v) => WriteFixedSizeNumericSeq(tag, v, sizeof(double));

        private static readonly System.Text.UTF8Encoding _utf8 = new System.Text.UTF8Encoding(false, true);

        /// <summary>Writes a string to the stream.</summary>
        /// <param name="v">The string to write to the stream.</param>
        public void WriteString(string v)
        {
            if (v.Length == 0)
            {
                WriteSize(0);
            }
            else
            {
                byte[] data = _utf8.GetBytes(v);
                WriteSize(data.Length);
                WriteByteSpan(data.AsSpan());
            }
        }

        /// <summary>Writes a tagged string to the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The string to write to the stream.</param>
        public void WriteString(int tag, string? v)
        {
            if (v is string value && WriteOptional(tag, OptionalFormat.VSize))
            {
                WriteString(value);
            }
        }

        /// <summary>Writes a sequence of strings to the stream.</summary>
        /// <param name="v">The string sequence to write to the stream.</param>
        public void WriteStringSeq(IEnumerable<string> v) => WriteSeq(v, IceWriterFromString);

        /// <summary>Writes a proxy to the stream.</summary>
        /// <param name="v">The proxy to write.</param>
        public void WriteProxy<T>(T? v) where T : class, IObjectPrx
        {
            if (v != null)
            {
                v.IceWrite(this);
            }
            else
            {
                Identity.Empty.IceWrite(this);
            }
        }

        /// <summary>Writes a tagged  proxy to the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The proxy to write.</param>
        public void WriteProxy(int tag, IObjectPrx? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.FSize))
            {
                Position pos = StartSize();
                WriteProxy(v);
                EndSize(pos);
            }
        }

        /// <summary>Writes a tagged enumerator to the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The enumerator.</param>
        public void WriteEnum(int tag, int? v)
        {
            if (v is int value && WriteOptional(tag, OptionalFormat.Size))
            {
                WriteSize(value);
            }
        }

        /// <summary>Writes a class instance to the stream.</summary>
        /// <param name="v">The class instance to write.</param>
        public void WriteClass(AnyClass? v)
        {
            Debug.Assert(_mainEncaps != null && _endpointEncaps == null);
            if (v == null)
            {
                WriteSize(0);
            }
            else if (_current != null && _format == FormatType.Sliced)
            {
                // If writing an instance within a slice and using the sliced format, write an index of that slice's
                // indirection table.
                if (_current.IndirectionMap != null && _current.IndirectionMap.TryGetValue(v, out int index))
                {
                    // Found, index is position in indirection table + 1
                    Debug.Assert(index > 0);
                }
                else
                {
                    _current.IndirectionTable ??= new List<AnyClass>();
                    _current.IndirectionMap ??= new Dictionary<AnyClass, int>();

                    _current.IndirectionTable.Add(v);
                    index = _current.IndirectionTable.Count; // Position + 1 (0 is reserved for null)
                    _current.IndirectionMap.Add(v, index);
                }
                WriteSize(index);
            }
            else
            {
                WriteInstance(v); // Write the instance or a reference if already marshaled.
            }
        }

        /// <summary>Writes a tagged class instance to the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The class instance to write.</param>
        public void WriteClass(int tag, AnyClass? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.Class))
            {
                WriteClass(v);
            }
        }

        /// <summary>Writes a mapped Slice struct to the stream.</summary>
        /// <param name="v">The struct instance to write.</param>
        public void WriteStruct<T>(in T v) where T : struct, IStreamableStruct => v.IceWrite(this);

        /// <summary>Writes a tagged fixed-size struct to the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The struct to write.</param>
        /// <param name="fixedSize">The size of the struct, in bytes.</param>
        public void WriteTaggedStruct<T>(int tag, T? v, int fixedSize) where T : struct, IStreamableStruct
        {
            if (v is T value && WriteOptional(tag, OptionalFormat.VSize))
            {
                WriteSize(fixedSize);
                value.IceWrite(this);
            }
        }

        /// <summary>Writes a tagged variable-size struct to the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The struct to write.</param>
        public void WriteTaggedStruct<T>(int tag, T? v) where T : struct, IStreamableStruct
        {
            if (v is T value && WriteOptional(tag, OptionalFormat.FSize))
            {
                var pos = StartSize();
                value.IceWrite(this);
                EndSize(pos);
            }
        }

        /// <summary>Writes a sequence to the stream.</summary>
        /// <param name="v">The sequence to write.</param>
        /// <param name="writer">The delegate that writes each element to the stream.</param>
        public void WriteSeq<T>(IEnumerable<T> v, OutputStreamWriter<T> writer)
        {
            WriteSize(v.Count()); // potentially slow Linq Count()
            foreach (T item in v)
            {
                writer(this, item);
            }
        }

        /// <summary>Writes a sequence to the stream. Elements of the sequence are mapped Slice structs.</summary>
        /// <param name="v">The sequence to write.</param>
        /// <param name="writer">The delegate that writes each element to the stream.</param>
        public void WriteSeq<T>(IEnumerable<T> v, OutputStreamStructWriter<T> writer)
            where T : struct, IStreamableStruct
        {
            WriteSize(v.Count()); // potentially slow Linq Count()
            foreach (T item in v)
            {
                writer(this, item);
            }
        }

        /// <summary>Writes a tagged sequence of fixed-size elements to the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The sequence to write.</param>
        /// <param name="elementSize">The fixed size of each element of the sequence, in bytes.</param>
        /// <param name="writer">The delegate that writes each element to the stream.</param>
        public void WriteTaggedSeq<T>(int tag, IEnumerable<T>? v, int elementSize, OutputStreamWriter<T> writer)
        {
            Debug.Assert(elementSize > 0);
            if (v is IEnumerable<T> value && WriteOptional(tag, OptionalFormat.VSize))
            {
                int count = value.Count(); // potentially slow Linq Count()

                if (elementSize > 1)
                {
                    // First write the size in bytes, so that the reader can skip it. We optimize-out this byte size
                    // when size is 1.
                    WriteSize(count == 0 ? 1 : (count * elementSize) + (count > 254 ? 5 : 1));
                }
                WriteSize(count);
                foreach (T item in value)
                {
                    writer(this, item);
                }
            }
        }

        /// <summary>Writes a tagged sequence of fixed-size elements to the stream. Elements of the sequence are
        /// mapped Slice structs.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The sequence to write.</param>
        /// <param name="elementSize">The fixed size of each element of the sequence, in bytes.</param>
        /// <param name="writer">The delegate that writes each element to the stream.</param>
        public void WriteTaggedSeq<T>(int tag, IEnumerable<T>? v, int elementSize,
            OutputStreamStructWriter<T> writer) where T : struct, IStreamableStruct
        {
            Debug.Assert(elementSize > 0);
            if (v is IEnumerable<T> value && WriteOptional(tag, OptionalFormat.VSize))
            {
                int count = value.Count(); // potentially slow Linq Count()

                if (elementSize > 1)
                {
                    // First write the size in bytes, so that the reader can skip it. We optimize-out this byte size
                    // when size is 1.
                    WriteSize(count == 0 ? 1 : (count * elementSize) + (count > 254 ? 5 : 1));
                }
                WriteSize(count);
                foreach (T item in value)
                {
                    writer(this, item);
                }
            }
        }

        /// <summary>Writes a tagged sequence of variable-size elements to the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The sequence to write.</param>
        /// <param name="writer">The delegate that writes each element to the stream.</param>
        public void WriteTaggedSeq<T>(int tag, IEnumerable<T>? v, OutputStreamWriter<T> writer)
        {
            if (v is IEnumerable<T> value && WriteOptional(tag, OptionalFormat.FSize))
            {
                Position pos = StartSize();
                WriteSeq(value, writer);
                EndSize(pos);
            }
        }

        /// <summary>Writes a tagged sequence of variable-size elements to the stream. Elements of the sequence are
        /// mapped Slice structs.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The sequence to write.</param>
        /// <param name="writer">The delegate that writes each element to the stream.</param>
        public void WriteTaggedSeq<T>(int tag, IEnumerable<T>? v, OutputStreamStructWriter<T> writer)
            where T : struct, IStreamableStruct
        {
            if (v is IEnumerable<T> value && WriteOptional(tag, OptionalFormat.FSize))
            {
                Position pos = StartSize();
                WriteSeq(value, writer);
                EndSize(pos);
            }
        }

        /// <summary>Writes a dictionary to the stream.</summary>
        /// <param name="v">The dictionary to write.</param>
        /// <param name="keyWriter">The delegate that writes each key to the stream.</param>
        /// <param name="valueWriter">The delegate that writes each value to the stream.</param>
        public void WriteDict<TKey, TValue>(IReadOnlyDictionary<TKey, TValue> v,
                                            OutputStreamWriter<TKey> keyWriter,
                                            OutputStreamWriter<TValue> valueWriter) where TKey : notnull
        {
            WriteSize(v.Count);
            foreach ((TKey key, TValue value) in v)
            {
                keyWriter(this, key);
                valueWriter(this, value);
            }
        }

        /// <summary>Writes a dictionary to the stream. The dictionary's key is a mapped Slice struct.</summary>
        /// <param name="v">The dictionary to write.</param>
        /// <param name="keyWriter">The delegate that writes each key to the stream.</param>
        /// <param name="valueWriter">The delegate that writes each value to the stream.</param>
        public void WriteDict<TKey, TValue>(IReadOnlyDictionary<TKey, TValue> v,
                                            OutputStreamStructWriter<TKey> keyWriter,
                                            OutputStreamWriter<TValue> valueWriter)
            where TKey : struct, IStreamableStruct
        {
            WriteSize(v.Count);
            foreach ((TKey key, TValue value) in v)
            {
                keyWriter(this, key);
                valueWriter(this, value);
            }
        }

        /// <summary>Writes a dictionary to the stream. The dictionary's value is a mapped Slice struct.</summary>
        /// <param name="v">The dictionary to write.</param>
        /// <param name="keyWriter">The delegate that writes each key to the stream.</param>
        /// <param name="valueWriter">The delegate that writes each value to the stream.</param>
        public void WriteDict<TKey, TValue>(IReadOnlyDictionary<TKey, TValue> v,
                                            OutputStreamWriter<TKey> keyWriter,
                                            OutputStreamStructWriter<TValue> valueWriter)
            where TKey : notnull
            where TValue : struct, IStreamableStruct
        {
            WriteSize(v.Count);
            foreach ((TKey key, TValue value) in v)
            {
                keyWriter(this, key);
                valueWriter(this, value);
            }
        }

        /// <summary>Writes a dictionary to the stream. Both the dictionary's key and value are mapped Slice structs.
        /// </summary>
        /// <param name="v">The dictionary to write.</param>
        /// <param name="keyWriter">The delegate that writes each key to the stream.</param>
        /// <param name="valueWriter">The delegate that writes each value to the stream.</param>
        public void WriteDict<TKey, TValue>(IReadOnlyDictionary<TKey, TValue> v,
                                            OutputStreamStructWriter<TKey> keyWriter,
                                            OutputStreamStructWriter<TValue> valueWriter)
            where TKey : struct, IStreamableStruct
            where TValue : struct, IStreamableStruct
        {
            WriteSize(v.Count);
            foreach ((TKey key, TValue value) in v)
            {
                keyWriter(this, key);
                valueWriter(this, value);
            }
        }

        /// <summary>Writes a tagged dictionary with fixed-size elements to the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The dictionary to write.</param>
        /// <param name="elementSize">The size of each element (key + value), in bytes.</param>
        /// <param name="keyWriter">The delegate that writes each key to the stream.</param>
        /// <param name="valueWriter">The delegate that writes each value to the stream.</param>
        public void WriteTaggedDict<TKey, TValue>(int tag,
                                                  IReadOnlyDictionary<TKey, TValue>? v,
                                                  int elementSize,
                                                  OutputStreamWriter<TKey> keyWriter,
                                                  OutputStreamWriter<TValue> valueWriter)
            where TKey : notnull
        {
            Debug.Assert(elementSize > 1);
            if (v is IReadOnlyDictionary<TKey, TValue> dict && WriteOptional(tag, OptionalFormat.VSize))
            {
                int count = dict.Count;
                WriteSize(count == 0 ? 1 : (count * elementSize) + (count > 254 ? 5 : 1));
                WriteDict(dict, keyWriter, valueWriter);
            }
        }

        /// <summary>Writes a tagged dictionary with fixed-size elements to the stream. The dictionary's key is a mapped
        /// Slice struct.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The dictionary to write.</param>
        /// <param name="elementSize">The size of each element (key + value), in bytes.</param>
        /// <param name="keyWriter">The delegate that writes each key to the stream.</param>
        /// <param name="valueWriter">The delegate that writes each value to the stream.</param>
        public void WriteTaggedDict<TKey, TValue>(int tag,
                                                  IReadOnlyDictionary<TKey, TValue>? v,
                                                  int elementSize,
                                                  OutputStreamStructWriter<TKey> keyWriter,
                                                  OutputStreamWriter<TValue> valueWriter)
            where TKey : struct, IStreamableStruct
        {
            Debug.Assert(elementSize > 1);
            if (v is IReadOnlyDictionary<TKey, TValue> dict && WriteOptional(tag, OptionalFormat.VSize))
            {
                int count = dict.Count;
                WriteSize(count == 0 ? 1 : (count * elementSize) + (count > 254 ? 5 : 1));
                WriteDict(dict, keyWriter, valueWriter);
            }
        }

        /// <summary>Writes a tagged dictionary with fixed-size elements to the stream. The dictionary's value is a
        /// mapped Slice struct.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The dictionary to write.</param>
        /// <param name="elementSize">The size of each element (key + value), in bytes.</param>
        /// <param name="keyWriter">The delegate that writes each key to the stream.</param>
        /// <param name="valueWriter">The delegate that writes each value to the stream.</param>
        public void WriteTaggedDict<TKey, TValue>(int tag,
                                                  IReadOnlyDictionary<TKey, TValue>? v,
                                                  int elementSize,
                                                  OutputStreamWriter<TKey> keyWriter,
                                                  OutputStreamStructWriter<TValue> valueWriter)
            where TKey : notnull
            where TValue : struct, IStreamableStruct
        {
            Debug.Assert(elementSize > 1);
            if (v is IReadOnlyDictionary<TKey, TValue> dict && WriteOptional(tag, OptionalFormat.VSize))
            {
                int count = dict.Count;
                WriteSize(count == 0 ? 1 : (count * elementSize) + (count > 254 ? 5 : 1));
                WriteDict(dict, keyWriter, valueWriter);
            }
        }

        /// <summary>Writes a tagged dictionary with fixed-size elements to the stream. Both the dictionary's key and
        /// value are mapped Slice structs.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The dictionary to write.</param>
        /// <param name="elementSize">The size of each element (key + value), in bytes.</param>
        /// <param name="keyWriter">The delegate that writes each key to the stream.</param>
        /// <param name="valueWriter">The delegate that writes each value to the stream.</param>
        public void WriteTaggedDict<TKey, TValue>(int tag,
                                                  IReadOnlyDictionary<TKey, TValue>? v,
                                                  int elementSize,
                                                  OutputStreamStructWriter<TKey> keyWriter,
                                                  OutputStreamStructWriter<TValue> valueWriter)
            where TKey : struct, IStreamableStruct
            where TValue : struct, IStreamableStruct
        {
            Debug.Assert(elementSize > 1);
            if (v is IReadOnlyDictionary<TKey, TValue> dict && WriteOptional(tag, OptionalFormat.VSize))
            {
                int count = dict.Count;
                WriteSize(count == 0 ? 1 : (count * elementSize) + (count > 254 ? 5 : 1));
                WriteDict(dict, keyWriter, valueWriter);
            }
        }

        /// <summary>Writes a tagged dictionary with variable-size elements to the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The dictionary to write.</param>
        /// <param name="keyWriter">The delegate that writes each key to the stream.</param>
        /// <param name="valueWriter">The delegate that writes each value to the stream.</param>
        public void WriteTaggedDict<TKey, TValue>(int tag,
                                                  IReadOnlyDictionary<TKey, TValue>? v,
                                                  OutputStreamWriter<TKey> keyWriter,
                                                  OutputStreamWriter<TValue> valueWriter)
            where TKey : notnull
        {
            if (v is IReadOnlyDictionary<TKey, TValue> dict && WriteOptional(tag, OptionalFormat.FSize))
            {
                Position pos = StartSize();
                WriteDict(dict, keyWriter, valueWriter);
                EndSize(pos);
            }
        }

        /// <summary>Writes a tagged dictionary with variable-size elements to the stream. The dictionary's key is
        /// a mapped Slice struct.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The dictionary to write.</param>
        /// <param name="keyWriter">The delegate that writes each key to the stream.</param>
        /// <param name="valueWriter">The delegate that writes each value to the stream.</param>
        public void WriteTaggedDict<TKey, TValue>(int tag,
                                                  IReadOnlyDictionary<TKey, TValue>? v,
                                                  OutputStreamStructWriter<TKey> keyWriter,
                                                  OutputStreamWriter<TValue> valueWriter)
            where TKey : struct, IStreamableStruct
        {
            if (v is IReadOnlyDictionary<TKey, TValue> dict && WriteOptional(tag, OptionalFormat.FSize))
            {
                Position pos = StartSize();
                WriteDict(dict, keyWriter, valueWriter);
                EndSize(pos);
            }
        }

        /// <summary>Writes a tagged dictionary with variable-size elements to the stream. The dictionary's value is
        /// a mapped Slice struct.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The dictionary to write.</param>
        /// <param name="keyWriter">The delegate that writes each key to the stream.</param>
        /// <param name="valueWriter">The delegate that writes each value to the stream.</param>
        public void WriteTaggedDict<TKey, TValue>(int tag,
                                                  IReadOnlyDictionary<TKey, TValue>? v,
                                                  OutputStreamWriter<TKey> keyWriter,
                                                  OutputStreamStructWriter<TValue> valueWriter)
            where TKey : notnull
            where TValue : struct, IStreamableStruct
        {
            if (v is IReadOnlyDictionary<TKey, TValue> dict && WriteOptional(tag, OptionalFormat.FSize))
            {
                Position pos = StartSize();
                WriteDict(dict, keyWriter, valueWriter);
                EndSize(pos);
            }
        }

        /// <summary>Writes a tagged dictionary with variable-size elements to the stream. Both the dictionary's key and
        /// value are mapped Slice structs.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The dictionary to write.</param>
        /// <param name="keyWriter">The delegate that writes each key to the stream.</param>
        /// <param name="valueWriter">The delegate that writes each value to the stream.</param>
        public void WriteTaggedDict<TKey, TValue>(int tag,
                                                  IReadOnlyDictionary<TKey, TValue>? v,
                                                  OutputStreamStructWriter<TKey> keyWriter,
                                                  OutputStreamStructWriter<TValue> valueWriter)
            where TKey : struct, IStreamableStruct
            where TValue : struct, IStreamableStruct
        {
            if (v is IReadOnlyDictionary<TKey, TValue> dict && WriteOptional(tag, OptionalFormat.FSize))
            {
                Position pos = StartSize();
                WriteDict(dict, keyWriter, valueWriter);
                EndSize(pos);
            }
        }

        /// <summary>Writes a user exception to the stream.</summary>
        /// <param name="v">The user exception to write.</param>
        public void WriteException(RemoteException v)
        {
            Debug.Assert(_mainEncaps != null && _endpointEncaps == null && _current == null);
            Debug.Assert(_format == FormatType.Sliced);
            Debug.Assert(!(v is ObjectNotExistException)); // temporary
            Debug.Assert(!(v is OperationNotExistException)); // temporary
            Debug.Assert(!(v is UnhandledException)); // temporary
            Push(InstanceType.Exception);

            v.Write(this);
            Pop(null);
        }

        /// <summary>Write a byte at a given position of the stream.</summary>
        /// <param name="v">The byte value to write.</param>
        /// <param name="pos">The position to write to.</param>
        private void RewriteByte(byte v, Position pos)
        {
            ArraySegment<byte> segment = _segmentList[pos.Segment];
            if (pos.Offset < segment.Count)
            {
                segment[pos.Offset] = v;
            }
            else
            {
                segment = _segmentList[pos.Segment + 1];
                segment[0] = v;
            }
        }

        /// <summary>Write an integer number (4 bytes) at a given position of the stream.</summary>
        /// <param name="v">The integer value to write.</param>
        /// <param name="pos">The position to write to.</param>
        private void RewriteInt(int v, Position pos)
        {
            Debug.Assert(pos.Segment < _segmentList.Count);
            Debug.Assert(pos.Offset <= Size - _segmentList.Take(pos.Segment).Sum(data => data.Count),
                $"offset: {pos.Offset} segment size: {Size - _segmentList.Take(pos.Segment).Sum(data => data.Count)}");
            Span<byte> data = stackalloc byte[4];
            MemoryMarshal.Write(data, ref v);
            RewriteSpan(data, pos);
        }

        internal static void WriteInt(int v, Span<byte> data) => MemoryMarshal.Write(data, ref v);

        internal void RewriteSize(int size, Position pos)
        {
            if (size < 255)
            {
                ArraySegment<byte> segment = _segmentList[pos.Segment];
                segment[pos.Offset] = (byte) size;
            }
            else
            {
                Span<byte> data = stackalloc byte[5];
                data[0] = 255;
                WriteInt(size, data.Slice(1, 4));
                RewriteSpan(data, pos);
            }
        }

        private void RewriteSpan(Span<byte> data, Position pos)
        {
            ArraySegment<byte> segment = _segmentList[pos.Segment];
            int remaining = Math.Min(data.Length, segment.Count - pos.Offset);
            if (remaining > 0)
            {
                data.Slice(0, remaining).CopyTo(segment.AsSpan(pos.Offset, remaining));
            }

            if (remaining < data.Length)
            {
                segment = _segmentList[pos.Segment + 1];
                data[remaining..].CopyTo(segment.AsSpan(0, data.Length - remaining));
            }
        }

        /// <summary>Return all the data as a byte array.</summary>
        /// <returns>A byte array with the contest of the buffer.</returns>
        public byte[] ToArray()
        {
            byte[] data = new byte[Size];
            int offset = 0;
            foreach (ArraySegment<byte> segment in _segmentList)
            {
                Debug.Assert(segment.Array != null);
                Buffer.BlockCopy(segment.Array, segment.Offset, data, offset, Math.Min(segment.Count, Size - offset));
                offset += segment.Count;
            }
            return data;
        }

        /// <summary>Returns the distance in bytes from start position to the current position.</summary>
        /// <param name="start">The start position from where to calculate distance to current position.</param>
        /// <returns>The distance in bytes from the current position to the start position.</returns>
        private int Distance(Position start)
        {
            Debug.Assert(_tail.Segment > start.Segment ||
                        (_tail.Segment == start.Segment && _tail.Offset >= start.Offset));

            return Distance(_segmentList, start, _tail);
        }

        internal static int Distance(IList<ArraySegment<byte>> data, Position start, Position end)
        {
            // If both the start and end position are in the same array segment just
            // compute the offsets distance.
            if (start.Segment == end.Segment)
            {
                return end.Offset - start.Offset;
            }

            // If start and end position are in different segments we need to accumulate the
            // size from start offset to the end of the start segment, the size of the intermediary
            // segments, and the current offset into the last segment.
            ArraySegment<byte> segment = data[start.Segment];
            int size = segment.Count - start.Offset;
            for (int i = start.Segment + 1; i < end.Segment; ++i)
            {
                size += data[i].Count;
            }
            return size + end.Offset;
        }

        /// <summary>Writes a span of bytes. The stream capacity is expanded if required, the size and tail position are
        /// increased according to the span length.</summary>
        /// <param name="span">The data to write as a span of bytes.</param>
        internal void WriteByteSpan(ReadOnlySpan<byte> span)
        {
            int length = span.Length;
            Expand(length);
            Size += length;
            int offset = _tail.Offset;
            int remaining = _currentSegment.Count - offset;
            if (remaining > 0)
            {
                int sz = Math.Min(length, remaining);
                if (length > remaining)
                {
                    span.Slice(0, remaining).CopyTo(_currentSegment.AsSpan(offset, sz));
                }
                else
                {
                    span.CopyTo(_currentSegment.AsSpan(offset, length));
                }
                _tail.Offset += sz;
                length -= sz;
            }

            if (length > 0)
            {
                _currentSegment = _segmentList[++_tail.Segment];
                if (remaining == 0)
                {
                    span.CopyTo(_currentSegment.AsSpan(0, length));
                }
                else
                {
                    span.Slice(remaining, length).CopyTo(_currentSegment.AsSpan(0, length));
                }
                _tail.Offset = length;
            }
        }

        /// <summary>Returns the current position and write an int (four bytes) placeholder
        /// for a fixed-length (32-bit) size value, the position can be used to calculate and
        /// re-write the size later.</summary>
        private Position StartSize()
        {
            Position pos = _tail;
            WriteInt(0); // Placeholder for 32-bit size
            return pos;
        }

        /// <summary>Computes the amount of data written from the start position to the current position
        /// and writes that value to the start position.</summary>
        /// <param name="start">The start position.</param>
        private void EndSize(Position start)
        {
            Debug.Assert(start.Offset >= 0);
            RewriteInt(Distance(start) - 4, start);
        }

        /// <summary>Expand the stream to make room for more data, if the stream
        /// remaining bytes are not enough to hold the given number of bytes allocate
        /// a new byte array, after this method return the stream has enough free space
        /// to write the given number of bytes.</summary>
        /// <param name="n">The number of bytes to accommodate in the stream.</param>
        private void Expand(int n)
        {
            int remaining = _capacity - Size;
            if (n > remaining)
            {
                int size = Math.Max(DefaultSegmentSize, _currentSegment.Count * 2);
                size = Math.Max(n - remaining, size);
                byte[] buffer = new byte[size];
                _segmentList.Add(buffer);
                if (_segmentList.Count == 1)
                {
                    _currentSegment = buffer;
                }
                _capacity += buffer.Length;
            }
        }

        /// <summary>Writes a fixed-size numeric to the stream.</summary>
        /// <param name="v">The numeric value to write to the stream.</param>
        /// <param name="elementSize">The size in bytes of the numeric type.</param>
        private void WriteFixedSizeNumeric<T>(T v, int elementSize) where T : struct
        {
            Debug.Assert(elementSize > 1); // for size 1, we write the byte directly
            Span<byte> data = stackalloc byte[elementSize];
            MemoryMarshal.Write(data, ref v);
            WriteByteSpan(data);
        }

        /// <summary>Writes a sequence of fixed-size numeric type, such as int and long, to the stream.</summary>
        /// <param name="v">The sequence of numeric types.</param>
        // This method works because (as long as) there is no padding in the memory representation of the
        // ReadOnlySpan.
        private void WriteFixedSizeNumericSeq<T>(ReadOnlySpan<T> v) where T : struct
        {
            WriteSize(v.Length);
            WriteByteSpan(MemoryMarshal.AsBytes(v));
        }

        /// <summary>Writes a tagged sequence of fixed-size numeric type to the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The sequence to write.</param>
        /// <param name="elementSize">The fixed size of each element of the sequence.</param>
        private void WriteFixedSizeNumericSeq<T>(int tag, ReadOnlySpan<T> v, int elementSize) where T : struct
        {
            Debug.Assert(elementSize > 0);

            // A null T[]? or List<T>? is implicitly converted into a default aka null ReadOnlyMemory<T> or
            // ReadOnlySpan<T>. Furthermore, the span of a default ReadOnlyMemory<T> is a default ReadOnlySpan<T>, which
            // is distinct from the span of an empty sequence. This is why the "v != null" below works correctly.
            if (v != null && WriteOptional(tag, OptionalFormat.VSize))
            {
                if (elementSize > 1)
                {
                    // This size is redundant and optimized out by the encoding when elementSize is 1.
                    WriteSize(v.Length == 0 ? 1 : (v.Length * elementSize) + (v.Length > 254 ? 5 : 1));
                }
                WriteFixedSizeNumericSeq(v);
            }
        }

        internal void StartEndpointEncapsulation() => StartEndpointEncapsulation(Encoding);

        internal void StartEndpointEncapsulation(Encoding encoding)
        {
            Debug.Assert(_endpointEncaps == null);
            encoding.CheckSupported();

            _endpointEncaps = new Encaps(Encoding, _tail);
            Encoding = encoding;
            // We didn't change _format, so no need to restore it.

            WriteEncapsulationHeader(0, Encoding);
        }

        internal void EndEndpointEncapsulation()
        {
            Debug.Assert(_endpointEncaps.HasValue);

            // Size includes size and version.
            RewriteInt(Distance(_endpointEncaps.Value.StartPos), _endpointEncaps.Value.StartPos);

            Encoding = _endpointEncaps.Value.OldEncoding;
            // No need to restore format since it didn't change.
            _endpointEncaps = null;
        }

        /// <summary>Writes an empty encapsulation.</summary>
        internal Position WriteEmptyEncapsulation(Encoding encoding)
        {
            WriteEncapsulationHeader(6, encoding);
            _segmentList[_tail.Segment] = _segmentList[_tail.Segment].Slice(0, _tail.Offset);
            return _tail;
        }

        internal void WriteSlicedData(SlicedData slicedData)
        {
            Debug.Assert(_current != null);
            // We only remarshal preserved slices if we are using the sliced format. Otherwise, we ignore the preserved
            // slices, which essentially "slices" the instance into the most-derived type known by the sender.
            if (_format != FormatType.Sliced)
            {
                throw new NotSupportedException($"cannot write sliced data into payload using {_format} format");
            }
            if (Encoding != slicedData.Encoding)
            {
                throw new NotSupportedException(@$"cannot write sliced data encoded with encoding {slicedData.Encoding
                    } into payload encoded with encoding {Encoding}");
            }

            bool firstSlice = true;
            // There is always at least one slice. See SlicedData constructor.
            foreach (SliceInfo info in slicedData.Slices)
            {
                IceStartSlice(info.TypeId ?? "", firstSlice, null, info.CompactId);
                firstSlice = false;

                // Write the bytes associated with this slice.
                WriteByteSpan(info.Bytes.Span);

                if (info.HasOptionalMembers)
                {
                    _current.SliceFlags |= EncodingDefinitions.SliceFlags.HasTaggedMembers;
                }

                // Make sure to also re-write the instance indirection table.
                // These instances will be marshaled (and assigned instance IDs) in IceEndSlice.
                if (info.Instances.Count > 0)
                {
                    _current.IndirectionTable ??= new List<AnyClass>();
                    Debug.Assert(_current.IndirectionTable.Count == 0);
                    _current.IndirectionTable.AddRange(info.Instances);
                }
                IceEndSlice(info.IsLastSlice); // TODO: can we check it's indeed the last slice?
            }
            Debug.Assert(!firstSlice);
        }

        private void WriteEncapsulationHeader(int size, Encoding encoding)
        {
            WriteInt(size);
            WriteByte(encoding.Major);
            WriteByte(encoding.Minor);
        }

        private int RegisterTypeId(string typeId)
        {
            _typeIdMap ??= new Dictionary<string, int>();

            if (_typeIdMap.TryGetValue(typeId, out int index))
            {
                return index;
            }
            else
            {
                index = _typeIdMap.Count + 1;
                _typeIdMap.Add(typeId, index);
                return -1;
            }
        }

        // Write this class instance inline if not previously marshaled, otherwise just write its instance ID.
        private void WriteInstance(AnyClass v)
        {
            Debug.Assert(v != null);

            // If the instance was already marshaled, just write its instance ID.
            if (_instanceMap != null && _instanceMap.TryGetValue(v, out int instanceId))
            {
                WriteSize(instanceId);
                return;
            }
            else
            {
                _instanceMap ??= new Dictionary<AnyClass, int>();

                // We haven't seen this instance previously, so we create a new instance ID and insert the instance
                // and its ID in the marshaled map, before writing the instance inline.
                // The instance IDs start at 2 (0 means null and 1 means the instance is written immediately after).
                instanceId = _instanceMap.Count + 2;
                _instanceMap.Add(v, instanceId);

                WriteSize(1); // Class instance marker.

                InstanceData? savedInstanceData = Push(InstanceType.Class);
                v.Write(this);
                Pop(savedInstanceData);
            }
        }

        private InstanceData? Push(InstanceType instanceType)
        {
            InstanceData? savedInstanceData = _current;
            _current = new InstanceData(instanceType);
            return savedInstanceData;
        }

        private void Pop(InstanceData? savedInstanceData) => _current = savedInstanceData;

        private enum InstanceType { Class, Exception }

        private sealed class InstanceData
        {
            internal readonly InstanceType InstanceType;

            // The following fields are used and reused for all the slices of a class or exception instance.
            internal EncodingDefinitions.SliceFlags SliceFlags = default;

            // Position of the optional slice size.
            internal Position SliceSizePos = new Position(0, 0);

            // Position of the first data member in the slice, just after the optional slice size.
            internal Position SliceFirstMemberPos = new Position(0, 0);

            // Position of the slice flags.
            internal Position SliceFlagsPos = new Position(0, 0);

            // The indirection table and indirection map are only used for the sliced format.
            internal List<AnyClass>? IndirectionTable;
            internal Dictionary<AnyClass, int>? IndirectionMap;

            internal InstanceData(InstanceType instanceType) => InstanceType = instanceType;
        }

        private readonly struct Encaps
        {
            // Old Encoding
            internal readonly Encoding OldEncoding;

            internal readonly Position StartPos;

            internal Encaps(Encoding oldEncoding, Position startPos)
            {
                OldEncoding = oldEncoding;
                StartPos = startPos;
            }
        }
    }
}
