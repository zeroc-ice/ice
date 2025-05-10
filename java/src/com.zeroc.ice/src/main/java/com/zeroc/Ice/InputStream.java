// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.io.IOException;
import java.io.ObjectStreamClass;
import java.io.Serializable;
import java.io.UnsupportedEncodingException;
import java.nio.BufferUnderflowException;
import java.nio.ByteBuffer;
import java.nio.DoubleBuffer;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;
import java.nio.LongBuffer;
import java.nio.ShortBuffer;
import java.nio.charset.Charset;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Deque;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Optional;
import java.util.OptionalDouble;
import java.util.OptionalInt;
import java.util.OptionalLong;
import java.util.TreeMap;
import java.util.function.Consumer;
import java.util.function.Function;

/**
 * Interface to read sequence of bytes encoded using the Ice encoding and recreate the corresponding
 * Slice types.
 *
 * @see OutputStream
 */
public final class InputStream {
    /**
     * This constructor uses the communicator's default encoding version.
     *
     * @param communicator The communicator to use when initializing the stream.
     * @param data The byte array containing encoded Slice types.
     */
    public InputStream(Communicator communicator, byte[] data) {
        this(
            communicator.getInstance(),
            communicator.getInstance().defaultsAndOverrides().defaultEncoding,
            new Buffer(data));
    }

    /**
     * This constructor uses the communicator's default encoding version.
     *
     * @param communicator The communicator to use when initializing the stream.
     * @param buf The byte buffer containing encoded Slice types.
     */
    public InputStream(Communicator communicator, ByteBuffer buf) {
        this(
            communicator.getInstance(),
            communicator.getInstance().defaultsAndOverrides().defaultEncoding,
            new Buffer(buf));
    }

    /**
     * This constructor uses the given communicator and encoding version.
     *
     * @param communicator The communicator to use when initializing the stream.
     * @param encoding The desired encoding version.
     * @param data The byte array containing encoded Slice types.
     */
    public InputStream(Communicator communicator, EncodingVersion encoding, byte[] data) {
        this(communicator.getInstance(), encoding, new Buffer(data));
    }

    /**
     * This constructor uses the given communicator and encoding version.
     *
     * @param communicator The communicator to use when initializing the stream.
     * @param encoding The desired encoding version.
     * @param buf The byte buffer containing encoded Slice types.
     */
    public InputStream(
            Communicator communicator, EncodingVersion encoding, ByteBuffer buf) {
        this(communicator.getInstance(), encoding, new Buffer(buf));
    }

    /** Constructs an InputStream with an empty buffer. */
    InputStream(Instance instance, EncodingVersion encoding, boolean direct) {
        // Create an empty non-direct buffer.
        this(instance, encoding, new Buffer(direct));
    }

    InputStream(Instance instance, EncodingVersion encoding, Buffer buf, boolean adopt) {
        this(instance, encoding, new Buffer(buf, adopt));
    }

    /** The primary constructor called by all other constructors. */
    private InputStream(Instance instance, EncodingVersion encoding, Buffer buf) {
        _instance = instance;
        _encoding = encoding;
        _buf = buf;

        // Everything below is cached from instance.
        _classGraphDepthMax = _instance.classGraphDepthMax();
    }

    /**
     * Resets this stream. This method allows the stream to be reused, to avoid creating unnecessary
     * garbage.
     */
    public void reset() {
        _buf.reset();
        clear();
    }

    /**
     * Releases any data retained by encapsulations. The {@link #reset} method internally calls
     * <code>clear</code>.
     */
    public void clear() {
        if (_encapsStack != null) {
            assert (_encapsStack.next == null);
            _encapsStack.next = _encapsCache;
            _encapsCache = _encapsStack;
            _encapsCache.reset();
            _encapsStack = null;
        }

        _startSeq = -1;
    }

    Instance instance() {
        return _instance;
    }

    /**
     * Swaps the contents of one stream with another.
     *
     * @param other The other stream.
     */
    public void swap(InputStream other) {
        assert (_instance == other._instance);

        Buffer tmpBuf = other._buf;
        other._buf = _buf;
        _buf = tmpBuf;

        EncodingVersion tmpEncoding = other._encoding;
        other._encoding = _encoding;
        _encoding = tmpEncoding;

        int tmpStartSeq = other._startSeq;
        other._startSeq = _startSeq;
        _startSeq = tmpStartSeq;

        int tmpMinSeqSize = other._minSeqSize;
        other._minSeqSize = _minSeqSize;
        _minSeqSize = tmpMinSeqSize;

        // Swap is never called for streams that have encapsulations being read. However,
        // encapsulations might still be set in case unmarshaling failed. We just reset the
        // encapsulations if there are still some set.
        resetEncapsulation();
        other.resetEncapsulation();
    }

    private void resetEncapsulation() {
        _encapsStack = null;
    }

    /**
     * Resizes the stream to a new size.
     *
     * @param sz The new size.
     */
    void resize(int sz) {
        _buf.resize(sz, true);
        _buf.position(sz);
    }

    Buffer getBuffer() {
        return _buf;
    }

    /** Marks the start of a class instance. */
    public void startValue() {
        assert (_encapsStack != null && _encapsStack.decoder != null);
        _encapsStack.decoder.startInstance(SliceType.ValueSlice);
    }

    /**
     * Marks the end of a class instance.
     *
     * @return An object that encapsulates the unknown slice data.
     */
    public SlicedData endValue() {
        assert (_encapsStack != null && _encapsStack.decoder != null);
        return _encapsStack.decoder.endInstance();
    }

    /** Marks the start of a user exception. */
    public void startException() {
        assert (_encapsStack != null && _encapsStack.decoder != null);
        _encapsStack.decoder.startInstance(SliceType.ExceptionSlice);
    }

    /**
     * Marks the end of a user exception.
     *
     * @return An object that encapsulates the unknown slice data.
     */
    public SlicedData endException() {
        assert (_encapsStack != null && _encapsStack.decoder != null);
        return _encapsStack.decoder.endInstance();
    }

