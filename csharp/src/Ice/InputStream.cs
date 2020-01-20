//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Binary;
using Protocol = IceInternal.Protocol;

namespace Ice
{
    /// <summary>
    /// Throws a UserException corresponding to the given Slice type Id, such as "::Module::MyException".
    /// If the implementation does not throw an exception, the Ice run time will fall back
    /// to using its default behavior for instantiating the user exception.
    /// </summary>
    /// <param name="id">A Slice type Id corresponding to a Slice user exception.</param>
    public delegate void UserExceptionFactory(string id);

    /// <summary>
    /// Interface for input streams used to extract Slice types from a sequence of bytes.
    /// </summary>
    public sealed class InputStream
    {
        /// <summary>
        /// Returns the current size of the stream.
        /// </summary>
        /// <value>The size of the stream.</value>
        public int Size => _buf.size();

        /// <summary>
        /// The communicator associated with this stream.
        /// </summary>
        /// <value>The communicator.</value>
        public Communicator Communicator
        {
            get; private set;
        }

        /// <summary>
        /// The encoding used when reading from this stream.
        /// </summary>
        /// <value>The encoding.</value>
        public EncodingVersion Encoding
        {
            get; private set;
        }

        // The position (offset) in the underlying buffer.
        internal int Pos
        {
            get => _buf.b.position();
            set => _buf.b.position(value);
        }

        // True if the internal buffer has no data, false otherwise.
        internal bool IsEmpty => _buf.empty();

        // Number of bytes remaining in the underlying buffer.
        private int Remaining => _limit - Pos ?? _buf.b.remaining();

        // When set, we are in reading a top-level encapsulation.
        private Encaps? _mainEncaps;

        // When set, we are reading an endpoint encapsulation. An endpoint encaps is a lightweight encaps that cannot
        // contain classes, exceptions, tagged members/parameters, or another endpoint. It is often but not always set
        // when _mainEncaps is set (so nested inside _mainEncaps).
        private Encaps? _endpointEncaps;

        // Temporary upper limit set by an encapsulation. See Remaining.
        private int? _limit;

        private readonly bool _traceSlicing;
        private readonly int _classGraphDepthMax;

        // The sum of all the mininum sizes (in bytes) of the sequences read in this buffer. Must not exceed the buffer
        // size.
        private int _minTotalSeqSize = 0;
        private readonly ILogger _logger;
        private readonly Func<string, Type?> _classResolver;
        private IceInternal.Buffer _buf;

        private byte[]? _stringBytes; // Reusable array for reading strings.

        // TODO: should we cache those per InputStream?
        //       should we clear the caches in ResetEncapsulation?
        private Dictionary<string, Type?>? _typeIdCache;
        private Dictionary<int, Type>? _compactIdCache;

        // Map of type-id index to type-id string.
        // When reading a top-level encapsulation, we assign a type-id index (starting with 1) to each type-id we
        // read, in order. Since this map is a list, we lookup a previously assigned type-id string with
        // _typeIdMap[index - 1].
        private List<string>? _typeIdMap;
        private int _posAfterLatestInsertedTypeId = 0;

        // The remaining fields are used for class/exception unmarshaling.
        // Class/exception unmarshaling is allowed only when _mainEncaps != null and _endpointEncaps == null.

        // Map of class instance ID to class instance.
        // When reading a top-level encapsulation:
        //  - Instance ID = 0 means null
        //  - Instance ID = 1 means the instance is encoded inline afterwards
        //  - Instance ID > 1 means a reference to a previously read instance, found in this map.
        // Since the map is actually a list, we use instance ID - 2 to lookup an instance.
        private List<AnyClass>? _instanceMap;
        private int _classGraphDepth = 0;
        private InstanceData? _current;

        /// <summary>
        /// This constructor uses the communicator's default encoding version.
        /// </summary>
        /// <param name="communicator">The communicator to use when initializing the stream.</param>
        /// <param name="data">The byte array containing encoded Slice types.</param>
        public InputStream(Communicator communicator, byte[] data)
            : this(communicator, null, new IceInternal.Buffer(data))
        {
        }

        /// <summary>
        /// This constructor uses the given encoding version.
        /// </summary>
        /// <param name="communicator">The communicator to use when initializing the stream.</param>
        /// <param name="encoding">The desired encoding version.</param>
        public InputStream(Communicator communicator, EncodingVersion encoding)
            : this(communicator, encoding, new IceInternal.Buffer())
        {
        }

        internal InputStream(Communicator communicator, EncodingVersion encoding, IceInternal.Buffer buf, bool adopt)
            : this(communicator, encoding, new IceInternal.Buffer(buf, adopt))
        {
        }

        // Helper constructor used by the constructors above.
        private InputStream(Communicator communicator, EncodingVersion? encoding, IceInternal.Buffer buf)
        {
            Encoding = encoding ?? communicator.defaultsAndOverrides().defaultEncoding;
            Communicator = communicator;
            _traceSlicing = communicator.traceLevels().slicing > 0;
            _classGraphDepthMax = communicator.classGraphDepthMax();
            _logger = communicator.Logger;
            _classResolver = communicator.resolveClass;
            _buf = buf;
        }

        /// <summary>
        /// Resets this stream. This method allows the stream to be reused, to avoid creating
        /// unnecessary garbage.
        /// </summary>
        public void Reset()
        {
            _buf.reset();
            _minTotalSeqSize = 0;
            Clear();
        }

        /// <summary>
        /// Releases any data retained by encapsulations.
        /// </summary>
        public void Clear()
        {
            ResetEncapsulation();
        }

        /// <summary>
        /// Swaps the contents of one stream with another.
        /// </summary>
        /// <param name="other">The other stream.</param>
        public void Swap(InputStream other)
        {
            Debug.Assert(Communicator == other.Communicator);

            IceInternal.Buffer tmpBuf = other._buf;
            other._buf = _buf;
            _buf = tmpBuf;

            EncodingVersion tmpEncoding = other.Encoding;
            other.Encoding = Encoding;
            Encoding = tmpEncoding;

            // Swap is never called for InputStreams that have encapsulations being read. However,
            // encapsulations might still be set in case un-marshalling failed. We just
            // reset the encapsulations if there are still some set.
            ResetEncapsulation();
            other.ResetEncapsulation();

            int tmpMinTotalSeqSize = other._minTotalSeqSize;
            other._minTotalSeqSize = _minTotalSeqSize;
            _minTotalSeqSize = tmpMinTotalSeqSize;
        }

        private void ResetEncapsulation()
        {
            _mainEncaps = null;
            _endpointEncaps = null;
            _instanceMap?.Clear();
            _classGraphDepth = 0;
            _typeIdMap?.Clear();
            _posAfterLatestInsertedTypeId = 0;
            _current = null;
            _limit = null;
        }

        /// <summary>
        /// Resizes the stream to a new size.
        /// </summary>
        /// <param name="sz">The new size.</param>
        internal void Resize(int sz)
        {
            _buf.resize(sz, true);
            _buf.b.position(sz);
        }

        internal IceInternal.Buffer GetBuffer()
        {
            return _buf;
        }

        /// <summary>
        /// Marks the start of a class instance.
        /// </summary>
        public void StartClass()
        {
            Debug.Assert(_mainEncaps != null && _endpointEncaps == null);
            StartInstance(SliceType.ClassSlice);
        }

