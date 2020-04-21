//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using IceInternal;

using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Binary;

namespace Ice
{
    public delegate T InputStreamReader<T>(InputStream ins);

    /// <summary>
    /// Interface for input streams used to extract Slice types from a sequence of bytes.
    /// </summary>
    public sealed class InputStream
    {
        public static readonly InputStreamReader<bool> IceReaderIntoBool = (istr) => istr.ReadBool();
        public static readonly InputStreamReader<byte> IceReaderIntoByte = (istr) => istr.ReadByte();
        public static readonly InputStreamReader<short> IceReaderIntoShort = (istr) => istr.ReadShort();
        public static readonly InputStreamReader<int> IceReaderIntoInt = (istr) => istr.ReadInt();
        public static readonly InputStreamReader<long> IceReaderIntoLong = (istr) => istr.ReadLong();
        public static readonly InputStreamReader<float> IceReaderIntoFloat = (istr) => istr.ReadFloat();
        public static readonly InputStreamReader<double> IceReaderIntoDouble = (istr) => istr.ReadDouble();
        public static readonly InputStreamReader<string> IceReaderIntoString = (istr) => istr.ReadString();
        public static readonly InputStreamReader<string[]> IceReaderIntoStringArray = (istr) => istr.ReadStringArray();

        /// <summary>
        /// The communicator associated with this stream.
        /// </summary>
        /// <value>The communicator.</value>
        public Communicator Communicator { get; }

        // Returns the sliced data held by the current instance.
        internal SlicedData? SlicedData
        {
            get
            {
                Debug.Assert(_current != null);
                if (_current.Slices == null)
                {
                    return null;
                }
                else
                {
                    Debug.Assert(_encoding != null);
                    return new SlicedData(_encoding.Value, _current.Slices);
                }
            }
        }

        /// <summary>The position (offset) in the underlying buffer.</summary>
        internal int Pos
        {
            get => _pos;
            set
            {
                if (value < 0 || value > _buffer.Count)
                {
                    throw new ArgumentOutOfRangeException(nameof(Pos),
                        "the position value is outside the buffer bounds");
                }
                _pos = value;
            }
        }

        /// <summary>Returns the current size of the stream.</summary>
        internal int Size => _buffer.Count;

        // When set, we are in reading a top-level encapsulation.
        private Encaps? _mainEncaps;

        // Temporary upper limit set by an encapsulation. See Remaining.
        private int? _limit;

        // The sum of all the minimum sizes (in bytes) of the sequences read in this buffer. Must not exceed the buffer
        // size.
        private int _minTotalSeqSize = 0;

        private readonly ArraySegment<byte> _buffer;
        private int _pos;

        // Map of type ID index to type ID string.
        // When reading a top-level encapsulation, we assign a type ID index (starting with 1) to each type ID we
        // read, in order. Since this map is a list, we lookup a previously assigned type ID string with
        // _typeIdMap[index - 1].
        private List<string>? _typeIdMap;
        private int _posAfterLatestInsertedTypeId = 0;

        // The remaining fields are used for class/exception unmarshaling.
        // Class/exception unmarshaling is allowed only when _mainEncaps != null

        // Map of class instance ID to class instance.
        // When reading a top-level encapsulation:
        //  - Instance ID = 0 means null
        //  - Instance ID = 1 means the instance is encoded inline afterwards
        //  - Instance ID > 1 means a reference to a previously read instance, found in this map.
        // Since the map is actually a list, we use instance ID - 2 to lookup an instance.
        private List<AnyClass>? _instanceMap;
        private int _classGraphDepth = 0;
        private Encoding? _encoding;

         // Data for the class or exception instance that is currently getting unmarshaled.
        private InstanceData? _current;

        /// <summary>This constructor uses the given encoding version.</summary>
        /// <param name="communicator">The communicator to use when initializing the stream.</param>
        /// <param name="buffer">The stream initial data.</param>
        /// <param name="pos">The zero base byte offset for the next read operation.</param>
        internal InputStream(Communicator communicator, ArraySegment<byte> buffer, int pos = 0)
        {
            Communicator = communicator;
            _buffer = buffer;
            _pos = pos;
        }

        /// <summary>Reads the start of an encapsulation.</summary>
        /// <returns>The encoding of the encapsulation.</returns>
        public Encoding StartEncapsulation()
        {
            Debug.Assert(_mainEncaps == null);
            (Encoding Encoding, int Size) encapsHeader = ReadEncapsulationHeader();
            Debug.Assert(encapsHeader.Encoding == Encoding.V1_1); // TODO: temporary
            _mainEncaps = new Encaps(_limit, encapsHeader.Size);
            _limit = _pos + encapsHeader.Size - 6;
            _encoding = encapsHeader.Encoding;
            return encapsHeader.Encoding;
        }

        /// <summary>Ends an encapsulation started with StartEncpasulation or RestartEncapsulation.</summary>
        public void EndEncapsulation()
        {
            Debug.Assert(_mainEncaps != null);
            SkipTaggedMembers();

            if (_buffer.Count - _pos != 0)
            {
                throw new InvalidDataException($"{_buffer.Count - _pos} bytes remaining in encapsulation");
            }
            _limit = _mainEncaps.Value.OldLimit;
        }

        /// <summary>Go to the end of the current main encapsulation, if we are in one.</summary>
        public void SkipCurrentEncapsulation()
        {
            if (_mainEncaps != null)
            {
                _pos = _limit!.Value;
                EndEncapsulation();
            }
        }

        /// <summary>
        /// Returns a blob of bytes representing an encapsulation. The encapsulation's encoding version
        /// is returned in the argument.
        /// </summary>
        /// <param name="encoding">The encapsulation's encoding version.</param>
        /// <returns>The encoded encapsulation.</returns>
        public ArraySegment<byte> ReadEncapsulation(out Encoding encoding)
        {
            (Encoding Encoding, int Size) encapsHeader = ReadEncapsulationHeader();
            _pos -= 6;
            encoding = encapsHeader.Encoding;
            ArraySegment<byte> data = _buffer.Slice(_pos, encapsHeader.Size);
            _pos += encapsHeader.Size;
            return data;
        }

        /// <summary>
        /// Skips over an encapsulation.
        /// </summary>
        /// <returns>The encoding version of the skipped encapsulation.</returns>
        public Encoding SkipEncapsulation()
        {
            (Encoding Encoding, int Size) encapsHeader = ReadEncapsulationHeader();

            int pos = _pos + encapsHeader.Size - 6;
            if (pos > _buffer.Count)
            {
                throw new InvalidDataException("the encapsulation's size extends beyond the end of the frame");
            }
            _pos = pos;
            return encapsHeader.Encoding;
        }

