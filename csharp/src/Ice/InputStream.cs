// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{

    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Runtime.Serialization;
    using System.Runtime.Serialization.Formatters.Binary;
    using Protocol = IceInternal.Protocol;

    /// <summary>
    /// After a call to readPendingValues(), delegates are called to supply the
    /// unmarshaled class instances.
    /// </summary>
    /// <param name="obj">The unmarshaled instance.</param>
    public delegate void ReadValueCallback(Value obj);

    /// <summary>
    /// A ClassResolver translates a Slice type Id into a type using
    /// an implementation-defined algorithm.
    /// </summary>
    /// <param name="id">A Slice type Id corresponding to a Slice value or user exception.</param>
    public delegate System.Type ClassResolver(string id);

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
    public class InputStream
    {

        /// <summary>
        /// Constructing an InputStream without providing a communicator means the stream will
        /// use the default encoding version. A communicator is required in order to unmarshal
        /// proxies. You can supply a communicator later by calling initialize().
        /// </summary>
        public InputStream()
        {
            initialize(Util.currentEncoding);
            _buf = new IceInternal.Buffer();
        }

        /// <summary>
        /// Constructing an InputStream without providing a communicator means the stream will
        /// use the default encoding version. A communicator is required in order to unmarshal
        /// proxies. You can supply a communicator later by calling initialize().
        /// </summary>
        /// <param name="data">The byte array containing encoded Slice types.</param>
        public InputStream(byte[] data)
        {
            initialize(Util.currentEncoding);
            _buf = new IceInternal.Buffer(data);
        }

        public InputStream(IceInternal.ByteBuffer buf)
        {
            initialize(Util.currentEncoding);
            _buf = new IceInternal.Buffer(buf);
        }

        public InputStream(IceInternal.Buffer buf) :
            this(buf, false)
        {
        }

        public InputStream(IceInternal.Buffer buf, bool adopt)
        {
            initialize(Util.currentEncoding);
            _buf = new IceInternal.Buffer(buf, adopt);
        }

        /// <summary>
        /// This constructor uses the communicator's default encoding version.
        /// </summary>
        /// <param name="communicator">The communicator to use when initializing the stream.</param>
        public InputStream(Communicator communicator)
        {
            initialize(communicator);
            _buf = new IceInternal.Buffer();
        }

        /// <summary>
        /// This constructor uses the communicator's default encoding version.
        /// </summary>
        /// <param name="communicator">The communicator to use when initializing the stream.</param>
        /// <param name="data">The byte array containing encoded Slice types.</param>
        public InputStream(Communicator communicator, byte[] data)
        {
            initialize(communicator);
            _buf = new IceInternal.Buffer(data);
        }

        public InputStream(Communicator communicator, IceInternal.ByteBuffer buf)
        {
            initialize(communicator);
            _buf = new IceInternal.Buffer(buf);
        }

        public InputStream(Communicator communicator, IceInternal.Buffer buf) :
            this(communicator, buf, false)
        {
        }

        public InputStream(Communicator communicator, IceInternal.Buffer buf, bool adopt)
        {
            initialize(communicator);
            _buf = new IceInternal.Buffer(buf, adopt);
        }

        /// <summary>
        /// This constructor uses the given encoding version.
        /// </summary>
        /// <param name="encoding">The desired encoding version.</param>
        public InputStream(EncodingVersion encoding)
        {
            initialize(encoding);
            _buf = new IceInternal.Buffer();
        }

        /// <summary>
        /// This constructor uses the given encoding version.
        /// </summary>
        /// <param name="encoding">The desired encoding version.</param>
        /// <param name="data">The byte array containing encoded Slice types.</param>
        public InputStream(EncodingVersion encoding, byte[] data)
        {
            initialize(encoding);
            _buf = new IceInternal.Buffer(data);
        }

        public InputStream(EncodingVersion encoding, IceInternal.ByteBuffer buf)
        {
            initialize(encoding);
            _buf = new IceInternal.Buffer(buf);
        }

        public InputStream(EncodingVersion encoding, IceInternal.Buffer buf) :
            this(encoding, buf, false)
        {
        }

        public InputStream(EncodingVersion encoding, IceInternal.Buffer buf, bool adopt)
        {
            initialize(encoding);
            _buf = new IceInternal.Buffer(buf, adopt);
        }

        /// <summary>
        /// This constructor uses the given encoding version.
        /// </summary>
        /// <param name="communicator">The communicator to use when initializing the stream.</param>
        /// <param name="encoding">The desired encoding version.</param>
        public InputStream(Communicator communicator, EncodingVersion encoding)
        {
            initialize(communicator, encoding);
            _buf = new IceInternal.Buffer();
        }

        /// <summary>
        /// This constructor uses the given encoding version.
        /// </summary>
        /// <param name="communicator">The communicator to use when initializing the stream.</param>
        /// <param name="encoding">The desired encoding version.</param>
        /// <param name="data">The byte array containing encoded Slice types.</param>
        public InputStream(Communicator communicator, EncodingVersion encoding, byte[] data)
        {
            initialize(communicator, encoding);
            _buf = new IceInternal.Buffer(data);
        }

        public InputStream(Communicator communicator, EncodingVersion encoding, IceInternal.ByteBuffer buf)
        {
            initialize(communicator, encoding);
            _buf = new IceInternal.Buffer(buf);
        }

        public InputStream(Communicator communicator, EncodingVersion encoding, IceInternal.Buffer buf) :
            this(communicator, encoding, buf, false)
        {
        }

        public InputStream(Communicator communicator, EncodingVersion encoding, IceInternal.Buffer buf, bool adopt)
        {
            initialize(communicator, encoding);
            _buf = new IceInternal.Buffer(buf, adopt);
        }

        public InputStream(IceInternal.Instance instance, EncodingVersion encoding)
        {
            initialize(instance, encoding);
            _buf = new IceInternal.Buffer();
        }

        public InputStream(IceInternal.Instance instance, EncodingVersion encoding, byte[] data)
        {
            initialize(instance, encoding);
            _buf = new IceInternal.Buffer(data);
        }

        public InputStream(IceInternal.Instance instance, EncodingVersion encoding, IceInternal.ByteBuffer buf)
        {
            initialize(instance, encoding);
            _buf = new IceInternal.Buffer(buf);
        }

        public InputStream(IceInternal.Instance instance, EncodingVersion encoding, IceInternal.Buffer buf) :
            this(instance, encoding, buf, false)
        {
        }

        public InputStream(IceInternal.Instance instance, EncodingVersion encoding, IceInternal.Buffer buf, bool adopt)
        {
            initialize(instance, encoding);
            _buf = new IceInternal.Buffer(buf, adopt);
        }

        /// <summary>
        /// Initializes the stream to use the communicator's default encoding version.
        /// </summary>
        /// <param name="communicator">The communicator to use when initializing the stream.</param>
        public void initialize(Communicator communicator)
        {
            Debug.Assert(communicator != null);
            IceInternal.Instance instance = IceInternal.Util.getInstance(communicator);
            initialize(instance, instance.defaultsAndOverrides().defaultEncoding);
        }

        /// <summary>
        /// Initializes the stream to use the given communicator and encoding version.
        /// </summary>
        /// <param name="communicator">The communicator to use when initializing the stream.</param>
        /// <param name="encoding">The desired encoding version.</param>
        public void initialize(Communicator communicator, EncodingVersion encoding)
        {
            Debug.Assert(communicator != null);
            IceInternal.Instance instance = IceInternal.Util.getInstance(communicator);
            initialize(instance, encoding);
        }

        private void initialize(IceInternal.Instance instance, EncodingVersion encoding)
        {
            initialize(encoding);

            _instance = instance;
            _traceSlicing = _instance.traceLevels().slicing > 0;

            _valueFactoryManager = _instance.initializationData().valueFactoryManager;
            _logger = _instance.initializationData().logger;
            _classResolver = _instance.resolveClass;
        }

        private void initialize(EncodingVersion encoding)
        {
            _instance = null;
            _encoding = encoding;
            _encapsStack = null;
            _encapsCache = null;
            _traceSlicing = false;
            _closure = null;
            _sliceValues = true;
            _startSeq = -1;
            _minSeqSize = 0;
        }

        /// <summary>
        /// Resets this stream. This method allows the stream to be reused, to avoid creating
        /// unnecessary garbage.
        /// </summary>
        public void reset()
        {
            _buf.reset();
            clear();
        }

        /// <summary>
        /// Releases any data retained by encapsulations. Internally calls clear().
        /// </summary>
        public void clear()
        {
            if(_encapsStack != null)
            {
                Debug.Assert(_encapsStack.next == null);
                _encapsStack.next = _encapsCache;
                _encapsCache = _encapsStack;
                _encapsStack = null;
                _encapsCache.reset();
            }

            _startSeq = -1;
            _sliceValues = true;
        }

        /// <summary>
        /// Sets the value factory manager to use when marshaling value instances. If the stream
        /// was initialized with a communicator, the communicator's value factory manager will
        /// be used by default.
        /// </summary>
        /// <param name="vfm">The value factory manager.</param>
        public void setValueFactoryManager(ValueFactoryManager vfm)
        {
            _valueFactoryManager = vfm;
        }

        /// <summary>
        /// Sets the logger to use when logging trace messages. If the stream
        /// was initialized with a communicator, the communicator's logger will
        /// be used by default.
        /// </summary>
        /// <param name="logger">The logger to use for logging trace messages.</param>
        public void setLogger(Logger logger)
        {
            _logger = logger;
        }

        /// <summary>
        /// Sets the compact ID resolver to use when unmarshaling value and exception
        /// instances. If the stream was initialized with a communicator, the communicator's
        /// resolver will be used by default.
        /// </summary>
        /// <param name="r">The compact ID resolver.</param>
        public void setCompactIdResolver(CompactIdResolver r)
        {
            _compactIdResolver = r;
        }

        /// <summary>
        /// Sets the class resolver, which the stream will use when attempting to unmarshal
        /// a value or exception. If the stream was initialized with a communicator, the communicator's
        /// resolver will be used by default.
        /// </summary>
        /// <param name="r">The class resolver.</param>
        public void setClassResolver(ClassResolver r)
        {
            _classResolver = r;
        }

        /// <summary>
        /// Determines the behavior of the stream when extracting instances of Slice classes.
        /// An instance is "sliced" when a factory cannot be found for a Slice type ID.
        /// The stream's default behavior is to slice instances.
        /// </summary>
        /// <param name="b">If true (the default), slicing is enabled; if false,
        /// slicing is disabled. If slicing is disabled and the stream encounters a Slice type ID
        /// during decoding for which no value factory is installed, it raises NoValueFactoryException.
        /// </param>
        public void setSliceValues(bool b)
        {
            _sliceValues = b;
        }

        /// <summary>
        /// Determines whether the stream logs messages about slicing instances of Slice values.
        /// </summary>
        /// <param name="b">True to enable logging, false to disable logging.</param>
        public void setTraceSlicing(bool b)
        {
            _traceSlicing = b;
        }

        /// <summary>
        /// Retrieves the closure object associated with this stream.
        /// </summary>
        /// <returns>The closure object.</returns>
        public object getClosure()
        {
            return _closure;
        }

        /// <summary>
        /// Associates a closure object with this stream.
        /// </summary>
        /// <param name="p">The new closure object.</param>
        /// <returns>The previous closure object, or null.</returns>
        public object setClosure(object p)
        {
            object prev = _closure;
            _closure = p;
            return prev;
        }

        public IceInternal.Instance instance()
        {
            return _instance;
        }

        /// <summary>
        /// Swaps the contents of one stream with another.
        /// </summary>
        /// <param name="other">The other stream.</param>
        public void swap(InputStream other)
        {
            Debug.Assert(_instance == other._instance);

            IceInternal.Buffer tmpBuf = other._buf;
            other._buf = _buf;
            _buf = tmpBuf;

            EncodingVersion tmpEncoding = other._encoding;
            other._encoding = _encoding;
            _encoding = tmpEncoding;

            bool tmpTraceSlicing = other._traceSlicing;
            other._traceSlicing = _traceSlicing;
            _traceSlicing = tmpTraceSlicing;

            object tmpClosure = other._closure;
            other._closure = _closure;
            _closure = tmpClosure;

            bool tmpSliceValues = other._sliceValues;
            other._sliceValues = _sliceValues;
            _sliceValues = tmpSliceValues;

            //
            // Swap is never called for InputStreams that have encapsulations being read. However,
            // encapsulations might still be set in case un-marshalling failed. We just
            // reset the encapsulations if there are still some set.
            //
            resetEncapsulation();
            other.resetEncapsulation();

            int tmpStartSeq = other._startSeq;
            other._startSeq = _startSeq;
            _startSeq = tmpStartSeq;

            int tmpMinSeqSize = other._minSeqSize;
            other._minSeqSize = _minSeqSize;
            _minSeqSize = tmpMinSeqSize;

            ValueFactoryManager tmpVfm = other._valueFactoryManager;
            other._valueFactoryManager = _valueFactoryManager;
            _valueFactoryManager = tmpVfm;

            Logger tmpLogger = other._logger;
            other._logger = _logger;
            _logger = tmpLogger;

            CompactIdResolver tmpCompactIdResolver = other._compactIdResolver;
            other._compactIdResolver = _compactIdResolver;
            _compactIdResolver = tmpCompactIdResolver;

            ClassResolver tmpClassResolver = other._classResolver;
            other._classResolver = _classResolver;
            _classResolver = tmpClassResolver;
        }

        private void resetEncapsulation()
        {
            _encapsStack = null;
        }

        /// <summary>
        /// Resizes the stream to a new size.
        /// </summary>
        /// <param name="sz">The new size.</param>
        public void resize(int sz)
        {
            _buf.resize(sz, true);
            _buf.b.position(sz);
        }

        public IceInternal.Buffer getBuffer()
        {
            return _buf;
        }

        /// <summary>
        /// Marks the start of a class instance.
        /// </summary>
        public void startValue()
        {
            Debug.Assert(_encapsStack != null && _encapsStack.decoder != null);
            _encapsStack.decoder.startInstance(SliceType.ValueSlice);
        }

        /// <summary>
        /// Marks the end of a class instance.
        /// </summary>
        /// <param name="preserve">True if unknown slices should be preserved, false otherwise.</param>
        /// <returns>A SlicedData object containing the preserved slices for unknown types.</returns>
        public SlicedData endValue(bool preserve)
        {
            Debug.Assert(_encapsStack != null && _encapsStack.decoder != null);
            return _encapsStack.decoder.endInstance(preserve);
        }

        /// <summary>
        /// Marks the start of a user exception.
        /// </summary>
        public void startException()
        {
            Debug.Assert(_encapsStack != null && _encapsStack.decoder != null);
            _encapsStack.decoder.startInstance(SliceType.ExceptionSlice);
        }

        /// <summary>
        /// Marks the end of a user exception.
        /// </summary>
        /// <param name="preserve">True if unknown slices should be preserved, false otherwise.</param>
        /// <returns>A SlicedData object containing the preserved slices for unknown types.</returns>
        public SlicedData endException(bool preserve)
        {
            Debug.Assert(_encapsStack != null && _encapsStack.decoder != null);
            return _encapsStack.decoder.endInstance(preserve);
        }

        /// <summary>
        /// Reads the start of an encapsulation.
        /// </summary>
        /// <returns>The encapsulation encoding version.</returns>
        public EncodingVersion startEncapsulation()
        {
            Encaps curr = _encapsCache;
            if(curr != null)
            {
                curr.reset();
                _encapsCache = _encapsCache.next;
            }
            else
            {
                curr = new Encaps();
            }
            curr.next = _encapsStack;
            _encapsStack = curr;

            _encapsStack.start = _buf.b.position();

            //
            // I don't use readSize() for encapsulations, because when creating an encapsulation,
            // I must know in advance how many bytes the size information will require in the data
            // stream. If I use an Int, it is always 4 bytes. For readSize(), it could be 1 or 5 bytes.
            //
            int sz = readInt();
            if(sz < 6)
            {
                throw new UnmarshalOutOfBoundsException();
            }
            if(sz - 4 > _buf.b.remaining())
            {
                throw new UnmarshalOutOfBoundsException();
            }
            _encapsStack.sz = sz;

            EncodingVersion encoding = new EncodingVersion();
            encoding.ice_readMembers(this);
            Protocol.checkSupportedEncoding(encoding); // Make sure the encoding is supported.
            _encapsStack.setEncoding(encoding);

            return encoding;
        }

        /// <summary>
        /// Ends the previous encapsulation.
        /// </summary>
        public void endEncapsulation()
        {
            Debug.Assert(_encapsStack != null);

            if(!_encapsStack.encoding_1_0)
            {
                skipOptionals();
                if(_buf.b.position() != _encapsStack.start + _encapsStack.sz)
                {
                    throw new EncapsulationException();
                }
            }
            else if(_buf.b.position() != _encapsStack.start + _encapsStack.sz)
            {
                if(_buf.b.position() + 1 != _encapsStack.start + _encapsStack.sz)
                {
                    throw new EncapsulationException();
                }

                //
                // Ice version < 3.3 had a bug where user exceptions with
                // class members could be encoded with a trailing byte
                // when dispatched with AMD. So we tolerate an extra byte
                // in the encapsulation.
                //
                try
                {
                    _buf.b.get();
                }
                catch(InvalidOperationException ex)
                {
                    throw new UnmarshalOutOfBoundsException(ex);
                }
            }

            Encaps curr = _encapsStack;
            _encapsStack = curr.next;
            curr.next = _encapsCache;
            _encapsCache = curr;
            _encapsCache.reset();
        }

        /// <summary>
        /// Skips an empty encapsulation.
        /// </summary>
        /// <returns>The encapsulation's encoding version.</returns>
        public EncodingVersion skipEmptyEncapsulation()
        {
            int sz = readInt();
            if(sz < 6)
            {
                throw new EncapsulationException();
            }
            if(sz - 4 > _buf.b.remaining())
            {
                throw new UnmarshalOutOfBoundsException();
            }

            var encoding = new EncodingVersion();
            encoding.ice_readMembers(this);
            Protocol.checkSupportedEncoding(encoding); // Make sure the encoding is supported.

            if(encoding.Equals(Util.Encoding_1_0))
            {
                if(sz != 6)
                {
                    throw new EncapsulationException();
                }
            }
            else
            {
                // Skip the optional content of the encapsulation if we are expecting an
                // empty encapsulation.
                _buf.b.position(_buf.b.position() + sz - 6);
            }
            return encoding;
        }

        /// <summary>
        /// Returns a blob of bytes representing an encapsulation. The encapsulation's encoding version
        /// is returned in the argument.
        /// </summary>
        /// <param name="encoding">The encapsulation's encoding version.</param>
        /// <returns>The encoded encapsulation.</returns>
        public byte[] readEncapsulation(out EncodingVersion encoding)
        {
            int sz = readInt();
            if(sz < 6)
            {
                throw new UnmarshalOutOfBoundsException();
            }

            if(sz - 4 > _buf.b.remaining())
            {
                throw new UnmarshalOutOfBoundsException();
            }

            encoding = new EncodingVersion();
            encoding.ice_readMembers(this);
            _buf.b.position(_buf.b.position() - 6);

            byte[] v = new byte[sz];
            try
            {
                _buf.b.get(v);
                return v;
            }
            catch(InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Determines the current encoding version.
        /// </summary>
        /// <returns>The encoding version.</returns>
        public EncodingVersion getEncoding()
        {
            return _encapsStack != null ? _encapsStack.encoding : _encoding;
        }

        /// <summary>
        /// Determines the size of the current encapsulation, excluding the encapsulation header.
        /// </summary>
        /// <returns>The size of the encapsulated data.</returns>
        public int getEncapsulationSize()
        {
            Debug.Assert(_encapsStack != null);
            return _encapsStack.sz - 6;
        }

        /// <summary>
        /// Skips over an encapsulation.
        /// </summary>
        /// <returns>The encoding version of the skipped encapsulation.</returns>
        public EncodingVersion skipEncapsulation()
        {
            int sz = readInt();
            if(sz < 6)
            {
                throw new UnmarshalOutOfBoundsException();
            }
            EncodingVersion encoding = new EncodingVersion();
            encoding.ice_readMembers(this);
            try
            {
                _buf.b.position(_buf.b.position() + sz - 6);
            }
            catch(ArgumentOutOfRangeException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
            return encoding;
        }

        /// <summary>
        /// Reads the start of a class instance or exception slice.
        /// </summary>
        /// <returns>The Slice type ID for this slice.</returns>
        public string startSlice() // Returns type ID of next slice
        {
            Debug.Assert(_encapsStack != null && _encapsStack.decoder != null);
            return _encapsStack.decoder.startSlice();
        }

        /// <summary>
        /// Indicates that the end of a class instance or exception slice has been reached.
        /// </summary>
        public void endSlice()
        {
            Debug.Assert(_encapsStack != null && _encapsStack.decoder != null);
            _encapsStack.decoder.endSlice();
        }

        /// <summary>
        /// Skips over a class instance or exception slice.
        /// </summary>
        public void skipSlice()
        {
            Debug.Assert(_encapsStack != null && _encapsStack.decoder != null);
            _encapsStack.decoder.skipSlice();
        }

        /// <summary>
        /// Indicates that unmarshaling is complete, except for any class instances. The application must call this
        /// method only if the stream actually contains class instances. Calling readPendingValues triggers the
        /// calls to the ReadValueCallback delegates to inform the application that unmarshaling of an instance
        /// is complete.
        /// </summary>
        public void readPendingValues()
        {
            if(_encapsStack != null && _encapsStack.decoder != null)
            {
                _encapsStack.decoder.readPendingValues();
            }
            else if(_encapsStack != null ? _encapsStack.encoding_1_0 : _encoding.Equals(Util.Encoding_1_0))
            {
                //
                // If using the 1.0 encoding and no instances were read, we
                // still read an empty sequence of pending instances if
                // requested (i.e.: if this is called).
                //
                // This is required by the 1.0 encoding, even if no instances
                // are written we do marshal an empty sequence if marshaled
                // data types use classes.
                //
                skipSize();
            }
        }

        /// <summary>
        /// Extracts a size from the stream.
        /// </summary>
        /// <returns>The extracted size.</returns>
        public int readSize()
        {
            try
            {
                byte b = _buf.b.get();
                if(b == 255)
                {
                    int v = _buf.b.getInt();
                    if(v < 0)
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
            catch(InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Reads and validates a sequence size.
        /// </summary>
        /// <returns>The extracted size.</returns>
        public int readAndCheckSeqSize(int minSize)
        {
            int sz = readSize();

            if(sz == 0)
            {
                return 0;
            }

            //
            // The _startSeq variable points to the start of the sequence for which
            // we expect to read at least _minSeqSize bytes from the stream.
            //
            // If not initialized or if we already read more data than _minSeqSize,
            // we reset _startSeq and _minSeqSize for this sequence (possibly a
            // top-level sequence or enclosed sequence it doesn't really matter).
            //
            // Otherwise, we are reading an enclosed sequence and we have to bump
            // _minSeqSize by the minimum size that this sequence will  require on
            // the stream.
            //
            // The goal of this check is to ensure that when we start un-marshalling
            // a new sequence, we check the minimal size of this new sequence against
            // the estimated remaining buffer size. This estimatation is based on
            // the minimum size of the enclosing sequences, it's _minSeqSize.
            //
            if(_startSeq == -1 || _buf.b.position() > (_startSeq + _minSeqSize))
            {
                _startSeq = _buf.b.position();
                _minSeqSize = sz * minSize;
            }
            else
            {
                _minSeqSize += sz * minSize;
            }

            //
            // If there isn't enough data to read on the stream for the sequence (and
            // possibly enclosed sequences), something is wrong with the marshalled
            // data: it's claiming having more data that what is possible to read.
            //
            if(_startSeq + _minSeqSize > _buf.size())
            {
                throw new UnmarshalOutOfBoundsException();
            }

            return sz;
        }

        /// <summary>
        /// Reads a blob of bytes from the stream. The length of the given array determines how many bytes are read.
        /// </summary>
        /// <param name="v">Bytes from the stream.</param>
        public void readBlob(byte[] v)
        {
            try
            {
                _buf.b.get(v);
            }
            catch(InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Reads a blob of bytes from the stream.
        /// </summary>
        /// <param name="sz">The number of bytes to read.</param>
        /// <returns>The requested bytes as a byte array.</returns>
        public byte[] readBlob(int sz)
        {
            if(_buf.b.remaining() < sz)
            {
                throw new UnmarshalOutOfBoundsException();
            }
            byte[] v = new byte[sz];
            try
            {
                _buf.b.get(v);
                return v;
            }
            catch(InvalidOperationException ex)
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
        public bool readOptional(int tag, OptionalFormat expectedFormat)
        {
            Debug.Assert(_encapsStack != null);
            if(_encapsStack.decoder != null)
            {
                return _encapsStack.decoder.readOptional(tag, expectedFormat);
            }
            else
            {
                return readOptImpl(tag, expectedFormat);
            }
        }

        /// <summary>
        /// Extracts a byte value from the stream.
        /// </summary>
        /// <returns>The extracted byte.</returns>
        public byte readByte()
        {
            try
            {
                return _buf.b.get();
            }
            catch(InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts an optional byte value from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public Optional<byte> readByte(int tag)
        {
            if(readOptional(tag, OptionalFormat.F1))
            {
                return new Optional<byte>(readByte());
            }
            else
            {
                return new Optional<byte>();
            }
        }

        /// <summary>
        /// Extracts an optional byte value from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <param name="isset">True if the optional value is present, false otherwise.</param>
        /// <param name="v">The optional value.</param>
        public void readByte(int tag, out bool isset, out byte v)
        {
            if(isset = readOptional(tag, OptionalFormat.F1))
            {
                v = readByte();
            }
            else
            {
                v = 0;
            }
        }

        /// <summary>
        /// Extracts a sequence of byte values from the stream.
        /// </summary>
        /// <returns>The extracted byte sequence.</returns>
        public byte[] readByteSeq()
        {
            try
            {
                int sz = readAndCheckSeqSize(1);
                byte[] v = new byte[sz];
                _buf.b.get(v);
                return v;
            }
            catch(InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of byte values from the stream.
        /// </summary>
        /// <param name="l">The extracted byte sequence as a list.</param>
        public void readByteSeq(out List<byte> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new List<byte>(readByteSeq());
        }

        /// <summary>
        /// Extracts a sequence of byte values from the stream.
        /// </summary>
        /// <param name="l">The extracted byte sequence as a linked list.</param>
        public void readByteSeq(out LinkedList<byte> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new LinkedList<byte>(readByteSeq());
        }

        /// <summary>
        /// Extracts a sequence of byte values from the stream.
        /// </summary>
        /// <param name="l">The extracted byte sequence as a queue.</param>
        public void readByteSeq(out Queue<byte> l)
        {
            //
            // Reading into an array and copy-constructing the
            // queue is faster than constructing the queue
            // and adding to it one element at a time.
            //
            l = new Queue<byte>(readByteSeq());
        }

        /// <summary>
        /// Extracts a sequence of byte values from the stream.
        /// </summary>
        /// <param name="l">The extracted byte sequence as a stack.</param>
        public void readByteSeq(out Stack<byte> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            byte[] array = readByteSeq();
            Array.Reverse(array);
            l = new Stack<byte>(array);
        }

        /// <summary>
        /// Extracts an optional byte sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public Optional<byte[]> readByteSeq(int tag)
        {
            if(readOptional(tag, OptionalFormat.VSize))
            {
                return new Optional<byte[]>(readByteSeq());
            }
            else
            {
                return new Optional<byte[]>();
            }
        }

        /// <summary>
        /// Extracts an optional byte sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <param name="isset">True if the optional value is present, false otherwise.</param>
        /// <param name="v">The optional value.</param>
        public void readByteSeq(int tag, out bool isset, out byte[] v)
        {
            if(isset = readOptional(tag, OptionalFormat.VSize))
            {
                v = readByteSeq();
            }
            else
            {
                v = null;
            }
        }

        /// <summary>
        /// Extracts a serializable object from the stream.
        /// </summary>
        /// <returns>The serializable object.</returns>
        public object readSerializable()
        {
            int sz = readAndCheckSeqSize(1);
            if(sz == 0)
            {
                return null;
            }
            try
            {
                var f = new BinaryFormatter(null, new StreamingContext(StreamingContextStates.All, _instance));
                return f.Deserialize(new IceInternal.InputStreamWrapper(sz, this));
            }
            catch(System.Exception ex)
            {
                throw new MarshalException("cannot deserialize object:", ex);
            }
        }

        /// <summary>
        /// Extracts a boolean value from the stream.
        /// </summary>
        /// <returns>The extracted boolean.</returns>
        public bool readBool()
        {
            try
            {
                return _buf.b.get() == 1;
            }
            catch(InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts an optional boolean value from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public Optional<bool> readBool(int tag)
        {
            if(readOptional(tag, OptionalFormat.F1))
            {
                return new Optional<bool>(readBool());
            }
            else
            {
                return new Optional<bool>();
            }
        }

        /// <summary>
        /// Extracts an optional boolean value from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <param name="isset">True if the optional value is present, false otherwise.</param>
        /// <param name="v">The optional value.</param>
        public void readBool(int tag, out bool isset, out bool v)
        {
            if(isset = readOptional(tag, OptionalFormat.F1))
            {
                v = readBool();
            }
            else
            {
                v = false;
            }
        }

        /// <summary>
        /// Extracts a sequence of boolean values from the stream.
        /// </summary>
        /// <returns>The extracted boolean sequence.</returns>
        public bool[] readBoolSeq()
        {
            try
            {
                int sz = readAndCheckSeqSize(1);
                bool[] v = new bool[sz];
                _buf.b.getBoolSeq(v);
                return v;
            }
            catch(InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of boolean values from the stream.
        /// </summary>
        /// <param name="l">The extracted boolean sequence as a list.</param>
        public void readBoolSeq(out List<bool> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new List<bool>(readBoolSeq());
        }

        /// <summary>
        /// Extracts a sequence of boolean values from the stream.
        /// </summary>
        /// <param name="l">The extracted boolean sequence as a linked list.</param>
        public void readBoolSeq(out LinkedList<bool> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new LinkedList<bool>(readBoolSeq());
        }

        /// <summary>
        /// Extracts a sequence of boolean values from the stream.
        /// </summary>
        /// <param name="l">The extracted boolean sequence as a queue.</param>
        public void readBoolSeq(out Queue<bool> l)
        {
            //
            // Reading into an array and copy-constructing the
            // queue is faster than constructing the queue
            // and adding to it one element at a time.
            //
            l = new Queue<bool>(readBoolSeq());
        }

        /// <summary>
        /// Extracts a sequence of boolean values from the stream.
        /// </summary>
        /// <param name="l">The extracted boolean sequence as a stack.</param>
        public void readBoolSeq(out Stack<bool> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            bool[] array = readBoolSeq();
            Array.Reverse(array);
            l = new Stack<bool>(array);
        }

        /// <summary>
        /// Extracts an optional boolean sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public Optional<bool[]> readBoolSeq(int tag)
        {
            if(readOptional(tag, OptionalFormat.VSize))
            {
                return new Optional<bool[]>(readBoolSeq());
            }
            else
            {
                return new Optional<bool[]>();
            }
        }

        /// <summary>
        /// Extracts an optional boolean sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <param name="isset">True if the optional value is present, false otherwise.</param>
        /// <param name="v">The optional value.</param>
        public void readBoolSeq(int tag, out bool isset, out bool[] v)
        {
            if(isset = readOptional(tag, OptionalFormat.VSize))
            {
                v = readBoolSeq();
            }
            else
            {
                v = null;
            }
        }

        /// <summary>
        /// Extracts a short value from the stream.
        /// </summary>
        /// <returns>The extracted short.</returns>
        public short readShort()
        {
            try
            {
                return _buf.b.getShort();
            }
            catch(InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts an optional short value from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public Optional<short> readShort(int tag)
        {
            if(readOptional(tag, OptionalFormat.F2))
            {
                return new Optional<short>(readShort());
            }
            else
            {
                return new Optional<short>();
            }
        }

        /// <summary>
        /// Extracts an optional short value from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <param name="isset">True if the optional value is present, false otherwise.</param>
        /// <param name="v">The optional value.</param>
        public void readShort(int tag, out bool isset, out short v)
        {
            if(isset = readOptional(tag, OptionalFormat.F2))
            {
                v = readShort();
            }
            else
            {
                v = 0;
            }
        }

        /// <summary>
        /// Extracts a sequence of short values from the stream.
        /// </summary>
        /// <returns>The extracted short sequence.</returns>
        public short[] readShortSeq()
        {
            try
            {
                int sz = readAndCheckSeqSize(2);
                short[] v = new short[sz];
                _buf.b.getShortSeq(v);
                return v;
            }
            catch(InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of short values from the stream.
        /// </summary>
        /// <param name="l">The extracted short sequence as a list.</param>
        public void readShortSeq(out List<short> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new List<short>(readShortSeq());
        }

        /// <summary>
        /// Extracts a sequence of short values from the stream.
        /// </summary>
        /// <param name="l">The extracted short sequence as a linked list.</param>
        public void readShortSeq(out LinkedList<short> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new LinkedList<short>(readShortSeq());
        }

        /// <summary>
        /// Extracts a sequence of short values from the stream.
        /// </summary>
        /// <param name="l">The extracted short sequence as a queue.</param>
        public void readShortSeq(out Queue<short> l)
        {
            //
            // Reading into an array and copy-constructing the
            // queue is faster than constructing the queue
            // and adding to it one element at a time.
            //
            l = new Queue<short>(readShortSeq());
        }

        /// <summary>
        /// Extracts a sequence of short values from the stream.
        /// </summary>
        /// <param name="l">The extracted short sequence as a stack.</param>
        public void readShortSeq(out Stack<short> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            short[] array = readShortSeq();
            Array.Reverse(array);
            l = new Stack<short>(array);
        }

        /// <summary>
        /// Extracts an optional short sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public Optional<short[]> readShortSeq(int tag)
        {
            if(readOptional(tag, OptionalFormat.VSize))
            {
                skipSize();
                return new Optional<short[]>(readShortSeq());
            }
            else
            {
                return new Optional<short[]>();
            }
        }

        /// <summary>
        /// Extracts an optional short sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <param name="isset">True if the optional value is present, false otherwise.</param>
        /// <param name="v">The optional value.</param>
        public void readShortSeq(int tag, out bool isset, out short[] v)
        {
            if(isset = readOptional(tag, OptionalFormat.VSize))
            {
                skipSize();
                v = readShortSeq();
            }
            else
            {
                v = null;
            }
        }

        /// <summary>
        /// Extracts an int value from the stream.
        /// </summary>
        /// <returns>The extracted int.</returns>
        public int readInt()
        {
            try
            {
                return _buf.b.getInt();
            }
            catch(InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts an optional int value from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public Optional<int> readInt(int tag)
        {
            if(readOptional(tag, OptionalFormat.F4))
            {
                return new Optional<int>(readInt());
            }
            else
            {
                return new Optional<int>();
            }
        }

        /// <summary>
        /// Extracts an optional int value from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <param name="isset">True if the optional value is present, false otherwise.</param>
        /// <param name="v">The optional value.</param>
        public void readInt(int tag, out bool isset, out int v)
        {
            if(isset = readOptional(tag, OptionalFormat.F4))
            {
                v = readInt();
            }
            else
            {
                v = 0;
            }
        }

        /// <summary>
        /// Extracts a sequence of int values from the stream.
        /// </summary>
        /// <returns>The extracted int sequence.</returns>
        public int[] readIntSeq()
        {
            try
            {
                int sz = readAndCheckSeqSize(4);
                int[] v = new int[sz];
                _buf.b.getIntSeq(v);
                return v;
            }
            catch(InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of int values from the stream.
        /// </summary>
        /// <param name="l">The extracted int sequence as a list.</param>
        public void readIntSeq(out List<int> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new List<int>(readIntSeq());
        }

        /// <summary>
        /// Extracts a sequence of int values from the stream.
        /// </summary>
        /// <param name="l">The extracted int sequence as a linked list.</param>
        public void readIntSeq(out LinkedList<int> l)
        {
            try
            {
                int sz = readAndCheckSeqSize(4);
                l = new LinkedList<int>();
                for(int i = 0; i < sz; ++i)
                {
                    l.AddLast(_buf.b.getInt());
                }
            }
            catch(InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of int values from the stream.
        /// </summary>
        /// <param name="l">The extracted int sequence as a queue.</param>
        public void readIntSeq(out Queue<int> l)
        {
            //
            // Reading into an array and copy-constructing the
            // queue takes the same time as constructing the queue
            // and adding to it one element at a time, so
            // we avoid the copy.
            //
            try
            {
                int sz = readAndCheckSeqSize(4);
                l = new Queue<int>(sz);
                for(int i = 0; i < sz; ++i)
                {
                    l.Enqueue(_buf.b.getInt());
                }
            }
            catch(InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of int values from the stream.
        /// </summary>
        /// <param name="l">The extracted int sequence as a stack.</param>
        public void readIntSeq(out Stack<int> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            int[] array = readIntSeq();
            Array.Reverse(array);
            l = new Stack<int>(array);
        }

        /// <summary>
        /// Extracts an optional int sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public Optional<int[]> readIntSeq(int tag)
        {
            if(readOptional(tag, OptionalFormat.VSize))
            {
                skipSize();
                return new Optional<int[]>(readIntSeq());
            }
            else
            {
                return new Optional<int[]>();
            }
        }

        /// <summary>
        /// Extracts an optional int sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <param name="isset">True if the optional value is present, false otherwise.</param>
        /// <param name="v">The optional value.</param>
        public void readIntSeq(int tag, out bool isset, out int[] v)
        {
            if(isset = readOptional(tag, OptionalFormat.VSize))
            {
                skipSize();
                v = readIntSeq();
            }
            else
            {
                v = null;
            }
        }

        /// <summary>
        /// Extracts a long value from the stream.
        /// </summary>
        /// <returns>The extracted long.</returns>
        public long readLong()
        {
            try
            {
                return _buf.b.getLong();
            }
            catch(InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts an optional long value from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public Optional<long> readLong(int tag)
        {
            if(readOptional(tag, OptionalFormat.F8))
            {
                return new Optional<long>(readLong());
            }
            else
            {
                return new Optional<long>();
            }
        }

        /// <summary>
        /// Extracts an optional long value from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <param name="isset">True if the optional value is present, false otherwise.</param>
        /// <param name="v">The optional value.</param>
        public void readLong(int tag, out bool isset, out long v)
        {
            if(isset = readOptional(tag, OptionalFormat.F8))
            {
                v = readLong();
            }
            else
            {
                v = 0;
            }
        }

        /// <summary>
        /// Extracts a sequence of long values from the stream.
        /// </summary>
        /// <returns>The extracted long sequence.</returns>
        public long[] readLongSeq()
        {
            try
            {
                int sz = readAndCheckSeqSize(8);
                long[] v = new long[sz];
                _buf.b.getLongSeq(v);
                return v;
            }
            catch(InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of long values from the stream.
        /// </summary>
        /// <param name="l">The extracted long sequence as a list.</param>
        public void readLongSeq(out List<long> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new List<long>(readLongSeq());
        }

        /// <summary>
        /// Extracts a sequence of long values from the stream.
        /// </summary>
        /// <param name="l">The extracted long sequence as a linked list.</param>
        public void readLongSeq(out LinkedList<long> l)
        {
            try
            {
                int sz = readAndCheckSeqSize(4);
                l = new LinkedList<long>();
                for(int i = 0; i < sz; ++i)
                {
                    l.AddLast(_buf.b.getLong());
                }
            }
            catch(InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of long values from the stream.
        /// </summary>
        /// <param name="l">The extracted long sequence as a queue.</param>
        public void readLongSeq(out Queue<long> l)
        {
            //
            // Reading into an array and copy-constructing the
            // queue takes the same time as constructing the queue
            // and adding to it one element at a time, so
            // we avoid the copy.
            //
            try
            {
                int sz = readAndCheckSeqSize(4);
                l = new Queue<long>(sz);
                for(int i = 0; i < sz; ++i)
                {
                    l.Enqueue(_buf.b.getLong());
                }
            }
            catch(InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of long values from the stream.
        /// </summary>
        /// <param name="l">The extracted long sequence as a stack.</param>
        public void readLongSeq(out Stack<long> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            long[] array = readLongSeq();
            Array.Reverse(array);
            l = new Stack<long>(array);
        }

        /// <summary>
        /// Extracts an optional long sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public Optional<long[]> readLongSeq(int tag)
        {
            if(readOptional(tag, OptionalFormat.VSize))
            {
                skipSize();
                return new Optional<long[]>(readLongSeq());
            }
            else
            {
                return new Optional<long[]>();
            }
        }

        /// <summary>
        /// Extracts an optional long sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <param name="isset">True if the optional value is present, false otherwise.</param>
        /// <param name="v">The optional value.</param>
        public void readLongSeq(int tag, out bool isset, out long[] v)
        {
            if(isset = readOptional(tag, OptionalFormat.VSize))
            {
                skipSize();
                v = readLongSeq();
            }
            else
            {
                v = null;
            }
        }

        /// <summary>
        /// Extracts a float value from the stream.
        /// </summary>
        /// <returns>The extracted float.</returns>
        public float readFloat()
        {
            try
            {
                return _buf.b.getFloat();
            }
            catch(InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts an optional float value from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public Optional<float> readFloat(int tag)
        {
            if(readOptional(tag, OptionalFormat.F4))
            {
                return new Optional<float>(readFloat());
            }
            else
            {
                return new Optional<float>();
            }
        }

        /// <summary>
        /// Extracts an optional float value from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <param name="isset">True if the optional value is present, false otherwise.</param>
        /// <param name="v">The optional value.</param>
        public void readFloat(int tag, out bool isset, out float v)
        {
            if(isset = readOptional(tag, OptionalFormat.F4))
            {
                v = readFloat();
            }
            else
            {
                v = 0;
            }
        }

        /// <summary>
        /// Extracts a sequence of float values from the stream.
        /// </summary>
        /// <returns>The extracted float sequence.</returns>
        public float[] readFloatSeq()
        {
            try
            {
                int sz = readAndCheckSeqSize(4);
                float[] v = new float[sz];
                _buf.b.getFloatSeq(v);
                return v;
            }
            catch(InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of float values from the stream.
        /// </summary>
        /// <param name="l">The extracted float sequence as a list.</param>
        public void readFloatSeq(out List<float> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new List<float>(readFloatSeq());
        }

        /// <summary>
        /// Extracts a sequence of float values from the stream.
        /// </summary>
        /// <param name="l">The extracted float sequence as a linked list.</param>
        public void readFloatSeq(out LinkedList<float> l)
        {
            try
            {
                int sz = readAndCheckSeqSize(4);
                l = new LinkedList<float>();
                for(int i = 0; i < sz; ++i)
                {
                    l.AddLast(_buf.b.getFloat());
                }
            }
            catch(InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of float values from the stream.
        /// </summary>
        /// <param name="l">The extracted float sequence as a queue.</param>
        public void readFloatSeq(out Queue<float> l)
        {
            //
            // Reading into an array and copy-constructing the
            // queue takes the same time as constructing the queue
            // and adding to it one element at a time, so
            // we avoid the copy.
            //
            try
            {
                int sz = readAndCheckSeqSize(4);
                l = new Queue<float>(sz);
                for(int i = 0; i < sz; ++i)
                {
                    l.Enqueue(_buf.b.getFloat());
                }
            }
            catch(InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of float values from the stream.
        /// </summary>
        /// <param name="l">The extracted float sequence as a stack.</param>
        public void readFloatSeq(out Stack<float> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            float[] array = readFloatSeq();
            Array.Reverse(array);
            l = new Stack<float>(array);
        }

        /// <summary>
        /// Extracts an optional float sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public Optional<float[]> readFloatSeq(int tag)
        {
            if(readOptional(tag, OptionalFormat.VSize))
            {
                skipSize();
                return new Optional<float[]>(readFloatSeq());
            }
            else
            {
                return new Optional<float[]>();
            }
        }

        /// <summary>
        /// Extracts an optional float sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <param name="isset">True if the optional value is present, false otherwise.</param>
        /// <param name="v">The optional value.</param>
        public void readFloatSeq(int tag, out bool isset, out float[] v)
        {
            if(isset = readOptional(tag, OptionalFormat.VSize))
            {
                skipSize();
                v = readFloatSeq();
            }
            else
            {
                v = null;
            }
        }

        /// <summary>
        /// Extracts a double value from the stream.
        /// </summary>
        /// <returns>The extracted double.</returns>
        public double readDouble()
        {
            try
            {
                return _buf.b.getDouble();
            }
            catch(InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts an optional double value from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public Optional<double> readDouble(int tag)
        {
            if(readOptional(tag, OptionalFormat.F8))
            {
                return new Optional<double>(readDouble());
            }
            else
            {
                return new Optional<double>();
            }
        }

        /// <summary>
        /// Extracts an optional double value from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <param name="isset">True if the optional value is present, false otherwise.</param>
        /// <param name="v">The optional value.</param>
        public void readDouble(int tag, out bool isset, out double v)
        {
            if(isset = readOptional(tag, OptionalFormat.F8))
            {
                v = readDouble();
            }
            else
            {
                v = 0;
            }
        }

        /// <summary>
        /// Extracts a sequence of double values from the stream.
        /// </summary>
        /// <returns>The extracted double sequence.</returns>
        public double[] readDoubleSeq()
        {
            try
            {
                int sz = readAndCheckSeqSize(8);
                double[] v = new double[sz];
                _buf.b.getDoubleSeq(v);
                return v;
            }
            catch(InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of double values from the stream.
        /// </summary>
        /// <param name="l">The extracted double sequence as a list.</param>
        public void readDoubleSeq(out List<double> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new List<double>(readDoubleSeq());
        }

        /// <summary>
        /// Extracts a sequence of double values from the stream.
        /// </summary>
        /// <param name="l">The extracted double sequence as a linked list.</param>
        public void readDoubleSeq(out LinkedList<double> l)
        {
            try
            {
                int sz = readAndCheckSeqSize(4);
                l = new LinkedList<double>();
                for(int i = 0; i < sz; ++i)
                {
                    l.AddLast(_buf.b.getDouble());
                }
            }
            catch(InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of double values from the stream.
        /// </summary>
        /// <param name="l">The extracted double sequence as a queue.</param>
        public void readDoubleSeq(out Queue<double> l)
        {
            //
            // Reading into an array and copy-constructing the
            // queue takes the same time as constructing the queue
            // and adding to it one element at a time, so
            // we avoid the copy.
            //
            try
            {
                int sz = readAndCheckSeqSize(4);
                l = new Queue<double>(sz);
                for(int i = 0; i < sz; ++i)
                {
                    l.Enqueue(_buf.b.getDouble());
                }
            }
            catch(InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
        }

        /// <summary>
        /// Extracts a sequence of double values from the stream.
        /// </summary>
        /// <param name="l">The extracted double sequence as a stack.</param>
        public void readDoubleSeq(out Stack<double> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            double[] array = readDoubleSeq();
            Array.Reverse(array);
            l = new Stack<double>(array);
        }

        /// <summary>
        /// Extracts an optional double sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public Optional<double[]> readDoubleSeq(int tag)
        {
            if(readOptional(tag, OptionalFormat.VSize))
            {
                skipSize();
                return new Optional<double[]>(readDoubleSeq());
            }
            else
            {
                return new Optional<double[]>();
            }
        }

        /// <summary>
        /// Extracts an optional double sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <param name="isset">True if the optional value is present, false otherwise.</param>
        /// <param name="v">The optional value.</param>
        public void readDoubleSeq(int tag, out bool isset, out double[] v)
        {
            if(isset = readOptional(tag, OptionalFormat.VSize))
            {
                skipSize();
                v = readDoubleSeq();
            }
            else
            {
                v = null;
            }
        }

        private static System.Text.UTF8Encoding utf8 = new System.Text.UTF8Encoding(false, true);

        /// <summary>
        /// Extracts a string from the stream.
        /// </summary>
        /// <returns>The extracted string.</returns>
        public string readString()
        {
            int len = readSize();

            if(len == 0)
            {
                return "";
            }

            //
            // Check the buffer has enough bytes to read.
            //
            if(_buf.b.remaining() < len)
            {
                throw new UnmarshalOutOfBoundsException();
            }

            try
            {
                //
                // We reuse the _stringBytes array to avoid creating
                // excessive garbage
                //
                if(_stringBytes == null || len > _stringBytes.Length)
                {
                    _stringBytes = new byte[len];
                }
                _buf.b.get(_stringBytes, 0, len);
                return utf8.GetString(_stringBytes, 0, len);
            }
            catch(InvalidOperationException ex)
            {
                throw new UnmarshalOutOfBoundsException(ex);
            }
            catch(ArgumentException ex)
            {
                throw new MarshalException("Invalid UTF8 string", ex);
            }
        }

        /// <summary>
        /// Extracts an optional string from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public Optional<string> readString(int tag)
        {
            if(readOptional(tag, OptionalFormat.VSize))
            {
                return new Optional<string>(readString());
            }
            else
            {
                return new Optional<string>();
            }
        }

        /// <summary>
        /// Extracts an optional string from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <param name="isset">True if the optional value is present, false otherwise.</param>
        /// <param name="v">The optional value.</param>
        public void readString(int tag, out bool isset, out string v)
        {
            if(isset = readOptional(tag, OptionalFormat.VSize))
            {
                v = readString();
            }
            else
            {
                v = null;
            }
        }

        /// <summary>
        /// Extracts a sequence of strings from the stream.
        /// </summary>
        /// <returns>The extracted string sequence.</returns>
        public string[] readStringSeq()
        {
            int sz = readAndCheckSeqSize(1);
            string[] v = new string[sz];
            for(int i = 0; i < sz; i++)
            {
                v[i] = readString();
            }
            return v;
        }

        /// <summary>
        /// Extracts a sequence of strings from the stream.
        /// </summary>
        /// <param name="l">The extracted string sequence as a list.</param>
        public void readStringSeq(out List<string> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is slower than constructing the list
            // and adding to it one element at a time.
            //
            int sz = readAndCheckSeqSize(1);
            l = new List<string>(sz);
            for(int i = 0; i < sz; ++i)
            {
                l.Add(readString());
            }
        }

        /// <summary>
        /// Extracts a sequence of strings from the stream.
        /// </summary>
        /// <param name="l">The extracted string sequence as a linked list.</param>
        public void readStringSeq(out LinkedList<string> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is slower than constructing the list
            // and adding to it one element at a time.
            //
            int sz = readAndCheckSeqSize(1);
            l = new LinkedList<string>();
            for(int i = 0; i < sz; ++i)
            {
                l.AddLast(readString());
            }
        }

        /// <summary>
        /// Extracts a sequence of strings from the stream.
        /// </summary>
        /// <param name="l">The extracted string sequence as a queue.</param>
        public void readStringSeq(out Queue<string> l)
        {
            //
            // Reading into an array and copy-constructing the
            // queue is slower than constructing the queue
            // and adding to it one element at a time.
            //
            int sz = readAndCheckSeqSize(1);
            l = new Queue<string>();
            for(int i = 0; i < sz; ++i)
            {
                l.Enqueue(readString());
            }
        }

        /// <summary>
        /// Extracts a sequence of strings from the stream.
        /// </summary>
        /// <param name="l">The extracted string sequence as a stack.</param>
        public void readStringSeq(out Stack<string> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            string[] array = readStringSeq();
            Array.Reverse(array);
            l = new Stack<string>(array);
        }

        /// <summary>
        /// Extracts an optional string sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public Optional<string[]> readStringSeq(int tag)
        {
            if(readOptional(tag, OptionalFormat.FSize))
            {
                skip(4);
                return new Optional<string[]>(readStringSeq());
            }
            else
            {
                return new Optional<string[]>();
            }
        }

        /// <summary>
        /// Extracts an optional string sequence from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <param name="isset">True if the optional value is present, false otherwise.</param>
        /// <param name="v">The optional value.</param>
        public void readStringSeq(int tag, out bool isset, out string[] v)
        {
            if(isset = readOptional(tag, OptionalFormat.FSize))
            {
                skip(4);
                v = readStringSeq();
            }
            else
            {
                v = null;
            }
        }

        /// <summary>
        /// Extracts a proxy from the stream. The stream must have been initialized with a communicator.
        /// </summary>
        /// <returns>The extracted proxy.</returns>
        public ObjectPrx readProxy()
        {
            return _instance.proxyFactory().streamToProxy(this);
        }

        /// <summary>
        /// Extracts an optional proxy from the stream. The stream must have been initialized with a communicator.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <returns>The optional value.</returns>
        public Optional<ObjectPrx> readProxy(int tag)
        {
            if(readOptional(tag, OptionalFormat.FSize))
            {
                skip(4);
                return new Optional<ObjectPrx>(readProxy());
            }
            else
            {
                return new Optional<ObjectPrx>();
            }
        }

        /// <summary>
        /// Extracts an optional proxy from the stream. The stream must have been initialized with a communicator.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <param name="isset">True if the optional value is present, false otherwise.</param>
        /// <param name="v">The optional value.</param>
        public void readProxy(int tag, out bool isset, out ObjectPrx v)
        {
            if(isset = readOptional(tag, OptionalFormat.FSize))
            {
                skip(4);
                v = readProxy();
            }
            else
            {
                v = null;
            }
        }

        /// <summary>
        /// Read an enumerated value.
        /// </summary>
        /// <param name="maxValue">The maximum enumerator value in the definition.</param>
        /// <returns>The enumerator.</returns>
        public int readEnum(int maxValue)
        {
            if(getEncoding().Equals(Util.Encoding_1_0))
            {
                if(maxValue < 127)
                {
                    return readByte();
                }
                else if(maxValue < 32767)
                {
                    return readShort();
                }
                else
                {
                    return readInt();
                }
            }
            else
            {
                return readSize();
            }
        }

        /// <summary>
        /// Extracts the index of a Slice value from the stream.
        /// </summary>
        /// <param name="cb">The callback to notify the application when the extracted instance is available.
        /// The stream extracts Slice values in stages. The Ice run time invokes the delegate when the
        /// corresponding instance has been fully unmarshaled.</param>
        public void readValue(ReadValueCallback cb)
        {
            initEncaps();
            _encapsStack.decoder.readValue(cb);
        }

        /// <summary>
        /// Extracts the index of an optional Slice value from the stream.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <param name="cb">The callback to notify the application when the extracted instance is available (if any).
        /// The stream extracts Slice values in stages. The Ice run time invokes the delegate when the
        /// corresponding instance has been fully unmarshaled.</param>
        public void readValue(int tag, ReadValueCallback cb)
        {
            if(readOptional(tag, OptionalFormat.Class))
            {
                readValue(cb);
            }
        }

        /// <summary>
        /// Extracts a user exception from the stream and throws it.
        /// </summary>
        public void throwException()
        {
            throwException(null);
        }

        /// <summary>
        /// Extracts a user exception from the stream and throws it.
        /// </summary>
        /// <param name="factory">The user exception factory, or null to use the stream's default behavior.</param>
        public void throwException(UserExceptionFactory factory)
        {
            initEncaps();
            _encapsStack.decoder.throwException(factory);
        }

        /// <summary>
        /// Skip the given number of bytes.
        /// </summary>
        /// <param name="size">The number of bytes to skip</param>
        public void skip(int size)
        {
            if(size < 0 || size > _buf.b.remaining())
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
            byte b = readByte();
            if(b == 255)
            {
                skip(4);
            }
        }

        /// <summary>
        /// Determines the current position in the stream.
        /// </summary>
        /// <returns>The current position.</returns>
        public int pos()
        {
            return _buf.b.position();
        }

        /// <summary>
        /// Sets the current position in the stream.
        /// </summary>
        /// <param name="n">The new position.</param>
        public void pos(int n)
        {
            _buf.b.position(n);
        }

        /// <summary>
        /// Determines the current size of the stream.
        /// </summary>
        /// <returns>The current size.</returns>
        public int size()
        {
            return _buf.size();
        }

        /// <summary>
        /// Determines whether the stream is empty.
        /// </summary>
        /// <returns>True if the internal buffer has no data, false otherwise.</returns>
        public bool isEmpty()
        {
            return _buf.empty();
        }

        private bool readOptImpl(int readTag, OptionalFormat expectedFormat)
        {
            if(isEncoding_1_0())
            {
                return false; // Optional members aren't supported with the 1.0 encoding.
            }

            while(true)
            {
                if(_buf.b.position() >= _encapsStack.start + _encapsStack.sz)
                {
                    return false; // End of encapsulation also indicates end of optionals.
                }

                int v = readByte();
                if(v == Protocol.OPTIONAL_END_MARKER)
                {
                    _buf.b.position(_buf.b.position() - 1); // Rewind.
                    return false;
                }

                OptionalFormat format = (OptionalFormat)(v & 0x07); // First 3 bits.
                int tag = v >> 3;
                if(tag == 30)
                {
                    tag = readSize();
                }

                if(tag > readTag)
                {
                    int offset = tag < 30 ? 1 : (tag < 255 ? 2 : 6); // Rewind
                    _buf.b.position(_buf.b.position() - offset);
                    return false; // No optional data members with the requested tag.
                }
                else if(tag < readTag)
                {
                    skipOptional(format); // Skip optional data members
                }
                else
                {
                    if(format != expectedFormat)
                    {
                        throw new MarshalException("invalid optional data member `" + tag + "': unexpected format");
                    }
                    return true;
                }
            }
        }

        private void skipOptional(OptionalFormat format)
        {
            switch(format)
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
                skip(readSize());
                break;
            }
            case OptionalFormat.FSize:
            {
                skip(readInt());
                break;
            }
            case OptionalFormat.Class:
            {
                readValue(null);
                break;
            }
            }
        }

        private bool skipOptionals()
        {
            //
            // Skip remaining un-read optional members.
            //
            while(true)
            {
                if(_buf.b.position() >= _encapsStack.start + _encapsStack.sz)
                {
                    return false; // End of encapsulation also indicates end of optionals.
                }

                int v = readByte();
                if(v == Protocol.OPTIONAL_END_MARKER)
                {
                    return true;
                }

                OptionalFormat format = (OptionalFormat)(v & 0x07); // Read first 3 bits.
                if((v >> 3) == 30)
                {
                    skipSize();
                }
                skipOptional(format);
            }
        }

        private UserException createUserException(string id)
        {
            UserException userEx = null;

            try
            {
                if(_classResolver != null)
                {
                    Type c = _classResolver(id);
                    if(c != null)
                    {
                        Debug.Assert(!c.IsAbstract && !c.IsInterface);
                        userEx = (UserException)IceInternal.AssemblyUtil.createInstance(c);
                    }
                }
            }
            catch(Exception ex)
            {
                throw new MarshalException(ex);
            }

            return userEx;
        }

        private IceInternal.Instance _instance;
        private IceInternal.Buffer _buf;
        private object _closure;
        private byte[] _stringBytes; // Reusable array for reading strings.

        private enum SliceType { NoSlice, ValueSlice, ExceptionSlice }

        abstract private class EncapsDecoder
        {
            internal EncapsDecoder(InputStream stream, Encaps encaps, bool sliceValues, ValueFactoryManager f,
                                   ClassResolver cr)
            {
                _stream = stream;
                _encaps = encaps;
                _sliceValues = sliceValues;
                _valueFactoryManager = f;
                _classResolver = cr;
                _typeIdIndex = 0;
                _unmarshaledMap = new Dictionary<int, Value>();
            }

            internal abstract void readValue(ReadValueCallback cb);
            internal abstract void throwException(UserExceptionFactory factory);

            internal abstract void startInstance(SliceType type);
            internal abstract SlicedData endInstance(bool preserve);
            internal abstract string startSlice();
            internal abstract void endSlice();
            internal abstract void skipSlice();

            internal virtual bool readOptional(int tag, OptionalFormat format)
            {
                return false;
            }

            internal virtual void readPendingValues()
            {
            }

            protected string readTypeId(bool isIndex)
            {
                if(_typeIdMap == null)
                {
                    _typeIdMap = new Dictionary<int, string>();
                }

                if(isIndex)
                {
                    int index = _stream.readSize();
                    string typeId;
                    if(!_typeIdMap.TryGetValue(index, out typeId))
                    {
                        throw new UnmarshalOutOfBoundsException();
                    }
                    return typeId;
                }
                else
                {
                    string typeId = _stream.readString();
                    _typeIdMap.Add(++_typeIdIndex, typeId);
                    return typeId;
                }
            }

            protected Type resolveClass(string typeId)
            {
                Type cls = null;
                if(_typeIdCache == null)
                {
                    _typeIdCache = new Dictionary<string, Type>(); // Lazy initialization.
                }
                else
                {
                    _typeIdCache.TryGetValue(typeId, out cls);
                }

                if(cls == typeof(EncapsDecoder)) // Marker for non-existent class.
                {
                    cls = null;
                }
                else if(cls == null)
                {
                    try
                    {
                        if(_classResolver != null)
                        {
                            cls = _classResolver(typeId);
                            _typeIdCache.Add(typeId, cls != null ? cls : typeof(EncapsDecoder));
                        }
                    }
                    catch(Exception ex)
                    {
                        throw new NoValueFactoryException("no value factory", typeId, ex);
                    }
                }

                return cls;
            }

            protected Value newInstance(string typeId)
            {
                //
                // Try to find a factory registered for the specific type.
                //
                var userFactory = _valueFactoryManager.find(typeId);
                Value v = null;
                if(userFactory != null)
                {
                    v = userFactory(typeId);
                }

                //
                // If that fails, invoke the default factory if one has been
                // registered.
                //
                if(v == null)
                {
                    userFactory = _valueFactoryManager.find("");
                    if(userFactory != null)
                    {
                        v = userFactory(typeId);
                    }
                }

                //
                // Last chance: try to instantiate the class dynamically.
                //
                if(v == null)
                {
                    Type cls = resolveClass(typeId);

                    if(cls != null)
                    {
                        try
                        {
                            Debug.Assert(!cls.IsAbstract && !cls.IsInterface);
                            v = (Value)IceInternal.AssemblyUtil.createInstance(cls);
                        }
                        catch(Exception ex)
                        {
                            throw new NoValueFactoryException("no value factory", typeId, ex);
                        }
                    }
                }

                return v;
            }

            protected void addPatchEntry(int index, ReadValueCallback cb)
            {
                Debug.Assert(index > 0);

                //
                // Check if we already unmarshaled the instance. If that's the case,
                // just call the callback and we're done.
                //
                Value obj;
                if(_unmarshaledMap.TryGetValue(index, out obj))
                {
                    cb(obj);
                    return;
                }

                if(_patchMap == null)
                {
                    _patchMap = new Dictionary<int, LinkedList<ReadValueCallback>>();
                }

                //
                // Add patch entry if the instance isn't unmarshaled yet,
                // the callback will be called when the instance is
                // unmarshaled.
                //
                LinkedList<ReadValueCallback> l;
                if(!_patchMap.TryGetValue(index, out l))
                {
                    //
                    // We have no outstanding instances to be patched for this
                    // index, so make a new entry in the patch map.
                    //
                    l = new LinkedList<ReadValueCallback>();
                    _patchMap.Add(index, l);
                }

                //
                // Append a patch entry for this instance.
                //
                l.AddLast(cb);
            }

            protected void unmarshal(int index, Value v)
            {
                //
                // Add the instance to the map of unmarshaled instances, this must
                // be done before reading the instances (for circular references).
                //
                _unmarshaledMap.Add(index, v);

                //
                // Read the instance.
                //
                v.iceRead(_stream);

                if(_patchMap != null)
                {
                    //
                    // Patch all instances now that the instance is unmarshaled.
                    //
                    LinkedList<ReadValueCallback> l;
                    if(_patchMap.TryGetValue(index, out l))
                    {
                        Debug.Assert(l.Count > 0);

                        //
                        // Patch all pointers that refer to the instance.
                        //
                        foreach(ReadValueCallback cb in l)
                        {
                            cb(v);
                        }

                        //
                        // Clear out the patch map for that index -- there is nothing left
                        // to patch for that index for the time being.
                        //
                        _patchMap.Remove(index);
                    }
                }

                if((_patchMap == null || _patchMap.Count == 0) && _valueList == null)
                {
                    try
                    {
                        v.ice_postUnmarshal();
                    }
                    catch(System.Exception ex)
                    {
                        string s = "exception raised by ice_postUnmarshal:\n" + ex;
                        _stream.instance().initializationData().logger.warning(s);
                    }
                }
                else
                {
                    if(_valueList == null)
                    {
                        _valueList = new List<Value>();
                    }
                    _valueList.Add(v);

                    if(_patchMap == null || _patchMap.Count == 0)
                    {
                        //
                        // Iterate over the instance list and invoke ice_postUnmarshal on
                        // each instance. We must do this after all instances have been
                        // unmarshaled in order to ensure that any instance data members
                        // have been properly patched.
                        //
                        foreach(var p in _valueList)
                        {
                            try
                            {
                                p.ice_postUnmarshal();
                            }
                            catch(System.Exception ex)
                            {
                                string s = "exception raised by ice_postUnmarshal:\n" + ex;
                                _stream.instance().initializationData().logger.warning(s);
                            }
                        }
                        _valueList.Clear();
                    }
                }
            }

            protected readonly InputStream _stream;
            protected readonly Encaps _encaps;
            protected readonly bool _sliceValues;
            protected ValueFactoryManager _valueFactoryManager;
            protected ClassResolver _classResolver;

            //
            // Encapsulation attributes for object unmarshaling.
            //
            protected Dictionary<int, LinkedList<ReadValueCallback> > _patchMap;
            private Dictionary<int, Value> _unmarshaledMap;
            private Dictionary<int, string> _typeIdMap;
            private int _typeIdIndex;
            private List<Value> _valueList;
            private Dictionary<string, Type> _typeIdCache;
        }

        private sealed class EncapsDecoder10 : EncapsDecoder
        {
            internal EncapsDecoder10(InputStream stream, Encaps encaps, bool sliceValues, ValueFactoryManager f,
                                     ClassResolver cr)
                : base(stream, encaps, sliceValues, f, cr)
            {
                _sliceType = SliceType.NoSlice;
            }

            internal override void readValue(ReadValueCallback cb)
            {
                Debug.Assert(cb != null);

                //
                // Object references are encoded as a negative integer in 1.0.
                //
                int index = _stream.readInt();
                if(index > 0)
                {
                    throw new MarshalException("invalid object id");
                }
                index = -index;

                if(index == 0)
                {
                    cb(null);
                }
                else
                {
                    addPatchEntry(index, cb);
                }
            }

            internal override void throwException(UserExceptionFactory factory)
            {
                Debug.Assert(_sliceType == SliceType.NoSlice);

                //
                // User exception with the 1.0 encoding start with a bool flag
                // that indicates whether or not the exception has classes.
                //
                // This allows reading the pending instances even if some part of
                // the exception was sliced.
                //
                bool usesClasses = _stream.readBool();

                _sliceType = SliceType.ExceptionSlice;
                _skipFirstSlice = false;

                //
                // Read the first slice header.
                //
                startSlice();
                string mostDerivedId = _typeId;
                while(true)
                {
                    UserException userEx = null;

                    //
                    // Use a factory if one was provided.
                    //
                    if(factory != null)
                    {
                        try
                        {
                            factory(_typeId);
                        }
                        catch(UserException ex)
                        {
                            userEx = ex;
                        }
                    }

                    if(userEx == null)
                    {
                        userEx = _stream.createUserException(_typeId);
                    }

                    //
                    // We found the exception.
                    //
                    if(userEx != null)
                    {
                        userEx.iceRead(_stream);
                        if(usesClasses)
                        {
                            readPendingValues();
                        }
                        throw userEx;

                        // Never reached.
                    }

                    //
                    // Slice off what we don't understand.
                    //
                    skipSlice();
                    try
                    {
                        startSlice();
                    }
                    catch(UnmarshalOutOfBoundsException ex)
                    {
                        //
                        // An oversight in the 1.0 encoding means there is no marker to indicate
                        // the last slice of an exception. As a result, we just try to read the
                        // next type ID, which raises UnmarshalOutOfBoundsException when the
                        // input buffer underflows.
                        //
                        // Set the reason member to a more helpful message.
                        //
                        ex.reason = "unknown exception type `" + mostDerivedId + "'";
                        throw;
                    }
                }
            }

            internal override void startInstance(SliceType sliceType)
            {
                Debug.Assert(_sliceType == sliceType);
                _skipFirstSlice = true;
            }

            internal override SlicedData endInstance(bool preserve)
            {
                //
                // Read the Ice::Object slice.
                //
                if(_sliceType == SliceType.ValueSlice)
                {
                    startSlice();
                    int sz = _stream.readSize(); // For compatibility with the old AFM.
                    if(sz != 0)
                    {
                        throw new MarshalException("invalid Object slice");
                    }
                    endSlice();
                }

                _sliceType = SliceType.NoSlice;
                return null;
            }

            internal override string startSlice()
            {
                //
                // If first slice, don't read the header, it was already read in
                // readInstance or throwException to find the factory.
                //
                if(_skipFirstSlice)
                {
                    _skipFirstSlice = false;
                    return _typeId;
                }

                //
                // For instances, first read the type ID bool which indicates
                // whether or not the type ID is encoded as a string or as an
                // index. For exceptions, the type ID is always encoded as a
                // string.
                //
                if(_sliceType == SliceType.ValueSlice) // For exceptions, the type ID is always encoded as a string
                {
                    bool isIndex = _stream.readBool();
                    _typeId = readTypeId(isIndex);
                }
                else
                {
                    _typeId = _stream.readString();
                }

                _sliceSize = _stream.readInt();
                if(_sliceSize < 4)
                {
                    throw new UnmarshalOutOfBoundsException();
                }

                return _typeId;
            }

            internal override void endSlice()
            {
            }

            internal override void skipSlice()
            {
                if(_stream.instance().traceLevels().slicing > 0)
                {
                    Logger logger = _stream.instance().initializationData().logger;
                    string slicingCat = _stream.instance().traceLevels().slicingCat;
                    if(_sliceType == SliceType.ValueSlice)
                    {
                        IceInternal.TraceUtil.traceSlicing("object", _typeId, slicingCat, logger);
                    }
                    else
                    {
                        IceInternal.TraceUtil.traceSlicing("exception", _typeId, slicingCat, logger);
                    }
                }

                Debug.Assert(_sliceSize >= 4);
                _stream.skip(_sliceSize - 4);
            }

            internal override void readPendingValues()
            {
                int num;
                do
                {
                    num = _stream.readSize();
                    for(int k = num; k > 0; --k)
                    {
                        readInstance();
                    }
                }
                while(num > 0);

                if(_patchMap != null && _patchMap.Count > 0)
                {
                    //
                    // If any entries remain in the patch map, the sender has sent an index for an instance, but failed
                    // to supply the instance.
                    //
                    throw new MarshalException("index for class received, but no instance");
                }
            }

            private void readInstance()
            {
                int index = _stream.readInt();

                if(index <= 0)
                {
                    throw new MarshalException("invalid object id");
                }

                _sliceType = SliceType.ValueSlice;
                _skipFirstSlice = false;

                //
                // Read the first slice header.
                //
                startSlice();
                string mostDerivedId = _typeId;
                Value v = null;
                while(true)
                {
                    //
                    // For the 1.0 encoding, the type ID for the base Object class
                    // marks the last slice.
                    //
                    if(_typeId.Equals(Value.ice_staticId()))
                    {
                        throw new NoValueFactoryException("", mostDerivedId);
                    }

                    v = newInstance(_typeId);

                    //
                    // We found a factory, we get out of this loop.
                    //
                    if(v != null)
                    {
                        break;
                    }

                    //
                    // If slicing is disabled, stop unmarshaling.
                    //
                    if(!_sliceValues)
                    {
                        throw new NoValueFactoryException("no value factory found and slicing is disabled", _typeId);
                    }

                    //
                    // Slice off what we don't understand.
                    //
                    skipSlice();
                    startSlice(); // Read next Slice header for next iteration.
                }

                //
                // Unmarshal the instance and add it to the map of unmarshaled instances.
                //
                unmarshal(index, v);
            }

            // Object/exception attributes
            private SliceType _sliceType;
            private bool _skipFirstSlice;

            // Slice attributes
            private int _sliceSize;
            private string _typeId;
        }

        private sealed class EncapsDecoder11 : EncapsDecoder
        {
            internal EncapsDecoder11(InputStream stream, Encaps encaps, bool sliceValues, ValueFactoryManager f,
                                     ClassResolver cr, CompactIdResolver r)
                : base(stream, encaps, sliceValues, f, cr)
            {
                _compactIdResolver = r;
                _current = null;
                _valueIdIndex = 1;
            }

            internal override void readValue(ReadValueCallback cb)
            {
                int index = _stream.readSize();
                if(index < 0)
                {
                    throw new MarshalException("invalid object id");
                }
                else if(index == 0)
                {
                    if(cb != null)
                    {
                        cb(null);
                    }
                }
                else if(_current != null && (_current.sliceFlags & Protocol.FLAG_HAS_INDIRECTION_TABLE) != 0)
                {
                    //
                    // When reading an instance within a slice and there's an
                    // indirect instance table, always read an indirect reference
                    // that points to an instance from the indirect instance table
                    // marshaled at the end of the Slice.
                    //
                    // Maintain a list of indirect references. Note that the
                    // indirect index starts at 1, so we decrement it by one to
                    // derive an index into the indirection table that we'll read
                    // at the end of the slice.
                    //
                    if(cb != null)
                    {
                        if(_current.indirectPatchList == null)
                        {
                            _current.indirectPatchList = new Stack<IndirectPatchEntry>();
                        }
                        IndirectPatchEntry e = new IndirectPatchEntry();
                        e.index = index - 1;
                        e.patcher = cb;
                        _current.indirectPatchList.Push(e);
                    }
                }
                else
                {
                    readInstance(index, cb);
                }
            }

            internal override void throwException(UserExceptionFactory factory)
            {
                Debug.Assert(_current == null);

                push(SliceType.ExceptionSlice);

                //
                // Read the first slice header.
                //
                startSlice();
                string mostDerivedId = _current.typeId;
                while(true)
                {
                    UserException userEx = null;

                    //
                    // Use a factory if one was provided.
                    //
                    if(factory != null)
                    {
                        try
                        {
                            factory(_current.typeId);
                        }
                        catch(UserException ex)
                        {
                            userEx = ex;
                        }
                    }

                    if(userEx == null)
                    {
                        userEx = _stream.createUserException(_current.typeId);
                    }

                    //
                    // We found the exception.
                    //
                    if(userEx != null)
                    {
                        userEx.iceRead(_stream);
                        throw userEx;

                        // Never reached.
                    }

                    //
                    // Slice off what we don't understand.
                    //
                    skipSlice();

                    if((_current.sliceFlags & Protocol.FLAG_IS_LAST_SLICE) != 0)
                    {
                        if(mostDerivedId.StartsWith("::", StringComparison.Ordinal))
                        {
                            throw new UnknownUserException(mostDerivedId.Substring(2));
                        }
                        else
                        {
                            throw new UnknownUserException(mostDerivedId);
                        }
                    }

                    startSlice();
                }
            }

            internal override void startInstance(SliceType sliceType)
            {
                Debug.Assert(_current.sliceType == sliceType);
                _current.skipFirstSlice = true;
            }

            internal override SlicedData endInstance(bool preserve)
            {
                SlicedData slicedData = null;
                if(preserve)
                {
                    slicedData = readSlicedData();
                }
                if(_current.slices != null)
                {
                    _current.slices.Clear();
                    _current.indirectionTables.Clear();
                }
                _current = _current.previous;
                return slicedData;
            }

            internal override string startSlice()
            {
                //
                // If first slice, don't read the header, it was already read in
                // readInstance or throwException to find the factory.
                //
                if(_current.skipFirstSlice)
                {
                    _current.skipFirstSlice = false;
                    return _current.typeId;
                }

                _current.sliceFlags = _stream.readByte();

                //
                // Read the type ID, for instance slices the type ID is encoded as a
                // string or as an index, for exceptions it's always encoded as a
                // string.
                //
                if(_current.sliceType == SliceType.ValueSlice)
                {
                    //
                    // Must be checked first!
                    //
                    if((_current.sliceFlags & Protocol.FLAG_HAS_TYPE_ID_COMPACT) == Protocol.FLAG_HAS_TYPE_ID_COMPACT)
                    {
                        _current.typeId = "";
                        _current.compactId = _stream.readSize();
                    }
                    else if((_current.sliceFlags &
                            (Protocol.FLAG_HAS_TYPE_ID_INDEX | Protocol.FLAG_HAS_TYPE_ID_STRING)) != 0)
                    {
                        _current.typeId = readTypeId((_current.sliceFlags & Protocol.FLAG_HAS_TYPE_ID_INDEX) != 0);
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
                    _current.typeId = _stream.readString();
                    _current.compactId = -1;
                }

                //
                // Read the slice size if necessary.
                //
                if((_current.sliceFlags & Protocol.FLAG_HAS_SLICE_SIZE) != 0)
                {
                    _current.sliceSize = _stream.readInt();
                    if(_current.sliceSize < 4)
                    {
                        throw new UnmarshalOutOfBoundsException();
                    }
                }
                else
                {
                    _current.sliceSize = 0;
                }

                return _current.typeId;
            }

            internal override void endSlice()
            {
                if((_current.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS) != 0)
                {
                    _stream.skipOptionals();
                }

                //
                // Read the indirection table if one is present and transform the
                // indirect patch list into patch entries with direct references.
                //
                if((_current.sliceFlags & Protocol.FLAG_HAS_INDIRECTION_TABLE) != 0)
                {
                    //
                    // The table is written as a sequence<size> to conserve space.
                    //
                    int[] indirectionTable = new int[_stream.readAndCheckSeqSize(1)];
                    for(int i = 0; i < indirectionTable.Length; ++i)
                    {
                        indirectionTable[i] = readInstance(_stream.readSize(), null);
                    }

                    //
                    // Sanity checks. If there are optional members, it's possible
                    // that not all instance references were read if they are from
                    // unknown optional data members.
                    //
                    if(indirectionTable.Length == 0)
                    {
                        throw new MarshalException("empty indirection table");
                    }
                    if((_current.indirectPatchList == null || _current.indirectPatchList.Count == 0) &&
                       (_current.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS) == 0)
                    {
                        throw new MarshalException("no references to indirection table");
                    }

                    //
                    // Convert indirect references into direct references.
                    //
                    if(_current.indirectPatchList != null)
                    {
                        foreach(IndirectPatchEntry e in _current.indirectPatchList)
                        {
                            Debug.Assert(e.index >= 0);
                            if(e.index >= indirectionTable.Length)
                            {
                                throw new MarshalException("indirection out of range");
                            }
                            addPatchEntry(indirectionTable[e.index], e.patcher);
                        }
                        _current.indirectPatchList.Clear();
                    }
                }
            }

            internal override void skipSlice()
            {
                if(_stream.instance().traceLevels().slicing > 0)
                {
                    Logger logger = _stream.instance().initializationData().logger;
                    string slicingCat = _stream.instance().traceLevels().slicingCat;
                    if(_current.sliceType == SliceType.ExceptionSlice)
                    {
                        IceInternal.TraceUtil.traceSlicing("exception", _current.typeId, slicingCat, logger);
                    }
                    else
                    {
                        IceInternal.TraceUtil.traceSlicing("object", _current.typeId, slicingCat, logger);
                    }
                }

                int start = _stream.pos();

                if((_current.sliceFlags & Protocol.FLAG_HAS_SLICE_SIZE) != 0)
                {
                    Debug.Assert(_current.sliceSize >= 4);
                    _stream.skip(_current.sliceSize - 4);
                }
                else
                {
                    if(_current.sliceType == SliceType.ValueSlice)
                    {
                        throw new NoValueFactoryException("no value factory found and compact format prevents " +
                                                          "slicing (the sender should use the sliced format " +
                                                          "instead)", _current.typeId);
                    }
                    else
                    {
                        if(_current.typeId.StartsWith("::", StringComparison.Ordinal))
                        {
                            throw new UnknownUserException(_current.typeId.Substring(2));
                        }
                        else
                        {
                            throw new UnknownUserException(_current.typeId);
                        }
                    }
                }

                //
                // Preserve this slice.
                //
                SliceInfo info = new SliceInfo();
                info.typeId = _current.typeId;
                info.compactId = _current.compactId;
                info.hasOptionalMembers = (_current.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS) != 0;
                info.isLastSlice = (_current.sliceFlags & Protocol.FLAG_IS_LAST_SLICE) != 0;
                IceInternal.ByteBuffer b = _stream.getBuffer().b;
                int end = b.position();
                int dataEnd = end;
                if(info.hasOptionalMembers)
                {
                    //
                    // Don't include the optional member end marker. It will be re-written by
                    // endSlice when the sliced data is re-written.
                    //
                    --dataEnd;
                }
                info.bytes = new byte[dataEnd - start];
                b.position(start);
                b.get(info.bytes);
                b.position(end);

                if(_current.slices == null)
                {
                    _current.slices = new List<SliceInfo>();
                    _current.indirectionTables = new List<int[]>();
                }

                //
                // Read the indirect instance table. We read the instances or their
                // IDs if the instance is a reference to an already unmarshaled
                // instance.
                //
                if((_current.sliceFlags & Protocol.FLAG_HAS_INDIRECTION_TABLE) != 0)
                {
                    int[] indirectionTable = new int[_stream.readAndCheckSeqSize(1)];
                    for(int i = 0; i < indirectionTable.Length; ++i)
                    {
                        indirectionTable[i] = readInstance(_stream.readSize(), null);
                    }
                    _current.indirectionTables.Add(indirectionTable);
                }
                else
                {
                    _current.indirectionTables.Add(null);
                }

                _current.slices.Add(info);
            }

            internal override bool readOptional(int readTag, OptionalFormat expectedFormat)
            {
                if(_current == null)
                {
                    return _stream.readOptImpl(readTag, expectedFormat);
                }
                else if((_current.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS) != 0)
                {
                    return _stream.readOptImpl(readTag, expectedFormat);
                }
                return false;
            }

            private int readInstance(int index, ReadValueCallback cb)
            {
                Debug.Assert(index > 0);

                if(index > 1)
                {
                    if(cb != null)
                    {
                        addPatchEntry(index, cb);
                    }
                    return index;
                }

                push(SliceType.ValueSlice);

                //
                // Get the instance ID before we start reading slices. If some
                // slices are skipped, the indirect instance table are still read and
                // might read other instances.
                //
                index = ++_valueIdIndex;

                //
                // Read the first slice header.
                //
                startSlice();
                string mostDerivedId = _current.typeId;
                Value v = null;
                while(true)
                {
                    bool updateCache = false;

                    if(_current.compactId >= 0)
                    {
                        updateCache = true;

                        //
                        // Translate a compact (numeric) type ID into a class.
                        //
                        if(_compactIdCache == null)
                        {
                            _compactIdCache = new Dictionary<int, Type>(); // Lazy initialization.
                        }
                        else
                        {
                            //
                            // Check the cache to see if we've already translated the compact type ID into a class.
                            //
                            Type cls = null;
                            _compactIdCache.TryGetValue(_current.compactId, out cls);
                            if(cls != null)
                            {
                                try
                                {
                                    Debug.Assert(!cls.IsAbstract && !cls.IsInterface);
                                    v = (Value)IceInternal.AssemblyUtil.createInstance(cls);
                                    updateCache = false;
                                }
                                catch(Exception ex)
                                {
                                    throw new NoValueFactoryException("no value factory", "compact ID " +
                                                                      _current.compactId, ex);
                                }
                            }
                        }

                        //
                        // If we haven't already cached a class for the compact ID, then try to translate the
                        // compact ID into a type ID.
                        //
                        if(v == null)
                        {
                            _current.typeId = "";
                            if(_compactIdResolver != null)
                            {
                                try
                                {
                                    _current.typeId = _compactIdResolver(_current.compactId);
                                }
                                catch(LocalException)
                                {
                                    throw;
                                }
                                catch(System.Exception ex)
                                {
                                    throw new MarshalException("exception in CompactIdResolver for ID " +
                                                                   _current.compactId, ex);
                                }
                            }

                            if(_current.typeId.Length == 0)
                            {
                                _current.typeId = _stream.instance().resolveCompactId(_current.compactId);
                            }
                        }
                    }

                    if(v == null && _current.typeId.Length > 0)
                    {
                        v = newInstance(_current.typeId);
                    }

                    if(v != null)
                    {
                        if(updateCache)
                        {
                            Debug.Assert(_current.compactId >= 0);
                            _compactIdCache.Add(_current.compactId, v.GetType());
                        }

                        //
                        // We have an instance, get out of this loop.
                        //
                        break;
                    }

                    //
                    // If slicing is disabled, stop unmarshaling.
                    //
                    if(!_sliceValues)
                    {
                        throw new NoValueFactoryException("no value factory found and slicing is disabled",
                                                          _current.typeId);
                    }

                    //
                    // Slice off what we don't understand.
                    //
                    skipSlice();

                    //
                    // If this is the last slice, keep the instance as an opaque
                    // UnknownSlicedValue object.
                    //
                    if((_current.sliceFlags & Protocol.FLAG_IS_LAST_SLICE) != 0)
                    {
                        //
                        // Provide a factory with an opportunity to supply the instance.
                        // We pass the "::Ice::Object" ID to indicate that this is the
                        // last chance to preserve the instance.
                        //
                        v = newInstance(Value.ice_staticId());
                        if(v == null)
                        {
                            v = new UnknownSlicedValue(mostDerivedId);
                        }

                        break;
                    }

                    startSlice(); // Read next Slice header for next iteration.
                }

                //
                // Unmarshal the instance.
                //
                unmarshal(index, v);

                if(_current == null && _patchMap != null && _patchMap.Count > 0)
                {
                    //
                    // If any entries remain in the patch map, the sender has sent an index for an instance, but failed
                    // to supply the instance.
                    //
                    throw new MarshalException("index for class received, but no instance");
                }

                if(cb != null)
                {
                    cb(v);
                }
                return index;
            }

            private SlicedData readSlicedData()
            {
                if(_current.slices == null) // No preserved slices.
                {
                    return null;
                }

                //
                // The _indirectionTables member holds the indirection table for each slice
                // in _slices.
                //
                Debug.Assert(_current.slices.Count == _current.indirectionTables.Count);
                for(int n = 0; n < _current.slices.Count; ++n)
                {
                    //
                    // We use the "instances" list in SliceInfo to hold references
                    // to the target instances. Note that the instances might not have
                    // been read yet in the case of a circular reference to an
                    // enclosing instance.
                    //
                    int[] table = _current.indirectionTables[n];
                    SliceInfo info = _current.slices[n];
                    info.instances = new Value[table != null ? table.Length : 0];
                    for(int j = 0; j < info.instances.Length; ++j)
                    {
                        IceInternal.ArrayPatcher<Value> patcher =
                            new IceInternal.ArrayPatcher<Value>(Value.ice_staticId(), info.instances, j);
                        addPatchEntry(table[j], patcher.patch);
                    }
                }

                return new SlicedData(_current.slices.ToArray());
            }

            private void push(SliceType sliceType)
            {
                if(_current == null)
                {
                    _current = new InstanceData(null);
                }
                else
                {
                    _current = _current.next == null ? new InstanceData(_current) : _current.next;
                }
                _current.sliceType = sliceType;
                _current.skipFirstSlice = false;
            }

            private sealed class IndirectPatchEntry
            {
                public int index;
                public ReadValueCallback patcher;
            }

            private sealed class InstanceData
            {
                internal InstanceData(InstanceData previous)
                {
                    if(previous != null)
                    {
                        previous.next = this;
                    }
                    this.previous = previous;
                    this.next = null;
                }

                // Instance attributes
                internal SliceType sliceType;
                internal bool skipFirstSlice;
                internal List<SliceInfo> slices;     // Preserved slices.
                internal List<int[]> indirectionTables;

                // Slice attributes
                internal byte sliceFlags;
                internal int sliceSize;
                internal string typeId;
                internal int compactId;
                internal Stack<IndirectPatchEntry> indirectPatchList;

                internal InstanceData previous;
                internal InstanceData next;
            }

            private CompactIdResolver _compactIdResolver;
            private InstanceData _current;
            private int _valueIdIndex; // The ID of the next instance to unmarshal.
            private Dictionary<int, Type> _compactIdCache;
        }

        private sealed class Encaps
        {
            internal void reset()
            {
                decoder = null;
            }

            internal void setEncoding(EncodingVersion encoding)
            {
                this.encoding = encoding;
                encoding_1_0 = encoding.Equals(Util.Encoding_1_0);
            }

            internal int start;
            internal int sz;
            internal EncodingVersion encoding;
            internal bool encoding_1_0;

            internal EncapsDecoder decoder;

            internal Encaps next;
        }

        //
        // The encoding version to use when there's no encapsulation to
        // read from. This is for example used to read message headers.
        //
        private EncodingVersion _encoding;

        private bool isEncoding_1_0()
        {
            return _encapsStack != null ? _encapsStack.encoding_1_0 : _encoding.Equals(Util.Encoding_1_0);
        }

        private Encaps _encapsStack;
        private Encaps _encapsCache;

        private void initEncaps()
        {
            if(_encapsStack == null) // Lazy initialization
            {
                _encapsStack = _encapsCache;
                if(_encapsStack != null)
                {
                    _encapsCache = _encapsCache.next;
                }
                else
                {
                    _encapsStack = new Encaps();
                }
                _encapsStack.setEncoding(_encoding);
                _encapsStack.sz = _buf.b.limit();
            }

            if(_encapsStack.decoder == null) // Lazy initialization.
            {
                if(_encapsStack.encoding_1_0)
                {
                    _encapsStack.decoder = new EncapsDecoder10(this, _encapsStack, _sliceValues, _valueFactoryManager,
                                                               _classResolver);
                }
                else
                {
                    _encapsStack.decoder = new EncapsDecoder11(this, _encapsStack, _sliceValues, _valueFactoryManager,
                                                               _classResolver, _compactIdResolver);
                }
            }
        }

        private bool _sliceValues;
        private bool _traceSlicing;

        private int _startSeq;
        private int _minSeqSize;

        private ValueFactoryManager _valueFactoryManager;
        private Logger _logger;
        private CompactIdResolver _compactIdResolver;
        private ClassResolver _classResolver;
    }

    /// <summary>
    /// Base class for extracting class instances from an input stream.
    /// </summary>
    public abstract class ValueReader : Value
    {
        /// <summary>
        /// Read the instance's data members.
        /// </summary>
        /// <param name="inStream">The input stream to read from.</param>
        public abstract void read(InputStream inStream);

        public override void iceWrite(OutputStream os)
        {
            Debug.Assert(false);
        }

        public override void iceRead(InputStream istr)
        {
            read(istr);
        }
    }

}