        /// <summary>
        /// Marks the end of a class instance.
        /// </summary>
        /// <param name="preserve">True if unknown slices should be preserved, false otherwise.</param>
        /// <returns>A SlicedData object containing the preserved slices for unknown types.</returns>
        public SlicedData? EndClass(bool preserve)
        {
            Debug.Assert(_mainEncaps != null && _endpointEncaps == null);
            return EndInstance(preserve);
        }

        /// <summary>
        /// Marks the start of a user exception.
        /// </summary>
        public void StartException()
        {
            Debug.Assert(_mainEncaps != null && _endpointEncaps == null);
            StartInstance(SliceType.ExceptionSlice);
        }

        /// <summary>
        /// Marks the end of a user exception.
        /// </summary>
        /// <param name="preserve">True if unknown slices should be preserved, false otherwise.</param>
        /// <returns>A SlicedData object containing the preserved slices for unknown types.</returns>
        public SlicedData? EndException(bool preserve)
        {
            Debug.Assert(_mainEncaps != null && _endpointEncaps == null);
            return EndInstance(preserve);
        }

        /// <summary>
        /// Reads the start of an encapsulation.
        /// </summary>
        /// <returns>The encapsulation encoding version.</returns>
        public EncodingVersion StartEncapsulation()
        {
            Debug.Assert(_mainEncaps == null && _endpointEncaps == null);
            var encapsHeader = ReadEncapsulationHeader();
            _mainEncaps = new Encaps(_limit, Encoding, encapsHeader.Size);
            // TODO: is this check necessary / correct?
            Protocol.checkSupportedEncoding(encapsHeader.Encoding);
            Debug.Assert(!encapsHeader.Encoding.Equals(Util.Encoding_1_0));
            Encoding = encapsHeader.Encoding;
            _limit = Pos + encapsHeader.Size - 6;
            return encapsHeader.Encoding;
        }

        // for endpoints
        internal EncodingVersion StartEndpointEncapsulation()
        {
            Debug.Assert(_endpointEncaps == null);
            var encapsHeader = ReadEncapsulationHeader();
            _endpointEncaps = new Encaps(_limit, Encoding, encapsHeader.Size);
            // TODO: is this check necessary / correct?
            Protocol.checkSupportedEncoding(encapsHeader.Encoding);
            Encoding = encapsHeader.Encoding;
            _limit = Pos + encapsHeader.Size - 6;
            return encapsHeader.Encoding;
        }

        private (EncodingVersion Encoding, int Size) ReadEncapsulationHeader()
        {
            // With the 1.1 encoding, the encaps size is encoded on a 4-bytes int and not on a variable-length size,
            // for ease of marshaling.
            int sz = ReadInt();
            if (sz < 6)
            {
                throw new UnmarshalOutOfBoundsException();
            }
            if (sz - 4 > Remaining)
            {
                throw new UnmarshalOutOfBoundsException();
            }
            byte major = ReadByte();
            byte minor = ReadByte();
            var encoding = new EncodingVersion(major, minor);
            return (encoding, sz);
        }

        /// <summary>
        /// Ends the previous encapsulation.
        /// </summary>
        public void EndEncapsulation()
        {
            Debug.Assert(_mainEncaps != null && _endpointEncaps == null);
            SkipTaggedMembers();

            if (Remaining != 0)
            {
                throw new EncapsulationException();
            }
            _limit = _mainEncaps.Value.OldLimit;
            Encoding = _mainEncaps.Value.OldEncoding;
            ResetEncapsulation();
        }

        // for endpoints
        internal void EndEndpointEncapsulation()
        {
            Debug.Assert(_endpointEncaps != null);

            if (Remaining != 0)
            {
                throw new EncapsulationException();
            }

            _limit = _endpointEncaps.Value.OldLimit;
            Encoding = _endpointEncaps.Value.OldEncoding;
            _endpointEncaps = null;
        }

        /// <summary>
        /// Skips an empty encapsulation.
        /// </summary>
        /// <returns>The encapsulation's encoding version.</returns>
        public EncodingVersion SkipEmptyEncapsulation()
        {
            var encapsHeader = ReadEncapsulationHeader();
            if (encapsHeader.Encoding.Equals(Util.Encoding_1_0))
            {
                if (encapsHeader.Size != 6)
                {
                    throw new EncapsulationException();
                }
            }
            else
            {
                // Skip the optional content of the encapsulation if we are expecting an
                // empty encapsulation.
                _buf.b.position(_buf.b.position() + encapsHeader.Size - 6);
            }
            return encapsHeader.Encoding;
        }

