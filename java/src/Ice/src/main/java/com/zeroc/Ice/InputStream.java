// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.Ice;

import java.io.IOException;

import com.zeroc.IceInternal.Buffer;
import com.zeroc.IceInternal.Instance;
import com.zeroc.IceInternal.Protocol;
import com.zeroc.IceInternal.SequencePatcher;

/**
 * Interface for input streams used to extract Slice types from a sequence
 * of bytes.
 *
 * @see OutputStream
 **/
public class InputStream
{
    /**
     * Constructing an InputStream without providing a communicator means the stream will
     * use the default encoding version. A communicator is required in order to unmarshal
     * proxies. You can supply a communicator later by calling initialize().
     **/
    public InputStream()
    {
        initialize(Protocol.currentEncoding);
        _buf = new Buffer(false);
    }

    /**
     * Constructing an InputStream without providing a communicator means the stream will
     * use the default encoding version. A communicator is required in order to unmarshal
     * proxies. You can supply a communicator later by calling initialize().
     *
     * @param data The byte array containing encoded Slice types.
     **/
    public InputStream(byte[] data)
    {
        initialize(Protocol.currentEncoding);
        _buf = new Buffer(data);
    }

    /**
     * Constructing an InputStream without providing a communicator means the stream will
     * use the default encoding version. A communicator is required in order to unmarshal
     * proxies. You can supply a communicator later by calling initialize().
     *
     * @param buf The byte buffer containing encoded Slice types.
     **/
    public InputStream(java.nio.ByteBuffer buf)
    {
        initialize(Protocol.currentEncoding);
        _buf = new Buffer(buf);
    }

    public InputStream(Buffer buf)
    {
        this(buf, false);
    }

    public InputStream(Buffer buf, boolean adopt)
    {
        initialize(Protocol.currentEncoding);
        _buf = new Buffer(buf, adopt);
    }

    /**
     * This constructor uses the communicator's default encoding version.
     *
     * @param communicator The communicator to use when initializing the stream.
     **/
    public InputStream(Communicator communicator)
    {
        Instance instance = com.zeroc.IceInternal.Util.getInstance(communicator);
        initialize(instance, instance.defaultsAndOverrides().defaultEncoding);
        _buf = new Buffer(instance.cacheMessageBuffers() > 1);
    }

    /**
     * This constructor uses the communicator's default encoding version.
     *
     * @param communicator The communicator to use when initializing the stream.
     * @param data The byte array containing encoded Slice types.
     **/
    public InputStream(Communicator communicator, byte[] data)
    {
        initialize(communicator);
        _buf = new Buffer(data);
    }

    /**
     * This constructor uses the communicator's default encoding version.
     *
     * @param communicator The communicator to use when initializing the stream.
     * @param buf The byte buffer containing encoded Slice types.
     **/
    public InputStream(Communicator communicator, java.nio.ByteBuffer buf)
    {
        initialize(communicator);
        _buf = new Buffer(buf);
    }

    public InputStream(Communicator communicator, Buffer buf)
    {
        this(communicator, buf, false);
    }

    public InputStream(Communicator communicator, Buffer buf, boolean adopt)
    {
        initialize(communicator);
        _buf = new Buffer(buf, adopt);
    }

    /**
     * This constructor uses the given encoding version.
     *
     * @param encoding The encoding version to use when extracting data.
     **/
    public InputStream(EncodingVersion encoding)
    {
        initialize(encoding);
        _buf = new Buffer(false);
    }

    /**
     * This constructor uses the given encoding version.
     *
     * @param encoding The encoding version to use when extracting data.
     * @param data The byte array containing encoded Slice types.
     **/
    public InputStream(EncodingVersion encoding, byte[] data)
    {
        initialize(encoding);
        _buf = new Buffer(data);
    }

    /**
     * This constructor uses the given encoding version.
     *
     * @param encoding The encoding version to use when extracting data.
     * @param buf The byte buffer containing encoded Slice types.
     **/
    public InputStream(EncodingVersion encoding, java.nio.ByteBuffer buf)
    {
        initialize(encoding);
        _buf = new Buffer(buf);
    }

    public InputStream(EncodingVersion encoding, Buffer buf)
    {
        this(encoding, buf, false);
    }

    public InputStream(EncodingVersion encoding, Buffer buf, boolean adopt)
    {
        initialize(encoding);
        _buf = new Buffer(buf, adopt);
    }

    /**
     * This constructor uses the given communicator and encoding version.
     *
     * @param communicator The communicator to use when initializing the stream.
     * @param encoding The desired encoding version.
     **/
    public InputStream(Communicator communicator, EncodingVersion encoding)
    {
        Instance instance = com.zeroc.IceInternal.Util.getInstance(communicator);
        initialize(instance, encoding);
        _buf = new Buffer(instance.cacheMessageBuffers() > 1);
    }

    /**
     * This constructor uses the given communicator and encoding version.
     *
     * @param communicator The communicator to use when initializing the stream.
     * @param encoding The desired encoding version.
     * @param data The byte array containing encoded Slice types.
     **/
    public InputStream(Communicator communicator, EncodingVersion encoding, byte[] data)
    {
        initialize(communicator, encoding);
        _buf = new Buffer(data);
    }

    /**
     * This constructor uses the given communicator and encoding version.
     *
     * @param communicator The communicator to use when initializing the stream.
     * @param encoding The desired encoding version.
     * @param buf The byte buffer containing encoded Slice types.
     **/
    public InputStream(Communicator communicator, EncodingVersion encoding, java.nio.ByteBuffer buf)
    {
        initialize(communicator, encoding);
        _buf = new Buffer(buf);
    }

    public InputStream(Communicator communicator, EncodingVersion encoding, Buffer buf)
    {
        this(communicator, encoding, buf, false);
    }

    public InputStream(Communicator communicator, EncodingVersion encoding, Buffer buf, boolean adopt)
    {
        initialize(communicator, encoding);
        _buf = new Buffer(buf, adopt);
    }

    public InputStream(Instance instance, EncodingVersion encoding)
    {
        this(instance, encoding, instance.cacheMessageBuffers() > 1);
    }

    public InputStream(Instance instance, EncodingVersion encoding, boolean direct)
    {
        initialize(instance, encoding);
        _buf = new Buffer(direct);
    }

    public InputStream(Instance instance, EncodingVersion encoding, byte[] data)
    {
        initialize(instance, encoding);
        _buf = new Buffer(data);
    }

    public InputStream(Instance instance, EncodingVersion encoding, java.nio.ByteBuffer data)
    {
        initialize(instance, encoding);
        _buf = new Buffer(data);
    }

    public InputStream(Instance instance, EncodingVersion encoding, Buffer buf, boolean adopt)
    {
        initialize(instance, encoding);
        _buf = new Buffer(buf, adopt);
    }

    /**
     * Initializes the stream to use the communicator's default encoding version.
     *
     * @param communicator The communicator to use when initializing the stream.
     **/
    public void initialize(Communicator communicator)
    {
        Instance instance = com.zeroc.IceInternal.Util.getInstance(communicator);
        initialize(instance, instance.defaultsAndOverrides().defaultEncoding);
    }

    /**
     * Initializes the stream to use the given communicator and encoding version.
     *
     * @param communicator The communicator to use when initializing the stream.
     * @param encoding The desired encoding version.
     **/
    public void initialize(Communicator communicator, EncodingVersion encoding)
    {
        Instance instance = com.zeroc.IceInternal.Util.getInstance(communicator);
        initialize(instance, encoding);
    }

