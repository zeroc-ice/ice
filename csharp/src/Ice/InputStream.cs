//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Binary;

namespace ZeroC.Ice
{
    /// <summary>A delegate that reads a value from an input stream.</summary>
    /// <typeparam name="T">The type of the value to read.</typeparam>
    /// <param name="istr">The input stream to read from.</param>
    public delegate T InputStreamReader<T>(InputStream istr);

    /// <summary>Reads a byte buffer encoded using the Ice encoding.</summary>
    public sealed partial class InputStream
    {
        //
        // Cached InputStreamWriter static objects used by the generated code
        //

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<bool> IceReaderIntoBool =
            istr => istr.ReadBool();

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<bool[]> IceReaderIntoBoolArray =
            istr => istr.ReadFixedSizeNumericArray<bool>();

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<byte> IceReaderIntoByte =
            istr => istr.ReadByte();

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<byte[]> IceReaderIntoByteArray =
            istr => istr.ReadFixedSizeNumericArray<byte>();

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<double> IceReaderIntoDouble =
            istr => istr.ReadDouble();

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<double[]> IceReaderIntoDoubleArray =
            istr => istr.ReadFixedSizeNumericArray<double>();

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<float> IceReaderIntoFloat =
            istr => istr.ReadFloat();

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<float[]> IceReaderIntoFloatArray =
            istr => istr.ReadFixedSizeNumericArray<float>();

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<int> IceReaderIntoInt =
            istr => istr.ReadInt();

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<int[]> IceReaderIntoIntArray =
            istr => istr.ReadFixedSizeNumericArray<int>();

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<long> IceReaderIntoLong =
            istr => istr.ReadLong();

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<long[]> IceReaderIntoLongArray =
            istr => istr.ReadFixedSizeNumericArray<long>();

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<short> IceReaderIntoShort =
            istr => istr.ReadShort();

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<short[]> IceReaderIntoShortArray =
            istr => istr.ReadFixedSizeNumericArray<short>();

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<string> IceReaderIntoString =
            istr => istr.ReadString();

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<uint> IceReaderIntoUInt =
            istr => istr.ReadUInt();

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<uint[]> IceReaderIntoUIntArray =
            istr => istr.ReadFixedSizeNumericArray<uint>();

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<ulong> IceReaderIntoULong =
            istr => istr.ReadULong();

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<ulong[]> IceReaderIntoULongArray =
            istr => istr.ReadFixedSizeNumericArray<ulong>();

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<ushort> IceReaderIntoUShort =
            istr => istr.ReadUShort();

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<ushort[]> IceReaderIntoUShortArray =
            istr => istr.ReadFixedSizeNumericArray<ushort>();

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<int> IceReaderIntoVarInt =
            istr => istr.ReadVarInt();

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<long> IceReaderIntoVarLong =
            istr => istr.ReadVarLong();

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<uint> IceReaderIntoVarUInt =
            istr => istr.ReadVarUInt();

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<ulong> IceReaderIntoVarULong =
            istr => istr.ReadVarULong();

        /// <summary>The communicator associated with this stream.</summary>
        /// <value>The communicator.</value>
        public Communicator Communicator { get; }

        /// <summary>The Ice encoding used by this stream when reading its byte buffer.</summary>
        /// <value>The current encoding.</value>
        public Encoding Encoding { get; private set; }

        /// <summary>The 0-based position (index) in the underlying buffer.</summary>
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

        private static readonly System.Text.UTF8Encoding _utf8 = new System.Text.UTF8Encoding(false, true);

        // True when reading a top-level encapsulation; otherwise, false.
        private bool InEncapsulation { get; }

        private bool OldEncoding => Encoding == Encoding.V1_1;

        // The byte buffer we are reading.
        private ArraySegment<byte> _buffer;

        // Data for the class or exception instance that is currently getting unmarshaled.
        private InstanceData? _current;

        // The maximum depth when reading nested class/exception instances.
        private int _classGraphDepth = 0;

        // Map of class instance ID to class instance.
        // When reading a top-level encapsulation:
        //  - Instance ID = 0 means null
        //  - Instance ID = 1 means the instance is encoded inline afterwards
        //  - Instance ID > 1 means a reference to a previously read instance, found in this map.
        // Since the map is actually a list, we use instance ID - 2 to lookup an instance.
        private List<AnyClass>? _instanceMap;

        // The sum of all the minimum sizes (in bytes) of the sequences read in this buffer. Must not exceed the buffer
        // size.
        private int _minTotalSeqSize = 0;

        // The 0-based index in the buffer.
        private int _pos;

        // See _typeIdMap.
        private int _posAfterLatestInsertedTypeId = 0;

        // Map of type ID index to type ID string.
        // When reading a top-level encapsulation, we assign a type ID index (starting with 1) to each type ID we
        // read, in order. Since this map is a list, we lookup a previously assigned type ID string with
        // _typeIdMap[index - 1].
        private List<string>? _typeIdMap;