        // Start reading a slice of a class or exception instance.
        // This is an Ice-internal method marked public because it's called by the generated code.
        // typeId is the expected type ID of this slice.
        // firstSlice is true when reading the first (most derived) slice of an instance.
        [EditorBrowsable(EditorBrowsableState.Never)]
        public void IceStartSlice(string typeId, bool firstSlice)
        {
            Debug.Assert(_mainEncaps != null);
            if (firstSlice)
            {
                Debug.Assert(_current != null && (_current.SliceTypeId == null || _current.SliceTypeId == typeId));
                if (_current.InstanceType == InstanceType.Class)
                {
                    // For exceptions, we read it for the first slice in ThrowException.
                    ReadIndirectionTableIntoCurrent();
                }

                // We can discard all the unknown slices: the generated code calls IceStartSliceAndGetSlicedData to
                // preserve them and it just called IceStartSlice instead.
                _current.Slices = null;
            }
            else
            {
                string? headerTypeId = ReadSliceHeaderIntoCurrent();
                Debug.Assert(headerTypeId == null || headerTypeId == typeId);
                ReadIndirectionTableIntoCurrent();
            }
        }

        // Start reading the first slice of an instance and get the unknown slices for this instances that were
        // previously saved (if any).
        // This is an Ice-internal method marked public because it's called by the generated code.
        // typeId is the expected typeId of this slice.
        [EditorBrowsable(EditorBrowsableState.Never)]
        public SlicedData? IceStartSliceAndGetSlicedData(string typeId)
        {
            Debug.Assert(_mainEncaps != null);
            // Called by generated code for first slice instead of IceStartSlice
            Debug.Assert(_current != null && (_current.SliceTypeId == null || _current.SliceTypeId == typeId));
            if (_current.InstanceType == InstanceType.Class)
            {
                    // For exceptions, we read it for the first slice in ThrowException.
                    ReadIndirectionTableIntoCurrent();
            }
            return SlicedData;
        }