        /// <summary>
        /// Returns a blob of bytes representing an encapsulation. The encapsulation's encoding version
        /// is returned in the argument.
        /// </summary>
        /// <param name="encoding">The encapsulation's encoding version.</param>
        /// <returns>The encoded encapsulation.</returns>
        public byte[] ReadEncapsulation(out EncodingVersion encoding)
        {
            var encapsHeader = ReadEncapsulationHeader();
            _buf.b.position(_buf.b.position() - 6);
            encoding = encapsHeader.Encoding;

            byte[] v = new byte[encapsHeader.Size];
            try
            {
                _buf.b.get(v);
                return v;
            }
            catch (InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Determines the size of the current encapsulation, excluding the encapsulation header.
        /// </summary>
        /// <returns>The size of the encapsulated data.</returns>
        public int GetEncapsulationSize()
        {
            Debug.Assert(_endpointEncaps != null || _mainEncaps != null);
            int size = _endpointEncaps?.Size ?? _mainEncaps?.Size ?? 0;
            return size - 6;
        }

        /// <summary>
        /// Skips over an encapsulation.
        /// </summary>
        /// <returns>The encoding version of the skipped encapsulation.</returns>
        public EncodingVersion SkipEncapsulation()
        {
            var encapsHeader = ReadEncapsulationHeader();
            try
            {
                _buf.b.position(_buf.b.position() + encapsHeader.Size - 6);
            }
            catch (ArgumentOutOfRangeException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
            return encapsHeader.Encoding;
        }

        /// <summary>
        /// Reads the start of a class instance or exception slice.
        /// </summary>
        /// <returns>The Slice type ID for this slice.</returns>
        public void StartSlice(string typeId, bool firstSlice)
        {
            Debug.Assert(_mainEncaps != null && _endpointEncaps == null);
            ReadSliceHeader(true, typeId, firstSlice);
            if (firstSlice)
            {
                // We can discard all the unknown slices: the generated code calls SaveUnknownSlices to
                // preserve them and it just called StartSlice instead.
            }
        }

        public SlicedData? SaveUnknownSlices(string typeId)
        {
            Debug.Assert(_mainEncaps != null && _endpointEncaps == null);
            ReadSliceHeader(true, typeId, true);

            SlicedData? slicedData = null;
            if (_current.slices?.Count > 0)
            {
                // The IndirectionTableList member holds the indirection table for each slice in _slices.
                Debug.Assert(_current.slices.Count == _current.IndirectionTableList.Count);
                for (int n = 0; n < _current.slices.Count; ++n)
                {
                    // We use the "instances" list in SliceInfo to hold references
                    // to the target instances. Note that the instances might not have
                    // been read yet in the case of a circular reference to an
                    // enclosing instance.
                    SliceInfo info = _current.slices[n];
                    info.instances = _current.IndirectionTableList[n];
                }

                slicedData = new SlicedData(_current.slices.ToArray());
                _current.slices.Clear();
            }

            // We may reuse this instance data (current) so we need to clean it well (see push)
            _current.IndirectionTableList?.Clear();
            Debug.Assert(_current.DeferredIndirectionTableList == null ||
                _current.DeferredIndirectionTableList.Count == 0);
            return slicedData;
        }

        /// <summary>
        /// Indicates that the end of a class instance or exception slice has been reached.
        /// </summary>
        public void EndSlice()
        {
            Debug.Assert(_mainEncaps != null && _endpointEncaps == null);
            if ((_current.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS) != 0)
            {
                SkipTaggedMembers();
            }
            if ((_current.sliceFlags & Protocol.FLAG_HAS_INDIRECTION_TABLE) != 0)
            {
                Debug.Assert(_current.PosAfterIndirectionTable.HasValue && _current.IndirectionTable != null);
                Pos = _current.PosAfterIndirectionTable.Value;
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
            try
            {
                byte b = _buf.b.get();
                if (b == 255)
                {
                    int v = _buf.b.getInt();
                    if (v < 0)
                    {
                        throw new UnmarshalOutOfBoundsException();
                    }
                    return v;
                }
                else
                {
                    return b; // byte is unsigned
                }
            }
            catch (InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
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

            // With _minTotalSeqSize, we make sure that multiple sequences within an InpuStream can't trigger
            // maliciously the allocation of a large amount of memory before we read these sequences from the buffer.
            _minTotalSeqSize += minSize;

            if (_buf.b.position() + minSize > _buf.size() || _minTotalSeqSize > _buf.size())
            {
                throw new UnmarshalOutOfBoundsException();
            }
            return sz;
        }

        /// <summary>
        /// Reads a blob of bytes from the stream. The length of the given array determines how many bytes are read.
        /// </summary>
        /// <param name="v">Bytes from the stream.</param>
        public void ReadBlob(byte[] v)
        {
            try
            {
                _buf.b.get(v);
            }
            catch (InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Reads a blob of bytes from the stream.
        /// </summary>
        /// <param name="sz">The number of bytes to read.</param>
        /// <returns>The requested bytes as a byte array.</returns>
        public byte[] ReadBlob(int sz)
        {
            if (Remaining < sz)
            {
                throw new UnmarshalOutOfBoundsException();
            }
            byte[] v = new byte[sz];
            try
            {
                _buf.b.get(v);
                return v;
            }
            catch (InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Determine if an optional value is available for reading.
        /// </summary>
        /// <param name="tag">The tag associated with the value.</param>
        /// <param name="expectedFormat">The optional format for the value.</param>
        /// <returns>True if the value is present, false otherwise.</returns>
        public bool ReadOptional(int tag, OptionalFormat expectedFormat)
        {
            // Tagged members/parameters can only be in the main encaps
            Debug.Assert(_mainEncaps != null && _endpointEncaps == null);

            // The current slice has no tagged member
            if (_current != null && (_current.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS) == 0)
            {
                return false;
            }

            int requestedTag = tag;

            while (true)
            {
                if (Remaining <= 0)
                {
                    return false; // End of encapsulation also indicates end of optionals.
                }

                int v = ReadByte();
                if (v == Protocol.OPTIONAL_END_MARKER)
                {
                    _buf.b.position(_buf.b.position() - 1); // Rewind.
                    return false;
                }

                OptionalFormat format = (OptionalFormat)(v & 0x07); // First 3 bits.
                tag = v >> 3;
                if (tag == 30)
                {
                    tag = ReadSize();
                }

                if (tag > requestedTag)
                {
                    int offset = tag < 30 ? 1 : (tag < 255 ? 2 : 6); // Rewind
                    _buf.b.position(_buf.b.position() - offset);
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
                        throw new MarshalException("invalid tagged data member `" + tag + "': unexpected format");
                    }
                    return true;
                }
            }
        }

        /// <summary>
        /// Extracts a byte value from the stream.
        /// </summary>
        /// <returns>The extracted byte.</returns>
        public byte ReadByte()
        {
            try
            {
                return _buf.b.get();
            }
            catch (InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts an optional byte value from the stream.
        /// </summary>
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

        /// <summary>
        /// Extracts a sequence of byte values from the stream.
        /// </summary>
        /// <returns>The extracted byte sequence.</returns>
        public byte[] ReadByteSeq()
        {
            try
            {
                int sz = ReadAndCheckSeqSize(1);
                byte[] v = new byte[sz];
                _buf.b.get(v);
                return v;
            }
            catch (InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of byte values from the stream.
        /// </summary>
        /// <param name="l">The extracted byte sequence as a list.</param>
        public void ReadByteSeq(out List<byte> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new List<byte>(ReadByteSeq());
        }

        /// <summary>
        /// Extracts a sequence of byte values from the stream.
        /// </summary>
        /// <param name="l">The extracted byte sequence as a linked list.</param>
        public void ReadByteSeq(out LinkedList<byte> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new LinkedList<byte>(ReadByteSeq());
        }

        /// <summary>
        /// Extracts a sequence of byte values from the stream.
        /// </summary>
        /// <param name="l">The extracted byte sequence as a queue.</param>
        public void ReadByteSeq(out Queue<byte> l)
        {
            //
            // Reading into an array and copy-constructing the
            // queue is faster than constructing the queue
            // and adding to it one element at a time.
            //
            l = new Queue<byte>(ReadByteSeq());
        }

        /// <summary>
        /// Extracts a sequence of byte values from the stream.
        /// </summary>
        /// <param name="l">The extracted byte sequence as a stack.</param>
        public void ReadByteSeq(out Stack<byte> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            byte[] array = ReadByteSeq();
            Array.Reverse(array);
            l = new Stack<byte>(array);
        }

        /// <summary>
        /// Extracts an optional byte sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public byte[]? ReadByteSeq(int tag)
        {
            if (ReadOptional(tag, OptionalFormat.VSize))
            {
                return ReadByteSeq();
            }
            else
            {
                return null;
            }
        }

        /// <summary>
        /// Extracts a serializable object from the stream.
        /// </summary>
        /// <returns>The serializable object.</returns>
        public object? ReadSerializable()
        {
            int sz = ReadAndCheckSeqSize(1);
            if (sz == 0)
            {
                return null;
            }
            try
            {
                var f = new BinaryFormatter(null, new StreamingContext(StreamingContextStates.All, Communicator));
                return f.Deserialize(new IceInternal.InputStreamWrapper(sz, this));
            }
            catch (System.Exception ex)
            {
                throw new MarshalException("cannot deserialize object:", ex);
            }
        }

        /// <summary>
        /// Extracts a boolean value from the stream.
        /// </summary>
        /// <returns>The extracted boolean.</returns>
        public bool ReadBool()
        {
            try
            {
                return _buf.b.get() == 1;
            }
            catch (InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

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
        public bool[] ReadBoolSeq()
        {
            try
            {
                int sz = ReadAndCheckSeqSize(1);
                bool[] v = new bool[sz];
                _buf.b.getBoolSeq(v);
                return v;
            }
            catch (InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of boolean values from the stream.
        /// </summary>
        /// <param name="l">The extracted boolean sequence as a list.</param>
        public void ReadBoolSeq(out List<bool> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new List<bool>(ReadBoolSeq());
        }

        /// <summary>
        /// Extracts a sequence of boolean values from the stream.
        /// </summary>
        /// <param name="l">The extracted boolean sequence as a linked list.</param>
        public void ReadBoolSeq(out LinkedList<bool> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new LinkedList<bool>(ReadBoolSeq());
        }

        /// <summary>
        /// Extracts a sequence of boolean values from the stream.
        /// </summary>
        /// <param name="l">The extracted boolean sequence as a queue.</param>
        public void ReadBoolSeq(out Queue<bool> l)
        {
            //
            // Reading into an array and copy-constructing the
            // queue is faster than constructing the queue
            // and adding to it one element at a time.
            //
            l = new Queue<bool>(ReadBoolSeq());
        }

        /// <summary>
        /// Extracts a sequence of boolean values from the stream.
        /// </summary>
        /// <param name="l">The extracted boolean sequence as a stack.</param>
        public void ReadBoolSeq(out Stack<bool> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            bool[] array = ReadBoolSeq();
            Array.Reverse(array);
            l = new Stack<bool>(array);
        }

        /// <summary>
        /// Extracts an optional boolean sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public bool[]? ReadBoolSeq(int tag)
        {
            if (ReadOptional(tag, OptionalFormat.VSize))
            {
                return ReadBoolSeq();
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
            try
            {
                return _buf.b.getShort();
            }
            catch (InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

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
        public short[] ReadShortSeq()
        {
            try
            {
                int sz = ReadAndCheckSeqSize(2);
                short[] v = new short[sz];
                _buf.b.getShortSeq(v);
                return v;
            }
            catch (InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of short values from the stream.
        /// </summary>
        /// <param name="l">The extracted short sequence as a list.</param>
        public void ReadShortSeq(out List<short> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new List<short>(ReadShortSeq());
        }

        /// <summary>
        /// Extracts a sequence of short values from the stream.
        /// </summary>
        /// <param name="l">The extracted short sequence as a linked list.</param>
        public void ReadShortSeq(out LinkedList<short> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new LinkedList<short>(ReadShortSeq());
        }

        /// <summary>
        /// Extracts a sequence of short values from the stream.
        /// </summary>
        /// <param name="l">The extracted short sequence as a queue.</param>
        public void ReadShortSeq(out Queue<short> l)
        {
            //
            // Reading into an array and copy-constructing the
            // queue is faster than constructing the queue
            // and adding to it one element at a time.
            //
            l = new Queue<short>(ReadShortSeq());
        }

        /// <summary>
        /// Extracts a sequence of short values from the stream.
        /// </summary>
        /// <param name="l">The extracted short sequence as a stack.</param>
        public void ReadShortSeq(out Stack<short> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            short[] array = ReadShortSeq();
            Array.Reverse(array);
            l = new Stack<short>(array);
        }

        /// <summary>
        /// Extracts an optional short sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public short[]? ReadShortSeq(int tag)
        {
            if (ReadOptional(tag, OptionalFormat.VSize))
            {
                skipSize();
                return ReadShortSeq();
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
            try
            {
                return _buf.b.getInt();
            }
            catch (InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

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
        public int[] ReadIntSeq()
        {
            try
            {
                int sz = ReadAndCheckSeqSize(4);
                int[] v = new int[sz];
                _buf.b.getIntSeq(v);
                return v;
            }
            catch (InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of int values from the stream.
        /// </summary>
        /// <param name="l">The extracted int sequence as a list.</param>
        public void ReadIntSeq(out List<int> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new List<int>(ReadIntSeq());
        }

        /// <summary>
        /// Extracts a sequence of int values from the stream.
        /// </summary>
        /// <param name="l">The extracted int sequence as a linked list.</param>
        public void ReadIntSeq(out LinkedList<int> l)
        {
            try
            {
                int sz = ReadAndCheckSeqSize(4);
                l = new LinkedList<int>();
                for (int i = 0; i < sz; ++i)
                {
                    l.AddLast(_buf.b.getInt());
                }
            }
            catch (InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of int values from the stream.
        /// </summary>
        /// <param name="l">The extracted int sequence as a queue.</param>
        public void ReadIntSeq(out Queue<int> l)
        {
            //
            // Reading into an array and copy-constructing the
            // queue takes the same time as constructing the queue
            // and adding to it one element at a time, so
            // we avoid the copy.
            //
            try
            {
                int sz = ReadAndCheckSeqSize(4);
                l = new Queue<int>(sz);
                for (int i = 0; i < sz; ++i)
                {
                    l.Enqueue(_buf.b.getInt());
                }
            }
            catch (InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of int values from the stream.
        /// </summary>
        /// <param name="l">The extracted int sequence as a stack.</param>
        public void ReadIntSeq(out Stack<int> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            int[] array = ReadIntSeq();
            Array.Reverse(array);
            l = new Stack<int>(array);
        }

        /// <summary>
        /// Extracts an optional int sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public int[]? ReadIntSeq(int tag)
        {
            if (ReadOptional(tag, OptionalFormat.VSize))
            {
                skipSize();
                return ReadIntSeq();
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
            try
            {
                return _buf.b.getLong();
            }
            catch (InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

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
        public long[] ReadLongSeq()
        {
            try
            {
                int sz = ReadAndCheckSeqSize(8);
                long[] v = new long[sz];
                _buf.b.getLongSeq(v);
                return v;
            }
            catch (InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of long values from the stream.
        /// </summary>
        /// <param name="l">The extracted long sequence as a list.</param>
        public void ReadLongSeq(out List<long> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new List<long>(ReadLongSeq());
        }

        /// <summary>
        /// Extracts a sequence of long values from the stream.
        /// </summary>
        /// <param name="l">The extracted long sequence as a linked list.</param>
        public void ReadLongSeq(out LinkedList<long> l)
        {
            try
            {
                int sz = ReadAndCheckSeqSize(4);
                l = new LinkedList<long>();
                for (int i = 0; i < sz; ++i)
                {
                    l.AddLast(_buf.b.getLong());
                }
            }
            catch (InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of long values from the stream.
        /// </summary>
        /// <param name="l">The extracted long sequence as a queue.</param>
        public void ReadLongSeq(out Queue<long> l)
        {
            //
            // Reading into an array and copy-constructing the
            // queue takes the same time as constructing the queue
            // and adding to it one element at a time, so
            // we avoid the copy.
            //
            try
            {
                int sz = ReadAndCheckSeqSize(4);
                l = new Queue<long>(sz);
                for (int i = 0; i < sz; ++i)
                {
                    l.Enqueue(_buf.b.getLong());
                }
            }
            catch (InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of long values from the stream.
        /// </summary>
        /// <param name="l">The extracted long sequence as a stack.</param>
        public void ReadLongSeq(out Stack<long> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            long[] array = ReadLongSeq();
            Array.Reverse(array);
            l = new Stack<long>(array);
        }

        /// <summary>
        /// Extracts an optional long sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public long[]? ReadLongSeq(int tag)
        {
            if (ReadOptional(tag, OptionalFormat.VSize))
            {
                skipSize();
                return ReadLongSeq();
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
            try
            {
                return _buf.b.getFloat();
            }
            catch (InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
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
        public float[] ReadFloatSeq()
        {
            try
            {
                int sz = ReadAndCheckSeqSize(4);
                float[] v = new float[sz];
                _buf.b.getFloatSeq(v);
                return v;
            }
            catch (InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of float values from the stream.
        /// </summary>
        /// <param name="l">The extracted float sequence as a list.</param>
        public void ReadFloatSeq(out List<float> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new List<float>(ReadFloatSeq());
        }

        /// <summary>
        /// Extracts a sequence of float values from the stream.
        /// </summary>
        /// <param name="l">The extracted float sequence as a linked list.</param>
        public void ReadFloatSeq(out LinkedList<float> l)
        {
            try
            {
                int sz = ReadAndCheckSeqSize(4);
                l = new LinkedList<float>();
                for (int i = 0; i < sz; ++i)
                {
                    l.AddLast(_buf.b.getFloat());
                }
            }
            catch (InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of float values from the stream.
        /// </summary>
        /// <param name="l">The extracted float sequence as a queue.</param>
        public void ReadFloatSeq(out Queue<float> l)
        {
            //
            // Reading into an array and copy-constructing the
            // queue takes the same time as constructing the queue
            // and adding to it one element at a time, so
            // we avoid the copy.
            //
            try
            {
                int sz = ReadAndCheckSeqSize(4);
                l = new Queue<float>(sz);
                for (int i = 0; i < sz; ++i)
                {
                    l.Enqueue(_buf.b.getFloat());
                }
            }
            catch (InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of float values from the stream.
        /// </summary>
        /// <param name="l">The extracted float sequence as a stack.</param>
        public void ReadFloatSeq(out Stack<float> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            float[] array = ReadFloatSeq();
            Array.Reverse(array);
            l = new Stack<float>(array);
        }

        /// <summary>
        /// Extracts an optional float sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public float[]? ReadFloatSeq(int tag)
        {
            if (ReadOptional(tag, OptionalFormat.VSize))
            {
                skipSize();
                return ReadFloatSeq();
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
            try
            {
                return _buf.b.getDouble();
            }
            catch (InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
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
        public double[] ReadDoubleSeq()
        {
            try
            {
                int sz = ReadAndCheckSeqSize(8);
                double[] v = new double[sz];
                _buf.b.getDoubleSeq(v);
                return v;
            }
            catch (InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of double values from the stream.
        /// </summary>
        /// <param name="l">The extracted double sequence as a list.</param>
        public void ReadDoubleSeq(out List<double> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new List<double>(ReadDoubleSeq());
        }

        /// <summary>
        /// Extracts a sequence of double values from the stream.
        /// </summary>
        /// <param name="l">The extracted double sequence as a linked list.</param>
        public void ReadDoubleSeq(out LinkedList<double> l)
        {
            try
            {
                int sz = ReadAndCheckSeqSize(4);
                l = new LinkedList<double>();
                for (int i = 0; i < sz; ++i)
                {
                    l.AddLast(_buf.b.getDouble());
                }
            }
            catch (InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of double values from the stream.
        /// </summary>
        /// <param name="l">The extracted double sequence as a queue.</param>
        public void ReadDoubleSeq(out Queue<double> l)
        {
            //
            // Reading into an array and copy-constructing the
            // queue takes the same time as constructing the queue
            // and adding to it one element at a time, so
            // we avoid the copy.
            //
            try
            {
                int sz = ReadAndCheckSeqSize(4);
                l = new Queue<double>(sz);
                for (int i = 0; i < sz; ++i)
                {
                    l.Enqueue(_buf.b.getDouble());
                }
            }
            catch (InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of double values from the stream.
        /// </summary>
        /// <param name="l">The extracted double sequence as a stack.</param>
        public void ReadDoubleSeq(out Stack<double> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            double[] array = ReadDoubleSeq();
            Array.Reverse(array);
            l = new Stack<double>(array);
        }

        /// <summary>
        /// Extracts an optional double sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public double[]? ReadDoubleSeq(int tag)
        {
            if (ReadOptional(tag, OptionalFormat.VSize))
            {
                skipSize();
                return ReadDoubleSeq();
            }
            else
            {
                return null;
            }
        }

        private static System.Text.UTF8Encoding utf8 = new System.Text.UTF8Encoding(false, true);

        /// <summary>
        /// Extracts a string from the stream.
        /// </summary>
        /// <returns>The extracted string.</returns>
        public string ReadString()
        {
            int len = ReadSize();

            if (len == 0)
            {
                return "";
            }

            //
            // Check the buffer has enough bytes to read.
            //
            if (Remaining < len)
            {
                throw new UnmarshalOutOfBoundsException();
            }

            try
            {
                //
                // We reuse the _stringBytes array to avoid creating
                // excessive garbage
                //
                if (_stringBytes == null || len > _stringBytes.Length)
                {
                    _stringBytes = new byte[len];
                }
                _buf.b.get(_stringBytes, 0, len);
                return utf8.GetString(_stringBytes, 0, len);
            }
            catch (InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
            catch (ArgumentException ex)
            {
                throw new MarshalException("Invalid UTF8 string", ex);
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

        /// <summary>
        /// Extracts a sequence of strings from the stream.
        /// </summary>
        /// <returns>The extracted string sequence.</returns>
        public string[] ReadStringSeq()
        {
            int sz = ReadAndCheckSeqSize(1);
            string[] v = new string[sz];
            for (int i = 0; i < sz; i++)
            {
                v[i] = ReadString();
            }
            return v;
        }

        /// <summary>
        /// Extracts a sequence of strings from the stream.
        /// </summary>
        /// <param name="l">The extracted string sequence as a list.</param>
        public void ReadStringSeq(out List<string> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is slower than constructing the list
            // and adding to it one element at a time.
            //
            int sz = ReadAndCheckSeqSize(1);
            l = new List<string>(sz);
            for (int i = 0; i < sz; ++i)
            {
                l.Add(ReadString());
            }
        }

        /// <summary>
        /// Extracts a sequence of strings from the stream.
        /// </summary>
        /// <param name="l">The extracted string sequence as a linked list.</param>
        public void ReadStringSeq(out LinkedList<string> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is slower than constructing the list
            // and adding to it one element at a time.
            //
            int sz = ReadAndCheckSeqSize(1);
            l = new LinkedList<string>();
            for (int i = 0; i < sz; ++i)
            {
                l.AddLast(ReadString());
            }
        }

        /// <summary>
        /// Extracts a sequence of strings from the stream.
        /// </summary>
        /// <param name="l">The extracted string sequence as a queue.</param>
        public void ReadStringSeq(out Queue<string> l)
        {
            //
            // Reading into an array and copy-constructing the
            // queue is slower than constructing the queue
            // and adding to it one element at a time.
            //
            int sz = ReadAndCheckSeqSize(1);
            l = new Queue<string>();
            for (int i = 0; i < sz; ++i)
            {
                l.Enqueue(ReadString());
            }
        }

        /// <summary>
        /// Extracts a sequence of strings from the stream.
        /// </summary>
        /// <param name="l">The extracted string sequence as a stack.</param>
        public void ReadStringSeq(out Stack<string> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            string[] array = ReadStringSeq();
            Array.Reverse(array);
            l = new Stack<string>(array);
        }

        /// <summary>
        /// Extracts an optional string sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public string[]? ReadStringSeq(int tag)
        {
            if (ReadOptional(tag, OptionalFormat.FSize))
            {
                skip(4);
                return ReadStringSeq();
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
            Identity ident = new Identity();
            ident.ice_readMembers(this);
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
                skip(4);
                return ReadProxy(factory);
            }
            else
            {
                return null;
            }
        }

        /// <summary>
        /// Read an enumerated value.
        /// </summary>
        /// <param name="maxValue">The maximum enumerator value in the definition.</param>
        /// <returns>The enumerator.</returns>
        public int ReadEnum(int maxValue)
        {
            if (Encoding.Equals(Util.Encoding_1_0))
            {
                if (maxValue < 127)
                {
                    return ReadByte();
                }
                else if (maxValue < 32767)
                {
                    return ReadShort();
                }
                else
                {
                    return ReadInt();
                }
            }
            else
            {
                return ReadSize();
            }
        }

        /// <summary>
        /// Read an instance of class T.
        /// </summary>
        /// <returns>The class instance, or null.</returns>
        public T? ReadClass<T>() where T : AnyClass
        {
            var obj = ReadAnyClass();
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
                IceInternal.Ex.throwUOE(typeof(T), obj);
                return null;
            }
        }

        /// <summary>
        /// Read a tagged parameter or data member of type class T.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the class parameter or data member.</param>
        /// <returns>The class instance, or null.</returns>
        public T? ReadClass<T>(int tag) where T : AnyClass
        {
            var obj = ReadAnyClass(tag);
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
                IceInternal.Ex.throwUOE(typeof(T), obj);
                return null;
            }
        }

        /// <summary>
        /// Read a tagged parameter or data member of type class.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the class parameter or data member.</param>
        /// <returns>The class instance, or null.</returns>
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

        /// <summary>
        /// Extracts a user exception from the stream and throws it.
        /// </summary>
        /// <param name="factory">The user exception factory, or null to use the stream's default behavior.</param>
        public void ThrowException(UserExceptionFactory? factory = null)
        {
            Debug.Assert(_current == null); // not currently reading a class
            Push(SliceType.ExceptionSlice);

            // Read the first slice header.
            string sliceTypeId = ReadSliceHeader(true); // we read the indirection table immediately
            var mostDerivedId = sliceTypeId;
            while (true)
            {
                UserException? userEx = null;

                // Use a factory if one was provided.
                try
                {
                    factory?.Invoke(sliceTypeId);
                }
                catch (UserException ex)
                {
                    userEx = ex;
                }

                if (userEx == null)
                {
                    try
                    {
                        Type? type = _classResolver?.Invoke(sliceTypeId);
                        if (type != null)
                        {
                            userEx = (UserException?)IceInternal.AssemblyUtil.createInstance(type);
                        }
                    }
                    catch (Exception ex)
                    {
                        throw new MarshalException(ex);
                    }
                }

                // We found the exception.
                if (userEx != null)
                {
                    userEx.iceRead(this, true);
                    throw userEx;
                    // Never reached.
                }

                // Slice off what we don't understand.
                SkipSlice(sliceTypeId);

                if ((_current.sliceFlags & Protocol.FLAG_IS_LAST_SLICE) != 0)
                {
                    if (mostDerivedId.StartsWith("::", StringComparison.Ordinal))
                    {
                        throw new UnknownUserException(mostDerivedId.Substring(2));
                    }
                    else
                    {
                        throw new UnknownUserException(mostDerivedId);
                    }
                }

                sliceTypeId = ReadSliceHeader(true);
            }
        }

        /// <summary>
        /// Skip the given number of bytes.
        /// </summary>
        /// <param name="size">The number of bytes to skip</param>
        public void skip(int size)
        {
            if (size < 0 || size > Remaining)
            {
                throw new UnmarshalOutOfBoundsException();
            }
            _buf.b.position(_buf.b.position() + size);
        }

        /// <summary>
        /// Skip over a size value.
        /// </summary>
        public void skipSize()
        {
            byte b = ReadByte();
            if (b == 255)
            {
                skip(4);
            }
        }

        private void SkipTagged(OptionalFormat format)
        {
            switch (format)
            {
                case OptionalFormat.F1:
                    {
                        skip(1);
                        break;
                    }
                case OptionalFormat.F2:
                    {
                        skip(2);
                        break;
                    }
                case OptionalFormat.F4:
                    {
                        skip(4);
                        break;
                    }
                case OptionalFormat.F8:
                    {
                        skip(8);
                        break;
                    }
                case OptionalFormat.Size:
                    {
                        skipSize();
                        break;
                    }
                case OptionalFormat.VSize:
                    {
                        skip(ReadSize());
                        break;
                    }
                case OptionalFormat.FSize:
                    {
                        skip(ReadInt());
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
                if (Remaining <= 0)
                {
                    return false; // End of encapsulation also indicates end of tagged members.
                }

                int v = ReadByte();
                if (v == Protocol.OPTIONAL_END_MARKER)
                {
                    return true;
                }

                OptionalFormat format = (OptionalFormat)(v & 0x07); // Read first 3 bits.
                if ((v >> 3) == 30)
                {
                    skipSize();
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
                    // The encoded type-id indices start at 1, not 0.
                    return _typeIdMap[index - 1];
                }
                throw new MarshalException($"read invalid typeId index {index}");
            }
            else
            {
                string typeId = ReadString();

                // We only want to add this typeId in the map list increment the Count
                // when it's the first time we read it, so we save the largest pos we
                // read to figure when to add.
                if (Pos > _posAfterLatestInsertedTypeId)
                {
                    _posAfterLatestInsertedTypeId = Pos;
                    _typeIdMap.Add(typeId);
                }

                return typeId;
            }
        }

        private Type? ResolveClass(string typeId)
        {
            Type? cls = null;
            if (_typeIdCache?.TryGetValue(typeId, out cls) != true)
            {
                // Not found in typeIdCache
                try
                {
                    cls = _classResolver?.Invoke(typeId);
                    _typeIdCache ??= new Dictionary<string, Type?>(); // Lazy initialization
                    _typeIdCache.Add(typeId, cls);
                }
                catch (Exception ex)
                {
                    throw new NoClassFactoryException("no class factory", typeId, ex);
                }
            }
            return cls;
        }

        private AnyClass? ReadAnyClass()
        {
            int index = ReadSize();
            if (index < 0)
            {
                throw new MarshalException("invalid object id");
            }
            else if (index == 0)
            {
                return null;
            }
            else if (_current != null && (_current.sliceFlags & Protocol.FLAG_HAS_INDIRECTION_TABLE) != 0)
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
                    throw new MarshalException("index too big for indirection table");
                }
            }
            else
            {
                return ReadInstance(index);
            }
        }

        private void StartInstance(SliceType sliceType)
        {
            Debug.Assert(_current.sliceType == sliceType);
        }

        private SlicedData? EndInstance(bool preserve)
        {
            SlicedData? slicedData = null;
            if (preserve && _current.slices?.Count > 0)
            {
                // The IndirectionTableList member holds the indirection table for each slice in _slices.
                Debug.Assert(_current.slices.Count == _current.IndirectionTableList.Count);
                for (int n = 0; n < _current.slices.Count; ++n)
                {
                    // We use the "instances" list in SliceInfo to hold references
                    // to the target instances. Note that the instances might not have
                    // been read yet in the case of a circular reference to an
                    // enclosing instance.
                    SliceInfo info = _current.slices[n];
                    info.instances = _current.IndirectionTableList[n];
                }

                slicedData = new SlicedData(_current.slices.ToArray());
                _current.slices.Clear();
            }

            // We may reuse this instance data (current) so we need to clean it well (see push)
            _current.IndirectionTableList?.Clear();
            Debug.Assert(_current.DeferredIndirectionTableList == null ||
                _current.DeferredIndirectionTableList.Count == 0);
            _current = _current.previous;
            return slicedData;
        }

        private string ReadSliceHeader(bool plusIndirectionTable, string? typeId = null, bool firstSlice = false)
        {
            // If first slice, don't read the header, it was already read in
            // ReadInstance or ThrowException to find the factory.
            if (!firstSlice)
            {
                Debug.Assert(firstSlice == false);
                _current.sliceFlags = ReadByte();

                // Read the type ID, for instance slices the type ID is encoded as a
                // string or as an index, for exceptions it's always encoded as a
                // string.
                if (_current.sliceType == SliceType.ClassSlice)
                {
                    // TYPE_ID_COMPACT must be checked first!
                    if ((_current.sliceFlags & Protocol.FLAG_HAS_TYPE_ID_COMPACT) ==
                        Protocol.FLAG_HAS_TYPE_ID_COMPACT)
                    {
                        _current.typeId = "";
                        _current.compactId = ReadSize();
                    }
                    else if ((_current.sliceFlags &
                            (Protocol.FLAG_HAS_TYPE_ID_INDEX | Protocol.FLAG_HAS_TYPE_ID_STRING)) != 0)
                    {
                        _current.typeId = ReadTypeId((_current.sliceFlags & Protocol.FLAG_HAS_TYPE_ID_INDEX) != 0);
                        _current.compactId = -1;
                    }
                    else
                    {
                        // Only the most derived slice encodes the type ID for the compact format.
                        _current.typeId = "";
                        _current.compactId = -1;
                    }
                }
                else
                {
                    _current.typeId = ReadString();
                    _current.compactId = -1;
                }

                // Read the slice size if necessary.
                if ((_current.sliceFlags & Protocol.FLAG_HAS_SLICE_SIZE) != 0)
                {
                    _current.sliceSize = ReadInt();
                    if (_current.sliceSize < 4)
                    {
                        throw new MarshalException("invalid slice size");
                    }
                }
                else
                {
                    _current.sliceSize = 0;
                }
            }

            // Make sure the type ids match!
            if (typeId != null && _current.typeId != "")
            {
                Debug.Assert(typeId == _current.typeId);
            }

            // Read the indirection table now
            if (plusIndirectionTable && (_current.sliceFlags & Protocol.FLAG_HAS_INDIRECTION_TABLE) != 0)
            {
                if (_current.IndirectionTable != null)
                {
                    Debug.Assert(firstSlice && _current.sliceType == SliceType.ExceptionSlice);
                    // We already read it (firstSlice is true and it's an exception), so nothing to do
                    // Note that for classes, we only read the indirection table for the first slice
                    // when firtSlice is true.
                }
                else
                {
                    int savedPos = Pos;
                    if (_current.sliceSize < 4)
                    {
                        throw new MarshalException("invalid slice size");
                    }
                    Pos = savedPos + _current.sliceSize - 4;
                    _current.IndirectionTable = ReadIndirectionTable();
                    _current.PosAfterIndirectionTable = Pos;
                    Pos = savedPos;
                }
            }

            return _current.typeId;
        }

        private void SkipSlice(string sliceTypeId)
        {
            if (Communicator.traceLevels().slicing > 0)
            {
                ILogger logger = Communicator.Logger;
                string slicingCat = Communicator.traceLevels().slicingCat;
                if (_current.sliceType == SliceType.ExceptionSlice)
                {
                    IceInternal.TraceUtil.traceSlicing("exception", sliceTypeId, slicingCat, logger);
                }
                else
                {
                    IceInternal.TraceUtil.traceSlicing("object", sliceTypeId, slicingCat, logger);
                }
            }

            int start = Pos;

            if ((_current.sliceFlags & Protocol.FLAG_HAS_SLICE_SIZE) != 0)
            {
                Debug.Assert(_current.sliceSize >= 4);
                skip(_current.sliceSize - 4);
            }
            else
            {
                if (_current.sliceType == SliceType.ClassSlice)
                {
                    throw new NoClassFactoryException("no class factory found and compact format prevents " +
                                                      "slicing (the sender should use the sliced format " +
                                                      "instead)", sliceTypeId);
                }
                else
                {
                    if (sliceTypeId.StartsWith("::", StringComparison.Ordinal))
                    {
                        throw new UnknownUserException(sliceTypeId.Substring(2));
                    }
                    else
                    {
                        throw new UnknownUserException(sliceTypeId);
                    }
                }
            }

            // Preserve this slice.
            int compactId = _current.compactId;
            bool hasOptionalMembers = (_current.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS) != 0;
            bool isLastSlice = (_current.sliceFlags & Protocol.FLAG_IS_LAST_SLICE) != 0;
            IceInternal.ByteBuffer b = GetBuffer().b;
            int end = b.position();
            int dataEnd = end;
            if (hasOptionalMembers)
            {
                // Don't include the tagged end marker. It will be re-written by EndSlice when the sliced data
                // is re-written.
                --dataEnd;
            }
            byte[] bytes = new byte[dataEnd - start];
            b.position(start);
            b.get(bytes);
            b.position(end);

            _current.slices ??= new List<SliceInfo>();
            var info = new SliceInfo(sliceTypeId, compactId, bytes, Array.Empty<AnyClass>(), hasOptionalMembers,
                                     isLastSlice);
            _current.slices.Add(info);

            // The deferred indirection table is only used by classes. For exceptions, the indirection table is
            // unmarshaled immediately.
            if (_current.sliceType == SliceType.ClassSlice)
            {
                _current.DeferredIndirectionTableList ??= new List<int>();
                if ((_current.sliceFlags & Protocol.FLAG_HAS_INDIRECTION_TABLE) != 0)
                {
                    _current.DeferredIndirectionTableList.Add(Pos);
                    SkipIndirectionTable();
                }
                else
                {
                    _current.DeferredIndirectionTableList.Add(0);
                }
            }
            else
            {
                _current.IndirectionTableList ??= new List<AnyClass[]?>();
                if ((_current.sliceFlags & Protocol.FLAG_HAS_INDIRECTION_TABLE) != 0)
                {
                    Debug.Assert(_current.IndirectionTable != null); // previously read by ReadSliceHeader
                    _current.IndirectionTableList.Add(_current.IndirectionTable);
                    Pos = _current.PosAfterIndirectionTable.Value;
                    _current.PosAfterIndirectionTable = null;
                    _current.IndirectionTable = null;
                }
                else
                {
                    _current.IndirectionTableList.Add(null);
                }
            }
        }

        // Skip the indirection table. The caller must save the current stream position before calling
        // SkipIndirectionTable (to read the indirection table at a later point) except when the caller
        // is SkipIndirectionTable itself.
        private void SkipIndirectionTable()
        {
            // We should never skip an exception's indirection table
            Debug.Assert(_current.sliceType == SliceType.ClassSlice);

            // We use ReadSize and not ReadAndCheckSeqSize here because we don't allocate memory for this
            // sequence, and since we are skipping this sequence to read it later, we don't want to double-count
            // its contribution to _minTotalSeqSize.
            var tableSize = ReadSize();
            for (int i = 0; i < tableSize; ++i)
            {
                var index = ReadSize();
                if (index <= 0)
                {
                    throw new MarshalException($"read invalid index {index} in indirection table");
                }
                if (index == 1)
                {
                    if (++_classGraphDepth > _classGraphDepthMax)
                    {
                        throw new MarshalException("maximum class graph depth reached");
                    }

                    // Read/skip this instance
                    byte sliceFlags = 0;
                    do
                    {
                        sliceFlags = ReadByte();
                        if ((sliceFlags & Protocol.FLAG_HAS_TYPE_ID_COMPACT) == Protocol.FLAG_HAS_TYPE_ID_COMPACT)
                        {
                            ReadSize(); // compact type-id
                        }
                        else if ((sliceFlags &
                            (Protocol.FLAG_HAS_TYPE_ID_INDEX | Protocol.FLAG_HAS_TYPE_ID_STRING)) != 0)
                        {
                            // This can update the typeIdMap
                            ReadTypeId((sliceFlags & Protocol.FLAG_HAS_TYPE_ID_INDEX) != 0);
                        }
                        else
                        {
                            throw new MarshalException(
                                "indirection table cannot hold an instance without a type-id");
                        }

                        // Read the slice size, then skip the slice
                        if ((sliceFlags & Protocol.FLAG_HAS_SLICE_SIZE) == 0)
                        {
                            throw new MarshalException("size of slice missing");
                        }
                        int sliceSize = ReadInt();
                        if (sliceSize < 4)
                        {
                            throw new MarshalException("invalid slice size");
                        }
                        Pos = Pos + sliceSize - 4;

                        // If this slice has an indirection table, skip it too
                        if ((sliceFlags & Protocol.FLAG_HAS_INDIRECTION_TABLE) != 0)
                        {
                            SkipIndirectionTable();
                        }
                    } while ((sliceFlags & Protocol.FLAG_IS_LAST_SLICE) == 0);
                    _classGraphDepth--;
                }
            }
        }

        private AnyClass[] ReadIndirectionTable()
        {
            var size = ReadAndCheckSeqSize(1);
            if (size == 0)
            {
                throw new MarshalException("invalid empty indirection table");
            }
            var indirectionTable = new AnyClass[size];
            for (int i = 0; i < indirectionTable.Length; ++i)
            {
                int index = ReadSize();
                if (index < 1)
                {
                    throw new MarshalException($"read invalid index {index} in indirection table");
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
                throw new MarshalException($"could not find index {index} in {nameof(_instanceMap)}");
            }

            Push(SliceType.ClassSlice);

            // Read the first slice header.
            string sliceTypeId = ReadSliceHeader(false);
            var mostDerivedId = sliceTypeId;
            AnyClass? v = null;
            while (true)
            {
                bool updateCache = false;

                if (_current.compactId >= 0)
                {
                    updateCache = true;

                    // Translate a compact (numeric) type ID into a class.
                    if (_compactIdCache == null)
                    {
                        _compactIdCache = new Dictionary<int, Type>(); // Lazy initialization.
                    }
                    else
                    {
                        // Check the cache to see if we've already translated the compact type ID into a class.
                        Type? cls = null;
                        _compactIdCache.TryGetValue(_current.compactId, out cls);
                        if (cls != null)
                        {
                            try
                            {
                                Debug.Assert(!cls.IsAbstract && !cls.IsInterface);
                                v = (AnyClass?)IceInternal.AssemblyUtil.createInstance(cls);
                                updateCache = false;
                            }
                            catch (Exception ex)
                            {
                                throw new NoClassFactoryException("no class factory", "compact ID " +
                                                                  _current.compactId, ex);
                            }
                        }
                    }

                    // If we haven't already cached a class for the compact ID, then try to translate the
                    // compact ID into a type ID.
                    if (v == null && sliceTypeId.Length == 0)
                    {
                        sliceTypeId = Communicator.resolveCompactId(_current.compactId);
                    }
                }

                if (v == null && sliceTypeId.Length > 0)
                {
                    Type? cls = ResolveClass(sliceTypeId);

                    if (cls != null)
                    {
                        try
                        {
                            Debug.Assert(!cls.IsAbstract && !cls.IsInterface);
                            v = (AnyClass?)IceInternal.AssemblyUtil.createInstance(cls);
                        }
                        catch (Exception ex)
                        {
                            throw new NoClassFactoryException("no class factory", sliceTypeId, ex);
                        }
                    }
                }

                if (v != null)
                {
                    if (updateCache)
                    {
                        Debug.Assert(_current.compactId >= 0);
                        _compactIdCache.Add(_current.compactId, v.GetType());
                    }

                    // We have an instance, get out of this loop.
                    break;
                }

                // Slice off what we don't understand.
                // TODO: pass compactId for the case where sliceTypeId is empty
                SkipSlice(sliceTypeId);

                // If this is the last slice, keep the instance as an opaque
                // UnknownSlicedClass object.
                if ((_current.sliceFlags & Protocol.FLAG_IS_LAST_SLICE) != 0)
                {
                    // Note that mostDerivedId can be empty with an unresolved compactId.
                    v = new UnknownSlicedClass(mostDerivedId);
                    break;
                }

                sliceTypeId = ReadSliceHeader(false); // Read next Slice header for next iteration.
            }

            if (++_classGraphDepth > _classGraphDepthMax)
            {
                throw new MarshalException("maximum class graph depth reached");
            }

            // Add the instance to the map/list of instances. This must be done before reading the instances (for
            // circular references).
            _instanceMap ??= new List<AnyClass>();
            _instanceMap.Add(v);

            // Read all the deferred indirection tables now that the instance is inserted in _instanceMap.
            if (_current.DeferredIndirectionTableList?.Count > 0)
            {
                int savedPos = Pos;

                Debug.Assert(_current.IndirectionTableList == null || _current.IndirectionTableList.Count == 0);
                _current.IndirectionTableList ??= new List<AnyClass[]?>(_current.DeferredIndirectionTableList.Count);
                foreach (int pos in _current.DeferredIndirectionTableList)
                {
                    if (pos > 0)
                    {
                        Pos = pos;
                        _current.IndirectionTableList.Add(ReadIndirectionTable());
                    }
                    else
                    {
                        _current.IndirectionTableList.Add(null);
                    }
                }
                Pos = savedPos;
                _current.DeferredIndirectionTableList.Clear();
            }

            // Read the instance.
            v.iceRead(this, true);

            --_classGraphDepth;
            return v;
        }

        private void Push(SliceType sliceType)
        {
            if (_current == null)
            {
                _current = new InstanceData(null);
            }
            else
            {
                _current = _current.next == null ? new InstanceData(_current) : _current.next;
            }
            _current.sliceType = sliceType;
        }

        private enum SliceType { ClassSlice, ExceptionSlice }

        private sealed class InstanceData
        {
            internal InstanceData(InstanceData? previous)
            {
                if (previous != null)
                {
                    previous.next = this;
                }
                this.previous = previous;
                this.next = null;
            }

            // Instance attributes
            internal SliceType sliceType;
            internal List<SliceInfo>? slices;     // Preserved slices.
            internal List<AnyClass[]?>? IndirectionTableList;

            // Position of indirection tables that we skipped for now and that will
            // unmarshal (into IndirectionTableList) once the instance is created
            internal List<int>? DeferredIndirectionTableList;

            // Slice attributes
            internal byte sliceFlags;
            internal int sliceSize;
            internal string? typeId;
            internal int compactId;

            // Indirection table of the current slice
            internal AnyClass[]? IndirectionTable;
            internal int? PosAfterIndirectionTable;

            // Other instances
            internal InstanceData? previous;
            internal InstanceData? next;
        }

        private readonly struct Encaps
        {
            // Previous upper limit of the buffer, if set
            internal readonly int? OldLimit;

            // Old Encoding
            internal readonly EncodingVersion OldEncoding;

            // Size of the encaps, as read from the stream
            internal readonly int Size;

            internal Encaps(int? oldLimit, EncodingVersion oldEncoding, int size)
            {
                OldLimit = oldLimit;
                OldEncoding = oldEncoding;
                Size = size;
            }
        }
    }
}