        //
        // Read methods for basic types
        //

        /// <summary>Reads a bool from the stream.</summary>
        /// <returns>The bool read from the stream.</returns>
        public bool ReadBool() => _buffer[_pos++] == 1;

        /// <summary>Reads a byte from the stream.</summary>
        /// <returns>The byte read from the stream.</returns>
        public byte ReadByte() => _buffer[_pos++];

        /// <summary>Reads a double from the stream.</summary>
        /// <returns>The double read from the stream.</returns>
        public double ReadDouble()
        {
            double value = BitConverter.ToDouble(_buffer.AsSpan(_pos, sizeof(double)));
            _pos += sizeof(double);
            return value;
        }

        /// <summary>Reads a float from the stream.</summary>
        /// <returns>The float read from the stream.</returns>
        public float ReadFloat()
        {
            float value = BitConverter.ToSingle(_buffer.AsSpan(_pos, sizeof(float)));
            _pos += sizeof(float);
            return value;
        }

        /// <summary>Reads an int from the stream.</summary>
        /// <returns>The int read from the stream.</returns>
        public int ReadInt()
        {
            int value = BitConverter.ToInt32(_buffer.AsSpan(_pos, sizeof(int)));
            _pos += sizeof(int);
            return value;
        }

        /// <summary>Reads a long from the stream.</summary>
        /// <returns>The long read from the stream.</returns>
        public long ReadLong()
        {
            long value = BitConverter.ToInt64(_buffer.AsSpan(_pos, sizeof(long)));
            _pos += sizeof(long);
            return value;
        }

        /// <summary>Reads a short from the stream.</summary>
        /// <returns>The short read from the stream.</returns>
        public short ReadShort()
        {
            short value = BitConverter.ToInt16(_buffer.AsSpan(_pos, sizeof(short)));
            _pos += sizeof(short);
            return value;
        }

        /// <summary>Reads a string from the stream.</summary>
        /// <returns>The string read from the stream.</returns>
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

        /// <summary>Reads a uint from the stream.</summary>
        /// <returns>The uint read from the stream.</returns>
        public uint ReadUInt()
        {
            uint value = BitConverter.ToUInt32(_buffer.AsSpan(_pos, sizeof(uint)));
            _pos += sizeof(uint);
            return value;
        }

        /// <summary>Reads a ulong from the stream.</summary>
        /// <returns>The ulong read from the stream.</returns>
        public ulong ReadULong()
        {
            ulong value = BitConverter.ToUInt64(_buffer.AsSpan(_pos, sizeof(ulong)));
            _pos += sizeof(ulong);
            return value;
        }

        /// <summary>Reads a ushort from the stream.</summary>
        /// <returns>The ushort read from the stream.</returns>
        public ushort ReadUShort()
        {
            ushort value = BitConverter.ToUInt16(_buffer.AsSpan(_pos, sizeof(ushort)));
            _pos += sizeof(ushort);
            return value;
        }

        /// <summary>Reads an int from the stream. This int is encoded using Ice's variable-length integer encoding.
        /// </summary>
        /// <returns>The int read from the stream.</returns>
        public int ReadVarInt()
        {
            try
            {
                checked
                {
                    return (int)ReadVarLong();
                }
            }
            catch (Exception ex)
            {
                throw new InvalidDataException("varint value is out of range", ex);
            }
        }

        /// <summary>Reads a long from the stream. This long is encoded using Ice's variable-length integer encoding.
        /// </summary>
        /// <returns>The long read from the stream.</returns>
        public long ReadVarLong() => OldEncoding ? ReadLong() :
            (_buffer[_pos] & 0x03) switch
            {
                0 => (sbyte)ReadByte() >> 2,
                1 => ReadShort() >> 2,
                2 => ReadInt() >> 2,
                _ => ReadLong() >> 2
            };

        /// <summary>Reads a uint from the stream. This uint is encoded using Ice's variable-length integer encoding.
        /// </summary>
        /// <returns>The uint read from the stream.</returns>
        public uint ReadVarUInt()
        {
            try
            {
                checked
                {
                    return (uint)ReadVarULong();
                }
            }
            catch (Exception ex)
            {
                throw new InvalidDataException("varuint value is out of range", ex);
            }
        }

        /// <summary>Reads a ulong from the stream. This ulong is encoded using Ice's variable-length integer encoding.
        /// </summary>
        /// <returns>The ulong read from the stream.</returns>
        public ulong ReadVarULong() => OldEncoding ? ReadULong() :
            (_buffer[_pos] & 0x03) switch
            {
                0 => (uint)ReadByte() >> 2,   // cast to uint to use operator >> for uint instead of int, which is
                1 => (uint)ReadUShort() >> 2, // later implicitly converted to ulong
                2 => ReadUInt() >> 2,
                _ => ReadULong() >> 2
            };