    /**
     * Reads the start of an encapsulation.
     *
     * @return The encoding version used by the encapsulation.
     */
    public EncodingVersion startEncapsulation() {
        Encaps curr = _encapsCache;
        if (curr != null) {
            curr.reset();
            _encapsCache = _encapsCache.next;
        } else {
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
        if (sz < 6) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE);
        }
        if (sz - 4 > _buf.b.remaining()) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE);
        }
        _encapsStack.sz = sz;

        EncodingVersion encoding = EncodingVersion.ice_read(this);
        Protocol.checkSupportedEncoding(encoding); // Make sure the encoding is supported.
        _encapsStack.setEncoding(encoding);

        return encoding;
    }

    /** Ends the current encapsulation. */
    public void endEncapsulation() {
        assert (_encapsStack != null);

        if (!_encapsStack.encoding_1_0) {
            skipOptionals();
            if (_buf.b.position() != _encapsStack.start + _encapsStack.sz) {
                throw new MarshalException("Failed to unmarshal encapsulation.");
            }
        } else if (_buf.b.position() != _encapsStack.start + _encapsStack.sz) {
            if (_buf.b.position() + 1 != _encapsStack.start + _encapsStack.sz) {
                throw new MarshalException("Failed to unmarshal encapsulation.");
            }

            //
            // Ice version < 3.3 had a bug where user exceptions with
            // class members could be encoded with a trailing byte
            // when dispatched with AMD. So we tolerate an extra byte
            // in the encapsulation.
            //
            try {
                _buf.b.get();
            } catch (BufferUnderflowException ex) {
                throw new MarshalException(END_OF_BUFFER_MESSAGE, ex);
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
     */
    public EncodingVersion skipEmptyEncapsulation() {
        int sz = readInt();
        if (sz < 6) {
            throw new MarshalException(sz + " is not a valid encapsulation size.");
        }
        if (sz - 4 > _buf.b.remaining()) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE);
        }

        EncodingVersion encoding = EncodingVersion.ice_read(this);
        Protocol.checkSupportedEncoding(encoding); // Make sure the encoding is supported.

        if (encoding.equals(Util.Encoding_1_0)) {
            if (sz != 6) {
                throw new MarshalException(
                    sz + "is not a valid encapsulation size for a 1.0 empty encapsulation.");
            }
        } else {
            //
            // Skip the optional content of the encapsulation if we are expecting an empty
            // encapsulation.
            //
            _buf.position(_buf.b.position() + sz - 6);
        }
        return encoding;
    }

    /**
     * Returns a blob of bytes representing an encapsulation. The encapsulation's encoding version
     * is returned in the argument.
     *
     * @param encoding The encapsulation's encoding version.
     * @return The encoded encapsulation.
     */
    public byte[] readEncapsulation(EncodingVersion encoding) {
        int sz = readInt();
        if (sz < 6) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE);
        }

        if (sz - 4 > _buf.b.remaining()) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE);
        }

        if (encoding != null) {
            encoding.ice_readMembers(this);
            _buf.position(_buf.b.position() - 6);
        } else {
            _buf.position(_buf.b.position() - 4);
        }

        byte[] v = new byte[sz];
        try {
            _buf.b.get(v);
            return v;
        } catch (BufferUnderflowException ex) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE, ex);
        }
    }

    /**
     * Determines the current encoding version.
     *
     * @return The encoding version.
     */
    public EncodingVersion getEncoding() {
        return _encapsStack != null ? _encapsStack.encoding : _encoding;
    }

    /**
     * Determines the size of the current encapsulation, excluding the encapsulation header.
     *
     * @return The size of the encapsulated data.
     */
    public int getEncapsulationSize() {
        assert (_encapsStack != null);
        return _encapsStack.sz - 6;
    }

    /**
     * Skips over an encapsulation.
     *
     * @return The encoding version of the skipped encapsulation.
     */
    public EncodingVersion skipEncapsulation() {
        int sz = readInt();
        if (sz < 6) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE);
        }
        EncodingVersion encoding = EncodingVersion.ice_read(this);
        try {
            _buf.position(_buf.b.position() + sz - 6);
        } catch (IllegalArgumentException ex) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE, ex);
        }
        return encoding;
    }

    /**
     * Reads the start of a value or exception slice.
     */
    public void startSlice()
        {
            assert (_encapsStack != null && _encapsStack.decoder != null);
            _encapsStack.decoder.startSlice();
        }

    /** Indicates that the end of a value or exception slice has been reached. */
    public void endSlice() {
        assert (_encapsStack != null && _encapsStack.decoder != null);
        _encapsStack.decoder.endSlice();
    }

    /** Skips over a value or exception slice. */
    public void skipSlice() {
        assert (_encapsStack != null && _encapsStack.decoder != null);
        _encapsStack.decoder.skipSlice();
    }

    /**
     * Indicates that unmarshaling is complete, except for any class instances. The application must
     * call this method only if the stream actually contains class instances. Calling <code>
     * readPendingValues</code> triggers the calls to consumers provided with {@link #readValue} to
     * inform the application that unmarshaling of an instance is complete.
     */
    public void readPendingValues() {
        if (_encapsStack != null && _encapsStack.decoder != null) {
            _encapsStack.decoder.readPendingValues();
        } else if (_encapsStack != null
            ? _encapsStack.encoding_1_0
            : _encoding.equals(Util.Encoding_1_0)) {
            //
            // If using the 1.0 encoding and no instances were read, we still read an empty sequence
            // of pending instances if requested (i.e.: if this is called).
            //
            // This is required by the 1.0 encoding, even if no instances are written we do marshal
            // an empty sequence if marshaled data types use classes.
            //
            skipSize();
        }
    }

    /**
     * Extracts a size from the stream.
     *
     * @return The extracted size.
     */
    public int readSize() {
        try {
            byte b = _buf.b.get();
            if (b == -1) {
                int v = _buf.b.getInt();
                if (v < 0) {
                    throw new MarshalException(END_OF_BUFFER_MESSAGE);
                }
                return v;
            } else {
                return b < 0 ? b + 256 : b;
            }
        } catch (BufferUnderflowException ex) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE, ex);
        }
    }

    /**
     * Reads and validates a sequence size.
     *
     * @param minSize The minimum size required by the sequence type.
     * @return The extracted size.
     */
    public int readAndCheckSeqSize(int minSize) {
        int sz = readSize();

        if (sz == 0) {
            return sz;
        }

        //
        // The _startSeq variable points to the start of the sequence for which
        // we expect to read at least _minSeqSize bytes from the stream.
        //
        // If not initialized or if we already read more data than _minSeqSize, we reset _startSeq
        // and _minSeqSize for this sequence (possibly a top-level sequence or enclosed sequence it
        // doesn't really matter).
        //
        // Otherwise, we are reading an enclosed sequence and we have to bump _minSeqSize by the
        // minimum size that this sequence will require on the stream.
        //
        // The goal of this check is to ensure that when we start un-marshaling a new sequence, we
        // check the minimal size of this new sequence against the estimated remaining buffer size.
        // This estimation is based on the minimum size of the enclosing sequences, it's
        // _minSeqSize.
        //
        if (_startSeq == -1 || _buf.b.position() > (_startSeq + _minSeqSize)) {
            _startSeq = _buf.b.position();
            _minSeqSize = sz * minSize;
        } else {
            _minSeqSize += sz * minSize;
        }

        //
        // If there isn't enough data to read on the stream for the sequence (and possibly enclosed
        // sequences), something is wrong with the marshaled data: it's claiming having more data
        // that what is possible to read.
        //
        if (_startSeq + _minSeqSize > _buf.size()) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE);
        }

        return sz;
    }

    /**
     * Reads a blob of bytes from the stream.
     *
     * @param sz The number of bytes to read.
     * @return The requested bytes as a byte array.
     */
    public byte[] readBlob(int sz) {
        if (_buf.b.remaining() < sz) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE);
        }
        byte[] v = new byte[sz];
        try {
            _buf.b.get(v);
            return v;
        } catch (BufferUnderflowException ex) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE, ex);
        }
    }

    /**
     * Determine if an optional value is available for reading.
     *
     * @param tag The tag associated with the value.
     * @param expectedFormat The optional format for the value.
     * @return True if the value is present, false otherwise.
     */
    public boolean readOptional(int tag, OptionalFormat expectedFormat) {
        assert (_encapsStack != null);
        if (_encapsStack.decoder != null) {
            return _encapsStack.decoder.readOptional(tag, expectedFormat);
        } else {
            return readOptImpl(tag, expectedFormat);
        }
    }

    /**
     * Extracts a byte value from the stream.
     *
     * @return The extracted byte.
     */
    public byte readByte() {
        try {
            return _buf.b.get();
        } catch (BufferUnderflowException ex) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE, ex);
        }
    }

    /**
     * Extracts an optional byte value from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     */
    public Optional<Byte> readByte(int tag) {
        if (readOptional(tag, OptionalFormat.F1)) {
            return Optional.of(readByte());
        } else {
            return Optional.empty();
        }
    }

    /**
     * Extracts a sequence of byte values from the stream.
     *
     * @return The extracted byte sequence.
     */
    public byte[] readByteSeq() {
        try {
            final int sz = readAndCheckSeqSize(1);
            byte[] v = new byte[sz];
            _buf.b.get(v);
            return v;
        } catch (BufferUnderflowException ex) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE, ex);
        }
    }

    /**
     * Extracts an optional byte sequence from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     */
    public Optional<byte[]> readByteSeq(int tag) {
        if (readOptional(tag, OptionalFormat.VSize)) {
            return Optional.of(readByteSeq());
        } else {
            return Optional.empty();
        }
    }

    /**
     * Returns a byte buffer representing a sequence of bytes. This method does not copy the data.
     *
     * @return A byte buffer "slice" of the internal buffer.
     */
    public ByteBuffer readByteBuffer() {
        try {
            final int sz = readAndCheckSeqSize(1);
            ByteBuffer v = _buf.b.slice();
            // Cast to java.nio.Buffer to avoid incompatible covariant
            // return type used in Java 9 java.nio.ByteBuffer
            ((java.nio.Buffer) v).limit(sz);
            _buf.position(_buf.b.position() + sz);
            return v.asReadOnlyBuffer();
        } catch (BufferUnderflowException ex) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE, ex);
        }
    }

    /**
     * Extracts a serializable Java object from the stream.
     *
     * @param <T> The serializable type.
     * @param cl The class for the serializable type.
     * @return The deserialized Java object.
     */
    public <T extends Serializable> T readSerializable(Class<T> cl) {
        int sz = readAndCheckSeqSize(1);
        if (sz == 0) {
            return null;
        }
        ObjectInputStreamWrapper in = null;
        try {
            var w = new InputStreamWrapper(sz, _buf.b);
            in = new ObjectInputStreamWrapper(_instance, w);
            return cl.cast(in.readObject());
        } catch (LocalException ex) {
            throw ex;
        } catch (Exception ex) {
            throw new MarshalException("cannot deserialize object", ex);
        } finally {
            if (in != null) {
                try {
                    in.close();
                } catch (IOException ex) {
                    throw new MarshalException("cannot deserialize object", ex);
                }
            }
        }
    }

    /**
     * Extracts a optional serializable Java object from the stream.
     *
     * @param <T> The serializable type.
     * @param tag The numeric tag associated with the value.
     * @param cl The class for the serializable type.
     * @return The optional value (if any).
     */
    public <T extends Serializable> Optional<T> readSerializable(
            int tag, Class<T> cl) {
        if (readOptional(tag, OptionalFormat.VSize)) {
            return Optional.of(readSerializable(cl));
        } else {
            return Optional.empty();
        }
    }

    /**
     * Extracts a boolean value from the stream.
     *
     * @return The extracted boolean.
     */
    public boolean readBool() {
        try {
            return _buf.b.get() == 1;
        } catch (BufferUnderflowException ex) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE, ex);
        }
    }

    /**
     * Extracts an optional boolean value from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     */
    public Optional<Boolean> readBool(int tag) {
        if (readOptional(tag, OptionalFormat.F1)) {
            return Optional.of(readBool());
        } else {
            return Optional.empty();
        }
    }

    /**
     * Extracts a sequence of boolean values from the stream.
     *
     * @return The extracted boolean sequence.
     */
    public boolean[] readBoolSeq() {
        try {
            final int sz = readAndCheckSeqSize(1);
            boolean[] v = new boolean[sz];
            for (int i = 0; i < sz; i++) {
                v[i] = _buf.b.get() == 1;
            }
            return v;
        } catch (BufferUnderflowException ex) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE, ex);
        }
    }

    /**
     * Extracts an optional boolean sequence from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     */
    public Optional<boolean[]> readBoolSeq(int tag) {
        if (readOptional(tag, OptionalFormat.VSize)) {
            return Optional.of(readBoolSeq());
        } else {
            return Optional.empty();
        }
    }

    /**
     * Extracts a short value from the stream.
     *
     * @return The extracted short.
     */
    public short readShort() {
        try {
            return _buf.b.getShort();
        } catch (BufferUnderflowException ex) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE, ex);
        }
    }

    /**
     * Extracts an optional short value from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     */
    public Optional<Short> readShort(int tag) {
        if (readOptional(tag, OptionalFormat.F2)) {
            return Optional.of(readShort());
        } else {
            return Optional.empty();
        }
    }

    /**
     * Extracts a sequence of short values from the stream.
     *
     * @return The extracted short sequence.
     */
    public short[] readShortSeq() {
        try {
            final int sz = readAndCheckSeqSize(2);
            short[] v = new short[sz];
            ShortBuffer shortBuf = _buf.b.asShortBuffer();
            shortBuf.get(v);
            _buf.position(_buf.b.position() + sz * 2);
            return v;
        } catch (BufferUnderflowException ex) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE, ex);
        }
    }

    /**
     * Extracts an optional short sequence from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     */
    public Optional<short[]> readShortSeq(int tag) {
        if (readOptional(tag, OptionalFormat.VSize)) {
            skipSize();
            return Optional.of(readShortSeq());
        } else {
            return Optional.empty();
        }
    }

    /**
     * Returns a short buffer representing a sequence of shorts. This method does not copy the data.
     *
     * @return A short buffer "slice" of the internal buffer.
     */
    public ShortBuffer readShortBuffer() {
        try {
            final int sz = readAndCheckSeqSize(2);
            ShortBuffer shortBuf = _buf.b.asShortBuffer();
            ShortBuffer v = shortBuf.slice();
            // Cast to java.nio.Buffer to avoid incompatible covariant
            // return type used in Java 9 java.nio.ShortBuffer
            ((java.nio.Buffer) v).limit(sz);
            _buf.position(_buf.b.position() + sz * 2);
            return v.asReadOnlyBuffer();
        } catch (BufferUnderflowException ex) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE, ex);
        }
    }

    /**
     * Extracts an int value from the stream.
     *
     * @return The extracted int.
     */
    public int readInt() {
        try {
            return _buf.b.getInt();
        } catch (BufferUnderflowException ex) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE, ex);
        }
    }

    /**
     * Extracts an optional int value from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     */
    public OptionalInt readInt(int tag) {
        if (readOptional(tag, OptionalFormat.F4)) {
            return OptionalInt.of(readInt());
        } else {
            return OptionalInt.empty();
        }
    }

    /**
     * Extracts a sequence of int values from the stream.
     *
     * @return The extracted int sequence.
     */
    public int[] readIntSeq() {
        try {
            final int sz = readAndCheckSeqSize(4);
            int[] v = new int[sz];
            IntBuffer intBuf = _buf.b.asIntBuffer();
            intBuf.get(v);
            _buf.position(_buf.b.position() + sz * 4);
            return v;
        } catch (BufferUnderflowException ex) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE, ex);
        }
    }

    /**
     * Extracts an optional int sequence from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     */
    public Optional<int[]> readIntSeq(int tag) {
        if (readOptional(tag, OptionalFormat.VSize)) {
            skipSize();
            return Optional.of(readIntSeq());
        } else {
            return Optional.empty();
        }
    }

    /**
     * Returns an int buffer representing a sequence of ints. This method does not copy the data.
     *
     * @return An int buffer "slice" of the internal buffer.
     */
    public IntBuffer readIntBuffer() {
        try {
            final int sz = readAndCheckSeqSize(4);
            IntBuffer intBuf = _buf.b.asIntBuffer();
            IntBuffer v = intBuf.slice();
            // Cast to java.nio.Buffer to avoid incompatible covariant
            // return type used in Java 9 java.nio.IntBuffer
            ((java.nio.Buffer) v).limit(sz);
            _buf.position(_buf.b.position() + sz * 4);
            return v.asReadOnlyBuffer();
        } catch (BufferUnderflowException ex) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE, ex);
        }
    }

    /**
     * Extracts a long value from the stream.
     *
     * @return The extracted long.
     */
    public long readLong() {
        try {
            return _buf.b.getLong();
        } catch (BufferUnderflowException ex) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE, ex);
        }
    }

    /**
     * Extracts an optional long value from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     */
    public OptionalLong readLong(int tag) {
        if (readOptional(tag, OptionalFormat.F8)) {
            return OptionalLong.of(readLong());
        } else {
            return OptionalLong.empty();
        }
    }

    /**
     * Extracts a sequence of long values from the stream.
     *
     * @return The extracted long sequence.
     */
    public long[] readLongSeq() {
        try {
            final int sz = readAndCheckSeqSize(8);
            long[] v = new long[sz];
            LongBuffer longBuf = _buf.b.asLongBuffer();
            longBuf.get(v);
            _buf.position(_buf.b.position() + sz * 8);
            return v;
        } catch (BufferUnderflowException ex) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE, ex);
        }
    }

    /**
     * Extracts an optional long sequence from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     */
    public Optional<long[]> readLongSeq(int tag) {
        if (readOptional(tag, OptionalFormat.VSize)) {
            skipSize();
            return Optional.of(readLongSeq());
        } else {
            return Optional.empty();
        }
    }

    /**
     * Returns a long buffer representing a sequence of longs. This method does not copy the data.
     *
     * @return A long buffer "slice" of the internal buffer.
     */
    public LongBuffer readLongBuffer() {
        try {
            final int sz = readAndCheckSeqSize(8);
            LongBuffer longBuf = _buf.b.asLongBuffer();
            LongBuffer v = longBuf.slice();
            // Cast to java.nio.Buffer to avoid incompatible covariant
            // return type used in Java 9 java.nio.LongBuffer
            ((java.nio.Buffer) v).limit(sz);
            _buf.position(_buf.b.position() + sz * 8);
            return v.asReadOnlyBuffer();
        } catch (BufferUnderflowException ex) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE, ex);
        }
    }

    /**
     * Extracts a float value from the stream.
     *
     * @return The extracted float.
     */
    public float readFloat() {
        try {
            return _buf.b.getFloat();
        } catch (BufferUnderflowException ex) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE, ex);
        }
    }

    /**
     * Extracts an optional float value from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     */
    public Optional<Float> readFloat(int tag) {
        if (readOptional(tag, OptionalFormat.F4)) {
            return Optional.of(readFloat());
        } else {
            return Optional.empty();
        }
    }

    /**
     * Extracts a sequence of float values from the stream.
     *
     * @return The extracted float sequence.
     */
    public float[] readFloatSeq() {
        try {
            final int sz = readAndCheckSeqSize(4);
            float[] v = new float[sz];
            FloatBuffer floatBuf = _buf.b.asFloatBuffer();
            floatBuf.get(v);
            _buf.position(_buf.b.position() + sz * 4);
            return v;
        } catch (BufferUnderflowException ex) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE, ex);
        }
    }

    /**
     * Extracts an optional float sequence from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     */
    public Optional<float[]> readFloatSeq(int tag) {
        if (readOptional(tag, OptionalFormat.VSize)) {
            skipSize();
            return Optional.of(readFloatSeq());
        } else {
            return Optional.empty();
        }
    }

    /**
     * Returns a float buffer representing a sequence of floats. This method does not copy the data.
     *
     * @return A float buffer "slice" of the internal buffer.
     */
    public FloatBuffer readFloatBuffer() {
        try {
            final int sz = readAndCheckSeqSize(4);
            FloatBuffer floatBuf = _buf.b.asFloatBuffer();
            FloatBuffer v = floatBuf.slice();
            // Cast to java.nio.Buffer to avoid incompatible covariant
            // return type used in Java 9 java.nio.FloatBuffer
            ((java.nio.Buffer) v).limit(sz);
            _buf.position(_buf.b.position() + sz * 4);
            return v.asReadOnlyBuffer();
        } catch (BufferUnderflowException ex) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE, ex);
        }
    }

    /**
     * Extracts a double value from the stream.
     *
     * @return The extracted double.
     */
    public double readDouble() {
        try {
            return _buf.b.getDouble();
        } catch (BufferUnderflowException ex) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE, ex);
        }
    }

    /**
     * Extracts an optional double value from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     */
    public OptionalDouble readDouble(int tag) {
        if (readOptional(tag, OptionalFormat.F8)) {
            return OptionalDouble.of(readDouble());
        } else {
            return OptionalDouble.empty();
        }
    }

    /**
     * Extracts a sequence of double values from the stream.
     *
     * @return The extracted double sequence.
     */
    public double[] readDoubleSeq() {
        try {
            final int sz = readAndCheckSeqSize(8);
            double[] v = new double[sz];
            DoubleBuffer doubleBuf = _buf.b.asDoubleBuffer();
            doubleBuf.get(v);
            _buf.position(_buf.b.position() + sz * 8);
            return v;
        } catch (BufferUnderflowException ex) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE, ex);
        }
    }

    /**
     * Extracts an optional double sequence from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     */
    public Optional<double[]> readDoubleSeq(int tag) {
        if (readOptional(tag, OptionalFormat.VSize)) {
            skipSize();
            return Optional.of(readDoubleSeq());
        } else {
            return Optional.empty();
        }
    }

    /**
     * Returns a double buffer representing a sequence of doubles. This method does not copy the
     * data.
     *
     * @return A double buffer "slice" of the internal buffer.
     */
    public DoubleBuffer readDoubleBuffer() {
        try {
            final int sz = readAndCheckSeqSize(8);
            DoubleBuffer doubleBuf = _buf.b.asDoubleBuffer();
            DoubleBuffer v = doubleBuf.slice();
            // Cast to java.nio.Buffer to avoid incompatible covariant
            // return type used in Java 9 java.nio.DoubleBuffer
            ((java.nio.Buffer) v).limit(sz);
            _buf.position(_buf.b.position() + sz * 8);
            return v.asReadOnlyBuffer();
        } catch (BufferUnderflowException ex) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE, ex);
        }
    }

    static final Charset _utf8 = Charset.forName("UTF8");

    /**
     * Extracts a string from the stream.
     *
     * @return The extracted string.
     */
    public String readString() {
        final int len = readSize();

        if (len == 0) {
            return "";
        } else {
            //
            // Check the buffer has enough bytes to read.
            //
            if (_buf.b.remaining() < len) {
                throw new MarshalException(END_OF_BUFFER_MESSAGE);
            }

            try {
                //
                // We reuse the _stringBytes array to avoid creating excessive garbage.
                //
                if (_stringBytes == null || len > _stringBytes.length) {
                    _stringBytes = new byte[len];
                }
                if (_stringChars == null || len > _stringChars.length) {
                    _stringChars = new char[len];
                }
                _buf.b.get(_stringBytes, 0, len);

                //
                // It's more efficient to construct a string using a
                // character array instead of a byte array, because
                // byte arrays require conversion.
                //
                for (int i = 0; i < len; i++) {
                    if (_stringBytes[i] < 0) {
                        //
                        // Multi-byte character found - we must use
                        // conversion.
                        //
                        // TODO: If the string contains garbage bytes
                        // that won't correctly decode as UTF, the behavior of this constructor is
                        // undefined. It would be better to explicitly decode using
                        // java.nio.charset.CharsetDecoder and to
                        // throw MarshalException if the string won't
                        // decode.
                        //
                        return new String(_stringBytes, 0, len, "UTF8");
                    } else {
                        _stringChars[i] = (char) _stringBytes[i];
                    }
                }
                return new String(_stringChars, 0, len);
            } catch (UnsupportedEncodingException ex) {
                assert false;
                return "";
            } catch (BufferUnderflowException ex) {
                throw new MarshalException(END_OF_BUFFER_MESSAGE, ex);
            }
        }
    }

    /**
     * Extracts an optional string value from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     */
    public Optional<String> readString(int tag) {
        if (readOptional(tag, OptionalFormat.VSize)) {
            return Optional.of(readString());
        } else {
            return Optional.empty();
        }
    }

    /**
     * Extracts a sequence of string values from the stream.
     *
     * @return The extracted string sequence.
     */
    public String[] readStringSeq() {
        final int sz = readAndCheckSeqSize(1);
        String[] v = new String[sz];
        for (int i = 0; i < sz; i++) {
            v[i] = readString();
        }
        return v;
    }

    /**
     * Extracts an optional string sequence from the stream.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     */
    public Optional<String[]> readStringSeq(int tag) {
        if (readOptional(tag, OptionalFormat.FSize)) {
            skip(4);
            return Optional.of(readStringSeq());
        } else {
            return Optional.empty();
        }
    }

    /**
     * Extracts a proxy from the stream. The stream must have been initialized with a communicator.
     *
     * @return The extracted proxy.
     */
    public ObjectPrx readProxy() {
        var ident = Identity.ice_read(this);
        if (ident.name.isEmpty()) {
            return null;
        } else {
            var ref = _instance.referenceFactory().create(ident, this);
            return new _ObjectPrxI(ref);
        }
    }

    public <T extends ObjectPrx> T readProxy(Function<ObjectPrx, T> cast) {
        return cast.apply(readProxy());
    }

    /**
     * Extracts an optional proxy from the stream. The stream must have been initialized with a
     * communicator.
     *
     * @param tag The numeric tag associated with the value.
     * @return The optional value (if any).
     */
    public Optional<ObjectPrx> readProxy(int tag) {
        if (readOptional(tag, OptionalFormat.FSize)) {
            skip(4);
            return Optional.of(readProxy());
        } else {
            return Optional.empty();
        }
    }

    /**
     * Extracts an optional proxy from the stream. The stream must have been initialized with a
     * communicator.
     *
     * @param <T> The proxy type.
     * @param tag The numeric tag associated with the value.
     * @param cast The uncheckedCast function to call on the unmarshaled proxy to obtain the correct
     *     proxy type.
     * @return The optional value (if any).
     */
    public <T extends ObjectPrx> Optional<T> readProxy(
            int tag, Function<ObjectPrx, T> cast) {
        if (readOptional(tag, OptionalFormat.FSize)) {
            skip(4);
            return Optional.of(readProxy(cast));
        } else {
            return Optional.empty();
        }
    }

    /**
     * Read an enumerated value.
     *
     * @param maxValue The maximum enumerator value in the definition.
     * @return The enumerator.
     */
    public int readEnum(int maxValue) {
        if (getEncoding().equals(Util.Encoding_1_0)) {
            if (maxValue < 127) {
                return readByte();
            } else if (maxValue < 32767) {
                return readShort();
            } else {
                return readInt();
            }
        } else {
            return readSize();
        }
    }

    /**
     * Extracts a Slice value from the stream.
     *
     * @param <T> The value type.
     * @param cb The consumer to notify when the extracted instance is available. The stream
     *     extracts Slice values in stages. The Ice run time calls accept on the consumer when the
     *     corresponding instance has been fully unmarshaled.
     * @param cls The type of the Ice.Value to unmarshal.
     */
    public <T extends Value> void readValue(Consumer<T> cb, Class<T> cls) {
        initEncaps();
        _encapsStack.decoder.readValue(
            v -> {
                if (v == null || cls.isInstance(v)) {
                    cb.accept(cls.cast(v));
                } else {
                    Ex.throwUOE(cls, v);
                }
            });
    }

    /**
     * Extracts a Slice value from the stream.
     *
     * @param cb The consumer to notify when the extracted instance is available. The stream
     *     extracts Slice values in stages. The Ice run time calls accept on the consumer when the
     *     corresponding instance has been fully unmarshaled.
     */
    public void readValue(Consumer<Value> cb) {
        readValue(cb, Value.class);
    }

    /**
     * Extracts a user exception from the stream and throws it.
     *
     * @throws UserException The user exception that was unmarshaled.
     */
    public void throwException() throws UserException {
        initEncaps();
        _encapsStack.decoder.throwException();
    }

    private boolean readOptImpl(int readTag, OptionalFormat expectedFormat) {
        if (isEncoding_1_0()) {
            return false; // Optional members aren't supported with the 1.0 encoding.
        }

        while (true) {
            if (_buf.b.position() >= _encapsStack.start + _encapsStack.sz) {
                return false; // End of encapsulation also indicates end of optionals.
            }

            final byte b = readByte();
            final int v = b < 0 ? b + 256 : b;
            if (v == Protocol.OPTIONAL_END_MARKER) {
                _buf.position(_buf.b.position() - 1); // Rewind.
                return false;
            }

            OptionalFormat format = OptionalFormat.valueOf(v & 0x07); // First 3 bits.
            int tag = v >> 3;
            if (tag == 30) {
                tag = readSize();
            }

            if (tag > readTag) {
                int offset = tag < 30 ? 1 : (tag < 255 ? 2 : 6); // Rewind
                _buf.position(_buf.b.position() - offset);
                return false; // No optional data members with the requested tag.
            } else if (tag < readTag) {
                skipOptional(format); // Skip optional data members
            } else {
                if (format != expectedFormat) {
                    throw new MarshalException(
                        "invalid optional data member `" + tag + "': unexpected format");
                }
                return true;
            }
        }
    }

    private void skipOptional(OptionalFormat format) {
        switch (format) {
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
                throw new MarshalException("cannot skip an optional class");
            }
        }
    }

    private void skipOptionals() {
        //
        // Skip remaining un-read optional members.
        //
        while (true) {
            if (_buf.b.position() >= _encapsStack.start + _encapsStack.sz) {
                return; // End of encapsulation also indicates end of optionals.
            }

            final byte b = readByte();
            final int v = b < 0 ? b + 256 : b;
            if (v == Protocol.OPTIONAL_END_MARKER) {
                return;
            }

            OptionalFormat format = OptionalFormat.valueOf(v & 0x07); // Read first 3 bits.
            if ((v >> 3) == 30) {
                skipSize();
            }
            skipOptional(format);
        }
    }

    /**
     * Skip the given number of bytes.
     *
     * @param size The number of bytes to skip.
     */
    public void skip(int size) {
        if (size < 0 || size > _buf.b.remaining()) {
            throw new MarshalException(END_OF_BUFFER_MESSAGE);
        }
        _buf.position(_buf.b.position() + size);
    }

    /** Skip over a size value. */
    public void skipSize() {
        byte b = readByte();
        if (b == -1) {
            skip(4);
        }
    }

    /**
     * Determines the current position in the stream.
     *
     * @return The current position.
     */
    public int pos() {
        return _buf.b.position();
    }

    /**
     * Sets the current position in the stream.
     *
     * @param n The new position.
     */
    public void pos(int n) {
        _buf.position(n);
    }

    /**
     * Determines the current size of the stream.
     *
     * @return The current size.
     */
    public int size() {
        return _buf.size();
    }

    /**
     * Determines whether the stream is empty.
     *
     * @return True if the internal buffer has no data, false otherwise.
     */
    public boolean isEmpty() {
        return _buf.empty();
    }

    private UserException createUserException(String id) {
        return (UserException) _instance.sliceLoader().newInstance(id);
    }

    private final Instance _instance;
    private Buffer _buf;
    private byte[] _stringBytes; // Reusable array for reading strings.
    private char[] _stringChars; // Reusable array for reading strings.

    private enum SliceType {
        NoSlice,
        ValueSlice,
        ExceptionSlice
    }

    private abstract static class EncapsDecoder {

        protected class PatchEntry {
            public PatchEntry(Consumer<Value> cb, int classGraphDepth) {
                this.cb = cb;
                this.classGraphDepth = classGraphDepth;
            }

            public Consumer<Value> cb;
            public int classGraphDepth;
        }

        EncapsDecoder(InputStream stream, int classGraphDepthMax, SliceLoader sliceLoader) {
            _stream = stream;
            _classGraphDepthMax = classGraphDepthMax;
            _classGraphDepth = 0;
            _sliceLoader = sliceLoader;
            _typeIdIndex = 0;
            _unmarshaledMap = new TreeMap<>();
        }

        abstract void readValue(Consumer<Value> cb);

        abstract void throwException() throws UserException;

        abstract void startInstance(SliceType type);

        abstract SlicedData endInstance();

        abstract void startSlice();

        abstract void endSlice();

        abstract void skipSlice();

        boolean readOptional(int tag, OptionalFormat format) {
            return false;
        }

        void readPendingValues() {}

        protected String readTypeId(boolean isIndex) {
            if (_typeIdMap == null) {// Lazy initialization
                _typeIdMap = new TreeMap<>();
            }

            if (isIndex) {
                int index = _stream.readSize();
                String typeId = _typeIdMap.get(index);
                if (typeId == null) {
                    throw new MarshalException(END_OF_BUFFER_MESSAGE);
                }
                return typeId;
            } else {
                String typeId = _stream.readString();
                _typeIdMap.put(++_typeIdIndex, typeId);
                return typeId;
            }
        }

        protected Value newInstance(String typeId) {
            return (Value) _sliceLoader.newInstance(typeId);
        }

        protected void addPatchEntry(int index, Consumer<Value> cb) {
            assert (index > 0);

            //
            // Check if we have already unmarshaled the instance. If that's the case, just invoke
            // the callback and we're done.
            //
            Value obj = _unmarshaledMap.get(index);
            if (obj != null) {
                cb.accept(obj);
                return;
            }

            if (_patchMap == null) { // Lazy initialization
                _patchMap = new TreeMap<>();
            }

            //
            // Add patch entry if the instance isn't unmarshaled yet, the callback will be called
            // when the instance is unmarshaled.
            //
            LinkedList<PatchEntry> l = _patchMap.get(index);
            if (l == null) {
                //
                // We have no outstanding instances to be patched for this index, so make a new
                // entry in the patch map.
                //
                l = new LinkedList<>();
                _patchMap.put(index, l);
            }

            //
            // Append a patch entry for this instance.
            //
            l.add(new PatchEntry(cb, _classGraphDepth));
        }

        protected void unmarshal(int index, Value v) {
            //
            // Add the instance to the map of unmarshaled instances, this must be done before
            // reading the instances (for circular references).
            //
            _unmarshaledMap.put(index, v);

            //
            // Read the instance.
            //
            v._iceRead(_stream);

            if (_patchMap != null) {
                //
                // Patch all instances now that the instance is unmarshaled.
                //
                LinkedList<PatchEntry> l = _patchMap.get(index);
                if (l != null) {
                    assert (!l.isEmpty());

                    //
                    // Patch all pointers that refer to the instance.
                    //
                    for (PatchEntry entry : l) {
                        entry.cb.accept(v);
                    }

                    //
                    // Clear out the patch map for that index -- there is nothing left to patch for
                    // that index for the time being.
                    //
                    _patchMap.remove(index);
                }
            }

            if ((_patchMap == null || _patchMap.isEmpty()) && _valueList == null) {
                v.ice_postUnmarshal();
            } else {
                if (_valueList == null) {// Lazy initialization
                    _valueList = new ArrayList<>();
                }
                _valueList.add(v);

                if (_patchMap == null || _patchMap.isEmpty()) {
                    // Iterate over the instance list and invoke ice_postUnmarshal on each instance.
                    // We must do this after all instances have been unmarshaled in order to ensure
                    // that any instance data members have been properly patched.
                    for (Value p : _valueList) {
                        p.ice_postUnmarshal();
                    }
                    _valueList.clear();
                }
            }
        }

        protected final InputStream _stream;
        protected final int _classGraphDepthMax;
        protected int _classGraphDepth;
        protected final SliceLoader _sliceLoader;

        //
        // Encapsulation attributes for value unmarshaling.
        //
        protected TreeMap<Integer, LinkedList<PatchEntry>> _patchMap;
        private TreeMap<Integer, Value> _unmarshaledMap;
        private TreeMap<Integer, String> _typeIdMap;
        private int _typeIdIndex;
        private List<Value> _valueList;
    }

    private static final class EncapsDecoder10 extends EncapsDecoder {
        EncapsDecoder10(
                InputStream stream,
                int classGraphDepthMax,
                SliceLoader sliceLoader) {
            super(stream, classGraphDepthMax, sliceLoader);
            _sliceType = SliceType.NoSlice;
        }

        @Override
        void readValue(Consumer<Value> cb) {
            //
            // Object references are encoded as a negative integer in 1.0.
            //
            int index = _stream.readInt();
            if (index > 0) {
                throw new MarshalException("invalid object id");
            }
            index = -index;

            if (index == 0) {
                cb.accept(null);
            } else {
                addPatchEntry(index, cb);
            }
        }

        @Override
        void throwException() throws UserException {
            assert (_sliceType == SliceType.NoSlice);

            //
            // User exception with the 1.0 encoding start with a boolean flag
            // that indicates whether or not the exception has classes.
            //
            // This allows reading the pending instances even if some part of the exception was
            // sliced.
            //
            boolean usesClasses = _stream.readBool();

            _sliceType = SliceType.ExceptionSlice;
            _skipFirstSlice = false;

            //
            // Read the first slice header.
            //
            startSlice();
            final String mostDerivedId = _typeId;
            while (true) {
                UserException userEx = _stream.createUserException(_typeId);

                //
                // We found the exception.
                //
                if (userEx != null) {
                    userEx._read(_stream);
                    if (usesClasses) {
                        readPendingValues();
                    }
                    throw userEx;

                    // Never reached.
                }

                //
                // Slice off what we don't understand.
                //
                skipSlice();
                try {
                    startSlice();
                } catch (MarshalException ex) {
                    //
                    // An oversight in the 1.0 encoding means there is no marker to indicate the
                    // last slice of an exception. As a result, we just try to read the
                    // next type ID, which raises MarshalException when the input buffer underflows.
                    throw new MarshalException("unknown exception type '" + mostDerivedId + "'");
                }
            }
        }

        @Override
        void startInstance(SliceType sliceType) {
            assert (_sliceType == sliceType);
            _skipFirstSlice = true;
        }

        @Override
        SlicedData endInstance() {
            //
            // Read the Ice::Object slice.
            //
            if (_sliceType == SliceType.ValueSlice) {
                startSlice();
                int sz = _stream.readSize(); // For compatibility with the old AFM.
                if (sz != 0) {
                    throw new MarshalException("invalid Object slice");
                }
                endSlice();
            }

            _sliceType = SliceType.NoSlice;
            return null;
        }

        @Override
        void startSlice() {
            //
            // If first slice, don't read the header, it was already read in
            // readInstance or throwException to find the factory.
            //
            if (_skipFirstSlice) {
                _skipFirstSlice = false;
                return;
            }

            //
            // For class instances, first read the type ID boolean which indicates
            // whether or not the type ID is encoded as a string or as an index. For exceptions, the
            // type ID is always encoded as a string.
            //
            if (_sliceType
                == SliceType.ValueSlice) // For exceptions, the type ID is always encoded as a
                // string
                {
                    boolean isIndex = _stream.readBool();
                    _typeId = readTypeId(isIndex);
                } else {
                _typeId = _stream.readString();
            }

            _sliceSize = _stream.readInt();
            if (_sliceSize < 4) {
                throw new MarshalException(END_OF_BUFFER_MESSAGE);
            }
        }

        @Override
        void endSlice() {}

        @Override
        void skipSlice() {
            _stream.traceSkipSlice(_typeId, _sliceType);
            assert (_sliceSize >= 4);
            _stream.skip(_sliceSize - 4);
        }

        @Override
        void readPendingValues() {
            int num;
            do {
                num = _stream.readSize();
                for (int k = num; k > 0; k--) {
                    readInstance();
                }
            } while (num > 0);

            if (_patchMap != null && !_patchMap.isEmpty()) {
                //
                // If any entries remain in the patch map, the sender has sent an index for an
                // object, but failed to supply the object.
                //
                throw new MarshalException("index for class received, but no instance");
            }
        }

        private void readInstance() {
            int index = _stream.readInt();

            if (index <= 0) {
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
            while (true) {
                // For the 1.0 encoding, the type ID for the base Object class marks the last slice.
                if (_typeId.equals(Value.ice_staticId())) {
                    throw new MarshalException(
                        "The Slice loader did not find a class for type ID '" + mostDerivedId + "'.");
                }

                v = newInstance(_typeId);

                //
                // We found a factory, we get out of this loop.
                //
                if (v != null) {
                    break;
                }

                //
                // Slice off what we don't understand.
                //
                skipSlice();
                startSlice(); // Read next Slice header for next iteration.
            }

            //
            // Compute the biggest class graph depth of this object. To compute this, we get the
            // class graph depth of each ancestor from the patch map and keep the biggest one.
            //
            _classGraphDepth = 0;
            var l = _patchMap != null ? _patchMap.get(index) : null;
            if (l != null) {
                assert (!l.isEmpty());
                for (PatchEntry entry : l) {
                    if (entry.classGraphDepth > _classGraphDepth) {
                        _classGraphDepth = entry.classGraphDepth;
                    }
                }
            }

            if (++_classGraphDepth > _classGraphDepthMax) {
                throw new MarshalException("maximum class graph depth reached");
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

    private static class EncapsDecoder11 extends EncapsDecoder {
        EncapsDecoder11(
                InputStream stream,
                int classGraphDepthMax,
                SliceLoader sliceLoader) {
            super(stream, classGraphDepthMax, sliceLoader);
            _current = null;
            _valueIdIndex = 1;
        }

        @Override
        void readValue(Consumer<Value> cb) {
            int index = _stream.readSize();
            if (index < 0) {
                throw new MarshalException("invalid object id");
            } else if (index == 0) {
                cb.accept(null);
            } else if (_current != null
                && (_current.sliceFlags & Protocol.FLAG_HAS_INDIRECTION_TABLE) != 0) {
                //
                // When reading a class instance within a slice and there's an indirect instance
                // table, always read an indirect reference
                // that points to an instance from the indirect instance table
                // marshaled at the end of the Slice.
                //
                // Maintain a list of indirect references. Note that the
                // indirect index starts at 1, so we decrement it by one to
                // derive an index into the indirection table that we'll read
                // at the end of the slice.
                //
                if (_current.indirectPatchList == null) {// Lazy initialization
                    _current.indirectPatchList = new ArrayDeque<>();
                }
                IndirectPatchEntry e = new IndirectPatchEntry();
                e.index = index - 1;
                e.cb = cb;
                _current.indirectPatchList.push(e);
            } else {
                readInstance(index, cb);
            }
        }

        @Override
        void throwException() throws UserException {
            assert (_current == null);

            push(SliceType.ExceptionSlice);

            //
            // Read the first slice header.
            //
            startSlice();
            final String mostDerivedId = _current.typeId;
            while (true) {
                UserException userEx = _stream.createUserException(_current.typeId);

                //
                // We found the exception.
                //
                if (userEx != null) {
                    userEx._read(_stream);
                    throw userEx;

                    // Never reached.
                }

                //
                // Slice off what we don't understand.
                //
                skipSlice();

                if ((_current.sliceFlags & Protocol.FLAG_IS_LAST_SLICE) != 0) {
                    throw new MarshalException(
                        "cannot unmarshal user exception with type ID '" + mostDerivedId + "'");
                }

                startSlice();
            }
        }

        @Override
        void startInstance(SliceType sliceType) {
            assert (_current.sliceType == sliceType);
            _current.skipFirstSlice = true;
        }

        @Override
        SlicedData endInstance() {
            SlicedData slicedData = readSlicedData();
            if (_current.slices != null) {
                _current.slices.clear();
            }
            if (_current.indirectionTables != null) {
                _current.indirectionTables.clear();
            }
            _current = _current.previous;
            return slicedData;
        }

        @Override
        void startSlice() {
            //
            // If first slice, don't read the header, it was already read in
            // readInstance or throwException to find the factory.
            //
            if (_current.skipFirstSlice) {
                _current.skipFirstSlice = false;
                return;
            }

            _current.sliceFlags = _stream.readByte();

            //
            // Read the type ID, for value slices the type ID is encoded as a string or as an index,
            // for exceptions it's always encoded as a string.
            //
            if (_current.sliceType == SliceType.ValueSlice) {
                if ((_current.sliceFlags & Protocol.FLAG_HAS_TYPE_ID_COMPACT)
                    == Protocol.FLAG_HAS_TYPE_ID_COMPACT) // Must be checked 1st!
                    {
                        _current.compactId = _stream.readSize();
                        _current.typeId = String.valueOf(_current.compactId);
                    } else if ((_current.sliceFlags
                    & (Protocol.FLAG_HAS_TYPE_ID_INDEX
                    | Protocol.FLAG_HAS_TYPE_ID_STRING))
                    != 0) {
                    _current.typeId =
                        readTypeId(
                            (_current.sliceFlags & Protocol.FLAG_HAS_TYPE_ID_INDEX) != 0);
                    _current.compactId = -1;
                } else {
                    //
                    // Only the most derived slice encodes the type ID for the compact format.
                    //
                    _current.typeId = "";
                    _current.compactId = -1;
                }
            } else {
                _current.typeId = _stream.readString();
                _current.compactId = -1;
            }

            //
            // Read the slice size if necessary.
            //
            if ((_current.sliceFlags & Protocol.FLAG_HAS_SLICE_SIZE) != 0) {
                _current.sliceSize = _stream.readInt();
                if (_current.sliceSize < 4) {
                    throw new MarshalException(END_OF_BUFFER_MESSAGE);
                }
            } else {
                _current.sliceSize = 0;
            }
        }

        @Override
        void endSlice() {
            if ((_current.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS) != 0) {
                _stream.skipOptionals();
            }

            //
            // Read the indirection table if one is present and transform the
            // indirect patch list into patch entries with direct references.
            //
            if ((_current.sliceFlags & Protocol.FLAG_HAS_INDIRECTION_TABLE) != 0) {
                //
                // The table is written as a sequence<size> to conserve space.
                //
                int[] indirectionTable = new int[_stream.readAndCheckSeqSize(1)];
                for (int i = 0; i < indirectionTable.length; i++) {
                    indirectionTable[i] = readInstance(_stream.readSize(), null);
                }

                //
                // Sanity checks. If there are optional members, it's possible that not all instance
                // references were read if they are from unknown optional data members.
                //
                if (indirectionTable.length == 0) {
                    throw new MarshalException("empty indirection table");
                }
                if ((_current.indirectPatchList == null || _current.indirectPatchList.isEmpty())
                    && (_current.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS) == 0) {
                    throw new MarshalException("no references to indirection table");
                }

                //
                // Convert indirect references into direct references.
                //
                if (_current.indirectPatchList != null) {
                    for (IndirectPatchEntry e : _current.indirectPatchList) {
                        assert (e.index >= 0);
                        if (e.index >= indirectionTable.length) {
                            throw new MarshalException("indirection out of range");
                        }
                        addPatchEntry(indirectionTable[e.index], e.cb);
                    }
                    _current.indirectPatchList.clear();
                }
            }
        }

        @Override
        void skipSlice() {
            _stream.traceSkipSlice(_current.typeId, _current.sliceType);

            int start = _stream.pos();

            if ((_current.sliceFlags & Protocol.FLAG_HAS_SLICE_SIZE) != 0) {
                assert (_current.sliceSize >= 4);
                _stream.skip(_current.sliceSize - 4);
            } else {
                if (_current.sliceType == SliceType.ValueSlice) {
                    throw new MarshalException(
                        "The Slice loader did not find a class for type ID '"
                            + _current.typeId
                            + "' and compact format prevents slicing.");
                } else {
                    throw new MarshalException(
                        "The Slice loader did not find a user exception class for type ID '"
                            + _current.typeId
                            + "' and compact format prevents slicing.");
                }
            }

            //
            // Preserve this slice if unmarshaling a value in Slice format. Exception slices are not
            // preserved.
            //
            if (_current.sliceType == SliceType.ValueSlice) {
                boolean hasOptionalMembers =
                    (_current.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS) != 0;

                Buffer buffer = _stream.getBuffer();
                final int end = buffer.b.position();
                int dataEnd = end;
                if (hasOptionalMembers) {
                    //
                    // Don't include the optional member end marker. It will be re-written by
                    // endSlice when the sliced data is re-written.
                    //
                    --dataEnd;
                }
                var bytes = new byte[dataEnd - start];
                buffer.position(start);
                buffer.b.get(bytes);
                buffer.position(end);

                var info =
                    new SliceInfo(
                        _current.compactId == -1 ? _current.typeId : "",
                        _current.compactId,
                        bytes,
                        hasOptionalMembers,
                        (_current.sliceFlags & Protocol.FLAG_IS_LAST_SLICE) != 0);

                if (_current.slices == null) {// Lazy initialization
                    _current.slices = new ArrayList<>();
                }
                _current.slices.add(info);
            }

            if (_current.indirectionTables == null) {// Lazy initialization
                _current.indirectionTables = new ArrayList<>();
            }

            //
            // Read the indirect instance table. We read the instances or their IDs if the instance
            // is a reference to an already unmarshaled instance.
            //
            if ((_current.sliceFlags & Protocol.FLAG_HAS_INDIRECTION_TABLE) != 0) {
                int[] indirectionTable = new int[_stream.readAndCheckSeqSize(1)];
                for (int i = 0; i < indirectionTable.length; i++) {
                    indirectionTable[i] = readInstance(_stream.readSize(), null);
                }
                _current.indirectionTables.add(indirectionTable);
            } else {
                _current.indirectionTables.add(null);
            }
        }

        @Override
        boolean readOptional(int readTag, OptionalFormat expectedFormat) {
            if (_current == null) {
                return _stream.readOptImpl(readTag, expectedFormat);
            } else if ((_current.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS) != 0) {
                return _stream.readOptImpl(readTag, expectedFormat);
            }
            return false;
        }

        private int readInstance(int index, Consumer<Value> cb) {
            assert (index > 0);

            if (index > 1) {
                if (cb != null) {
                    addPatchEntry(index, cb);
                }
                return index;
            }

            push(SliceType.ValueSlice);

            //
            // Get the instance ID before we start reading slices. If some slices are skipped, the
            // indirect instance table is still read and might read other instances.
            //
            index = ++_valueIdIndex;

            //
            // Read the first slice header.
            //
            startSlice();
            final String mostDerivedId = _current.typeId;
            Value v = null;
            while (true) {
                if (!_current.typeId.isEmpty()) { // we can read an empty typeId with the compact format
                    v = newInstance(_current.typeId);
                    if (v != null) {
                        break;
                    }
                }

                // Slice off what we don't understand.
                skipSlice();

                //
                // If this is the last slice, keep the instance as an opaque
                // UnknownSlicedValue object.
                //
                if ((_current.sliceFlags & Protocol.FLAG_IS_LAST_SLICE) != 0) {
                    //
                    // Provide a factory with an opportunity to supply the instance. We pass the
                    // "::Ice::Object" ID to indicate that this is the last chance to preserve the
                    // instance.
                    //
                    v = newInstance(Value.ice_staticId());
                    if (v == null) {
                        v = new UnknownSlicedValue(mostDerivedId);
                    }

                    break;
                }

                startSlice(); // Read next Slice header for next iteration.
            }

            if (++_classGraphDepth > _classGraphDepthMax) {
                throw new MarshalException("maximum class graph depth reached");
            }

            //
            // Unmarshal the instance.
            //
            unmarshal(index, v);

            --_classGraphDepth;

            if (_current == null && _patchMap != null && !_patchMap.isEmpty()) {
                //
                // If any entries remain in the patch map, the sender has sent an index for an
                // instance, but failed to supply the instance.
                //
                throw new MarshalException("index for class received, but no instance");
            }

            if (cb != null) {
                cb.accept(v);
            }

            return index;
        }

        private SlicedData readSlicedData() {
            if (_current.slices == null) {// No preserved slices.
                return null;
            }

            //
            // The _indirectionTables member holds the indirection table for each slice in _slices.
            //
            assert (_current.slices.size() == _current.indirectionTables.size());
            for (int n = 0; n < _current.slices.size(); n++) {
                //
                // We use the "instances" list in SliceInfo to hold references
                // to the target instances. Note that the instances might not have been read yet in
                // the case of a circular reference to an enclosing instance.
                //
                final int[] table = _current.indirectionTables.get(n);
                SliceInfo info = _current.slices.get(n);
                info.instances = new Value[table != null ? table.length : 0];
                for (int j = 0; j < info.instances.length; j++) {
                    final int k = j;
                    addPatchEntry(table[j], v -> info.instances[k] = v);
                }
            }

            SliceInfo[] arr = new SliceInfo[_current.slices.size()];
            _current.slices.toArray(arr);
            return new SlicedData(arr);
        }

        private void push(SliceType sliceType) {
            if (_current == null) {
                _current = new InstanceData(null);
            } else {
                _current = _current.next == null ? new InstanceData(_current) : _current.next;
            }
            _current.sliceType = sliceType;
            _current.skipFirstSlice = false;
        }

        private static final class IndirectPatchEntry {
            int index;
            Consumer<Value> cb;
        }

        private static final class InstanceData {
            InstanceData(InstanceData previous) {
                if (previous != null) {
                    previous.next = this;
                }
                this.previous = previous;
                this.next = null;
            }

            // Instance attributes
            SliceType sliceType;
            boolean skipFirstSlice;
            List<SliceInfo> slices; // Preserved slices.
            List<int[]> indirectionTables;

            // Slice attributes
            byte sliceFlags;
            int sliceSize;
            String typeId;
            int compactId;
            Deque<IndirectPatchEntry> indirectPatchList;

            final InstanceData previous;
            InstanceData next;
        }

        private InstanceData _current;
        private int _valueIdIndex; // The ID of the next instance to unmarshal.
    }

    private static final class Encaps {
        void reset() {
            decoder = null;
        }

        void setEncoding(EncodingVersion encoding) {
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
    // The encoding version to use when there's no encapsulation to read from. This is for example
    // used to read message headers.
    //
    private EncodingVersion _encoding;

    private boolean isEncoding_1_0() {
        return _encapsStack != null
            ? _encapsStack.encoding_1_0
            : _encoding.equals(Util.Encoding_1_0);
    }

    private Encaps _encapsStack;
    private Encaps _encapsCache;

    private void initEncaps() {
        if (_encapsStack == null) // Lazy initialization
            {
                _encapsStack = _encapsCache;
                if (_encapsStack != null) {
                    _encapsCache = _encapsCache.next;
                } else {
                    _encapsStack = new Encaps();
                }
                _encapsStack.setEncoding(_encoding);
                _encapsStack.sz = _buf.b.limit();
            }

        if (_encapsStack.decoder == null) {// Lazy initialization.
            if (_encapsStack.encoding_1_0) {
                _encapsStack.decoder =
                    new EncapsDecoder10(
                        this, _classGraphDepthMax, _instance.sliceLoader());
            } else {
                _encapsStack.decoder =
                    new EncapsDecoder11(
                        this, _classGraphDepthMax, _instance.sliceLoader());
            }
        }
    }

    private void traceSkipSlice(String typeId, SliceType sliceType) {
        if (_instance.traceLevels().slicing > 0) {
            TraceUtil.traceSlicing(
                sliceType == SliceType.ExceptionSlice ? "exception" : "object",
                typeId,
                "Slicing",
                _instance.initializationData().logger);
        }
    }

    /**
     * We need to override the resolveClass method of ObjectInputStream so that we can use the same
     * class-lookup mechanism as elsewhere in the Ice run time.
     */
    private class ObjectInputStreamWrapper extends java.io.ObjectInputStream {
        public ObjectInputStreamWrapper(Instance instance, java.io.InputStream in)
            throws IOException {
            super(in);
            _instance = instance;
        }

        @Override
        protected Class<?> resolveClass(ObjectStreamClass cls)
            throws IOException, ClassNotFoundException {

            try {
                Class<?> c = _instance.findClass(cls.getName());
                if (c != null) {
                    return c;
                }
                throw new ClassNotFoundException("unable to resolve class" + cls.getName());
            } catch (Exception ex) {
                throw new ClassNotFoundException("unable to resolve class " + cls.getName(), ex);
            }
        }

        private final Instance _instance;
    }

    private final int _classGraphDepthMax;

    private int _startSeq = -1;
    private int _minSeqSize;

    private static final String END_OF_BUFFER_MESSAGE =
        "Attempting to unmarshal past the end of the buffer.";
}
