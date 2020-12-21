// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace ZeroC.Ice
{
    /// <summary>A delegate that reads a value from an input stream.</summary>
    /// <typeparam name="T">The type of the value to read.</typeparam>
    /// <param name="istr">The input stream to read from.</param>
    public delegate T InputStreamReader<T>(InputStream istr);

    /// <summary>A delegate that reads a value from an input stream. The value contains a stream parameter that
    /// will be received using the given socket stream.</summary>
    /// <typeparam name="T">The type of the value to read.</typeparam>
    /// <param name="istr">The input stream to read from.</param>
    /// <param name="socketStream">The socket stream to receive the streamable data from.</param>
    public delegate T InputStreamReaderWithStreamable<T>(InputStream istr, SocketStream socketStream);

    /// <summary>Reads a byte buffer encoded using the Ice encoding.</summary>
    public sealed partial class InputStream
    {
        // Cached InputStreamReader static objects used by the generated code

        /// <summary>A <see cref="InputStreamReader{T}"/> used to read <c>bool</c> values.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<bool> IceReaderIntoBool =
            istr => istr.ReadBool();

        /// <summary>A <see cref="InputStreamReader{T}"/> used to read <c>byte</c> values.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<byte> IceReaderIntoByte =
            istr => istr.ReadByte();

        /// <summary>A <see cref="InputStreamReader{T}"/> used to read <c>double</c> values.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<double> IceReaderIntoDouble =
            istr => istr.ReadDouble();

        /// <summary>A <see cref="InputStreamReader{T}"/> used to read <c>float</c> values.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<float> IceReaderIntoFloat =
            istr => istr.ReadFloat();

        /// <summary>A <see cref="InputStreamReader{T}"/> used to read <c>int</c> values.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<int> IceReaderIntoInt =
            istr => istr.ReadInt();

        /// <summary>A <see cref="InputStreamReader{T}"/> used to read <c>long</c> values.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<long> IceReaderIntoLong =
            istr => istr.ReadLong();

        /// <summary>A <see cref="InputStreamReader{T}"/> used to read <c>short</c> values.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<short> IceReaderIntoShort =
            istr => istr.ReadShort();

        /// <summary>A <see cref="InputStreamReader{T}"/> used to read <c>string</c> instances.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<string> IceReaderIntoString =
            istr => istr.ReadString();

        /// <summary>A <see cref="InputStreamReader{T}"/> used to read <c>uint</c> values.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<uint> IceReaderIntoUInt =
            istr => istr.ReadUInt();

        /// <summary>A <see cref="InputStreamReader{T}"/> used to read <c>ulong</c> values.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<ulong> IceReaderIntoULong =
            istr => istr.ReadULong();

        /// <summary>A <see cref="InputStreamReader{T}"/> used to read <c>ushort</c> values.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<ushort> IceReaderIntoUShort =
            istr => istr.ReadUShort();

        /// <summary>A <see cref="InputStreamReader{T}"/> used to read var int values.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<int> IceReaderIntoVarInt =
            istr => istr.ReadVarInt();

        /// <summary>A <see cref="InputStreamReader{T}"/> used to read var long values.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<long> IceReaderIntoVarLong =
            istr => istr.ReadVarLong();

        /// <summary>A <see cref="InputStreamReader{T}"/> used to read var uint values.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<uint> IceReaderIntoVarUInt =
            istr => istr.ReadVarUInt();

        /// <summary>A <see cref="InputStreamReader{T}"/> used to read var ulong values.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<ulong> IceReaderIntoVarULong =
            istr => istr.ReadVarULong();

        /// <summary>The Communicator associated with this stream. It cannot be null when reading a proxy, class, or
        /// exception.</summary>
        public Communicator? Communicator { get; }

        /// <summary>The Connection used to read relative proxies. When not null, a relative proxy is unmarshaled into
        /// a fixed proxy bound to this connection.</summary>
        public Connection? Connection { get; }

        /// <summary>The Ice encoding used by this stream when reading its byte buffer.</summary>
        /// <value>The encoding.</value>
        public Encoding Encoding { get; }

        /// <summary>The 0-based position (index) in the underlying buffer.</summary>
        internal int Pos { get; private set; }

        /// <summary>The Reference used to read relative proxies. When not null, a relative proxy is unmarshaled into a
        /// clone of this reference (with various updates). Reference and Connection are mutually exclusive: only one of
        /// them can be non-null.</summary>
        internal Reference? Reference { get; }

        /// <summary>The sliced-off slices held by the current instance, if any.</summary>
        internal SlicedData? SlicedData
        {
            get
            {
                Debug.Assert(_current.InstanceType != InstanceType.None);
                if (_current.Slices == null)
                {
                    return null;
                }
                else
                {
                    return new SlicedData(Encoding, _current.Slices);
                }
            }
        }

        private bool OldEncoding => Encoding == Encoding.V11;

        // The byte buffer we are reading.
        private readonly ReadOnlyMemory<byte> _buffer;

        // Data for the class or exception instance that is currently getting unmarshaled.
        private InstanceData _current;

        // The current depth when reading nested class instances.
        private int _classGraphDepth;

        // True when reading a top-level encapsulation; otherwise, false.
        private readonly bool _inEncapsulation;

        // Map of class instance ID to class instance.
        // When reading a top-level encapsulation:
        //  - Instance ID = 0 means null
        //  - Instance ID = 1 means the instance is encoded inline afterwards
        //  - Instance ID > 1 means a reference to a previously read instance, found in this map.
        // Since the map is actually a list, we use instance ID - 2 to lookup an instance.
        private List<AnyClass>? _instanceMap;

        // The sum of all the minimum sizes (in bytes) of the sequences read in this buffer. Must not exceed the buffer
        // size.
        private int _minTotalSeqSize;

        // See ReadTypeId11.
        private int _posAfterLatestInsertedTypeId11;

        // Map of type ID index to type ID sequence, used only for classes.
        // We assign a type ID index (starting with 1) to each type ID (type ID sequence) we read, in order.
        // Since this map is a list, we lookup a previously assigned type ID (type ID sequence) with
        // _typeIdMap[index - 1]. With the 2.0 encoding, each entry has at least 1 element.
        private List<string>? _typeIdMap11;
        private List<string[]>? _typeIdMap20;

        // Read methods for basic types

        /// <summary>Reads a bool from the stream.</summary>
        /// <returns>The bool read from the stream.</returns>
        public bool ReadBool() => _buffer.Span[Pos++] == 1;

        /// <summary>Reads a byte from the stream.</summary>
        /// <returns>The byte read from the stream.</returns>
        public byte ReadByte() => _buffer.Span[Pos++];

        /// <summary>Reads a double from the stream.</summary>
        /// <returns>The double read from the stream.</returns>
        public double ReadDouble()
        {
            double value = BitConverter.ToDouble(_buffer.Span.Slice(Pos, sizeof(double)));
            Pos += sizeof(double);
            return value;
        }

        /// <summary>Reads a float from the stream.</summary>
        /// <returns>The float read from the stream.</returns>
        public float ReadFloat()
        {
            float value = BitConverter.ToSingle(_buffer.Span.Slice(Pos, sizeof(float)));
            Pos += sizeof(float);
            return value;
        }

        /// <summary>Reads an int from the stream.</summary>
        /// <returns>The int read from the stream.</returns>
        public int ReadInt()
        {
            int value = BitConverter.ToInt32(_buffer.Span.Slice(Pos, sizeof(int)));
            Pos += sizeof(int);
            return value;
        }

        /// <summary>Reads a long from the stream.</summary>
        /// <returns>The long read from the stream.</returns>
        public long ReadLong()
        {
            long value = BitConverter.ToInt64(_buffer.Span.Slice(Pos, sizeof(long)));
            Pos += sizeof(long);
            return value;
        }

        /// <summary>Reads a short from the stream.</summary>
        /// <returns>The short read from the stream.</returns>
        public short ReadShort()
        {
            short value = BitConverter.ToInt16(_buffer.Span.Slice(Pos, sizeof(short)));
            Pos += sizeof(short);
            return value;
        }

        /// <summary>Reads a size from the stream. This size's encoding is variable-length.</summary>
        /// <returns>The size read from the stream.</returns>
        public int ReadSize() => OldEncoding ? ReadSize11() : ReadSize20();

        /// <summary>Reads a string from the stream.</summary>
        /// <returns>The string read from the stream.</returns>
        public string ReadString()
        {
            int size = ReadSize();
            if (size == 0)
            {
                return "";
            }
            else
            {
                string value = _buffer.Slice(Pos, size).Span.ReadString();
                Pos += size;
                return value;
            }
        }

        /// <summary>Reads a uint from the stream.</summary>
        /// <returns>The uint read from the stream.</returns>
        public uint ReadUInt()
        {
            uint value = BitConverter.ToUInt32(_buffer.Span.Slice(Pos, sizeof(uint)));
            Pos += sizeof(uint);
            return value;
        }

        /// <summary>Reads a ulong from the stream.</summary>
        /// <returns>The ulong read from the stream.</returns>
        public ulong ReadULong()
        {
            ulong value = BitConverter.ToUInt64(_buffer.Span.Slice(Pos, sizeof(ulong)));
            Pos += sizeof(ulong);
            return value;
        }

        /// <summary>Reads a ushort from the stream.</summary>
        /// <returns>The ushort read from the stream.</returns>
        public ushort ReadUShort()
        {
            ushort value = BitConverter.ToUInt16(_buffer.Span.Slice(Pos, sizeof(ushort)));
            Pos += sizeof(ushort);
            return value;
        }

        /// <summary>Reads an int from the stream. This int is encoded using Ice's variable-size integer encoding.
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

        /// <summary>Reads a long from the stream. This long is encoded using Ice's variable-size integer encoding.
        /// </summary>
        /// <returns>The long read from the stream.</returns>
        public long ReadVarLong() =>
            (_buffer.Span[Pos] & 0x03) switch
            {
                0 => (sbyte)ReadByte() >> 2,
                1 => ReadShort() >> 2,
                2 => ReadInt() >> 2,
                _ => ReadLong() >> 2
            };

        /// <summary>Reads a uint from the stream. This uint is encoded using Ice's variable-size integer encoding.
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

        /// <summary>Reads a ulong from the stream. This ulong is encoded using Ice's variable-size integer encoding.
        /// </summary>
        /// <returns>The ulong read from the stream.</returns>
        public ulong ReadVarULong() =>
            (_buffer.Span[Pos] & 0x03) switch
            {
                0 => (uint)ReadByte() >> 2,   // cast to uint to use operator >> for uint instead of int, which is
                1 => (uint)ReadUShort() >> 2, // later implicitly converted to ulong
                2 => ReadUInt() >> 2,
                _ => ReadULong() >> 2
            };

        // Read methods for constructed types except class and exception

        /// <summary>Reads a sequence of fixed-size numeric values from the stream and returns an array.</summary>
        /// <returns>The sequence read from the stream, as an array.</returns>
        public T[] ReadArray<T>() where T : struct
        {
            int elementSize = Unsafe.SizeOf<T>();
            var value = new T[ReadAndCheckSeqSize(elementSize)];
            int byteCount = elementSize * value.Length;
            _buffer.Span.Slice(Pos, byteCount).CopyTo(MemoryMarshal.Cast<T, byte>(value));
            Pos += byteCount;
            return value;
        }

        /// <summary>Reads a sequence of fixed-size numeric values from the stream and returns an array.</summary>
        /// <param name="checkElement">A delegate use to checks each element of the array.</param>
        /// <returns>The sequence read from the stream, as an array.</returns>
        public T[] ReadArray<T>(Action<T> checkElement) where T : struct
        {
            T[] value = ReadArray<T>();
            foreach (T e in value)
            {
                checkElement(e);
            }
            return value;
        }

        /// <summary>Reads a sequence from the stream and returns an array.</summary>
        /// <param name="minElementSize">The minimum size of each element of the sequence, in bytes.</param>
        /// <param name="reader">The input stream reader used to read each element of the sequence.</param>
        /// <returns>The sequence read from the stream, as an array.</returns>
        public T[] ReadArray<T>(int minElementSize, InputStreamReader<T> reader) =>
            ReadSequence(minElementSize, reader).ToArray();

        /// <summary>Reads a sequence of nullable elements from the stream and returns an array.</summary>
        /// <param name="withBitSequence">True when null elements are encoded using a bit sequence; otherwise, false.
        /// </param>
        /// <param name="reader">The input stream reader used to read each non-null element of the sequence.</param>
        /// <returns>The sequence read from the stream, as an array.</returns>
        public T?[] ReadArray<T>(bool withBitSequence, InputStreamReader<T> reader) where T : class =>
            ReadSequence(withBitSequence, reader).ToArray();

        /// <summary>Reads a sequence of nullable values from the stream and returns an array.</summary>
        /// <param name="reader">The input stream reader used to read each non-null element of the sequence.</param>
        /// <returns>The sequence read from the stream, as an array.</returns>
        public T?[] ReadArray<T>(InputStreamReader<T> reader) where T : struct => ReadSequence(reader).ToArray();

        /// <summary>Reads a dictionary from the stream.</summary>
        /// <param name="minKeySize">The minimum size of each key of the dictionary, in bytes.</param>
        /// <param name="minValueSize">The minimum size of each value of the dictionary, in bytes.</param>
        /// <param name="keyReader">The input stream reader used to read each key of the dictionary.</param>
        /// <param name="valueReader">The input stream reader used to read each value of the dictionary.</param>
        /// <returns>The dictionary read from the stream.</returns>
        public Dictionary<TKey, TValue> ReadDictionary<TKey, TValue>(
            int minKeySize,
            int minValueSize,
            InputStreamReader<TKey> keyReader,
            InputStreamReader<TValue> valueReader)
            where TKey : notnull
        {
            int sz = ReadAndCheckSeqSize(minKeySize + minValueSize);
            var dict = new Dictionary<TKey, TValue>(sz);
            for (int i = 0; i < sz; ++i)
            {
                TKey key = keyReader(this);
                TValue value = valueReader(this);
                dict.Add(key, value);
            }
            return dict;
        }

        /// <summary>Reads a dictionary from the stream.</summary>
        /// <param name="minKeySize">The minimum size of each key of the dictionary, in bytes.</param>
        /// <param name="withBitSequence">When true, null dictionary values are encoded using a bit sequence.</param>
        /// <param name="keyReader">The input stream reader used to read each key of the dictionary.</param>
        /// <param name="valueReader">The input stream reader used to read each non-null value of the dictionary.
        /// </param>
        /// <returns>The dictionary read from the stream.</returns>
        public Dictionary<TKey, TValue?> ReadDictionary<TKey, TValue>(
            int minKeySize,
            bool withBitSequence,
            InputStreamReader<TKey> keyReader,
            InputStreamReader<TValue> valueReader)
            where TKey : notnull
            where TValue : class
        {
            int sz = ReadAndCheckSeqSize(minKeySize);
            return ReadDictionary(new Dictionary<TKey, TValue?>(sz), sz, withBitSequence, keyReader, valueReader);
        }

        /// <summary>Reads a dictionary from the stream.</summary>
        /// <param name="minKeySize">The minimum size of each key of the dictionary, in bytes.</param>
        /// <param name="keyReader">The input stream reader used to read each key of the dictionary.</param>
        /// <param name="valueReader">The input stream reader used to read each non-null value of the dictionary.
        /// </param>
        /// <returns>The dictionary read from the stream.</returns>
        public Dictionary<TKey, TValue?> ReadDictionary<TKey, TValue>(
            int minKeySize,
            InputStreamReader<TKey> keyReader,
            InputStreamReader<TValue> valueReader)
            where TKey : notnull
            where TValue : struct
        {
            int sz = ReadAndCheckSeqSize(minKeySize);
            return ReadDictionary(new Dictionary<TKey, TValue?>(sz), sz, keyReader, valueReader);
        }

        /// <summary>Reads a nullable proxy from the stream.</summary>
        /// <param name="factory">The proxy factory used to create the typed proxy.</param>
        /// <returns>The proxy read from the stream, or null.</returns>
        public T? ReadNullableProxy<T>(ProxyFactory<T> factory) where T : class, IObjectPrx
        {
            if (Communicator == null)
            {
                throw new InvalidOperationException(
                    "cannot read a proxy from an InputStream with a null communicator");
            }
            return Reference.Read(this) is Reference reference ? factory(reference) : null;
        }

        /// <summary>Reads a proxy from the stream.</summary>
        /// <param name="factory">The proxy factory used to create the typed proxy.</param>
        /// <returns>The proxy read from the stream; this proxy cannot be null.</returns>
        public T ReadProxy<T>(ProxyFactory<T> factory) where T : class, IObjectPrx =>
            ReadNullableProxy(factory) ?? throw new InvalidDataException("read null for a non-nullable proxy");

        /// <summary>Reads a sequence from the stream.</summary>
        /// <param name="minElementSize">The minimum size of each element of the sequence, in bytes.</param>
        /// <param name="reader">The input stream reader used to read each element of the sequence.</param>
        /// <returns>A collection that provides the size of the sequence and allows you read the sequence from the
        /// the stream. The return value does not fully implement ICollection{T}, in particular you can only call
        /// GetEnumerator() once on this collection. You would typically use this collection to construct a List{T} or
        /// some other generic collection that can be constructed from an IEnumerable{T}.</returns>
        public ICollection<T> ReadSequence<T>(int minElementSize, InputStreamReader<T> reader) =>
            new Collection<T>(this, minElementSize, reader);

        /// <summary>Reads a sequence of nullable elements from the stream. The element type is a reference type.
        /// </summary>
        /// <param name="withBitSequence">True when null elements are encoded using a bit sequence; otherwise, false.
        /// </param>
        /// <param name="reader">The input stream reader used to read each non-null element of the sequence.</param>
        /// <returns>A collection that provides the size of the sequence and allows you read the sequence from the
        /// the stream. The returned collection does not fully implement ICollection{T?}, in particular you can only
        /// call GetEnumerator() once on this collection. You would typically use this collection to construct a
        /// List{T?} or some other generic collection that can be constructed from an IEnumerable{T?}.</returns>
        public ICollection<T?> ReadSequence<T>(bool withBitSequence, InputStreamReader<T> reader) where T : class =>
            withBitSequence ? new NullableCollection<T>(this, reader) : (ICollection<T?>)ReadSequence(1, reader);

        /// <summary>Reads a sequence of nullable values from the stream.</summary>
        /// <param name="reader">The input stream reader used to read each non-null element (value) of the sequence.
        /// </param>
        /// <returns>A collection that provides the size of the sequence and allows you read the sequence from the
        /// the stream. The returned collection does not fully implement ICollection{T?}, in particular you can only
        /// call GetEnumerator() once on this collection. You would typically use this collection to construct a
        /// List{T?} or some other generic collection that can be constructed from an IEnumerable{T?}.</returns>
        public ICollection<T?> ReadSequence<T>(InputStreamReader<T> reader) where T : struct =>
            new NullableValueCollection<T>(this, reader);

        /// <summary>Reads a sorted dictionary from the stream.</summary>
        /// <param name="minKeySize">The minimum size of each key of the dictionary, in bytes.</param>
        /// <param name="minValueSize">The minimum size of each value of the dictionary, in bytes.</param>
        /// <param name="keyReader">The input stream reader used to read each key of the dictionary.</param>
        /// <param name="valueReader">The input stream reader used to read each value of the dictionary.</param>
        /// <returns>The sorted dictionary read from the stream.</returns>
        public SortedDictionary<TKey, TValue> ReadSortedDictionary<TKey, TValue>(
            int minKeySize,
            int minValueSize,
            InputStreamReader<TKey> keyReader,
            InputStreamReader<TValue> valueReader)
            where TKey : notnull
        {
            int sz = ReadAndCheckSeqSize(minKeySize + minValueSize);
            var dict = new SortedDictionary<TKey, TValue>();
            for (int i = 0; i < sz; ++i)
            {
                TKey key = keyReader(this);
                TValue value = valueReader(this);
                dict.Add(key, value);
            }
            return dict;
        }

        /// <summary>Reads a sorted dictionary from the stream.</summary>
        /// <param name="minKeySize">The minimum size of each key of the dictionary, in bytes.</param>
        /// <param name="withBitSequence">When true, null dictionary values are encoded using a bit sequence.</param>
        /// <param name="keyReader">The input stream reader used to read each key of the dictionary.</param>
        /// <param name="valueReader">The input stream reader used to read each non-null value of the dictionary.
        /// </param>
        /// <returns>The sorted dictionary read from the stream.</returns>
        public SortedDictionary<TKey, TValue?> ReadSortedDictionary<TKey, TValue>(
            int minKeySize,
            bool withBitSequence,
            InputStreamReader<TKey> keyReader,
            InputStreamReader<TValue> valueReader)
            where TKey : notnull
            where TValue : class =>
            ReadDictionary(
                new SortedDictionary<TKey, TValue?>(),
                ReadAndCheckSeqSize(minKeySize),
                withBitSequence,
                keyReader,
                valueReader);

        /// <summary>Reads a sorted dictionary from the stream. The dictionary's value type is a nullable value type.
        /// </summary>
        /// <param name="minKeySize">The minimum size of each key of the dictionary, in bytes.</param>
        /// <param name="keyReader">The input stream reader used to read each key of the dictionary.</param>
        /// <param name="valueReader">The input stream reader used to read each non-null value of the dictionary.
        /// </param>
        /// <returns>The sorted dictionary read from the stream.</returns>
        public SortedDictionary<TKey, TValue?> ReadSortedDictionary<TKey, TValue>(
            int minKeySize,
            InputStreamReader<TKey> keyReader,
            InputStreamReader<TValue> valueReader)
            where TKey : notnull
            where TValue : struct =>
            ReadDictionary(
                new SortedDictionary<TKey, TValue?>(),
                ReadAndCheckSeqSize(minKeySize),
                keyReader,
                valueReader);

        // Read methods for tagged basic types

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

        /// <summary>Reads a tagged size from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <returns>The size read from the stream, or null.</returns>
        public int? ReadTaggedSize(int tag) =>
            ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.Size) ? ReadSize() : (int?)null;

        /// <summary>Reads a tagged string from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <returns>The string read from the stream, or null.</returns>
        public string? ReadTaggedString(int tag) =>
            ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.VSize) ? ReadString() : null;

        /// <summary>Reads a tagged uint from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <returns>The uint read from the stream, or null.</returns>
        public uint? ReadTaggedUInt(int tag) =>
            ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.F4) ? ReadUInt() : (uint?)null;

        /// <summary>Reads a tagged ulong from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <returns>The ulong read from the stream, or null.</returns>
        public ulong? ReadTaggedULong(int tag) =>
            ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.F8) ? ReadULong() : (ulong?)null;

        /// <summary>Reads a tagged ushort from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <returns>The ushort read from the stream, or null.</returns>
        public ushort? ReadTaggedUShort(int tag) =>
            ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.F2) ? ReadUShort() : (ushort?)null;

        /// <summary>Reads a tagged varint from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <returns>The int read from the stream, or null.</returns>
        public int? ReadTaggedVarInt(int tag) =>
            ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.VInt) ? ReadVarInt() : (int?)null;

        /// <summary>Reads a tagged varlong from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <returns>The long read from the stream, or null.</returns>
        public long? ReadTaggedVarLong(int tag) =>
            ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.VInt) ? ReadVarLong() : (long?)null;

        /// <summary>Reads a tagged varuint from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <returns>The uint read from the stream, or null.</returns>
        public uint? ReadTaggedVarUInt(int tag) =>
            ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.VInt) ? ReadVarUInt() : (uint?)null;

        /// <summary>Reads a tagged varulong from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <returns>The ulong read from the stream, or null.</returns>
        public ulong? ReadTaggedVarULong(int tag) =>
            ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.VInt) ? ReadVarULong() : (ulong?)null;

        // Read methods for tagged constructed types except class

        /// <summary>Reads a tagged array of a fixed-size numeric type from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <returns>The sequence read from the stream as an array, or null.</returns>
        public T[]? ReadTaggedArray<T>(int tag) where T : struct
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
                return ReadArray<T>();
            }
            else
            {
                return null;
            }
        }

        /// <summary>Reads a tagged array of a fixed-size numeric type from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="checkElement">A delegate use to checks each element of the array.</param>
        /// <returns>The sequence read from the stream as an array, or null.</returns>
        public T[]? ReadTaggedArray<T>(int tag, Action<T> checkElement) where T : struct
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
                return ReadArray(checkElement);
            }
            else
            {
                return null;
            }
        }

        /// <summary>Reads a tagged array from the stream. The element type can be nullable only if it corresponds to
        /// a proxy class or mapped Slice class.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="minElementSize">The minimum size of each element, in bytes.</param>
        /// <param name="fixedSize">True when the element size is fixed; otherwise, false.</param>
        /// <param name="reader">The input stream reader used to read each element of the sequence.</param>
        /// <returns>The sequence read from the stream as an array, or null.</returns>
        public T[]? ReadTaggedArray<T>(int tag, int minElementSize, bool fixedSize, InputStreamReader<T> reader) =>
            ReadTaggedSequence(tag, minElementSize, fixedSize, reader)?.ToArray();

        /// <summary>Reads a tagged array of nullable elements from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="withBitSequence">True when null elements are encoded using a bit sequence; otherwise, false.
        /// </param>
        /// <param name="reader">The input stream reader used to read each non-null element of the array.</param>
        /// <returns>The array read from the stream, or null.</returns>
        public T?[]? ReadTaggedArray<T>(int tag, bool withBitSequence, InputStreamReader<T> reader) where T : class =>
            ReadTaggedSequence(tag, withBitSequence, reader)?.ToArray();

        /// <summary>Reads a tagged array of nullable values from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="reader">The input stream reader used to read each non-null value of the array.</param>
        /// <returns>The array read from the stream, or null.</returns>
        public T?[]? ReadTaggedArray<T>(int tag, InputStreamReader<T> reader) where T : struct =>
            ReadTaggedSequence(tag, reader)?.ToArray();

        /// <summary>Reads a tagged dictionary from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="minKeySize">The minimum size of each key, in bytes.</param>
        /// <param name="minValueSize">The minimum size of each value, in bytes.</param>
        /// <param name="fixedSize">When true, the entry size is fixed; otherwise, false.</param>
        /// <param name="keyReader">The input stream reader used to read each key of the dictionary.</param>
        /// <param name="valueReader">The input stream reader used to read each value of the dictionary.</param>
        /// <returns>The dictionary read from the stream, or null.</returns>
        public Dictionary<TKey, TValue>? ReadTaggedDictionary<TKey, TValue>(
            int tag,
            int minKeySize,
            int minValueSize,
            bool fixedSize,
            InputStreamReader<TKey> keyReader,
            InputStreamReader<TValue> valueReader)
            where TKey : notnull
        {
            if (ReadTaggedParamHeader(tag,
                    fixedSize ? EncodingDefinitions.TagFormat.VSize : EncodingDefinitions.TagFormat.FSize))
            {
                SkipSize(fixedLength: !fixedSize);
                return ReadDictionary(minKeySize, minValueSize, keyReader, valueReader);
            }
            return null;
        }

        /// <summary>Reads a tagged dictionary from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="minKeySize">The minimum size of each key, in bytes.</param>
        /// <param name="withBitSequence">When true, null dictionary values are encoded using a bit sequence.</param>
        /// <param name="keyReader">The input stream reader used to read each key of the dictionary.</param>
        /// <param name="valueReader">The input stream reader used to read each non-null value of the dictionary.
        /// </param>
        /// <returns>The dictionary read from the stream, or null.</returns>
        public Dictionary<TKey, TValue?>? ReadTaggedDictionary<TKey, TValue>(
            int tag,
            int minKeySize,
            bool withBitSequence,
            InputStreamReader<TKey> keyReader,
            InputStreamReader<TValue> valueReader)
            where TKey : notnull
            where TValue : class
        {
            if (ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.FSize))
            {
                SkipSize(fixedLength: true);
                return ReadDictionary(minKeySize, withBitSequence, keyReader, valueReader);
            }
            return null;
        }

        /// <summary>Reads a tagged dictionary from the stream. The dictionary's value type is a nullable value type.
        /// </summary>
        /// <param name="tag">The tag.</param>
        /// <param name="minKeySize">The minimum size of each key, in bytes.</param>
        /// <param name="keyReader">The input stream reader used to read each key of the dictionary.</param>
        /// <param name="valueReader">The input stream reader used to read each non-null value of the dictionary.
        /// </param>
        /// <returns>The dictionary read from the stream, or null.</returns>
        public Dictionary<TKey, TValue?>? ReadTaggedDictionary<TKey, TValue>(
            int tag,
            int minKeySize,
            InputStreamReader<TKey> keyReader,
            InputStreamReader<TValue> valueReader)
            where TKey : notnull
            where TValue : struct
        {
            if (ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.FSize))
            {
                SkipSize(fixedLength: true);
                return ReadDictionary(minKeySize, keyReader, valueReader);
            }
            return null;
        }

        /// <summary>Reads a tagged proxy from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="factory">The proxy factory used to create the typed proxy.</param>
        /// <returns>The proxy read from the stream, or null.</returns>
        public T? ReadTaggedProxy<T>(int tag, ProxyFactory<T> factory) where T : class, IObjectPrx
        {
            if (ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.FSize))
            {
                SkipSize(fixedLength: true);
                return ReadProxy(factory);
            }
            else
            {
                return null;
            }
        }

        /// <summary>Reads a tagged sequence from the stream. The element type can be nullable only if it corresponds to
        /// a proxy class or mapped Slice class.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="minElementSize">The minimum size of each element, in bytes.</param>
        /// <param name="fixedSize">True when the element size is fixed; otherwise, false.</param>
        /// <param name="reader">The input stream reader used to read each element of the sequence.</param>
        /// <returns>The sequence read from the stream as an ICollection{T}, or null.</returns>
        public ICollection<T>? ReadTaggedSequence<T>(
            int tag,
            int minElementSize,
            bool fixedSize,
            InputStreamReader<T> reader)
        {
            if (ReadTaggedParamHeader(tag,
                    fixedSize ? EncodingDefinitions.TagFormat.VSize : EncodingDefinitions.TagFormat.FSize))
            {
                if (!fixedSize || minElementSize > 1) // the size is optimized out for a fixed element size of 1
                {
                    SkipSize(fixedLength: !fixedSize);
                }
                return ReadSequence(minElementSize, reader);
            }
            return null;
        }

        /// <summary>Reads a tagged sequence of nullable elements from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="withBitSequence">True when null elements are encoded using a bit sequence; otherwise, false.
        /// </param>
        /// <param name="reader">The input stream reader used to read each non-null element of the sequence.</param>
        /// <returns>The sequence read from the stream as an ICollection{T?}, or null.</returns>
        public ICollection<T?>? ReadTaggedSequence<T>(int tag, bool withBitSequence, InputStreamReader<T> reader)
            where T : class
        {
            if (ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.FSize))
            {
                SkipSize(fixedLength: true);
                return ReadSequence(withBitSequence, reader);
            }
            else
            {
                return null;
            }
        }

        /// <summary>Reads a tagged sequence of nullable values from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="reader">The input stream reader used to read each non-null value of the sequence.</param>
        /// <returns>The sequence read from the stream as an ICollection{T?}, or null.</returns>
        public ICollection<T?>? ReadTaggedSequence<T>(int tag, InputStreamReader<T> reader)
            where T : struct
        {
            if (ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.FSize))
            {
                SkipSize(fixedLength: true);
                return ReadSequence(reader);
            }
            else
            {
                return null;
            }
        }

        /// <summary>Reads a tagged sorted dictionary from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="minKeySize">The minimum size of each key, in bytes.</param>
        /// <param name="minValueSize">The minimum size of each value, in bytes.</param>
        /// <param name="fixedSize">True when the entry size is fixed; otherwise, false.</param>
        /// <param name="keyReader">The input stream reader used to read each key of the dictionary.</param>
        /// <param name="valueReader">The input stream reader used to read each value of the dictionary.</param>
        /// <returns>The sorted dictionary read from the stream, or null.</returns>
        public SortedDictionary<TKey, TValue>? ReadTaggedSortedDictionary<TKey, TValue>(
            int tag,
            int minKeySize,
            int minValueSize,
            bool fixedSize,
            InputStreamReader<TKey> keyReader,
            InputStreamReader<TValue> valueReader) where TKey : notnull
        {
            if (ReadTaggedParamHeader(tag,
                    fixedSize ? EncodingDefinitions.TagFormat.VSize : EncodingDefinitions.TagFormat.FSize))
            {
                SkipSize(fixedLength: !fixedSize);
                return ReadSortedDictionary(minKeySize, minValueSize, keyReader, valueReader);
            }
            return null;
        }

        /// <summary>Reads a tagged sorted dictionary from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="minKeySize">The minimum size of each key, in bytes.</param>
        /// <param name="withBitSequence">When true, null dictionary values are encoded using a bit sequence.</param>
        /// <param name="keyReader">The input stream reader used to read each key of the dictionary.</param>
        /// <param name="valueReader">The input stream reader used to read each non-null value of the dictionary.
        /// </param>
        /// <returns>The dictionary read from the stream, or null.</returns>
        public SortedDictionary<TKey, TValue?>? ReadTaggeSorteddDictionary<TKey, TValue>(
            int tag,
            int minKeySize,
            bool withBitSequence,
            InputStreamReader<TKey> keyReader,
            InputStreamReader<TValue> valueReader)
            where TKey : notnull
            where TValue : class
        {
            if (ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.FSize))
            {
                SkipSize(fixedLength: true);
                return ReadSortedDictionary(minKeySize, withBitSequence, keyReader, valueReader);
            }
            return null;
        }

        /// <summary>Reads a tagged sorted dictionary from the stream. The dictionary's value type is a nullable value
        /// type.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="minKeySize">The minimum size of each key, in bytes.</param>
        /// <param name="keyReader">The input stream reader used to read each key of the dictionary.</param>
        /// <param name="valueReader">The input stream reader used to read each non-null value of the dictionary.
        /// </param>
        /// <returns>The dictionary read from the stream, or null.</returns>
        public SortedDictionary<TKey, TValue?>? ReadTaggedSortedDictionary<TKey, TValue>(
            int tag,
            int minKeySize,
            InputStreamReader<TKey> keyReader,
            InputStreamReader<TValue> valueReader)
            where TKey : notnull
            where TValue : struct
        {
            if (ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.FSize))
            {
                SkipSize(fixedLength: true);
                return ReadSortedDictionary(minKeySize, keyReader, valueReader);
            }
            return null;
        }

        /// <summary>Reads a tagged struct from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="fixedSize">True when the struct has a fixed size on the wire; otherwise, false.</param>
        /// <param name="reader">The input stream reader used to create and read the struct.</param>
        /// <returns>The struct T read from the stream, or null.</returns>
        public T? ReadTaggedStruct<T>(int tag, bool fixedSize, InputStreamReader<T> reader) where T : struct
        {
            if (ReadTaggedParamHeader(tag,
                    fixedSize ? EncodingDefinitions.TagFormat.VSize : EncodingDefinitions.TagFormat.FSize))
            {
                SkipSize(fixedLength: !fixedSize);
                return reader(this);
            }
            return null;
        }

        // Other methods

        /// <summary>Reads a bit sequence from the stream.</summary>
        /// <param name="bitSequenceSize">The minimum number of bits in the sequence.</param>
        /// <returns>The read-only bit sequence read from the stream.</returns>
        public ReadOnlyBitSequence ReadBitSequence(int bitSequenceSize)
        {
            int size = (bitSequenceSize >> 3) + ((bitSequenceSize & 0x07) != 0 ? 1 : 0);
            int startPos = Pos;
            Pos += size;
            return new ReadOnlyBitSequence(_buffer.Span.Slice(startPos, size));
        }

        /// <summary>Constructs a new InputStream over a byte buffer.</summary>
        /// <param name="buffer">The byte buffer.</param>
        /// <param name="encoding">The encoding of the buffer.</param>
        /// <param name="communicator">The communicator (optional).</param>
        /// <param name="connection">The connection (optional).</param>
        /// <param name="reference">The reference (optional).</param>
        /// <param name="startEncapsulation">When true, start reading an encapsulation in this byte buffer, and
        /// <c>encoding</c> represents the encoding of the header.</param>
        internal InputStream(
            ReadOnlyMemory<byte> buffer,
            Encoding encoding,
            Communicator? communicator = null,
            Connection? connection = null,
            Reference? reference = null,
            bool startEncapsulation = false)
        {
            // Connection and reference are mutually exclusive - it's neither or one or the other.
            Debug.Assert(connection == null || reference == null);

            Communicator = communicator ?? connection?.Communicator ?? reference?.Communicator;
            Connection = connection;
            Reference = reference;

            Pos = 0;
            _buffer = buffer;
            Encoding = encoding;
            Encoding.CheckSupported();

            if (startEncapsulation)
            {
                // When startEncapsulation is true, the buffer must extend until the end of the encapsulation - it
                // cannot include extra bytes.
                Encoding = ReadEncapsulationHeader(checkFullBuffer: true).Encoding;
                Encoding.CheckSupported();

                // We slice the provided buffer to the encapsulation (minus its header).
                _buffer = buffer.Slice(Pos);
                Pos = 0;

                if (Encoding == Encoding.V20)
                {
                    CompressionFormat compressionFormat = this.ReadCompressionFormat();
                    if (compressionFormat != CompressionFormat.Decompressed)
                    {
                        throw new InvalidDataException("the buffer encapsulation is compressed");
                    }
                }
            }
            _inEncapsulation = startEncapsulation;
        }

        /// <summary>Verifies the input stream has reached the end of its underlying buffer.</summary>
        /// <param name="skipTaggedParams">When true, first skips all remaining tagged parameters in the current
        /// encapsulation.</param>
        internal void CheckEndOfBuffer(bool skipTaggedParams)
        {
            if (skipTaggedParams)
            {
                Debug.Assert(_inEncapsulation);
                SkipTaggedParams();
            }

            if (Pos != _buffer.Length)
            {
                throw new InvalidDataException($"{_buffer.Length - Pos} bytes remaining in the InputStream buffer");
            }
        }

        /// <summary>Reads a binary context entry.</summary>
        /// <returns>The binary context key and value. The read-only memory for the value is backed by the input
        /// stream's buffer, the data is not copied.</returns>
        internal (int Key, ReadOnlyMemory<byte> Value) ReadBinaryContextEntry()
        {
            int key = ReadVarInt();
            int entrySize = ReadSize();
            ReadOnlyMemory<byte> value = _buffer.Slice(Pos, entrySize);
            Pos += entrySize;
            return (key, value);
        }

        /// <summary>Reads an encapsulation header from the stream.</summary>
        /// <param name="checkFullBuffer">When true, the encapsulation is expected to consume all the bytes of the
        /// current buffer. When false, bytes can remain in the buffer after the encapsulation.</param>
        /// <returns>The encapsulation header read from the stream. The size does not include the bytes to the size
        /// length; it does however include the two byte for the encoding.</returns>
        internal (int Size, Encoding Encoding) ReadEncapsulationHeader(bool checkFullBuffer)
        {
            int size;

            if (OldEncoding)
            {
                size = ReadInt();
                if (size < 4)
                {
                    throw new InvalidDataException($"the 1.1 encapsulation's size ({size}) is too small");
                }
                size -= 4; // remove the size length which is included with the 1.1 encoding
            }
            else
            {
                size = ReadSize20();
            }

            if (checkFullBuffer)
            {
                if (size != _buffer.Length - Pos)
                {
                    throw new InvalidDataException(
                        $"expected an encapsulation size of {_buffer.Length - Pos} bytes, but read {size}");
                }
            }
            else if (size > _buffer.Length - Pos)
            {
                throw new InvalidDataException(
                    $"the encapsulation's size ({size}) extends beyond the end of the buffer");
            }

            var encoding = new Encoding(this);
            return (size, encoding);
        }

        /// <summary>Reads an endpoint from the stream.</summary>
        /// <param name="protocol">The Ice protocol of this endpoint.</param>
        /// <returns>The endpoint read from the stream.</returns>
        internal Endpoint ReadEndpoint(Protocol protocol)
        {
            Debug.Assert(Communicator != null);
            Endpoint endpoint;

            if (protocol == Protocol.Ice1 || OldEncoding)
            {
                Transport transport = this.ReadTransport();
                (int size, Encoding encoding) = ReadEncapsulationHeader(checkFullBuffer: false);

                Ice1EndpointFactory? ice1Factory = protocol == Protocol.Ice1 && encoding.IsSupported ?
                    Communicator.FindIce1EndpointFactory(transport) : null;

                // Remove the two bytes of the encoding included in size. Endpoint encapsulations don't include a
                // compression byte.
                size -= 2;

                // We need to read the encapsulation except for ice1 + null factory.
                if (protocol == Protocol.Ice1 && ice1Factory == null)
                {
                    endpoint = OpaqueEndpoint.Create(transport,
                                                     encoding,
                                                     _buffer.Slice(Pos, size),
                                                     Communicator);
                    Pos += size;
                }
                else if (encoding.IsSupported)
                {
                    int oldPos = Pos;

                    // The common situation is an ice1 proxy in 1.1 encapsulation, with endpoints encoded with 1.1 (no
                    // need to create a new InputStream). A less common situation is an ice1 proxy in 2.0 encapsulation
                    // with 1.1-encoded endpoints (we need a new InputStream in this case).
                    InputStream istr = encoding == Encoding ?
                        this : new InputStream(_buffer.Slice(Pos, size), encoding, Communicator);

                    if (protocol == Protocol.Ice1)
                    {
                        Debug.Assert(ice1Factory != null); // see if block above with OpaqueEndpoint creation
                        endpoint = ice1Factory(transport, istr);
                    }
                    else
                    {
                        var data = new EndpointData(transport,
                                                    host: istr.ReadString(),
                                                    port: istr.ReadUShort(),
                                                    options: istr.ReadArray(1, IceReaderIntoString));

                        endpoint = data.ToEndpoint(Communicator, protocol);
                    }

                    if (ReferenceEquals(istr, this))
                    {
                        // Make sure we read the full encapsulation.
                        if (Pos != oldPos + size)
                        {
                            throw new InvalidDataException(
                                $"{oldPos + size - Pos} bytes left in endpoint encapsulation");
                        }
                    }
                    else
                    {
                        istr.CheckEndOfBuffer(skipTaggedParams: false);
                        Pos += size;
                    }
                }
                else
                {
                    string transportName = transport.ToString().ToLowerInvariant();
                    throw new InvalidDataException(
                        @$"cannot read endpoint for protocol `{protocol.GetName()}' and transport `{transportName
                        }' with endpoint encapsulation encoded with encoding `{encoding}'");
                }
            }
            else
            {
                endpoint = new EndpointData(this).ToEndpoint(Communicator, protocol);
            }

            return endpoint;
        }

        internal void Skip(int size)
        {
            if (size < 0 || size > _buffer.Length - Pos)
            {
                throw new IndexOutOfRangeException($"cannot skip {size} bytes");
            }
            Pos += size;
        }

        /// <summary>Reads a sequence size and makes sure there is enough space in the underlying buffer to read the
        /// sequence. This validation is performed to make sure we do not allocate a large container based on an
        /// invalid encoded size.</summary>
        /// <param name="minElementSize">The minimum encoded size of an element of the sequence, in bytes. This value is
        /// 0 for sequence of nullable types other than mapped Slice classes and proxies.</param>
        /// <returns>The number of elements in the sequence.</returns>
        private int ReadAndCheckSeqSize(int minElementSize)
        {
            int sz = ReadSize();

            if (sz == 0)
            {
                return 0;
            }

            // When minElementSize is 0, we only count of bytes that hold the bit sequence.
            int minSize = minElementSize > 0 ? sz * minElementSize : (sz >> 3) + ((sz & 0x07) != 0 ? 1 : 0);

            // With _minTotalSeqSize, we make sure that multiple sequences within an InputStream can't trigger
            // maliciously the allocation of a large amount of memory before we read these sequences from the buffer.
            _minTotalSeqSize += minSize;

            if (Pos + minSize > _buffer.Length || _minTotalSeqSize > _buffer.Length)
            {
                throw new InvalidDataException("invalid sequence size");
            }
            return sz;
        }

        private ReadOnlyMemory<byte> ReadBitSequenceMemory(int bitSequenceSize)
        {
            int size = (bitSequenceSize >> 3) + ((bitSequenceSize & 0x07) != 0 ? 1 : 0);
            int startPos = Pos;
            Pos += size;
            return _buffer.Slice(startPos, size);
        }

        private TDict ReadDictionary<TDict, TKey, TValue>(
            TDict dict,
            int size,
            bool withBitSequence,
            InputStreamReader<TKey> keyReader,
            InputStreamReader<TValue> valueReader)
            where TDict : IDictionary<TKey, TValue?>
            where TKey : notnull
            where TValue : class
        {
            if (withBitSequence)
            {
                ReadOnlyBitSequence bitSequence = ReadBitSequence(size);
                for (int i = 0; i < size; ++i)
                {
                    TKey key = keyReader(this);
                    TValue? value = bitSequence[i] ? valueReader(this) : (TValue?)null;
                    dict.Add(key, value);
                }
            }
            else
            {
                for (int i = 0; i < size; ++i)
                {
                    TKey key = keyReader(this);
                    TValue value = valueReader(this);
                    dict.Add(key, value);
                }
            }
            return dict;
        }

        private TDict ReadDictionary<TDict, TKey, TValue>(
            TDict dict,
            int size,
            InputStreamReader<TKey> keyReader,
            InputStreamReader<TValue> valueReader)
            where TDict : IDictionary<TKey, TValue?>
            where TKey : notnull
            where TValue : struct
        {
            ReadOnlyBitSequence bitSequence = ReadBitSequence(size);
            for (int i = 0; i < size; ++i)
            {
                TKey key = keyReader(this);
                TValue? value = bitSequence[i] ? valueReader(this) : (TValue?)null;
                dict.Add(key, value);
            }
            return dict;
        }

        private int ReadSize11()
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

        private int ReadSize20()
        {
            checked
            {
                return (int)ReadVarULong();
            }
        }

        private int ReadSpan(Span<byte> span)
        {
            int length = Math.Min(span.Length, _buffer.Length - Pos);
            _buffer.Span.Slice(Pos, length).CopyTo(span);
            Pos += length;
            return length;
        }

        /// <summary>Determines if a tagged parameter or data member is available for reading.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="expectedFormat">The expected format of the tagged parameter.</param>
        /// <returns>True if the tagged parameter is present; otherwise, false.</returns>
        private bool ReadTaggedParamHeader(int tag, EncodingDefinitions.TagFormat expectedFormat)
        {
            // Tagged members/parameters can only be in the main encapsulation.
            Debug.Assert(_inEncapsulation);

            // The current slice has no tagged parameter.
            if (_current.InstanceType != InstanceType.None &&
                (_current.SliceFlags & EncodingDefinitions.SliceFlags.HasTaggedMembers) == 0)
            {
                return false;
            }

            int requestedTag = tag;

            while (true)
            {
                if (_buffer.Length - Pos <= 0)
                {
                    return false; // End of encapsulation also indicates end of tagged parameters.
                }

                int savedPos = Pos;

                int v = ReadByte();
                if (v == EncodingDefinitions.TaggedEndMarker)
                {
                    Pos = savedPos; // rewind
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
                    Pos = savedPos; // rewind
                    return false; // No tagged parameter with the requested tag.
                }
                else if (tag < requestedTag)
                {
                    SkipTagged(format);
                }
                else
                {
                    // When expected format is VInt, format can be any of F1 through F8. Note that the exact format
                    // received does not matter in this case.
                    if (format != expectedFormat && (expectedFormat != EncodingDefinitions.TagFormat.VInt ||
                            (int)format > (int)EncodingDefinitions.TagFormat.F8))
                    {
                        throw new InvalidDataException($"invalid tagged parameter `{tag}': unexpected format");
                    }
                    return true;
                }
            }
        }

        /// <summary>Skips over a size value.</summary>
        /// <param name="fixedLength">When true and the encoding is 1.1, it's a fixed length size encoded on 4 bytes.
        /// When false, or the encoding is not 1.1, it's a variable-length size.</param>
        private void SkipSize(bool fixedLength = false)
        {
            if (OldEncoding)
            {
                if (fixedLength)
                {
                    Skip(4);
                }
                else
                {
                    byte b = ReadByte();
                    if (b == 255)
                    {
                        Skip(4);
                    }
                }
            }
            else
            {
                Skip(_buffer.Span[Pos].ReadSizeLength20());
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
                        Skip(ReadSize20());
                    }
                    break;
                default:
                    throw new InvalidDataException(
                        $"cannot skip tagged parameter or data member with tag format `{format}'");
            }
        }

        private void SkipTaggedParams()
        {
            while (true)
            {
                if (_buffer.Length - Pos <= 0)
                {
                    break;
                }

                int v = ReadByte();
                if (v == EncodingDefinitions.TaggedEndMarker)
                {
                    break;
                }

                var format = (EncodingDefinitions.TagFormat)(v & 0x07); // Read first 3 bits.
                if ((v >> 3) == 30)
                {
                    SkipSize();
                }
                SkipTagged(format);
            }
        }

        // Helper base class for the concrete collection implementations.
        private abstract class CollectionBase<T> : ICollection<T>, IEnumerator<T>
        {
            public int Count { get; }

            public T Current
            {
                get
                {
                    if (Pos == 0 || Pos > Count)
                    {
                        throw new InvalidOperationException();
                    }
                    return _current;
                }

                protected set => _current = value;
            }

            object? IEnumerator.Current => Current;
            public bool IsReadOnly => true;
            protected readonly InputStream InputStream;
            protected int Pos;
            private T _current;
            private bool _enumeratorRetrieved;

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

            public abstract bool MoveNext();

            public bool Remove(T item) => throw new NotSupportedException();
            public void Reset() => throw new NotSupportedException();

            // Disable this warning as the _current field is never read before it is initialized in MoveNext. Declaring
            // this field as nullable is not an option for a generic T that can be used with reference and value types.
#pragma warning disable CS8618 // Non-nullable field is uninitialized. Consider declaring as nullable.
            protected CollectionBase(InputStream istr, int minElementSize)
#pragma warning restore CS8618
            {
                Count = istr.ReadAndCheckSeqSize(minElementSize);
                InputStream = istr;
            }
        }

        // Collection<T> holds the size of a Slice sequence and reads the sequence elements from the InputStream
        // on-demand. It does not fully implement IEnumerable<T> and ICollection<T> (i.e. some methods throw
        // NotSupportedException) because it's not resettable: you can't use it to unmarshal the same bytes multiple
        // times.
        private sealed class Collection<T> : CollectionBase<T>
        {
            private readonly InputStreamReader<T> _reader;

            internal Collection(InputStream istr, int minElementSize, InputStreamReader<T> reader)
                : base(istr, minElementSize) => _reader = reader;

            public override bool MoveNext()
            {
                if (Pos < Count)
                {
                    Current = _reader(InputStream);
                    Pos++;
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }

        // Similar to Collection<T>, except we are reading a sequence<T?> where T is a reference type. T here must not
        // correspond to a mapped Slice class or to a proxy class.
        private sealed class NullableCollection<T> : CollectionBase<T?> where T : class
        {
            private readonly ReadOnlyMemory<byte> _bitSequenceMemory;
            private readonly InputStreamReader<T> _reader;

            internal NullableCollection(InputStream istr, InputStreamReader<T> reader)
                : base(istr, 0)
            {
                _bitSequenceMemory = istr.ReadBitSequenceMemory(Count);
                _reader = reader;
            }

            public override bool MoveNext()
            {
                if (Pos < Count)
                {
                    var bitSequence = new ReadOnlyBitSequence(_bitSequenceMemory.Span);
                    Current = bitSequence[Pos++] ? _reader(InputStream) : null;
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }

        // Similar to Collection<T>, except we are reading a sequence<T?> where T is a value type.
        private sealed class NullableValueCollection<T> : CollectionBase<T?> where T : struct
        {
            private readonly ReadOnlyMemory<byte> _bitSequenceMemory;
            private readonly InputStreamReader<T> _reader;

            internal NullableValueCollection(InputStream istr, InputStreamReader<T> reader)
                : base(istr, 0)
            {
                _bitSequenceMemory = istr.ReadBitSequenceMemory(Count);
                _reader = reader;
            }

            public override bool MoveNext()
            {
                if (Pos < Count)
                {
                    var bitSequence = new ReadOnlyBitSequence(_bitSequenceMemory.Span);
                    Current = bitSequence[Pos++] ? _reader(InputStream) : (T?)null;
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }
    }
}