        //
        // Read methods for constructed types except class and exception
        //

        /// <summary>Reads a sequence from the stream and returns an array.</summary>
        /// <param name="minElementSize">The minimum size of each element of the sequence, in bytes.</param>
        /// <param name="reader">The input stream reader used to read each element of the sequence.</param>
        /// <returns>The sequence read from the stream, as an array.</returns>
        public T[] ReadArray<T>(int minElementSize, InputStreamReader<T> reader)
        {
            ICollection<T> collection = ReadSequence(minElementSize, reader);
            var array = new T[collection.Count];
            collection.CopyTo(array, 0);
            return array;
        }

        /// <summary>Reads a dictionary from the stream.</summary>
        /// <param name="minEntrySize">The minimum size of each entry of the dictionary, in bytes.</param>
        /// <param name="keyReader">The input stream reader used to read each key of the dictionary.</param>
        /// <param name="valueReader">The input stream reader used to read each value of the dictionary.</param>
        /// <returns>The dictionary read from the stream.</returns>
        public Dictionary<TKey, TValue> ReadDictionary<TKey, TValue>(int minEntrySize,
                                                                     InputStreamReader<TKey> keyReader,
                                                                     InputStreamReader<TValue> valueReader)
            where TKey : notnull
        {
            int sz = ReadAndCheckSeqSize(minEntrySize);
            var dict = new Dictionary<TKey, TValue>(sz);
            for (int i = 0; i < sz; ++i)
            {
                TKey key = keyReader(this);
                TValue value = valueReader(this);
                dict.Add(key, value);
            }
            return dict;
        }

        /// <summary>Reads an enum value from the stream; this method does not validate the value.</summary>
        /// <returns>The enum value (int) read from the stream.</returns>
        public int ReadEnumValue() => ReadSize();

        /// <summary>Reads a sequence of fixed-size numeric type from the stream and returns an array.</summary>
        /// <returns>The sequence read from the stream, as an array.</returns>
        public T[] ReadFixedSizeNumericArray<T>() where T : struct
        {
            int elementSize = Unsafe.SizeOf<T>();
            var value = new T[ReadAndCheckSeqSize(elementSize)];
            int byteCount = elementSize * value.Length;
            _buffer.AsSpan(_pos, byteCount).CopyTo(MemoryMarshal.Cast<T, byte>(value));
            _pos += byteCount;
            return value;
        }

        /// <summary>Reads a proxy from the stream.</summary>
        /// <param name="factory">The proxy factory used to create the typed proxy.</param>
        /// <returns>The proxy read from the stream.</returns>
        public T? ReadProxy<T>(ProxyFactory<T> factory) where T : class, IObjectPrx =>
            Reference.Read(this) is Reference reference ? factory(reference) : null;

        /// <summary>Reads a sequence from the stream.</summary>
        /// <param name="minElementSize">The minimum size of each element of the sequence, in bytes.</param>
        /// <param name="reader">The input stream reader used to read each element of the sequence.</param>
        /// <returns>A collection that provides the size of the sequence and allows you read the sequence from the
        /// the stream. The return value does not fully implement ICollection{T}, in particular you can only call
        /// GetEnumerator() once on this collection. You would typically use this collection to construct a List{T} or
        /// some other generic collection that can be constructed from an IEnumerable{T}.</returns>
        public ICollection<T> ReadSequence<T>(int minElementSize, InputStreamReader<T> reader) =>
            new Collection<T>(this, minElementSize, reader);

        /// <summary>Reads a serializable object from the stream.</summary>
        /// <returns>The object read from the stream.</returns>
        public object ReadSerializable()
        {
            int sz = ReadAndCheckSeqSize(1);
            if (sz == 0)
            {
                throw new InvalidDataException("read an empty byte sequence for non-null serializable object");
            }
            var f = new BinaryFormatter(null, new StreamingContext(StreamingContextStates.All, Communicator));
            return f.Deserialize(new StreamWrapper(this));
        }

        /// <summary>Reads a sorted dictionary from the stream.</summary>
        /// <param name="minEntrySize">The minimum size of each entry of the dictionary, in bytes.</param>
        /// <param name="keyReader">The input stream reader used to read each key of the dictionary.</param>
        /// <param name="valueReader">The input stream reader used to read each value of the dictionary.</param>
        /// <returns>The sorted dictionary read from the stream.</returns>
        public SortedDictionary<TKey, TValue> ReadSortedDictionary<TKey, TValue>(
            int minEntrySize,
            InputStreamReader<TKey> keyReader,
            InputStreamReader<TValue> valueReader) where TKey : notnull
        {
            int sz = ReadAndCheckSeqSize(minEntrySize);
            var dict = new SortedDictionary<TKey, TValue>();
            for (int i = 0; i < sz; ++i)
            {
                TKey key = keyReader(this);
                TValue value = valueReader(this);
                dict.Add(key, value);
            }
            return dict;
        }