    private void initialize(Instance instance, EncodingVersion encoding)
    {
        initialize(encoding);

        _instance = instance;
        _traceSlicing = _instance.traceLevels().slicing > 0;

        _valueFactoryManager = _instance.initializationData().valueFactoryManager;
        _logger = _instance.initializationData().logger;
        _classResolver = _instance;
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

    /**
     * Resets this stream. This method allows the stream to be reused, to avoid creating
     * unnecessary garbage.
     **/
    public void reset()
    {
        _buf.reset();
        clear();
    }

    /**
     * Releases any data retained by encapsulations. The {@link #reset} method internally calls <code>clear</code>.
     **/
    public void clear()
    {
        if(_encapsStack != null)
        {
            assert(_encapsStack.next == null);
            _encapsStack.next = _encapsCache;
            _encapsCache = _encapsStack;
            _encapsCache.reset();
            _encapsStack = null;
        }

        _startSeq = -1;
        _sliceValues = true;
    }

    /**
     * Sets the value factory manager to use when marshaling value instances. If the stream
     * was initialized with a communicator, the communicator's value factory manager will
     * be used by default.
     *
     * @param vfm The value factory manager.
     **/
    public void setValueFactoryManager(ValueFactoryManager vfm)
    {
        _valueFactoryManager = vfm;
    }

    /**
     * Sets the logger to use when logging trace messages. If the stream
     * was initialized with a communicator, the communicator's logger will
     * be used by default.
     *
     * @param logger The logger to use for logging trace messages.
     **/
    public void setLogger(Logger logger)
    {
        _logger = logger;
    }

    /**
     * Sets the compact ID resolver to use when unmarshaling value and exception
     * instances. If the stream was initialized with a communicator, the communicator's
     * resolver will be used by default.
     *
     * @param r The compact ID resolver.
     **/
    public void setCompactIdResolver(CompactIdResolver r)
    {
        _compactIdResolver = r;
    }

    /**
     * Sets the class resolver, which the stream will use when attempting to unmarshal
     * a value or exception. If the stream was initialized with a communicator, the communicator's
     * resolver will be used by default.
     *
     * @param r The class resolver.
     **/
    public void setClassResolver(ClassResolver r)
    {
        _classResolver = r;
    }

    /**
     * Determines the behavior of the stream when extracting instances of Slice classes.
     * An instance is "sliced" when a factory cannot be found for a Slice type ID.
     * The stream's default behavior is to slice instances.
     *
     * @param b If <code>true</code> (the default), slicing is enabled; if <code>false</code>,
     * slicing is disabled. If slicing is disabled and the stream encounters a Slice type ID
     * during decoding for which no value factory is installed, it raises {@link NoValueFactoryException}.
     **/
    public void setSliceValues(boolean b)
    {
        _sliceValues = b;
    }

    /**
     * Determines whether the stream logs messages about slicing instances of Slice values.
     *
     * @param b True to enable logging, false to disable logging.
     **/
    public void setTraceSlicing(boolean b)
    {
        _traceSlicing = b;
    }

    /**
     * Retrieves the closure object associated with this stream.
     *
     * @return The closure object.
     **/
    public Object getClosure()
    {
        return _closure;
    }

    /**
     * Associates a closure object with this stream.
     *
     * @param p The new closure object.
     * @return The previous closure object, or null.
     **/
    public Object setClosure(Object p)
    {
        Object prev = _closure;
        _closure = p;
        return prev;
    }

    public Instance instance()
    {
        return _instance;
    }

    /**
     * Swaps the contents of one stream with another.
     *
     * @param other The other stream.
     **/
    public void swap(InputStream other)
    {
        assert(_instance == other._instance);

        Buffer tmpBuf = other._buf;
        other._buf = _buf;
        _buf = tmpBuf;

        EncodingVersion tmpEncoding = other._encoding;
        other._encoding = _encoding;
        _encoding = tmpEncoding;

        boolean tmpTraceSlicing = other._traceSlicing;
        other._traceSlicing = _traceSlicing;
        _traceSlicing = tmpTraceSlicing;

        Object tmpClosure = other._closure;
        other._closure = _closure;
        _closure = tmpClosure;

        boolean tmpSliceValues = other._sliceValues;
        other._sliceValues = _sliceValues;
        _sliceValues = tmpSliceValues;

        //
        // Swap is never called for streams that have encapsulations being read. However,
        // encapsulations might still be set in case unmarshaling failed. We just
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

    /**
     * Resizes the stream to a new size.
     *
     * @param sz The new size.
     **/
    public void resize(int sz)
    {
        _buf.resize(sz, true);
        _buf.b.position(sz);
    }

    public Buffer getBuffer()
    {
        return _buf;
    }

    /**
     * Marks the start of a class instance.
     **/
    public void startValue()
    {
        assert(_encapsStack != null && _encapsStack.decoder != null);
        _encapsStack.decoder.startInstance(SliceType.ValueSlice);
    }

    /**
     * Marks the end of a class instance.
     *
     * @param preserve Pass true and the stream will preserve the unknown slices of the instance, or false
     * to discard the unknown slices.
     * @return An object that encapsulates the unknown slice data.
     **/
    public SlicedData endValue(boolean preserve)
    {
        assert(_encapsStack != null && _encapsStack.decoder != null);
        return _encapsStack.decoder.endInstance(preserve);
    }

    /**
     * Marks the start of a user exception.
     **/
    public void startException()
    {
        assert(_encapsStack != null && _encapsStack.decoder != null);
        _encapsStack.decoder.startInstance(SliceType.ExceptionSlice);
    }

    /**
     * Marks the end of a user exception.
     *
     * @param preserve Pass true and the stream will preserve the unknown slices of the exception, or false
     * to discard the unknown slices.
     * @return An object that encapsulates the unknown slice data.
     **/
    public SlicedData endException(boolean preserve)
    {
        assert(_encapsStack != null && _encapsStack.decoder != null);
        return _encapsStack.decoder.endInstance(preserve);
    }

    /**
     * Reads the start of an encapsulation.
     *
     * @return The encoding version used by the encapsulation.
     **/
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

        EncodingVersion encoding = EncodingVersion.read(this, null);
        Protocol.checkSupportedEncoding(encoding); // Make sure the encoding is supported.
        _encapsStack.setEncoding(encoding);

        return encoding;
    }

    /**
     * Ends the previous encapsulation.
     **/
    public void endEncapsulation()
    {
        assert(_encapsStack != null);

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
            catch(java.nio.BufferUnderflowException ex)
            {
                throw new UnmarshalOutOfBoundsException();
            }
        }

        Encaps curr = _encapsStack;
        _encapsStack = curr.next;
        curr.next = _encapsCache;
        _encapsCache = curr;
        _encapsCache.reset();
    }

    /**
     * Skips an empty encapsulation.
     *
     * @return The encapsulation's encoding version.
     **/
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