        // Tells the InputStream the end of a class or exception slice was reached.
        // This is an Ice-internal method marked public because it's called by the generated code.
        [EditorBrowsable(EditorBrowsableState.Never)]
        public void IceEndSlice()
        {
            // Note that IceEndSlice is not called when we call SkipSlice.
            Debug.Assert(_mainEncaps != null && _current != null);
            if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasTaggedMembers) != 0)
            {
                SkipTaggedMembers();
            }
            if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasIndirectionTable) != 0)
            {
                Debug.Assert(_current.PosAfterIndirectionTable.HasValue && _current.IndirectionTable != null);
                _pos = _current.PosAfterIndirectionTable.Value;
                _current.PosAfterIndirectionTable = null;
                _current.IndirectionTable = null;
            }
        }

        /// <summary>
        /// Extracts a size from the stream.
        /// </summary>
        /// <returns>The extracted size.</returns>
        public int ReadSize()
        {
            byte b = ReadByte();
            if (b < 255)
            {
                return b;
            }

            int size = ReadInt();
            if (size < 0)
            {
                throw new InvalidDataException($"read invalid size: {size}");
            }
            return size;
        }

        public static int ReadSize(ArraySegment<byte> buffer)
        {
            byte b = buffer[0];
            if (b < 255)
            {
                return b;
            }

            int size = ReadInt(buffer.AsSpan(1, 4));
            if (size < 0)
            {
                throw new InvalidDataException($"read invalid size: {size}");
            }
            return size;
        }

        /// <summary>
        /// Reads a sequence size and make sure there is enough space in the underlying buffer to read the sequence.
        /// This validation is performed to make sure we do not allocate a large container based on an invalid encoded
        /// size.
        /// </summary>
        /// <param name="minElementSize">The minimum encoded size of an element of the sequence, in bytes.</param>
        /// <returns>The number of elements in the sequence.</returns>
        public int ReadAndCheckSeqSize(int minElementSize)
        {
            int sz = ReadSize();

            if (sz == 0)
            {
                return 0;
            }

            int minSize = sz * minElementSize;

            // With _minTotalSeqSize, we make sure that multiple sequences within an InputStream can't trigger
            // maliciously the allocation of a large amount of memory before we read these sequences from the buffer.
            _minTotalSeqSize += minSize;

            if (_pos + minSize > _buffer.Count || _minTotalSeqSize > _buffer.Count)
            {
                throw new InvalidDataException("invalid sequence size");
            }
            return sz;
        }

        /// <summary>Determine if an optional value is available for reading.</summary>
        /// <param name="tag">The tag associated with the value.</param>
        /// <param name="expectedFormat">The optional format for the value.</param>
        /// <returns>True if the value is present, false otherwise.</returns>
        public bool ReadOptional(int tag, OptionalFormat expectedFormat)
        {
            // Tagged members/parameters can only be in the main encapsulation
            Debug.Assert(_mainEncaps != null);

            // The current slice has no tagged member
            if (_current != null && (_current.SliceFlags & EncodingDefinitions.SliceFlags.HasTaggedMembers) == 0)
            {
                return false;
            }

            int requestedTag = tag;

            while (true)
            {
                if (_buffer.Count - _pos <= 0)
                {
                    return false; // End of encapsulation also indicates end of optionals.
                }

                int v = ReadByte();
                if (v == EncodingDefinitions.TaggedEndMarker)
                {
                    _pos--; // Rewind.
                    return false;
                }

                var format = (OptionalFormat)(v & 0x07); // First 3 bits.
                tag = v >> 3;
                if (tag == 30)
                {
                    tag = ReadSize();
                }

                if (tag > requestedTag)
                {
                    int offset = tag < 30 ? 1 : (tag < 255 ? 2 : 6); // Rewind
                    _pos -= offset;
                    return false; // No tagged member with the requested tag.
                }
                else if (tag < requestedTag)
                {
                    SkipTagged(format);
                }
                else
                {
                    if (format != expectedFormat)
                    {
                        throw new InvalidDataException($"invalid tagged data member `{tag}': unexpected format");
                    }
                    return true;
                }
            }
        }

        /// <summary>Extracts a byte value from the stream.</summary>
        /// <returns>The extracted byte.</returns>
        public byte ReadByte() => _buffer[_pos++];

        /// <summary>Extracts an optional byte value from the stream.</summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public byte? ReadByte(int tag)
        {
            if (ReadOptional(tag, OptionalFormat.F1))
            {
                return ReadByte();
            }
            else
            {
                return null;
            }
        }

        /// <summary>Extracts a sequence of byte values from the stream.</summary>
        /// <returns>The extracted byte sequence.</returns>
        public byte[] ReadByteArray()
        {
            byte[] value = new byte[ReadAndCheckSeqSize(1)];
            ReadNumericArray(value);
            return value;
        }

        public int ReadSpan(Span<byte> span)
        {
            int length = Math.Min(span.Length, _buffer.Count - _pos);
            _buffer.AsSpan(_pos, length).CopyTo(span);
            _pos += length;
            return length;
        }

        /// <summary>Extracts an optional byte sequence from the stream.</summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public byte[]? ReadByteArray(int tag)
        {
            if (ReadOptional(tag, OptionalFormat.VSize))
            {
                return ReadByteArray();
            }
            else
            {
                return null;
            }
        }

        /// <summary>Extracts a serializable object from the stream.</summary>
        /// <returns>The serializable object.</returns>
        public object ReadSerializable()
        {
            int sz = ReadAndCheckSeqSize(1);
            if (sz == 0)
            {
                throw new InvalidDataException("read an empty byte sequence for non-null serializable object");
            }
            var f = new BinaryFormatter(null, new StreamingContext(StreamingContextStates.All, Communicator));
            return f.Deserialize(new InputStreamWrapper(this));
        }

        /// <summary>
        /// Extracts a boolean value from the stream.
        /// </summary>
        /// <returns>The extracted boolean.</returns>
        public bool ReadBool() => _buffer[_pos++] == 1;

        /// <summary>
        /// Extracts an optional boolean value from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public bool? ReadBool(int tag)
        {
            if (ReadOptional(tag, OptionalFormat.F1))
            {
                return ReadBool();
            }
            else
            {
                return null;
            }
        }

        /// <summary>
        /// Extracts a sequence of boolean values from the stream.
        /// </summary>
        /// <returns>The extracted boolean sequence.</returns>
        public bool[] ReadBoolArray()
        {
            bool[] value = new bool[ReadAndCheckSeqSize(1)];
            ReadNumericArray(value);
            return value;
        }

        /// <summary>
        /// Extracts an optional boolean sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public bool[]? ReadBoolArray(int tag)
        {
            if (ReadOptional(tag, OptionalFormat.VSize))
            {
                return ReadBoolArray();
            }
            else
            {
                return null;
            }
        }

        /// <summary>
        /// Extracts a short value from the stream.
        /// </summary>
        /// <returns>The extracted short.</returns>
        public short ReadShort()
        {
            Debug.Assert(_buffer.Array != null);
            short value = BitConverter.ToInt16(_buffer.Array, _buffer.Offset + _pos);
            _pos += 2;
            return value;
        }

        public static short ReadShort(ReadOnlySpan<byte> buffer) => BitConverter.ToInt16(buffer);

        /// <summary>
        /// Extracts an optional short value from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public short? ReadShort(int tag)
        {
            if (ReadOptional(tag, OptionalFormat.F2))
            {
                return ReadShort();
            }
            else
            {
                return null;
            }
        }

        /// <summary>
        /// Extracts a sequence of short values from the stream.
        /// </summary>
        /// <returns>The extracted short sequence.</returns>
        public short[] ReadShortArray()
        {
            short[] value = new short[ReadAndCheckSeqSize(2)];
            ReadNumericArray(value);
            return value;
        }

        /// <summary>
        /// Extracts an optional short sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public short[]? ReadShortArray(int tag)
        {
            if (ReadOptional(tag, OptionalFormat.VSize))
            {
                SkipSize();
                return ReadShortArray();
            }
            else
            {
                return null;
            }
        }

        /// <summary>
        /// Extracts an int value from the stream.
        /// </summary>
        /// <returns>The extracted int.</returns>
        public int ReadInt()
        {
            Debug.Assert(_buffer.Array != null);
            int value = BitConverter.ToInt32(_buffer.Array, _buffer.Offset + _pos);
            _pos += 4;
            return value;
        }

        public static int ReadInt(ReadOnlySpan<byte> buffer) => BitConverter.ToInt32(buffer);

        /// <summary>
        /// Extracts an optional int value from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public int? ReadInt(int tag)
        {
            if (ReadOptional(tag, OptionalFormat.F4))
            {
                return ReadInt();
            }
            else
            {
                return null;
            }
        }

        /// <summary>
        /// Extracts a sequence of int values from the stream.
        /// </summary>
        /// <returns>The extracted int sequence.</returns>
        public int[] ReadIntArray()
        {
            int[] value = new int[ReadAndCheckSeqSize(4)];
            ReadNumericArray(value);
            return value;
        }

        /// <summary>
        /// Extracts an optional int sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public int[]? ReadIntArray(int tag)
        {
            if (ReadOptional(tag, OptionalFormat.VSize))
            {
                SkipSize();
                return ReadIntArray();
            }
            else
            {
                return null;
            }
        }

        /// <summary>
        /// Extracts a long value from the stream.
        /// </summary>
        /// <returns>The extracted long.</returns>
        public long ReadLong()
        {
            Debug.Assert(_buffer.Array != null);
            long value = BitConverter.ToInt64(_buffer.Array, _buffer.Offset + _pos);
            _pos += 8;
            return value;
        }

        public static long ReadLong(ReadOnlySpan<byte> buffer) => BitConverter.ToInt64(buffer);

        /// <summary>
        /// Extracts an optional long value from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public long? ReadLong(int tag)
        {
            if (ReadOptional(tag, OptionalFormat.F8))
            {
                return ReadLong();
            }
            else
            {
                return null;
            }
        }

        /// <summary>
        /// Extracts a sequence of long values from the stream.
        /// </summary>
        /// <returns>The extracted long sequence.</returns>
        public long[] ReadLongArray()
        {
            long[] value = new long[ReadAndCheckSeqSize(8)];
            ReadNumericArray(value);
            return value;
        }

        /// <summary>
        /// Extracts an optional long sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public long[]? ReadLongArray(int tag)
        {
            if (ReadOptional(tag, OptionalFormat.VSize))
            {
                SkipSize();
                return ReadLongArray();
            }
            else
            {
                return null;
            }
        }

        /// <summary>
        /// Extracts a float value from the stream.
        /// </summary>
        /// <returns>The extracted float.</returns>
        public float ReadFloat()
        {
            Debug.Assert(_buffer.Array != null);
            float value = BitConverter.ToSingle(_buffer.Array, _buffer.Offset + _pos);
            _pos += 4;
            return value;
        }

        /// <summary>
        /// Extracts an optional float value from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public float? ReadFloat(int tag)
        {
            if (ReadOptional(tag, OptionalFormat.F4))
            {
                return ReadFloat();
            }
            else
            {
                return null;
            }
        }

        /// <summary>
        /// Extracts a sequence of float values from the stream.
        /// </summary>
        /// <returns>The extracted float sequence.</returns>
        public float[] ReadFloatArray()
        {
            float[] value = new float[ReadAndCheckSeqSize(4)];
            ReadNumericArray(value);
            return value;
        }

        /// <summary>
        /// Extracts an optional float sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public float[]? ReadFloatArray(int tag)
        {
            if (ReadOptional(tag, OptionalFormat.VSize))
            {
                SkipSize();
                return ReadFloatArray();
            }
            else
            {
                return null;
            }
        }

        /// <summary>
        /// Extracts a double value from the stream.
        /// </summary>
        /// <returns>The extracted double.</returns>
        public double ReadDouble()
        {
            Debug.Assert(_buffer.Array != null);
            double value = BitConverter.ToDouble(_buffer.Array, _buffer.Offset + _pos);
            _pos += 8;
            return value;
        }

        /// <summary>
        /// Extracts an optional double value from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public double? ReadDouble(int tag)
        {
            if (ReadOptional(tag, OptionalFormat.F8))
            {
                return ReadDouble();
            }
            else
            {
                return null;
            }
        }

        /// <summary>
        /// Extracts a sequence of double values from the stream.
        /// </summary>
        /// <returns>The extracted double sequence.</returns>
        public double[] ReadDoubleArray()
        {
            double[] value = new double[ReadAndCheckSeqSize(8)];
            ReadNumericArray(value);
            return value;
        }

        /// <summary>
        /// Extracts an optional double sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public double[]? ReadDoubleArray(int tag)
        {
            if (ReadOptional(tag, OptionalFormat.VSize))
            {
                SkipSize();
                return ReadDoubleArray();
            }
            else
            {
                return null;
            }
        }

        private static readonly System.Text.UTF8Encoding _utf8 = new System.Text.UTF8Encoding(false, true);

        /// <summary>
        /// Extracts a string from the stream.
        /// </summary>
        /// <returns>The extracted string.</returns>
        public string ReadString()
        {
            int size = ReadSize();
            if (size == 0)
            {
                return "";
            }
            string value = _utf8.GetString(_buffer.AsSpan(_pos, size));
            _pos += size;
            return value;
        }

        public static string ReadString(ArraySegment<byte> buffer)
        {
            int size = ReadSize(buffer);
            if (size == 0)
            {
                return "";
            }
            else
            {
                return _utf8.GetString(buffer.AsSpan(size < 254 ? 1 : 4, size));
            }
        }

        /// <summary>
        /// Extracts an optional string from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public string? ReadString(int tag)
        {
            if (ReadOptional(tag, OptionalFormat.VSize))
            {
                return ReadString();
            }
            else
            {
                return null;
            }
        }

        public T[] ReadArray<T>(InputStreamReader<T> reader, int minSize)
        {
            var enumerable = new Collection<T>(this, reader, minSize);
            var arr = new T[enumerable.Count];
            int pos = 0;
            foreach (T item in enumerable)
            {
                arr[pos++] = item;
            }
            return arr;
        }

        public IEnumerable<T> ReadCollection<T>(InputStreamReader<T> reader, int minSize) =>
            new Collection<T>(this, reader, minSize);

        public Dictionary<TKey, TValue> ReadDict<TKey, TValue>(InputStreamReader<TKey> keyReader,
            InputStreamReader<TValue> valueReader, int minWireSize = 1) where TKey : notnull
        {
            int sz = ReadAndCheckSeqSize(minWireSize);
            var dict = new Dictionary<TKey, TValue>(sz);
            for (int i = 0; i < sz; ++i)
            {
                TKey key = keyReader(this);
                TValue value = valueReader(this);
                dict.Add(key, value);
            }
            return dict;
        }

        public SortedDictionary<TKey, TValue> ReadSortedDict<TKey, TValue>(InputStreamReader<TKey> keyReader,
            InputStreamReader<TValue> valueReader) where TKey : notnull
        {
            int sz = ReadSize();
            var dict = new SortedDictionary<TKey, TValue>();
            for (int i = 0; i < sz; ++i)
            {
                TKey key = keyReader(this);
                TValue value = valueReader(this);
                dict.Add(key, value);
            }
            return dict;
        }

        /// <summary>
        /// Extracts a sequence of strings from the stream.
        /// </summary>
        /// <returns>The extracted string sequence.</returns>
        public string[] ReadStringArray() => ReadArray(IceReaderIntoString, 1);

        public IEnumerable<string> ReadStringCollection() => ReadCollection(IceReaderIntoString, 1);

        /// <summary>
        /// Extracts an optional string sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public string[]? ReadStringArray(int tag)
        {
            if (ReadOptional(tag, OptionalFormat.FSize))
            {
                Skip(4);
                return ReadStringArray();
            }
            else
            {
                return null;
            }
        }

        /// <summary>
        /// Extracts a proxy from the stream. The stream must have been initialized with a communicator.
        /// </summary>
        /// <returns>The extracted proxy.</returns>
        public T? ReadProxy<T>(ProxyFactory<T> factory) where T : class, IObjectPrx
        {
            var ident = new Identity(this);
            if (ident.Name.Length == 0)
            {
                return null;
            }
            else
            {
                return factory(Communicator.CreateReference(ident, this));
            }
        }

        /// <summary>
        /// Extracts an optional proxy from the stream. The stream must have been initialized with a communicator.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <param name="factory">The proxy factory used to create the typed proxy.</param>
        /// <returns>The optional value.</returns>
        public T? ReadProxy<T>(int tag, ProxyFactory<T> factory) where T : class, IObjectPrx
        {
            if (ReadOptional(tag, OptionalFormat.FSize))
            {
                Skip(4);
                return ReadProxy(factory);
            }
            else
            {
                return null;
            }
        }

        /// <summary>
        /// Read an instance of class T.
        /// </summary>
        /// <returns>The class instance, or null.</returns>
        public T? ReadClass<T>() where T : AnyClass
        {
            AnyClass? obj = ReadAnyClass();
            if (obj == null)
            {
                return null;
            }
            else if (obj is T)
            {
                return (T)obj;
            }
            else
            {
                throw new InvalidDataException(@$"read instance of type `{obj.GetType().FullName
                    }' but expected instance of type `{typeof(T).FullName}'");
            }
        }

        /// <summary>
        /// Read a tagged parameter or data member of type class T.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the class parameter or data member.</param>
        /// <returns>The class instance, or null.</returns>
        public T? ReadClass<T>(int tag) where T : AnyClass
        {
            AnyClass? obj = ReadAnyClass(tag);
            if (obj == null)
            {
                return null;
            }
            else if (obj is T)
            {
                return (T)obj;
            }
            else
            {
                throw new InvalidDataException(@$"read instance of type `{obj.GetType().FullName
                    }' but expected instance of type `{typeof(T).FullName}'");
            }
        }

        /// <summary>
        /// Extracts a remote exception from the stream and throws it.
        /// </summary>
        public RemoteException ReadException()
        {
            Push(InstanceType.Exception);
            Debug.Assert(_current != null);

            // Read the first slice header, and exception's type ID cannot be null.
            string typeId = ReadSliceHeaderIntoCurrent()!;
            ReadIndirectionTableIntoCurrent(); // we read the indirection table immediately

            while (true)
            {
                RemoteException? remoteEx = null;
                Type? type = Communicator.ResolveClass(typeId);
                if (type != null)
                {
                    try
                    {
                        remoteEx = (RemoteException?)Activator.CreateInstance(type);
                    }
                    catch (Exception ex)
                    {
                        throw new InvalidDataException(
                            @$"failed to create an instance of type `{type.Name
                            }' while reading a remote exception with type ID `{typeId}'", ex);
                    }
                }

                // We found the exception.
                if (remoteEx != null)
                {
                    remoteEx.ConvertToUnhandled = true;
                    remoteEx.Read(this);
                    Pop(null);
                    return remoteEx;
                }

                // Slice off what we don't understand.
                SkipSlice();

                if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.IsLastSlice) != 0)
                {
                    // Create and throw a plain RemoteException with the SlicedData.
                    Debug.Assert(SlicedData != null);
                    remoteEx = new RemoteException(SlicedData.Value);
                    remoteEx.ConvertToUnhandled = true;
                    return remoteEx;
                }

                typeId = ReadSliceHeaderIntoCurrent()!;
                ReadIndirectionTableIntoCurrent();
            }
        }

        /// <summary>
        /// Skip the given number of bytes.
        /// </summary>
        /// <param name="size">The number of bytes to skip</param>
        public void Skip(int size)
        {
            if (size < 0 || size > _buffer.Count - _pos)
            {
                throw new IndexOutOfRangeException($"cannot skip {size} bytes");
            }
            _pos += size;
        }

        /// <summary>
        /// Skip over a size value.
        /// </summary>
        public void SkipSize()
        {
            byte b = ReadByte();
            if (b == 255)
            {
                Skip(4);
            }
        }

        internal Endpoint ReadEndpoint()
        {
            var type = (EndpointType)ReadShort();

            Encoding? oldEncoding = _encoding; // TODO: update once we restore the Encoding property!
            int size;
            (_encoding, size) = ReadEncapsulationHeader();
            Debug.Assert(_encoding != null);
            int? oldLimit = _limit;
            _limit = _pos + size - 6;

            Endpoint result;
            if (_encoding.Value.IsSupported && Communicator.GetEndpointFactory(type) is IEndpointFactory factory)
            {
                result = factory.Read(this);
            }
            else
            {
                byte[] data = new byte[size - 6];
                int bytesRead = ReadSpan(data);
                if (bytesRead < data.Length)
                {
                    throw new InvalidDataException("invalid endpoint encapsulation size while reading opaque endpoint");
                }
                result = new OpaqueEndpoint(type, _encoding.Value, data);
            }

            if (_limit.Value - _pos != 0)
            {
                throw new InvalidDataException(
                    $"there are {_limit.Value - _pos} bytes remaining in endpoint encapsulation");
            }

            // Exceptions when reading InputStream are considered fatal to the InputStream so no need to restore
            // _limit or _encoding unless we succeed.
            _limit = oldLimit;
            _encoding = oldEncoding;
            return result;
        }

        internal (Encoding Encoding, int Size) ReadEncapsulationHeader()
        {
            // With the 1.1 encoding, the encapsulation size is encoded on a 4-bytes int and
            // not on a variable-length size, for ease of marshaling.
            int sz = ReadInt();
            if (sz < 6)
            {
                throw new InvalidDataException($"encapsulation has only {sz} bytes");
            }
            if (sz - 4 > _buffer.Count - _pos)
            {
                throw new InvalidDataException("the encapsulation's size extends beyond the end of the frame");
            }
            byte major = ReadByte();
            byte minor = ReadByte();
            return (new Encoding(major, minor), sz);
        }

        private void SkipTagged(OptionalFormat format)
        {
            switch (format)
            {
                case OptionalFormat.F1:
                    {
                        Skip(1);
                        break;
                    }
                case OptionalFormat.F2:
                    {
                        Skip(2);
                        break;
                    }
                case OptionalFormat.F4:
                    {
                        Skip(4);
                        break;
                    }
                case OptionalFormat.F8:
                    {
                        Skip(8);
                        break;
                    }
                case OptionalFormat.Size:
                    {
                        SkipSize();
                        break;
                    }
                case OptionalFormat.VSize:
                    {
                        Skip(ReadSize());
                        break;
                    }
                case OptionalFormat.FSize:
                    {
                        Skip(ReadInt());
                        break;
                    }
                case OptionalFormat.Class:
                    {
                        ReadAnyClass();
                        break;
                    }
            }
        }

        private bool SkipTaggedMembers()
        {
            // Skip remaining unread tagged members.
            while (true)
            {
                if (_buffer.Count - _pos <= 0)
                {
                    return false; // End of encapsulation also indicates end of tagged members.
                }

                int v = ReadByte();
                if (v == EncodingDefinitions.TaggedEndMarker)
                {
                    return true;
                }

                var format = (OptionalFormat)(v & 0x07); // Read first 3 bits.
                if ((v >> 3) == 30)
                {
                    SkipSize();
                }
                SkipTagged(format);
            }
        }

        private string ReadTypeId(bool isIndex)
        {
            _typeIdMap ??= new List<string>();

            if (isIndex)
            {
                int index = ReadSize();
                if (index > 0 && index - 1 < _typeIdMap.Count)
                {
                    // The encoded type-id indexes start at 1, not 0.
                    return _typeIdMap[index - 1];
                }
                throw new InvalidDataException($"read invalid type ID index {index}");
            }
            else
            {
                string typeId = ReadString();

                // The typeIds of slices in indirection tables can be read several times: when we skip the
                // indirection table and later on when we read it. We only want to add this typeId to the list
                // and assign it an index when it's the first time we read it, so we save the largest position we
                // read to figure out when to add to the list.
                if (_pos > _posAfterLatestInsertedTypeId)
                {
                    _posAfterLatestInsertedTypeId = _pos;
                    _typeIdMap.Add(typeId);
                }

                return typeId;
            }
        }

        private AnyClass? ReadAnyClass()
        {
            int index = ReadSize();
            if (index < 0)
            {
                throw new InvalidDataException($"invalid index {index} while reading a class");
            }
            else if (index == 0)
            {
                return null;
            }
            else if (_current != null && (_current.SliceFlags & EncodingDefinitions.SliceFlags.HasIndirectionTable) != 0)
            {
                // When reading an instance within a slice and there is an
                // indirection table, we have an index within this indirection table.
                //
                // We need to decrement index since position 0 in the indirection table
                // corresponds to index 1.
                index--;
                if (index < _current.IndirectionTable?.Length)
                {
                    return _current.IndirectionTable[index];
                }
                else
                {
                    throw new InvalidDataException("index too big for indirection table");
                }
            }
            else
            {
                return ReadInstance(index);
            }
        }

        // Read a tagged parameter or data member of type class.
        private AnyClass? ReadAnyClass(int tag)
        {
            if (ReadOptional(tag, OptionalFormat.Class))
            {
                return ReadAnyClass();
            }
            else
            {
                return null;
            }
        }

        // Read a slice header into _current.
        // Returns the type ID of that slice. Null means it's a slice in compact format without a type ID,
        // or a slice with a compact ID we could not resolve.
        private string? ReadSliceHeaderIntoCurrent()
        {
            Debug.Assert(_current != null);

            _current.SliceFlags = (EncodingDefinitions.SliceFlags)ReadByte();

            // Read the type ID. For class slices, the type ID is encoded as a
            // string or as an index or as a compact ID, for exceptions it's always encoded as a
            // string.
            if (_current.InstanceType == InstanceType.Class)
            {
                // TYPE_ID_COMPACT must be checked first!
                if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasTypeIdCompact) ==
                    EncodingDefinitions.SliceFlags.HasTypeIdCompact)
                {
                    _current.SliceCompactId = ReadSize();
                    _current.SliceTypeId = null;
                }
                else if ((_current.SliceFlags &
                        (EncodingDefinitions.SliceFlags.HasTypeIdIndex | EncodingDefinitions.SliceFlags.HasTypeIdString)) != 0)
                {
                    _current.SliceTypeId = ReadTypeId((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasTypeIdIndex) != 0);
                    _current.SliceCompactId = null;
                }
                else
                {
                    // Slice in compact format, without a type ID or compact ID.
                    Debug.Assert((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasSliceSize) == 0);
                    _current.SliceTypeId = null;
                    _current.SliceCompactId = null;
                }
            }
            else
            {
                _current.SliceTypeId = ReadString();
                Debug.Assert(_current.SliceCompactId == null); // no compact ID for exceptions
            }

            // Read the slice size if necessary.
            if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasSliceSize) != 0)
            {
                _current.SliceSize = ReadInt();
                if (_current.SliceSize < 4)
                {
                    throw new InvalidDataException($"invalid slice size: {_current.SliceSize}");
                }
            }
            else
            {
                _current.SliceSize = 0;
            }

            // Clear other per-slice fields:
            _current.IndirectionTable = null;
            _current.PosAfterIndirectionTable = null;

            return _current.SliceTypeId;
        }

        // Read the indirection table into _current's fields if there is an indirection table.
        // Precondition: called after reading the slice's header.
        // This method does not change Pos.
        private void ReadIndirectionTableIntoCurrent()
        {
            Debug.Assert(_current != null && _current.IndirectionTable == null);
            if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasIndirectionTable) != 0)
            {
                int savedPos = _pos;
                if (_current.SliceSize < 4)
                {
                    throw new InvalidDataException($"invalid slice size: {_current.SliceSize}");
                }
                _pos = savedPos + _current.SliceSize - 4;
                _current.IndirectionTable = ReadIndirectionTable();
                _current.PosAfterIndirectionTable = _pos;
                _pos = savedPos;
            }
        }

        // Skip the body of the current slice and it indirection table (if any).
        // When it's a class instance and there is an indirection table, it returns the starting position of that
        // indirection table; otherwise, it return 0.
        private int SkipSlice()
        {
            Debug.Assert(_current != null);
            if (Communicator.TraceLevels.Slicing > 0)
            {
                ILogger logger = Communicator.Logger;
                string slicingCat = Communicator.TraceLevels.SlicingCat;
                if (_current.InstanceType == InstanceType.Exception)
                {
                    IceInternal.TraceUtil.TraceSlicing("exception", _current.SliceTypeId ?? "", slicingCat, logger);
                }
                else
                {
                    IceInternal.TraceUtil.TraceSlicing("object", _current.SliceTypeId ?? "", slicingCat, logger);
                }
            }

            int start = _pos;

            if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasSliceSize) != 0)
            {
                Debug.Assert(_current.SliceSize >= 4);
                Skip(_current.SliceSize - 4);
            }
            else
            {
                if (_current.InstanceType == InstanceType.Class)
                {
                    string typeId = _current.SliceTypeId ?? _current.SliceCompactId!.ToString()!;
                    throw new InvalidDataException(@$"no class found for type ID `{typeId
                        }' and compact format prevents slicing (the sender should use the sliced format instead)");
                }
                else
                {
                    string typeId = _current.SliceTypeId!;
                    throw new InvalidDataException(@$"no exception class found for type ID `{typeId
                        }' and compact format prevents slicing (the sender should use the sliced format instead)");
                }
            }

            // Preserve this slice.
            bool hasOptionalMembers = (_current.SliceFlags & EncodingDefinitions.SliceFlags.HasTaggedMembers) != 0;
            int end = _pos;
            int dataEnd = end;
            if (hasOptionalMembers)
            {
                // Don't include the tagged end marker. It will be re-written by IceEndSlice when the sliced data
                // is re-written.
                --dataEnd;
            }
            byte[] bytes = new byte[dataEnd - start];
            _buffer.Slice(start, bytes.Length).CopyTo(bytes);

            int startOfIndirectionTable = 0;

            if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasIndirectionTable) != 0)
            {
                if (_current.InstanceType == InstanceType.Class)
                {
                    startOfIndirectionTable = _pos;
                    SkipIndirectionTable();
                }
                else
                {
                    Debug.Assert(_current.PosAfterIndirectionTable != null);
                    // Move past indirection table
                    _pos = _current.PosAfterIndirectionTable.Value;
                    _current.PosAfterIndirectionTable = null;
                }
            }
            _current.Slices ??= new List<SliceInfo>();
            var info = new SliceInfo(_current.SliceTypeId,
                                     _current.SliceCompactId,
                                     new ReadOnlyMemory<byte>(bytes),
                                     Array.AsReadOnly(_current.IndirectionTable ?? Array.Empty<AnyClass>()),
                                     hasOptionalMembers,
                                     (_current.SliceFlags & EncodingDefinitions.SliceFlags.IsLastSlice) != 0);
            _current.Slices.Add(info);

            // An exception slice may have an indirection table (saved above). We don't need it anymore
            // since we're skipping this slice.
            _current.IndirectionTable = null;
            return startOfIndirectionTable;
        }

        // Skip the indirection table. The caller must save the current stream position before calling
        // SkipIndirectionTable (to read the indirection table at a later point) except when the caller
        // is SkipIndirectionTable itself.
        private void SkipIndirectionTable()
        {
            Debug.Assert(_current != null);
            // We should never skip an exception's indirection table
            Debug.Assert(_current.InstanceType == InstanceType.Class);

            // We use ReadSize and not ReadAndCheckSeqSize here because we don't allocate memory for this
            // sequence, and since we are skipping this sequence to read it later, we don't want to double-count
            // its contribution to _minTotalSeqSize.
            int tableSize = ReadSize();
            for (int i = 0; i < tableSize; ++i)
            {
                int index = ReadSize();
                if (index <= 0)
                {
                    throw new InvalidDataException($"read invalid index {index} in indirection table");
                }
                if (index == 1)
                {
                    if (++_classGraphDepth > Communicator.ClassGraphDepthMax)
                    {
                        throw new InvalidDataException("maximum class graph depth reached");
                    }

                    // Read/skip this instance
                    EncodingDefinitions.SliceFlags sliceFlags;
                    do
                    {
                        sliceFlags = (EncodingDefinitions.SliceFlags)ReadByte();
                        if ((sliceFlags & EncodingDefinitions.SliceFlags.HasTypeIdCompact) == EncodingDefinitions.SliceFlags.HasTypeIdCompact)
                        {
                            ReadSize(); // compact type-id
                        }
                        else if ((sliceFlags &
                            (EncodingDefinitions.SliceFlags.HasTypeIdIndex | EncodingDefinitions.SliceFlags.HasTypeIdString)) != 0)
                        {
                            // This can update the typeIdMap
                            ReadTypeId((sliceFlags & EncodingDefinitions.SliceFlags.HasTypeIdIndex) != 0);
                        }
                        else
                        {
                            throw new InvalidDataException(
                                "indirection table cannot hold an instance without a type ID");
                        }

                        // Read the slice size, then skip the slice
                        if ((sliceFlags & EncodingDefinitions.SliceFlags.HasSliceSize) == 0)
                        {
                            throw new InvalidDataException("size of slice missing");
                        }
                        int sliceSize = ReadInt();
                        if (sliceSize < 4)
                        {
                            throw new InvalidDataException($"invalid slice size: {sliceSize}");
                        }
                        _pos = _pos + sliceSize - 4;

                        // If this slice has an indirection table, skip it too
                        if ((sliceFlags & EncodingDefinitions.SliceFlags.HasIndirectionTable) != 0)
                        {
                            SkipIndirectionTable();
                        }
                    } while ((sliceFlags & EncodingDefinitions.SliceFlags.IsLastSlice) == 0);
                    _classGraphDepth--;
                }
            }
        }

        private AnyClass[] ReadIndirectionTable()
        {
            int size = ReadAndCheckSeqSize(1);
            if (size == 0)
            {
                throw new InvalidDataException("invalid empty indirection table");
            }
            var indirectionTable = new AnyClass[size];
            for (int i = 0; i < indirectionTable.Length; ++i)
            {
                int index = ReadSize();
                if (index < 1)
                {
                    throw new InvalidDataException($"read invalid index {index} in indirection table");
                }
                indirectionTable[i] = ReadInstance(index);
            }
            return indirectionTable;
        }

        private AnyClass ReadInstance(int index)
        {
            Debug.Assert(index > 0);

            if (index > 1)
            {
                if (_instanceMap != null && _instanceMap.Count > index - 2)
                {
                    return _instanceMap[index - 2];
                }
                throw new InvalidDataException($"could not find index {index} in {nameof(_instanceMap)}");
            }

            InstanceData? previousCurrent = Push(InstanceType.Class);
            Debug.Assert(_current != null);

            // Read the first slice header.
            string? mostDerivedId = ReadSliceHeaderIntoCurrent();
            string? typeId = mostDerivedId;
            // We cannot read the indirection table at this point as it may reference the new instance that is not
            // created yet.

            AnyClass? v = null;
            List<int>? deferredIndirectionTableList = null;

            while (true)
            {
                Type? cls = null;
                if (typeId != null)
                {
                    Debug.Assert(_current.SliceCompactId == null);
                    cls = Communicator.ResolveClass(typeId);
                }
                else if (_current.SliceCompactId.HasValue)
                {
                    cls = Communicator.ResolveCompactId(_current.SliceCompactId.Value);
                }

                if (cls != null)
                {
                    try
                    {
                        Debug.Assert(!cls.IsAbstract && !cls.IsInterface);
                        v = (AnyClass?)Activator.CreateInstance(cls);
                    }
                    catch (Exception ex)
                    {
                        string typeIdString = typeId ?? _current.SliceCompactId!.ToString()!;
                        throw new InvalidDataException(@$"failed to create an instance of type `{cls.Name
                            } while reading a class with type ID {typeIdString}", ex);
                    }
                }

                if (v != null)
                {
                    // We have an instance, get out of this loop.
                    break;
                }

                // Slice off what we don't understand, and save the indirection table (if any) in
                // deferredIndirectionTableList.
                deferredIndirectionTableList ??= new List<int>();
                deferredIndirectionTableList.Add(SkipSlice());

                // If this is the last slice, keep the instance as an opaque UnknownSlicedClass object.
                if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.IsLastSlice) != 0)
                {
                    v = new UnknownSlicedClass();
                    break;
                }

                typeId = ReadSliceHeaderIntoCurrent(); // Read next Slice header for next iteration.
            }

            if (++_classGraphDepth > Communicator.ClassGraphDepthMax)
            {
                throw new InvalidDataException("maximum class graph depth reached");
            }

            // Add the instance to the map/list of instances. This must be done before reading the instances (for
            // circular references).
            _instanceMap ??= new List<AnyClass>();
            _instanceMap.Add(v);

            // Read all the deferred indirection tables now that the instance is inserted in _instanceMap.
            if (deferredIndirectionTableList?.Count > 0)
            {
                int savedPos = _pos;

                Debug.Assert(_current.Slices?.Count == deferredIndirectionTableList.Count);
                for (int i = 0; i < deferredIndirectionTableList.Count; ++i)
                {
                    int pos = deferredIndirectionTableList[i];
                    if (pos > 0)
                    {
                        _pos = pos;
                        _current.Slices[i].Instances = Array.AsReadOnly(ReadIndirectionTable());
                    }
                    // else remains empty
                }
                _pos = savedPos;
            }

            // Read the instance.
            v.Read(this);
            Pop(previousCurrent);

            --_classGraphDepth;
            return v;
        }

        // Create a new current instance of the specified slice type
        // and return the previous current instance, if any.
        private InstanceData? Push(InstanceType instanceType)
        {
            // Can't have a current instance already if we are reading an exception
            Debug.Assert(instanceType == InstanceType.Class || _current == null);
            InstanceData? oldInstance = _current;
            _current = new InstanceData(instanceType);
            return oldInstance;
        }

        // Replace the current instance by savedInstance
        private void Pop(InstanceData? savedInstance)
        {
            Debug.Assert(_current != null);
            _current = savedInstance;
        }

        /// <summary>Helper method for read numeric arrays, the array is fill using Buffer.BlockCopy.</summary>
        /// <param name="dst">The numeric array to read.</param>
        private void ReadNumericArray(Array dst)
        {
            Debug.Assert(_buffer.Array != null);
            int byteCount = Buffer.ByteLength(dst);
            Debug.Assert(_buffer.Count - _pos >= byteCount);
            Buffer.BlockCopy(_buffer.Array, _buffer.Offset + _pos, dst, 0, byteCount);
            _pos += byteCount;
        }

        private enum InstanceType { Class, Exception }

        private sealed class InstanceData
        {
            internal InstanceData(InstanceType instanceType) => InstanceType = instanceType;

            // Instance attributes
            internal readonly InstanceType InstanceType;
            internal List<SliceInfo>? Slices; // Preserved slices.

            // Slice attributes
            internal EncodingDefinitions.SliceFlags SliceFlags = default;
            internal int SliceSize = 0;
            internal string? SliceTypeId;
            internal int? SliceCompactId;
            // Indirection table of the current slice
            internal AnyClass[]? IndirectionTable;
            internal int? PosAfterIndirectionTable;
        }

        private readonly struct Encaps
        {
            // Previous upper limit of the buffer, if set
            internal readonly int? OldLimit;

            // Size of the encapsulation, as read from the stream
            internal readonly int Size;

            internal Encaps(int? oldLimit, int size)
            {
                OldLimit = oldLimit;
                Size = size;
            }
        }

        private readonly struct MainEncapsBackup
        {
            internal readonly Encaps Encaps;
            internal readonly int Pos;

            internal readonly Encoding Encoding;
            internal readonly int MinTotalSeqSize;

            internal MainEncapsBackup(Encaps encaps, int pos, Encoding encoding, int minTotalSeqSize)
            {
                Encaps = encaps;
                Pos = pos;
                Encoding = encoding;
                MinTotalSeqSize = minTotalSeqSize;
            }
        }

        // Collection<T> holds the size of a Slice sequence and reads the sequence elements from the InputStream
        // on-demand. It does not fully implement IEnumerable<T> and ICollection<T> (i.e. some methods throw
        // NotSupportedException) because it's not resettable: you can't use it to unmarshal the same bytes
        // multiple times.
        private sealed class Collection<T> : ICollection<T>, IEnumerator<T>
        {
            public int Count { get; }

            public T Current
            {
                get
                {
                    if (_pos == 0 || _pos > Count)
                    {
                        throw new InvalidOperationException();
                    }
                    return _current;
                }
            }

            object? IEnumerator.Current => Current;
            public bool IsReadOnly => true;
            private T _current;
            private bool _enumeratorRetrieved = false;
            private readonly InputStream _inputStream;
            private int _pos = 0;
            private readonly InputStreamReader<T> _reader;

            // Disable this warning as the _current field is never read before it is initialized in MoveNext. Declaring
            // this field as nullable is not an option for a genericT  that can be used with reference and value types.
#pragma warning disable CS8618 // Non-nullable field is uninitialized. Consider declaring as nullable.
            internal Collection(InputStream istr, InputStreamReader<T> reader, int minSize)
#pragma warning restore CS8618
            {
                _inputStream = istr;
                _reader = reader;
                Count = istr.ReadAndCheckSeqSize(minSize);
            }

            public IEnumerator<T> GetEnumerator()
            {
                if (_enumeratorRetrieved)
                {
                    throw new NotSupportedException("cannot get a second enumerator for this enumerable");
                }
                _enumeratorRetrieved = true;
                return this;
            }

            IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();

            public void Add(T item) => throw new NotSupportedException();
            public void Clear() => throw new NotSupportedException();
            public bool Contains(T item) => throw new NotSupportedException();

            public void CopyTo(T[] array, int arrayIndex)
            {
                foreach (T value in this)
                {
                    array[arrayIndex++] = value;
                }
            }

            public void Dispose()
            {
            }

            public bool MoveNext()
            {
                if (++_pos > Count)
                {
                    _pos = Count + 1;
                    return false;
                }
                else
                {
                    _current = _reader(_inputStream);
                    return true;
                }
            }

            public bool Remove(T item) => throw new NotSupportedException();

            public void Reset() => throw new NotSupportedException();
        }
    }
}