        //
        // Read methods for tagged basic types
        //

        /// <summary>Reads a tagged bool from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <returns>The bool read from the stream, or null.</returns>
        public bool? ReadTaggedBool(int tag) =>
            ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.F1) ? ReadBool() : (bool?)null;

        /// <summary>Reads a tagged byte from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <returns>The byte read from the stream, or null.</returns>
        public byte? ReadTaggedByte(int tag) =>
            ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.F1) ? ReadByte() : (byte?)null;

        /// <summary>Reads a tagged double from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <returns>The double read from the stream, or null.</returns>
        public double? ReadTaggedDouble(int tag) =>
            ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.F8) ? ReadDouble() : (double?)null;

        /// <summary>Reads a tagged float from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <returns>The float read from the stream, or null.</returns>
        public float? ReadTaggedFloat(int tag) =>
            ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.F4) ? ReadFloat() : (float?)null;

        /// <summary>Reads a tagged int from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <returns>The int read from the stream, or null.</returns>
        public int? ReadTaggedInt(int tag) =>
            ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.F4) ? ReadInt() : (int?)null;

        /// <summary>Reads a tagged long from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <returns>The long read from the stream, or null.</returns>
        public long? ReadTaggedLong(int tag) =>
            ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.F8) ? ReadLong() : (long?)null;

        /// <summary>Reads a tagged short from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <returns>The short read from the stream, or null.</returns>
        public short? ReadTaggedShort(int tag) =>
            ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.F2) ? ReadShort() : (short?)null;

        /// <summary>Reads a tagged string from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <returns>The string read from the stream, or null.</returns>
        public string? ReadTaggedString(int tag) =>
            ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.VSize) ? ReadString() : null;

        //
        // Read methods for tagged constructed types except class
        //

        /// <summary>Reads a tagged enum from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="reader">The input stream reader used to create and validate the enum.</param>
        /// <returns>The enum read from the stream, or null.</returns>
        public T? ReadTaggedEnum<T>(int tag, InputStreamReader<T> reader) where T : struct, Enum =>
            ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.Size) ? reader(this) : (T?)null;

        /// <summary>Reads a tagged sequence with fixed-size elements from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="elementSize">The size of each element, in bytes.</param>
        /// <param name="reader">The input stream reader used to read each element of the sequence.</param>
        /// <returns>The sequence read from the stream as an array, or null.</returns>
        public T[]? ReadTaggedFixedSizeElementArray<T>(int tag, int elementSize, InputStreamReader<T> reader)
        {
            if (ReadTaggedFixedSizeElementSequence(tag, elementSize, reader) is ICollection<T> collection)
            {
                var array = new T[collection.Count];
                collection.CopyTo(array, 0);
                return array;
            }
            else
            {
                return null;
            }
        }

        /// <summary>Reads a tagged sequence with fixed-size elements from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="elementSize">The size of each element, in bytes.</param>
        /// <param name="reader">The input stream reader used to read each element of the sequence.</param>
        /// <returns>The sequence read from the stream as an ICollection{T}, or null.</returns>
        public ICollection<T>? ReadTaggedFixedSizeElementSequence<T>(int tag,
                                                                     int elementSize,
                                                                     InputStreamReader<T> reader)
        {
            if (ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.VSize))
            {
                if (elementSize > 1)
                {
                    SkipSize(); // this size represents the number of bytes in the tagged parameter.
                }
                return ReadSequence(elementSize, reader);
            }
            else
            {
                return null;
            }
        }

        /// <summary>Reads a tagged dictionary with fixed-size entries from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="entrySize">The size of each entry, in bytes.</param>
        /// <param name="keyReader">The input stream reader used to read each key of the dictionary.</param>
        /// <param name="valueReader">The input stream reader used to read each value of the dictionary.</param>
        /// <returns>The dictionary read from the stream, or null.</returns>
        public Dictionary<TKey, TValue>? ReadTaggedFixedSizeEntryDictionary<TKey, TValue>(
            int tag,
            int entrySize,
            InputStreamReader<TKey> keyReader,
            InputStreamReader<TValue> valueReader) where TKey : notnull
        {
            if (ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.VSize))
            {
                SkipSize();
                return ReadDictionary(entrySize, keyReader, valueReader);
            }
            else
            {
                return null;
            }
        }

        /// <summary>Reads a tagged sorted dictionary with fixed-size entries from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="entrySize">The size of each entry, in bytes.</param>
        /// <param name="keyReader">The input stream reader used to read each key of the dictionary.</param>
        /// <param name="valueReader">The input stream reader used to read each value of the dictionary.</param>
        /// <returns>The sorted dictionary read from the stream, or null.</returns>
        public SortedDictionary<TKey, TValue>? ReadTaggedFixedSizeEntrySortedDictionary<TKey, TValue>(
            int tag,
            int entrySize,
            InputStreamReader<TKey> keyReader,
            InputStreamReader<TValue> valueReader) where TKey : notnull
        {
            if (ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.VSize))
            {
                SkipSize();
                return ReadSortedDictionary(entrySize, keyReader, valueReader);
            }
            else
            {
                return null;
            }
        }

        /// <summary>Reads a tagged sequence of a fixed-size numeric type from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <returns>The sequence read from the stream as an array, or null.</returns>
        public T[]? ReadTaggedFixedSizeNumericArray<T>(int tag) where T : struct
        {
            int elementSize = Unsafe.SizeOf<T>();
            if (ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.VSize))
            {
                if (elementSize > 1)
                {
                    // For elements with size > 1, the encoding includes a size (number of bytes in the tagged
                    // parameter) that we skip.
                    SkipSize();
                }
                return ReadFixedSizeNumericArray<T>();
            }
            else
            {
                return null;
            }
        }

        /// <summary>Reads a tagged fixed-size struct from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="reader">The input stream reader used to create and read the struct.</param>
        /// <returns>The struct T read from the stream, or null.</returns>
        public T? ReadTaggedFixedSizeStruct<T>(int tag, InputStreamReader<T> reader) where T : struct
        {
            if (ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.VSize))
            {
                SkipSize();
                return reader(this);
            }
            else
            {
                return null;
            }
        }

        /// <summary>Reads a tagged proxy from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="factory">The proxy factory used to create the typed proxy.</param>
        /// <returns>The proxy read from the stream, or null.</returns>
        public T? ReadTaggedProxy<T>(int tag, ProxyFactory<T> factory) where T : class, IObjectPrx
        {
            if (ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.FSize))
            {
                SkipFixedLengthSize();
                return ReadProxy(factory);
            }
            else
            {
                return null;
            }
        }

        /// <summary>Reads a tagged serializable object from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <returns>The object read from the stream, or null.</returns>
        public object? ReadTaggedSerializable(int tag) =>
            ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.VSize) ? ReadSerializable() : null;

        /// <summary>Reads a tagged sequence with variable-size elements from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="minElementSize">The minimum size of each element, in bytes.</param>
        /// <param name="reader">The input stream reader used to read each element of the sequence.</param>
        /// <returns>The sequence read from the stream as an array, or null.</returns>
        public T[]? ReadTaggedVariableSizeElementArray<T>(int tag, int minElementSize, InputStreamReader<T> reader)
        {
            if (ReadTaggedVariableSizeElementSequence(tag, minElementSize, reader) is ICollection<T> collection)
            {
                var array = new T[collection.Count];
                collection.CopyTo(array, 0);
                return array;
            }
            else
            {
                return null;
            }
        }

        /// <summary>Reads a tagged sequence with variable-size elements from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="minElementSize">The minimum size of each element, in bytes.</param>
        /// <param name="reader">The input stream reader used to read each element of the sequence.</param>
        /// <returns>The sequence read from the stream as an ICollection{T}, or null.</returns>
        public ICollection<T>? ReadTaggedVariableSizeElementSequence<T>(int tag,
                                                                        int minElementSize,
                                                                        InputStreamReader<T> reader)
        {
            if (ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.FSize))
            {
                SkipFixedLengthSize();
                return ReadSequence(minElementSize, reader);
            }
            else
            {
                return null;
            }
        }

        /// <summary>Reads a tagged dictionary with variable-size entries from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="minEntrySize">The minimum size of each entry, in bytes.</param>
        /// <param name="keyReader">The input stream reader used to read each key of the dictionary.</param>
        /// <param name="valueReader">The input stream reader used to read each value of the dictionary.</param>
        /// <returns>The dictionary read from the stream, or null.</returns>
        public Dictionary<TKey, TValue>? ReadTaggedVariableSizeEntryDictionary<TKey, TValue>(
            int tag,
            int minEntrySize,
            InputStreamReader<TKey> keyReader,
            InputStreamReader<TValue> valueReader) where TKey : notnull
        {
            if (ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.FSize))
            {
                SkipFixedLengthSize();
                return ReadDictionary(minEntrySize, keyReader, valueReader);
            }
            else
            {
                return null;
            }
        }

        /// <summary>Reads a tagged sorted dictionary with variable-size entries from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="minEntrySize">The minimum size of each entry, in bytes.</param>
        /// <param name="keyReader">The input stream reader used to read each key of the dictionary.</param>
        /// <param name="valueReader">The input stream reader used to read each value of the dictionary.</param>
        /// <returns>The sorted dictionary read from the stream, or null.</returns>
        public SortedDictionary<TKey, TValue>? ReadTaggedVariableSizeEntrySortedDictionary<TKey, TValue>(
            int tag,
            int minEntrySize,
            InputStreamReader<TKey> keyReader,
            InputStreamReader<TValue> valueReader) where TKey : notnull
        {
            if (ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.FSize))
            {
                SkipFixedLengthSize();
                return ReadSortedDictionary(minEntrySize, keyReader, valueReader);
            }
            else
            {
                return null;
            }
        }

        /// <summary>Reads a tagged variable-size struct from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="reader">The input stream reader used to create and read the struct.</param>
        /// <returns>The struct T read from the stream, or null.</returns>
        public T? ReadTaggedVariableSizeStruct<T>(int tag, InputStreamReader<T> reader) where T : struct
        {
            if (ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.FSize))
            {
                SkipFixedLengthSize();
                return reader(this);
            }
            else
            {
                return null;
            }
        }

        //
        // Internal and private methods
        //

        internal static string Read11String(ArraySegment<byte> buffer)
        {
            int size = Read11Size(buffer.AsSpan());
            if (size == 0)
            {
                return "";
            }
            else
            {
                return _utf8.GetString(buffer.AsSpan(size < 255 ? 1 : 5, size));
            }
        }

        /// <summary>Reads an empty encapsulation from the provided byte buffer.</summary>
        /// <param name="communicator">The communicator.</param>
        /// <param name="encoding">The encoding of the buffer.</param>
        /// <param name="buffer">The byte buffer.</param>
        internal static void ReadEmptyEncapsulation(
            Communicator communicator, Encoding encoding, ArraySegment<byte> buffer)
        {
            var istr = new InputStream(communicator, encoding, buffer, startEncaps: true, 0);
            istr.SkipTaggedParams();
            istr.CheckEndOfBuffer();
        }

        /// <summary>Reads the contents of an encapsulation from the provided byte buffer.</summary>
        /// <param name="communicator">The communicator.</param>
        /// <param name="encoding">The encoding of the buffer.</param>
        /// <param name="buffer">The byte buffer.</param>
        /// <param name="payloadReader">The reader used to read the payload of this encapsulation.</param>
        internal static T ReadEncapsulation<T>(
            Communicator communicator, Encoding encoding, ArraySegment<byte> buffer, InputStreamReader<T> payloadReader)
        {
            var istr = new InputStream(communicator, encoding, buffer, startEncaps: true, 0);
            T result = payloadReader(istr);
            istr.SkipTaggedParams();
            istr.CheckEndOfBuffer();
            return result;
        }

        internal static (int Size, Encoding Encoding) ReadEncapsulationHeader(
            Encoding encoding, ReadOnlySpan<byte> buffer)
        {
            int sizeLength;
            int size;

            if (encoding == Encoding.V1_1)
            {
                sizeLength = 4;
                size = ReadInt(buffer) - sizeLength; // Remove the size length which is included with the 1.1 encoding.
                if (size < 0)
                {
                    throw new InvalidDataException(
                        $"the 1.1 encapsulation's size ({size + sizeLength}) is too small");
                }
            }
            else
            {
                sizeLength = 1 << (buffer[0] & 0x03);
                size = Read20Size(buffer);
            }

            if (sizeLength + size > buffer.Length)
            {
                throw new InvalidDataException(
                    $"the encapsulation's size ({size}) extends beyond the end of the buffer");
            }

            return (size, new Encoding(buffer[sizeLength], buffer[sizeLength + 1]));
        }

        internal static int ReadInt(ReadOnlySpan<byte> buffer) => BitConverter.ToInt32(buffer);
        internal static long ReadLong(ReadOnlySpan<byte> buffer) => BitConverter.ToInt64(buffer);
        internal static short ReadShort(ReadOnlySpan<byte> buffer) => BitConverter.ToInt16(buffer);

        /// <summary>Constructs a new InputStream over a byte buffer.</summary>
        /// <param name="communicator">The communicator.</param>
        /// <param name="encoding">The encoding of the buffer.</param>
        /// <param name="buffer">The byte buffer.</param>
        /// <param name="pos">The initial position in the buffer.</param>
        internal InputStream(Communicator communicator, Encoding encoding, ArraySegment<byte> buffer, int pos = 0)
            : this(communicator, encoding, buffer, false, pos)
        {
            // TODO: pos should always be 0 and buffer should be a slice as needed.
            // Currently this does not work because of the tracing code that resets Pos to 0 to read the protocol frame
            // headers.
        }

        /// <summary>Reads an encapsulation header from the stream.</summary>
        /// <returns>The encapsulation header read from the stream.</returns>
        internal (int Size, Encoding Encoding) ReadEncapsulationHeader()
        {
            (int Size, Encoding Encoding) result = ReadEncapsulationHeader(Encoding, _buffer.Slice(_pos));
            if (OldEncoding)
            {
                _pos += 6; // 4 (size length) + 2 (encoding length)
            }
            else
            {
                _pos += (1 << (_buffer[_pos] & 0x03)) + 2; // size length + encoding length
            }
            return result;
        }

        /// <summary>Reads an endpoint from the stream.</summary>
        /// <returns>The endpoint read from the stream.</returns>
        internal Endpoint ReadEndpoint()
        {
            var type = (EndpointType)ReadShort();
            (int size, Encoding encoding) = ReadEncapsulationHeader();

            Endpoint endpoint;
            if (encoding.IsSupported && Communicator.FindEndpointFactory(type) is IEndpointFactory factory)
            {
                Encoding oldEncoding = Encoding;
                ArraySegment<byte> oldBuffer = _buffer;
                int oldPos = _pos;
                int oldMinTotalSeqSize = _minTotalSeqSize;
                Encoding = encoding;
                _buffer = _buffer.Slice(_pos, size - 2);
                _pos = 0;
                _minTotalSeqSize = 0;

                endpoint = factory.Read(this);
                CheckEndOfBuffer();

                // Exceptions when reading InputStream are considered fatal to the InputStream so no need to restore
                // anything unless we succeed.
                Encoding = oldEncoding;
                _buffer = oldBuffer;
                _pos = oldPos + size - 2;
                _minTotalSeqSize = oldMinTotalSeqSize;
            }
            else
            {
                endpoint = new OpaqueEndpoint(type, encoding, _buffer.Slice(_pos, size - 2).ToArray());
                _pos += size - 2;
            }

            return endpoint;
        }

        /// <summary>Reads a facet from the stream.</summary>
        /// <returns>The facet read from the stream.</returns>
        internal string ReadFacet()
        {
            string[] facets = ReadArray(1, IceReaderIntoString);
            if (facets.Length > 1)
            {
                throw new InvalidDataException($"read ice1 facet path with {facets.Length} elements");
            }
            return facets.Length == 1 ? facets[0] : "";
        }

        /// <summary>Reads a size from the stream.</summary>
        /// <returns>The size read from the stream.</returns>
        internal int ReadSize() => OldEncoding ? Read11Size() : Read20Size();

        /// <summary>Skips over an encapsulation without reading it.</summary>
        /// <returns>The encoding version of the skipped encapsulation.</returns>
        internal Encoding SkipEncapsulation()
        {
            (int size, Encoding encoding) = ReadEncapsulationHeader();
            _pos += size - 2;
            if (_pos > _buffer.Count)
            {
                throw new InvalidDataException(
                    $"the encapsulation's size ({size}) extends beyond the end of the frame");
            }
            return encoding;
        }

        private static int Read11Size(ReadOnlySpan<byte> buffer)
        {
            byte b = buffer[0];
            if (b < 255)
            {
                return b;
            }

            int size = ReadInt(buffer.Slice(1, 4));
            if (size < 0)
            {
                throw new InvalidDataException($"read invalid size: {size}");
            }
            return size;
        }

        private static int Read20Size(ReadOnlySpan<byte> buffer)
        {
            ulong size = (buffer[0] & 0x03) switch
            {
                0 => (uint)buffer[0] >> 2,
                1 => (uint)BitConverter.ToUInt16(buffer) >> 2,
                2 => BitConverter.ToUInt32(buffer) >> 2,
                _ => BitConverter.ToUInt64(buffer) >> 2
            };

            checked // make sure we don't overflow
            {
                return (int)size;
            }
        }

        private InputStream(
            Communicator communicator, Encoding encoding, ArraySegment<byte> buffer, bool startEncaps, int pos)
        {
            Debug.Assert(pos == 0 || !startEncaps); // while pos is still there, it's 0 when startEncaps is true
            Communicator = communicator;

            if (startEncaps)
            {
                _pos = 0;
                _buffer = buffer;
                Encoding = encoding;
                Encoding.CheckSupported();

                (int size, Encoding encapsEncoding) = ReadEncapsulationHeader();
                // We slice the provided buffer to the encapsulation (minus its header). This way, we can easily prevent
                // reads past the end of the encapsulation.
                _buffer = buffer.Slice(_pos, size - 2);
                _pos = 0;

                Encoding = encapsEncoding;
                Encoding.CheckSupported();
                InEncapsulation = true;
            }
            else
            {
                _buffer = buffer;
                _pos = pos;
                Encoding = encoding;
                InEncapsulation = false;
            }
        }

        private void CheckEndOfBuffer()
        {
            if (_pos != _buffer.Count)
            {
                throw new InvalidDataException($"{_buffer.Count - _pos} bytes remaining in the InputStream buffer");
            }
        }

        private int Read11Size()
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

        private int Read20Size()
        {
            checked
            {
                return (int)ReadVarULong();
            }
        }

        /// <summary>Reads a sequence size and makes sure there is enough space in the underlying buffer to read the
        /// sequence. This validation is performed to make sure we do not allocate a large container based on an
        /// invalid encoded size.</summary>
        /// <param name="minElementSize">The minimum encoded size of an element of the sequence, in bytes.</param>
        /// <returns>The number of elements in the sequence.</returns>
        private int ReadAndCheckSeqSize(int minElementSize)
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

        private int ReadSpan(Span<byte> span)
        {
            int length = Math.Min(span.Length, _buffer.Count - _pos);
            _buffer.AsSpan(_pos, length).CopyTo(span);
            _pos += length;
            return length;
        }

        /// <summary>Determines if a tagged parameter or data member is available for reading.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="expectedFormat">The format of the tagged parameter.</param>
        /// <returns>True if the tagged parameter is present; otherwise, false.</returns>
        private bool ReadTaggedParamHeader(int tag, EncodingDefinitions.TagFormat expectedFormat)
        {
            // Tagged members/parameters can only be in the main encapsulation
            Debug.Assert(InEncapsulation);

            // The current slice has no tagged parameter
            if (_current != null && (_current.SliceFlags & EncodingDefinitions.SliceFlags.HasTaggedMembers) == 0)
            {
                return false;
            }

            int requestedTag = tag;

            while (true)
            {
                if (_buffer.Count - _pos <= 0)
                {
                    return false; // End of encapsulation also indicates end of tagged parameters.
                }

                int savedPos = _pos;

                int v = ReadByte();
                if (v == EncodingDefinitions.TaggedEndMarker)
                {
                    _pos = savedPos; // rewind
                    return false;
                }

                var format = (EncodingDefinitions.TagFormat)(v & 0x07); // First 3 bits.
                tag = v >> 3;
                if (tag == 30)
                {
                    tag = ReadSize();
                }

                if (tag > requestedTag)
                {
                    _pos = savedPos; // rewind
                    return false; // No tagged parameter with the requested tag.
                }
                else if (tag < requestedTag)
                {
                    SkipTagged(format);
                }
                else
                {
                    if (format != expectedFormat)
                    {
                        throw new InvalidDataException($"invalid tagged parameter `{tag}': unexpected format");
                    }
                    return true;
                }
            }
        }

        private void Skip(int size)
        {
            if (size < 0 || size > _buffer.Count - _pos)
            {
                throw new IndexOutOfRangeException($"cannot skip {size} bytes");
            }
            _pos += size;
        }

        private void SkipFixedLengthSize()
        {
            if (OldEncoding)
            {
                Skip(4);
            }
            else
            {
                // With the 2.0 encoding, there is only one encoding for size, and the writer decides how many bytes
                // to use.
                byte b = _buffer[_pos];
                Skip(1 << (b & 0x03));
            }
        }

        /// <summary>Skips over a size value. Equivalent to ReadSize()</summary>
        private void SkipSize()
        {
            if (OldEncoding)
            {
                byte b = ReadByte();
                if (b == 255)
                {
                    Skip(4);
                }
            }
            else
            {
                byte b = _buffer[_pos];
                Skip(1 << (b & 0x03));
            }
        }

        private void SkipTagged(EncodingDefinitions.TagFormat format)
        {
            switch (format)
            {
                case EncodingDefinitions.TagFormat.F1:
                    Skip(1);
                    break;
                case EncodingDefinitions.TagFormat.F2:
                    Skip(2);
                    break;
                case EncodingDefinitions.TagFormat.F4:
                    Skip(4);
                    break;
                case EncodingDefinitions.TagFormat.F8:
                    Skip(8);
                    break;
                case EncodingDefinitions.TagFormat.Size:
                    SkipSize();
                    break;
                case EncodingDefinitions.TagFormat.VSize:
                    Skip(ReadSize());
                    break;
                case EncodingDefinitions.TagFormat.FSize:
                    if (OldEncoding)
                    {
                        int size = ReadInt();
                        if (size < 0)
                        {
                            throw new InvalidDataException("invalid negative fixed-length size");
                        }
                        Skip(size);
                    }
                    else
                    {
                        Skip(Read20Size());
                    }
                    break;
                case EncodingDefinitions.TagFormat.Class:
                    ReadAnyClass();
                    break;
            }
        }

        private bool SkipTaggedParams()
        {
            // Skip remaining unread tagged parameters.
            while (true)
            {
                if (_buffer.Count - _pos <= 0)
                {
                    return false; // End of encapsulation also indicates end of tagged parameters.
                }

                int v = ReadByte();
                if (v == EncodingDefinitions.TaggedEndMarker)
                {
                    return true;
                }

                var format = (EncodingDefinitions.TagFormat)(v & 0x07); // Read first 3 bits.
                if ((v >> 3) == 30)
                {
                    SkipSize();
                }
                SkipTagged(format);
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
            internal Collection(InputStream istr, int minElementSize, InputStreamReader<T> reader)
#pragma warning restore CS8618
            {
                Count = istr.ReadAndCheckSeqSize(minElementSize);
                _inputStream = istr;
                _reader = reader;
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