        EncodingVersion encoding = EncodingVersion.read(this, null);
        if(encoding.equals(Util.Encoding_1_0))
        {
            if(sz != 6)
            {
                throw new EncapsulationException();
            }
        }
        else
        {
            //
            // Skip the optional content of the encapsulation if we are expecting an
            // empty encapsulation.
            //
            _buf.b.position(_buf.b.position() + sz - 6);
        }
        return encoding;
    }

    /**
     * Returns a blob of bytes representing an encapsulation. The encapsulation's encoding version
     * is returned in the argument.
     *
     * @param encoding The encapsulation's encoding version.
     * @return The encoded encapuslation.
     **/
    public byte[] readEncapsulation(EncodingVersion encoding)
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

        if(encoding != null)
        {
            encoding.read(this);
            _buf.b.position(_buf.b.position() - 6);
        }
        else
        {
            _buf.b.position(_buf.b.position() - 4);
        }

        byte[] v = new byte[sz];
        try
        {
            _buf.b.get(v);
            return v;
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new UnmarshalOutOfBoundsException();
        }
    }

    /**
     * Determines the current encoding version.
     *
     * @return The encoding version.
     **/
    public EncodingVersion getEncoding()
    {
        return _encapsStack != null ? _encapsStack.encoding : _encoding;
    }

    /**
     * Determines the size of the current encapsulation, excluding the encapsulation header.
     *
     * @return The size of the encapsulated data.
     **/
    public int getEncapsulationSize()
    {
        assert(_encapsStack != null);
        return _encapsStack.sz - 6;
    }

    /**
     * Skips over an encapsulation.
     *
     * @return The encoding version of the skipped encapsulation.
     **/
    public EncodingVersion skipEncapsulation()
    {
        int sz = readInt();
        if(sz < 6)
        {
            throw new UnmarshalOutOfBoundsException();
        }
        EncodingVersion encoding = EncodingVersion.read(this, null);
        try
        {
            _buf.b.position(_buf.b.position() + sz - 6);
        }
        catch(IllegalArgumentException ex)
        {
            throw new UnmarshalOutOfBoundsException();
        }
        return encoding;
    }

    /**
     * Reads the start of a value or exception slice.
     *
     * @return The Slice type ID for this slice.
     **/
    public String startSlice() // Returns type ID of next slice
    {
        assert(_encapsStack != null && _encapsStack.decoder != null);
        return _encapsStack.decoder.startSlice();
    }

    /**
     * Indicates that the end of a value or exception slice has been reached.
     **/
    public void endSlice()
    {
        assert(_encapsStack != null && _encapsStack.decoder != null);
        _encapsStack.decoder.endSlice();
    }

    /**
     * Skips over a value or exception slice.
     **/
    public void skipSlice()
    {
        assert(_encapsStack != null && _encapsStack.decoder != null);
        _encapsStack.decoder.skipSlice();
    }

    /**
     * Indicates that unmarshaling is complete, except for any class instances. The application must call this method
     * only if the stream actually contains class instances. Calling <code>readPendingValues</code> triggers the
     * calls to {@link ReadValueCallback#valueReady} that inform the application that unmarshaling of an instance
     * is complete.
     **/
    public void readPendingValues()
    {
        if(_encapsStack != null && _encapsStack.decoder != null)
        {
            _encapsStack.decoder.readPendingValues();
        }
        else if(_encapsStack != null ? _encapsStack.encoding_1_0 : _encoding.equals(Util.Encoding_1_0))
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

    /**
     * Extracts a size from the stream.
     *
     * @return The extracted size.
     **/
    public int readSize()
    {
        try
        {
            byte b = _buf.b.get();
            if(b == -1)
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
                return b < 0 ? b + 256 : b;
            }
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new UnmarshalOutOfBoundsException();
        }
    }

    /**
     * Reads and validates a sequence size.
     *
     * @param minSize The minimum size required by the sequence type.
     * @return The extracted size.
     **/
    public int readAndCheckSeqSize(int minSize)
    {
        int sz = readSize();

        if(sz == 0)
        {
            return sz;
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

    /**
     * Reads a blob of bytes from the stream.
     *
     * @param sz The number of bytes to read.
     * @return The requested bytes as a byte array.
     **/
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
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new UnmarshalOutOfBoundsException();
        }
    }

    /**
     * Determine if an optional value is available for reading.
     *
     * @param tag The tag associated with the value.
     * @param expectedFormat The optional format for the value.
     * @return True if the value is present, false otherwise.
     **/
    public boolean readOptional(int tag, OptionalFormat expectedFormat)
    {
        assert(_encapsStack != null);
        if(_encapsStack.decoder != null)
        {
            return _encapsStack.decoder.readOptional(tag, expectedFormat);
        }
        else
        {
            return readOptImpl(tag, expectedFormat);
        }
    }

    /**
     * Extracts a byte value from the stream.
     *
     * @return The extracted byte.
     **/
    public byte readByte()
    {
        try
        {
            return _buf.b.get();
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new UnmarshalOutOfBoundsException();
        }
    }

    /**
     * Extracts an optional byte value from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     **/
    public java.util.Optional<Byte> readByte(int tag)
    {
        if(readOptional(tag, OptionalFormat.F1))
        {
            return java.util.Optional.of(readByte());
        }
        else
        {
            return java.util.Optional.empty();
        }
    }

    /**
     * Extracts a sequence of byte values from the stream.
     *
     * @return The extracted byte sequence.
     **/
    public byte[] readByteSeq()
    {
        try
        {
            final int sz = readAndCheckSeqSize(1);
            byte[] v = new byte[sz];
            _buf.b.get(v);
            return v;
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new UnmarshalOutOfBoundsException();
        }
    }

    /**
     * Extracts an optional byte sequence from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     **/
    public java.util.Optional<byte[]> readByteSeq(int tag)
    {
        if(readOptional(tag, OptionalFormat.VSize))
        {
            return java.util.Optional.of(readByteSeq());
        }
        else
        {
            return java.util.Optional.empty();
        }
    }

    /**
     * Returns a byte buffer representing a sequence of bytes. This method does not copy the data.
     *
     * @return A byte buffer "slice" of the internal buffer.
     **/
    public java.nio.ByteBuffer readByteBuffer()
    {
        try
        {
            final int sz = readAndCheckSeqSize(1);
            java.nio.ByteBuffer v = _buf.b.slice();
            v.limit(sz);
            _buf.b.position(_buf.b.position() + sz);
            return v.asReadOnlyBuffer();
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new UnmarshalOutOfBoundsException();
        }
    }

    /**
     * Extracts a serializable Java object from the stream.
     *
     * @return The deserialized Java object.
     **/
    public java.io.Serializable readSerializable()
    {
        int sz = readAndCheckSeqSize(1);
        if (sz == 0)
        {
            return null;
        }
        com.zeroc.IceInternal.ObjectInputStream in = null;
        try
        {
            com.zeroc.IceInternal.InputStreamWrapper w = new com.zeroc.IceInternal.InputStreamWrapper(sz, _buf.b);
            in = new com.zeroc.IceInternal.ObjectInputStream(_instance, w);
            return (java.io.Serializable)in.readObject();
        }
        catch(LocalException ex)
        {
            throw ex;
        }
        catch(java.lang.Exception ex)
        {
            throw new MarshalException("cannot deserialize object", ex);
        }
        finally
        {
            if(in != null)
            {
                try
                {
                    in.close();
                }
                catch (IOException ex)
                {
                    throw new MarshalException("cannot deserialize object", ex);
                }
            }
        }
    }

    /**
     * Extracts a boolean value from the stream.
     *
     * @return The extracted boolean.
     **/
    public boolean readBool()
    {
        try
        {
            return _buf.b.get() == 1;
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new UnmarshalOutOfBoundsException();
        }
    }

    /**
     * Extracts an optional boolean value from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     **/
    public java.util.Optional<Boolean> readBool(int tag)
    {
        if(readOptional(tag, OptionalFormat.F1))
        {
            return java.util.Optional.of(readBool());
        }
        else
        {
            return java.util.Optional.empty();
        }
    }

    /**
     * Extracts a sequence of boolean values from the stream.
     *
     * @return The extracted boolean sequence.
     **/
    public boolean[] readBoolSeq()
    {
        try
        {
            final int sz = readAndCheckSeqSize(1);
            boolean[] v = new boolean[sz];
            for(int i = 0; i < sz; i++)
            {
                v[i] = _buf.b.get() == 1;
            }
            return v;
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new UnmarshalOutOfBoundsException();
        }
    }

    /**
     * Extracts an optional boolean sequence from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     **/
    public java.util.Optional<boolean[]> readBoolSeq(int tag)
    {
        if(readOptional(tag, OptionalFormat.VSize))
        {
            return java.util.Optional.of(readBoolSeq());
        }
        else
        {
            return java.util.Optional.empty();
        }
    }

    /**
     * Extracts a short value from the stream.
     *
     * @return The extracted short.
     **/
    public short readShort()
    {
        try
        {
            return _buf.b.getShort();
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new UnmarshalOutOfBoundsException();
        }
    }

    /**
     * Extracts an optional short value from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     **/
    public java.util.Optional<Short> readShort(int tag)
    {
        if(readOptional(tag, OptionalFormat.F2))
        {
            return java.util.Optional.of(readShort());
        }
        else
        {
            return java.util.Optional.empty();
        }
    }

    /**
     * Extracts a sequence of short values from the stream.
     *
     * @return The extracted short sequence.
     **/
    public short[] readShortSeq()
    {
        try
        {
            final int sz = readAndCheckSeqSize(2);
            short[] v = new short[sz];
            java.nio.ShortBuffer shortBuf = _buf.b.asShortBuffer();
            shortBuf.get(v);
            _buf.b.position(_buf.b.position() + sz * 2);
            return v;
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new UnmarshalOutOfBoundsException();
        }
    }

    /**
     * Extracts an optional short sequence from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     **/
    public java.util.Optional<short[]> readShortSeq(int tag)
    {
        if(readOptional(tag, OptionalFormat.VSize))
        {
            skipSize();
            return java.util.Optional.of(readShortSeq());
        }
        else
        {
            return java.util.Optional.empty();
        }
    }

    /**
     * Returns a short buffer representing a sequence of shorts. This method does not copy the data.
     *
     * @return A short buffer "slice" of the internal buffer.
     **/
    public java.nio.ShortBuffer readShortBuffer()
    {
        try
        {
            final int sz = readAndCheckSeqSize(2);
            java.nio.ShortBuffer shortBuf = _buf.b.asShortBuffer();
            java.nio.ShortBuffer v = shortBuf.slice();
            v.limit(sz);
            _buf.b.position(_buf.b.position() + sz * 2);
            return v.asReadOnlyBuffer();
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new UnmarshalOutOfBoundsException();
        }
    }

    /**
     * Extracts an int value from the stream.
     *
     * @return The extracted int.
     **/
    public int readInt()
    {
        try
        {
            return _buf.b.getInt();
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new UnmarshalOutOfBoundsException();
        }
    }

    /**
     * Extracts an optional int value from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     **/
    public java.util.OptionalInt readInt(int tag)
    {
        if(readOptional(tag, OptionalFormat.F4))
        {
            return java.util.OptionalInt.of(readInt());
        }
        else
        {
            return java.util.OptionalInt.empty();
        }
    }

    /**
     * Extracts a sequence of int values from the stream.
     *
     * @return The extracted int sequence.
     **/
    public int[] readIntSeq()
    {
        try
        {
            final int sz = readAndCheckSeqSize(4);
            int[] v = new int[sz];
            java.nio.IntBuffer intBuf = _buf.b.asIntBuffer();
            intBuf.get(v);
            _buf.b.position(_buf.b.position() + sz * 4);
            return v;
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new UnmarshalOutOfBoundsException();
        }
    }

    /**
     * Extracts an optional int sequence from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     **/
    public java.util.Optional<int[]> readIntSeq(int tag)
    {
        if(readOptional(tag, OptionalFormat.VSize))
        {
            skipSize();
            return java.util.Optional.of(readIntSeq());
        }
        else
        {
            return java.util.Optional.empty();
        }
    }

    /**
     * Returns an int buffer representing a sequence of ints. This method does not copy the data.
     *
     * @return An int buffer "slice" of the internal buffer.
     **/
    public java.nio.IntBuffer readIntBuffer()
    {
        try
        {
            final int sz = readAndCheckSeqSize(4);
            java.nio.IntBuffer intBuf = _buf.b.asIntBuffer();
            java.nio.IntBuffer v = intBuf.slice();
            v.limit(sz);
            _buf.b.position(_buf.b.position() + sz * 4);
            return v.asReadOnlyBuffer();
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new UnmarshalOutOfBoundsException();
        }
    }

    /**
     * Extracts a long value from the stream.
     *
     * @return The extracted long.
     **/
    public long readLong()
    {
        try
        {
            return _buf.b.getLong();
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new UnmarshalOutOfBoundsException();
        }
    }

    /**
     * Extracts an optional long value from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     **/
    public java.util.OptionalLong readLong(int tag)
    {
        if(readOptional(tag, OptionalFormat.F8))
        {
            return java.util.OptionalLong.of(readLong());
        }
        else
        {
            return java.util.OptionalLong.empty();
        }
    }

    /**
     * Extracts a sequence of long values from the stream.
     *
     * @return The extracted long sequence.
     **/
    public long[] readLongSeq()
    {
        try
        {
            final int sz = readAndCheckSeqSize(8);
            long[] v = new long[sz];
            java.nio.LongBuffer longBuf = _buf.b.asLongBuffer();
            longBuf.get(v);
            _buf.b.position(_buf.b.position() + sz * 8);
            return v;
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new UnmarshalOutOfBoundsException();
        }
    }

    /**
     * Extracts an optional long sequence from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     **/
    public java.util.Optional<long[]> readLongSeq(int tag)
    {
        if(readOptional(tag, OptionalFormat.VSize))
        {
            skipSize();
            return java.util.Optional.of(readLongSeq());
        }
        else
        {
            return java.util.Optional.empty();
        }
    }

    /**
     * Returns a long buffer representing a sequence of longs. This method does not copy the data.
     *
     * @return A long buffer "slice" of the internal buffer.
     **/
    public java.nio.LongBuffer readLongBuffer()
    {
        try
        {
            final int sz = readAndCheckSeqSize(8);
            java.nio.LongBuffer longBuf = _buf.b.asLongBuffer();
            java.nio.LongBuffer v = longBuf.slice();
            v.limit(sz);
            _buf.b.position(_buf.b.position() + sz * 8);
            return v.asReadOnlyBuffer();
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new UnmarshalOutOfBoundsException();
        }
    }

    /**
     * Extracts a float value from the stream.
     *
     * @return The extracted float.
     **/
    public float readFloat()
    {
        try
        {
            return _buf.b.getFloat();
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new UnmarshalOutOfBoundsException();
        }
    }

    /**
     * Extracts an optional float value from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     **/
    public java.util.Optional<Float> readFloat(int tag)
    {
        if(readOptional(tag, OptionalFormat.F4))
        {
            return java.util.Optional.of(readFloat());
        }
        else
        {
            return java.util.Optional.empty();
        }
    }

    /**
     * Extracts a sequence of float values from the stream.
     *
     * @return The extracted float sequence.
     **/
    public float[] readFloatSeq()
    {
        try
        {
            final int sz = readAndCheckSeqSize(4);
            float[] v = new float[sz];
            java.nio.FloatBuffer floatBuf = _buf.b.asFloatBuffer();
            floatBuf.get(v);
            _buf.b.position(_buf.b.position() + sz * 4);
            return v;
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new UnmarshalOutOfBoundsException();
        }
    }

    /**
     * Extracts an optional float sequence from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     **/
    public java.util.Optional<float[]> readFloatSeq(int tag)
    {
        if(readOptional(tag, OptionalFormat.VSize))
        {
            skipSize();
            return java.util.Optional.of(readFloatSeq());
        }
        else
        {
            return java.util.Optional.empty();
        }
    }

    /**
     * Returns a float buffer representing a sequence of floats. This method does not copy the data.
     *
     * @return A float buffer "slice" of the internal buffer.
     **/
    public java.nio.FloatBuffer readFloatBuffer()
    {
        try
        {
            final int sz = readAndCheckSeqSize(4);
            java.nio.FloatBuffer floatBuf = _buf.b.asFloatBuffer();
            java.nio.FloatBuffer v = floatBuf.slice();
            v.limit(sz);
            _buf.b.position(_buf.b.position() + sz * 4);
            return v.asReadOnlyBuffer();
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new UnmarshalOutOfBoundsException();
        }
    }

    /**
     * Extracts a double value from the stream.
     *
     * @return The extracted double.
     **/
    public double readDouble()
    {
        try
        {
            return _buf.b.getDouble();
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new UnmarshalOutOfBoundsException();
        }
    }

    /**
     * Extracts an optional double value from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     **/
    public java.util.OptionalDouble readDouble(int tag)
    {
        if(readOptional(tag, OptionalFormat.F8))
        {
            return java.util.OptionalDouble.of(readDouble());
        }
        else
        {
            return java.util.OptionalDouble.empty();
        }
    }

    /**
     * Extracts a sequence of double values from the stream.
     *
     * @return The extracted double sequence.
     **/
    public double[] readDoubleSeq()
    {
        try
        {
            final int sz = readAndCheckSeqSize(8);
            double[] v = new double[sz];
            java.nio.DoubleBuffer doubleBuf = _buf.b.asDoubleBuffer();
            doubleBuf.get(v);
            _buf.b.position(_buf.b.position() + sz * 8);
            return v;
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new UnmarshalOutOfBoundsException();
        }
    }

    /**
     * Extracts an optional double sequence from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     **/
    public java.util.Optional<double[]> readDoubleSeq(int tag)
    {
        if(readOptional(tag, OptionalFormat.VSize))
        {
            skipSize();
            return java.util.Optional.of(readDoubleSeq());
        }
        else
        {
            return java.util.Optional.empty();
        }
    }

    /**
     * Returns a double buffer representing a sequence of doubles. This method does not copy the data.
     *
     * @return A double buffer "slice" of the internal buffer.
     **/
    public java.nio.DoubleBuffer readDoubleBuffer()
    {
        try
        {
            final int sz = readAndCheckSeqSize(8);
            java.nio.DoubleBuffer doubleBuf = _buf.b.asDoubleBuffer();
            java.nio.DoubleBuffer v = doubleBuf.slice();
            v.limit(sz);
            _buf.b.position(_buf.b.position() + sz * 8);
            return v.asReadOnlyBuffer();
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new UnmarshalOutOfBoundsException();
        }
    }

    final static java.nio.charset.Charset _utf8 = java.nio.charset.Charset.forName("UTF8");
    private java.nio.charset.CharsetEncoder _charEncoder = null;

    /**
     * Extracts a string from the stream.
     *
     * @return The extracted string.
     **/
    public String readString()
    {
        final int len = readSize();

        if(len == 0)
        {
            return "";
        }
        else
        {
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
                // excessive garbage.
                //
                if(_stringBytes == null || len > _stringBytes.length)
                {
                    _stringBytes = new byte[len];
                }
                if(_stringChars == null || len > _stringChars.length)
                {
                    _stringChars = new char[len];
                }
                _buf.b.get(_stringBytes, 0, len);

                //
                // It's more efficient to construct a string using a
                // character array instead of a byte array, because
                // byte arrays require conversion.
                //
                for(int i = 0; i < len; i++)
                {
                    if(_stringBytes[i] < 0)
                    {
                        //
                        // Multi-byte character found - we must use
                        // conversion.
                        //
                        // TODO: If the string contains garbage bytes
                        // that won't correctly decode as UTF, the
                        // behavior of this constructor is
                        // undefined. It would be better to explicitly
                        // decode using
                        // java.nio.charset.CharsetDecoder and to
                        // throw MarshalException if the string won't
                        // decode.
                        //
                        return new String(_stringBytes, 0, len, "UTF8");
                    }
                    else
                    {
                        _stringChars[i] = (char)_stringBytes[i];
                    }
                }
                return new String(_stringChars, 0, len);
            }
            catch(java.io.UnsupportedEncodingException ex)
            {
                assert(false);
                return "";
            }
            catch(java.nio.BufferUnderflowException ex)
            {
                throw new UnmarshalOutOfBoundsException();
            }
        }
    }

    /**
     * Extracts an optional string value from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     **/
    public java.util.Optional<String> readString(int tag)
    {
        if(readOptional(tag, OptionalFormat.VSize))
        {
            return java.util.Optional.of(readString());
        }
        else
        {
            return java.util.Optional.empty();
        }
    }

    /**
     * Extracts a sequence of string values from the stream.
     *
     * @return The extracted string sequence.
     **/
    public String[] readStringSeq()
    {
        final int sz = readAndCheckSeqSize(1);
        String[] v = new String[sz];
        for(int i = 0; i < sz; i++)
        {
            v[i] = readString();
        }
        return v;
    }

    /**
     * Extracts an optional string sequence from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     **/
    public java.util.Optional<String[]> readStringSeq(int tag)
    {
        if(readOptional(tag, OptionalFormat.FSize))
        {
            skip(4);
            return java.util.Optional.of(readStringSeq());
        }
        else
        {
            return java.util.Optional.empty();
        }
    }

    /**
     * Extracts a proxy from the stream. The stream must have been initialized with a communicator.
     *
     * @return The extracted proxy.
     **/
    public ObjectPrx readProxy()
    {
        if(_instance == null)
        {
            throw new MarshalException("cannot unmarshal a proxy without a communicator");
        }

        return _instance.proxyFactory().streamToProxy(this);
    }

    /**
     * Extracts an optional proxy from the stream. The stream must have been initialized with a communicator.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     **/
    public java.util.Optional<ObjectPrx> readProxy(int tag)
    {
        if(readOptional(tag, OptionalFormat.FSize))
        {
            skip(4);
            return java.util.Optional.of(readProxy());
        }
        else
        {
            return java.util.Optional.empty();
        }
    }

    /**
     * Read an enumerated value.
     *
     * @param maxValue The maximum enumerator value in the definition.
     * @return The enumerator.
     **/
    public int readEnum(int maxValue)
    {
        if(getEncoding().equals(Util.Encoding_1_0))
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

    /**
     * Extracts the index of a Slice value from the stream.
     *
     * @param cb The callback to notify the application when the extracted instance is available.
     * The stream extracts Slice values in stages. The Ice run time calls {@link ReadValueCallback#valueReady}
     * when the corresponding instance has been fully unmarshaled.
     *
     * @see ReadValueCallback
     **/
    public void readValue(ReadValueCallback cb)
    {
        initEncaps();
        _encapsStack.decoder.readValue(cb);
    }

    /**
     * Extracts the index of an optional Slice value from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @param cb The callback to notify the application when the extracted instance is available.
     * The stream extracts Slice values in stages. The Ice run time calls {@link ReadValueCallback#valueReady}
     * when the corresponding instance has been fully unmarshaled.
     *
     * @see ReadValueCallback
     **/
    public void readValue(int tag, ReadValueCallback cb)
    {
        if(readOptional(tag, OptionalFormat.Class))
        {
            readValue(cb);
        }
        else
        {
            cb.valueReady(null);
        }
    }

    /**
     * Extracts a user exception from the stream and throws it.
     *
     * @throws UserException The user exception that was unmarshaled.
     **/
    public void throwException()
        throws UserException
    {
        throwException(null);
    }

    /**
     * Extracts a user exception from the stream and throws it. The caller can supply a factory
     * to instantiate exception instances.
     *
     * @param factory The user exception factory, or null to use the stream's default behavior.
     *
     * @throws UserException The user exception that was unmarshaled.
     **/
    public void throwException(UserExceptionFactory factory)
        throws UserException
    {
        initEncaps();
        _encapsStack.decoder.throwException(factory);
    }

    private boolean readOptImpl(int readTag, OptionalFormat expectedFormat)
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

            final byte b = readByte();
            final int v = b < 0 ? b + 256 : b;
            if(v == Protocol.OPTIONAL_END_MARKER)
            {
                _buf.b.position(_buf.b.position() - 1); // Rewind.
                return false;
            }

            OptionalFormat format = OptionalFormat.valueOf(v & 0x07); // First 3 bits.
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
        case F1:
        {
            skip(1);
            break;
        }
        case F2:
        {
            skip(2);
            break;
        }
        case F4:
        {
            skip(4);
            break;
        }
        case F8:
        {
            skip(8);
            break;
        }
        case Size:
        {
            skipSize();
            break;
        }
        case VSize:
        {
            skip(readSize());
            break;
        }
        case FSize:
        {
            skip(readInt());
            break;
        }
        case Class:
        {
            readValue(null);
            break;
        }
        }
    }

    private void skipOptionals()
    {
        //
        // Skip remaining un-read optional members.
        //
        while(true)
        {
            if(_buf.b.position() >= _encapsStack.start + _encapsStack.sz)
            {
                return; // End of encapsulation also indicates end of optionals.
            }

            final byte b = readByte();
            final int v = b < 0 ? b + 256 : b;
            if(v == Protocol.OPTIONAL_END_MARKER)
            {
                return;
            }

            OptionalFormat format = OptionalFormat.valueOf(v & 0x07); // Read first 3 bits.
            if((v >> 3) == 30)
            {
                skipSize();
            }
            skipOptional(format);
        }
    }

    /**
     * Skip the given number of bytes.
     *
     * @param size The number of bytes to skip.
     **/
    public void skip(int size)
    {
        if(size < 0 || size > _buf.b.remaining())
        {
            throw new UnmarshalOutOfBoundsException();
        }
        _buf.b.position(_buf.b.position() + size);
    }

    /**
     * Skip over a size value.
     **/
    public void skipSize()
    {
        byte b = readByte();
        if(b == -1)
        {
            skip(4);
        }
    }

    /**
     * Determines the current position in the stream.
     *
     * @return The current position.
     **/
    public int pos()
    {
        return _buf.b.position();
    }

    /**
     * Sets the current position in the stream.
     *
     * @param n The new position.
     **/
    public void pos(int n)
    {
        _buf.b.position(n);
    }

    /**
     * Determines the current size of the stream.
     *
     * @return The current size.
     **/
    public int size()
    {
        return _buf.size();
    }

    /**
     * Determines whether the stream is empty.
     *
     * @return True if the internal buffer has no data, false otherwise.
     **/
    public boolean isEmpty()
    {
        return _buf.empty();
    }

    private UserException createUserException(String id)
    {
        UserException userEx = null;

        try
        {
            if(_classResolver != null)
            {
                Class<?> c = _classResolver.resolveClass(id);
                if(c != null)
                {
                    userEx = (UserException)c.newInstance();
                }
            }
        }
        catch(java.lang.Exception ex)
        {
            throw new MarshalException(ex);
        }

        return userEx;
    }

    private Instance _instance;
    private Buffer _buf;
    private Object _closure;
    private byte[] _stringBytes; // Reusable array for reading strings.
    private char[] _stringChars; // Reusable array for reading strings.

    private enum SliceType { NoSlice, ValueSlice, ExceptionSlice }

    abstract private static class EncapsDecoder
    {
        EncapsDecoder(InputStream stream, boolean sliceValues, ValueFactoryManager f, ClassResolver cr)
        {
            _stream = stream;
            _sliceValues = sliceValues;
            _valueFactoryManager = f;
            _classResolver = cr;
            _typeIdIndex = 0;
            _unmarshaledMap = new java.util.TreeMap<>();
        }

        abstract void readValue(ReadValueCallback cb);
        abstract void throwException(UserExceptionFactory factory)
            throws UserException;

        abstract void startInstance(SliceType type);
        abstract SlicedData endInstance(boolean preserve);
        abstract String startSlice();
        abstract void endSlice();
        abstract void skipSlice();

        boolean readOptional(int tag, OptionalFormat format)
        {
            return false;
        }

        void readPendingValues()
        {
        }

        protected String readTypeId(boolean isIndex)
        {
            if(_typeIdMap == null) // Lazy initialization
            {
                _typeIdMap = new java.util.TreeMap<>();
            }

            if(isIndex)
            {
                int index = _stream.readSize();
                String typeId = _typeIdMap.get(index);
                if(typeId == null)
                {
                    throw new UnmarshalOutOfBoundsException();
                }
                return typeId;
            }
            else
            {
                String typeId = _stream.readString();
                _typeIdMap.put(++_typeIdIndex, typeId);
                return typeId;
            }
        }

        protected Class<?> resolveClass(String typeId)
        {
            Class<?> cls = null;
            if(_typeIdCache == null)
            {
                _typeIdCache = new java.util.HashMap<>(); // Lazy initialization.
            }
            else
            {
                cls = _typeIdCache.get(typeId);
            }

            if(cls == EncapsDecoder.class) // Marker for non-existent class.
            {
                cls = null;
            }
            else if(cls == null)
            {
                try
                {
                    if(_classResolver != null)
                    {
                        cls = _classResolver.resolveClass(typeId);
                        _typeIdCache.put(typeId, cls != null ? cls : EncapsDecoder.class);
                    }
                }
                catch(java.lang.Exception ex)
                {
                    throw new NoValueFactoryException("no value factory", typeId, ex);
                }
            }

            return cls;
        }

        protected Value newInstance(String typeId)
        {
            //
            // Try to find a factory registered for the specific type.
            //
            ValueFactory userFactory = _valueFactoryManager.find(typeId);
            Value v = null;
            if(userFactory != null)
            {
                v = userFactory.create(typeId);
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
                    v = userFactory.create(typeId);
                }
            }

            //
            // Last chance: try to instantiate the class dynamically.
            //
            if(v == null)
            {
                Class<?> cls = resolveClass(typeId);

                if(cls != null)
                {
                    try
                    {
                        v = (Value)cls.newInstance();
                    }
                    catch(java.lang.Exception ex)
                    {
                        throw new NoValueFactoryException("no value factory", typeId, ex);
                    }
                }
            }

            return v;
        }

        protected void addPatchEntry(int index, ReadValueCallback cb)
        {
            assert(index > 0);

            //
            // Check if we have already unmarshalled the instance. If that's the case,
            // just invoke the callback and we're done.
            //
            Value obj = _unmarshaledMap.get(index);
            if(obj != null)
            {
                cb.valueReady(obj);
                return;
            }

            if(_patchMap == null) // Lazy initialization
            {
                _patchMap = new java.util.TreeMap<>();
            }

            //
            // Add patch entry if the instance isn't unmarshaled yet,
            // the callback will be called when the instance is
            // unmarshaled.
            //
            java.util.LinkedList<ReadValueCallback> l = _patchMap.get(index);
            if(l == null)
            {
                //
                // We have no outstanding instances to be patched for this
                // index, so make a new entry in the patch map.
                //
                l = new java.util.LinkedList<>();
                _patchMap.put(index, l);
            }

            //
            // Append a patch entry for this instance.
            //
            l.add(cb);
        }

        protected void unmarshal(int index, Value v)
        {
            //
            // Add the instance to the map of unmarshaled instances, this must
            // be done before reading the instances (for circular references).
            //
            _unmarshaledMap.put(index, v);

            //
            // Read the instance.
            //
            v._iceRead(_stream);

            if(_patchMap != null)
            {
                //
                // Patch all instances now that the instance is unmarshaled.
                //
                java.util.LinkedList<ReadValueCallback> l = _patchMap.get(index);
                if(l != null)
                {
                    assert(l.size() > 0);

                    //
                    // Patch all pointers that refer to the instance.
                    //
                    for(ReadValueCallback cb : l)
                    {
                        cb.valueReady(v);
                    }

                    //
                    // Clear out the patch map for that index -- there is nothing left
                    // to patch for that index for the time being.
                    //
                    _patchMap.remove(index);
                }
            }

            if((_patchMap == null || _patchMap.isEmpty()) && _valueList == null)
            {
                try
                {
                    v.ice_postUnmarshal();
                }
                catch(java.lang.Exception ex)
                {
                    String s = "exception raised by ice_postUnmarshal:\n" + com.zeroc.IceInternal.Ex.toString(ex);
                    _stream.instance().initializationData().logger.warning(s);
                }
            }
            else
            {
                if(_valueList == null) // Lazy initialization
                {
                    _valueList = new java.util.ArrayList<>();
                }
                _valueList.add(v);

                if(_patchMap == null || _patchMap.isEmpty())
                {
                    //
                    // Iterate over the instance list and invoke ice_postUnmarshal on
                    // each instance. We must do this after all instances have been
                    // unmarshaled in order to ensure that any instance data members
                    // have been properly patched.
                    //
                    for(Value p : _valueList)
                    {
                        try
                        {
                            p.ice_postUnmarshal();
                        }
                        catch(java.lang.Exception ex)
                        {
                            String s = "exception raised by ice_postUnmarshal:\n" +
                                com.zeroc.IceInternal.Ex.toString(ex);
                            _stream.instance().initializationData().logger.warning(s);
                        }
                    }
                    _valueList.clear();
                }
            }
        }

        protected final InputStream _stream;
        protected final boolean _sliceValues;
        protected ValueFactoryManager _valueFactoryManager;
        protected ClassResolver _classResolver;

        //
        // Encapsulation attributes for value unmarshaling.
        //
        protected java.util.TreeMap<Integer, java.util.LinkedList<ReadValueCallback> > _patchMap;
        private java.util.TreeMap<Integer, Value> _unmarshaledMap;
        private java.util.TreeMap<Integer, String> _typeIdMap;
        private int _typeIdIndex;
        private java.util.List<Value> _valueList;
        private java.util.HashMap<String, Class<?> > _typeIdCache;
    }

    private static final class EncapsDecoder10 extends EncapsDecoder
    {
        EncapsDecoder10(InputStream stream, boolean sliceValues, ValueFactoryManager f, ClassResolver cr)
        {
            super(stream, sliceValues, f, cr);
            _sliceType = SliceType.NoSlice;
        }

        @Override
        void readValue(ReadValueCallback cb)
        {
            assert(cb != null);

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
                cb.valueReady(null);
            }
            else
            {
                addPatchEntry(index, cb);
            }
        }

        @Override
        void throwException(UserExceptionFactory factory)
            throws UserException
        {
            assert(_sliceType == SliceType.NoSlice);

            //
            // User exception with the 1.0 encoding start with a boolean flag
            // that indicates whether or not the exception has classes.
            //
            // This allows reading the pending instances even if some part of
            // the exception was sliced.
            //
            boolean usesClasses = _stream.readBool();

            _sliceType = SliceType.ExceptionSlice;
            _skipFirstSlice = false;

            //
            // Read the first slice header.
            //
            startSlice();
            final String mostDerivedId = _typeId;
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
                        factory.createAndThrow(_typeId);
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
                    userEx._read(_stream);
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
                    throw ex;
                }
            }
        }

        @Override
        void startInstance(SliceType sliceType)
        {
            assert(_sliceType == sliceType);
            _skipFirstSlice = true;
        }

        @Override
        SlicedData endInstance(boolean preserve)
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

        @Override
        String startSlice()
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
            // For class instances, first read the type ID boolean which indicates
            // whether or not the type ID is encoded as a string or as an
            // index. For exceptions, the type ID is always encoded as a
            // string.
            //
            if(_sliceType == SliceType.ValueSlice) // For exceptions, the type ID is always encoded as a string
            {
                boolean isIndex = _stream.readBool();
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

        @Override
        void endSlice()
        {
        }

        @Override
        void skipSlice()
        {
            _stream.traceSkipSlice(_typeId, _sliceType);
            assert(_sliceSize >= 4);
            _stream.skip(_sliceSize - 4);
        }

        @Override
        void readPendingValues()
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

            if(_patchMap != null && !_patchMap.isEmpty())
            {
                //
                // If any entries remain in the patch map, the sender has sent an index for an object, but failed
                // to supply the object.
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
            final String mostDerivedId = _typeId;
            Value v = null;
            while(true)
            {
                //
                // For the 1.0 encoding, the type ID for the base Object class
                // marks the last slice.
                //
                if(_typeId.equals(Value.ice_staticId()))
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

        // Value/exception attributes
        private SliceType _sliceType;
        private boolean _skipFirstSlice;

        // Slice attributes
        private int _sliceSize;
        private String _typeId;
    }

    private static class EncapsDecoder11 extends EncapsDecoder
    {
        EncapsDecoder11(InputStream stream, boolean sliceValues, ValueFactoryManager f, ClassResolver cr,
                        CompactIdResolver r)
        {
            super(stream, sliceValues, f, cr);
            _compactIdResolver = r;
            _current = null;
            _valueIdIndex = 1;
        }

        @Override
        void readValue(ReadValueCallback cb)
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
                    cb.valueReady(null);
                }
            }
            else if(_current != null && (_current.sliceFlags & Protocol.FLAG_HAS_INDIRECTION_TABLE) != 0)
            {
                //
                // When reading a class instance within a slice and there's an
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
                    if(_current.indirectPatchList == null) // Lazy initialization
                    {
                        _current.indirectPatchList = new java.util.ArrayDeque<>();
                    }
                    IndirectPatchEntry e = new IndirectPatchEntry();
                    e.index = index - 1;
                    e.cb = cb;
                    _current.indirectPatchList.push(e);
                }
            }
            else
            {
                readInstance(index, cb);
            }
        }

        @Override
        void throwException(UserExceptionFactory factory)
            throws UserException
        {
            assert(_current == null);

            push(SliceType.ExceptionSlice);

            //
            // Read the first slice header.
            //
            startSlice();
            final String mostDerivedId = _current.typeId;
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
                        factory.createAndThrow(_current.typeId);
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
                    userEx._read(_stream);
                    throw userEx;

                    // Never reached.
                }

                //
                // Slice off what we don't understand.
                //
                skipSlice();

                if((_current.sliceFlags & Protocol.FLAG_IS_LAST_SLICE) != 0)
                {
                    if(mostDerivedId.startsWith("::"))
                    {
                        throw new UnknownUserException(mostDerivedId.substring(2));
                    }
                    else
                    {
                        throw new UnknownUserException(mostDerivedId);
                    }
                }

                startSlice();
            }
        }

        @Override
        void startInstance(SliceType sliceType)
        {
            assert(_current.sliceType == sliceType);
            _current.skipFirstSlice = true;
        }

        @Override
        SlicedData endInstance(boolean preserve)
        {
            SlicedData slicedData = null;
            if(preserve)
            {
                slicedData = readSlicedData();
            }
            if(_current.slices != null)
            {
                _current.slices.clear();
                _current.indirectionTables.clear();
            }
            _current = _current.previous;
            return slicedData;
        }

        @Override
        String startSlice()
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
            // Read the type ID, for value slices the type ID is encoded as a
            // string or as an index, for exceptions it's always encoded as a
            // string.
            //
            if(_current.sliceType == SliceType.ValueSlice)
            {
                if((_current.sliceFlags & Protocol.FLAG_HAS_TYPE_ID_COMPACT) ==
                    Protocol.FLAG_HAS_TYPE_ID_COMPACT) // Must be checked 1st!
                {
                    _current.typeId = "";
                    _current.compactId = _stream.readSize();
                }
                else if((_current.sliceFlags & (Protocol.FLAG_HAS_TYPE_ID_INDEX |
                            Protocol.FLAG_HAS_TYPE_ID_STRING)) != 0)
                {
                    _current.typeId =
                        readTypeId((_current.sliceFlags & Protocol.FLAG_HAS_TYPE_ID_INDEX) != 0);
                    _current.compactId = -1;
                }
                else
                {
                    //
                    // Only the most derived slice encodes the type ID for the compact format.
                    //
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

        @Override
        void endSlice()
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
                for(int i = 0; i < indirectionTable.length; ++i)
                {
                    indirectionTable[i] = readInstance(_stream.readSize(), null);
                }

                //
                // Sanity checks. If there are optional members, it's possible
                // that not all instance references were read if they are from
                // unknown optional data members.
                //
                if(indirectionTable.length == 0)
                {
                    throw new MarshalException("empty indirection table");
                }
                if((_current.indirectPatchList == null || _current.indirectPatchList.isEmpty()) &&
                   (_current.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS) == 0)
                {
                    throw new MarshalException("no references to indirection table");
                }

                //
                // Convert indirect references into direct references.
                //
                if(_current.indirectPatchList != null)
                {
                    for(IndirectPatchEntry e : _current.indirectPatchList)
                    {
                        assert(e.index >= 0);
                        if(e.index >= indirectionTable.length)
                        {
                            throw new MarshalException("indirection out of range");
                        }
                        addPatchEntry(indirectionTable[e.index], e.cb);
                    }
                    _current.indirectPatchList.clear();
                }
            }
        }

        @Override
        void skipSlice()
        {
            _stream.traceSkipSlice(_current.typeId, _current.sliceType);

            int start = _stream.pos();

            if((_current.sliceFlags & Protocol.FLAG_HAS_SLICE_SIZE) != 0)
            {
                assert(_current.sliceSize >= 4);
                _stream.skip(_current.sliceSize - 4);
            }
            else
            {
                if(_current.sliceType == SliceType.ValueSlice)
                {
                    throw new NoValueFactoryException("no value factory found and compact format prevents " +
                                                      "slicing (the sender should use the sliced format instead)",
                                                      _current.typeId);
                }
                else
                {
                    if(_current.typeId.startsWith("::"))
                    {
                        throw new UnknownUserException(_current.typeId.substring(2));
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
            java.nio.ByteBuffer b = _stream.getBuffer().b;
            final int end = b.position();
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

            if(_current.slices == null) // Lazy initialization
            {
                _current.slices = new java.util.ArrayList<>();
                _current.indirectionTables = new java.util.ArrayList<>();
            }

            //
            // Read the indirect instance table. We read the instances or their
            // IDs if the instance is a reference to an already unmarhsaled
            // instance.
            //
            // The SliceInfo object sequence is initialized only if
            // readSlicedData is called.
            //

            if((_current.sliceFlags & Protocol.FLAG_HAS_INDIRECTION_TABLE) != 0)
            {
                int[] indirectionTable = new int[_stream.readAndCheckSeqSize(1)];
                for(int i = 0; i < indirectionTable.length; ++i)
                {
                    indirectionTable[i] = readInstance(_stream.readSize(), null);
                }
                _current.indirectionTables.add(indirectionTable);
            }
            else
            {
                _current.indirectionTables.add(null);
            }

            _current.slices.add(info);
        }

        @Override
        boolean readOptional(int readTag, OptionalFormat expectedFormat)
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
            assert(index > 0);

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
            // slices are skipped, the indirect instance table is still read and
            // might read other instances.
            //
            index = ++_valueIdIndex;

            //
            // Read the first slice header.
            //
            startSlice();
            final String mostDerivedId = _current.typeId;
            Value v = null;
            while(true)
            {
                boolean updateCache = false;

                if(_current.compactId >= 0)
                {
                    updateCache = true;

                    //
                    // Translate a compact (numeric) type ID into a class.
                    //
                    if(_compactIdCache == null)
                    {
                        _compactIdCache = new java.util.TreeMap<>(); // Lazy initialization.
                    }
                    else
                    {
                        //
                        // Check the cache to see if we've already translated the compact type ID into a class.
                        //
                        Class<?> cls = _compactIdCache.get(_current.compactId);
                        if(cls != null)
                        {
                            try
                            {
                                v = (Value)cls.newInstance();
                                updateCache = false;
                            }
                            catch(java.lang.Exception ex)
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
                                _current.typeId = _compactIdResolver.resolve(_current.compactId);
                            }
                            catch(LocalException ex)
                            {
                                throw ex;
                            }
                            catch(Throwable ex)
                            {
                                throw new MarshalException("exception in CompactIdResolver for ID " +
                                                           _current.compactId, ex);
                            }
                        }

                        if(_current.typeId.isEmpty())
                        {
                            _current.typeId = _stream.instance().resolveCompactId(_current.compactId);
                        }
                    }
                }

                if(v == null && !_current.typeId.isEmpty())
                {
                    v = newInstance(_current.typeId);
                }

                if(v != null)
                {
                    if(updateCache)
                    {
                        assert(_current.compactId >= 0);
                        _compactIdCache.put(_current.compactId, v.getClass());
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

            if(_current == null && _patchMap != null && !_patchMap.isEmpty())
            {
                //
                // If any entries remain in the patch map, the sender has sent an index for an instance, but failed
                // to supply the instance.
                //
                throw new MarshalException("index for class received, but no instance");
            }

            if(cb != null)
            {
                cb.valueReady(v);
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
            assert(_current.slices.size() == _current.indirectionTables.size());
            for(int n = 0; n < _current.slices.size(); ++n)
            {
                //
                // We use the "instances" list in SliceInfo to hold references
                // to the target instances. Note that the instances might not have
                // been read yet in the case of a circular reference to an
                // enclosing instance.
                //
                final int[] table = _current.indirectionTables.get(n);
                SliceInfo info = _current.slices.get(n);
                info.instances = new Value[table != null ? table.length : 0];
                for(int j = 0; j < info.instances.length; ++j)
                {
                    addPatchEntry(table[j],
                                  new SequencePatcher<Value>(info.instances, Value.class, Value.ice_staticId(), j));
                }
            }

            SliceInfo[] arr = new SliceInfo[_current.slices.size()];
            _current.slices.toArray(arr);
            return new SlicedData(arr);
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

        private static final class IndirectPatchEntry
        {
            int index;
            ReadValueCallback cb;
        }

        private static final class InstanceData
        {
            InstanceData(InstanceData previous)
            {
                if(previous != null)
                {
                    previous.next = this;
                }
                this.previous = previous;
                this.next = null;
            }

            // Instance attributes
            SliceType sliceType;
            boolean skipFirstSlice;
            java.util.List<SliceInfo> slices;     // Preserved slices.
            java.util.List<int[]> indirectionTables;

            // Slice attributes
            byte sliceFlags;
            int sliceSize;
            String typeId;
            int compactId;
            java.util.Deque<IndirectPatchEntry> indirectPatchList;

            final InstanceData previous;
            InstanceData next;
        }

        private CompactIdResolver _compactIdResolver;
        private InstanceData _current;
        private int _valueIdIndex; // The ID of the next instance to unmarshal.
        private java.util.TreeMap<Integer, Class<?> > _compactIdCache; // Cache of compact type IDs.
    }

    private static final class Encaps
    {
        void reset()
        {
            decoder = null;
        }

        void setEncoding(EncodingVersion encoding)
        {
            this.encoding = encoding;
            encoding_1_0 = encoding.equals(Util.Encoding_1_0);
        }

        int start;
        int sz;
        EncodingVersion encoding;
        boolean encoding_1_0;

        EncapsDecoder decoder;

        Encaps next;
    }

    //
    // The encoding version to use when there's no encapsulation to
    // read from. This is for example used to read message headers.
    //
    private EncodingVersion _encoding;

    private boolean isEncoding_1_0()
    {
        return _encapsStack != null ? _encapsStack.encoding_1_0 : _encoding.equals(Util.Encoding_1_0);
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
                _encapsStack.decoder = new EncapsDecoder10(this, _sliceValues, _valueFactoryManager, _classResolver);
            }
            else
            {
                _encapsStack.decoder = new EncapsDecoder11(this, _sliceValues, _valueFactoryManager, _classResolver,
                                                           _compactIdResolver);
            }
        }
    }

    private void traceSkipSlice(String typeId, SliceType sliceType)
    {
        if(_traceSlicing && _logger != null)
        {
            com.zeroc.IceInternal.TraceUtil.traceSlicing(
                sliceType == SliceType.ExceptionSlice ? "exception" : "object", typeId, "Slicing", _logger);
        }
    }

    @FunctionalInterface
    static public interface Unmarshaler
    {
        void unmarshal(InputStream istr);
    }

    private boolean _sliceValues;
    private boolean _traceSlicing;

    private int _startSeq;
    private int _minSeqSize;

    private ValueFactoryManager _valueFactoryManager;
    private Logger _logger;
    private CompactIdResolver _compactIdResolver;
    private ClassResolver _classResolver;
}
