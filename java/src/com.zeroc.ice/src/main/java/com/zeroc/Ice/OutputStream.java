// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.DoubleBuffer;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;
import java.nio.LongBuffer;
import java.nio.ShortBuffer;
import java.nio.charset.CharacterCodingException;
import java.nio.charset.Charset;
import java.nio.charset.CharsetEncoder;
import java.util.ArrayList;
import java.util.IdentityHashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.OptionalDouble;
import java.util.OptionalInt;
import java.util.OptionalLong;
import java.util.TreeMap;

/**
 * Interface to marshal (write) Slice types into sequence of bytes encoded using the Ice encoding.
 *
 * @see InputStream
 */
public final class OutputStream {

    /**
     * Construct a new instance of the OutputStream class. The output stream is initially empty, and
     * uses the 1.1 encoding, and compact class format.
     */
    public OutputStream() {
        this(Util.currentEncoding(), FormatType.CompactFormat, false);
    }

    /**
     * Construct a new instance of the OutputStream class. The output stream is initially empty, and
     * uses the specified encoding, and compact class format.
     *
     * @param encoding The encoding version to use.
     */
    public OutputStream(EncodingVersion encoding) {
        this(encoding, FormatType.CompactFormat, false);
    }

    /**
     * Construct a new instance of the OutputStream class. The output stream is initially empty, and
     * uses the specified encoding, and compact class format.
     *
     * @param encoding The encoding version to use.
     * @param direct Indicates whether to use a direct buffer.
     */
    public OutputStream(EncodingVersion encoding, boolean direct) {
        this(encoding, FormatType.CompactFormat, direct);
    }

    /**
     * Construct a new instance of the OutputStream class. The output stream is initially empty, and
     * uses the specified encoding, and class format.
     *
     * @param encoding The encoding version to use.
     * @param format The format to use for class encoding.
     * @param direct Indicates whether to use a direct buffer.
     */
    public OutputStream(EncodingVersion encoding, FormatType format, boolean direct) {
        // The 1.0 encoding doesn't use the class format type, but we still have to set it in case
        // the stream reads an 1.1 encapsulation, in which case it would use the format type set in
        // the stream.
        _buf = new Buffer(direct);
        _encoding = encoding;
        _format = format;
    }

    /**
     * Construct a new instance of the OutputStream class. The output stream is initially empty, and
     * uses the communicator's default encoding version and default class format.
     *
     * @param communicator The communicator that provides the encoding version and class format.
     */
    public OutputStream(Communicator communicator) {
        this(
            communicator.getInstance().defaultsAndOverrides().defaultEncoding,
            communicator.getInstance().defaultsAndOverrides().defaultFormat,
            communicator.getInstance().cacheMessageBuffers() > 1);
    }

    OutputStream(Buffer buf, EncodingVersion encoding) {
        _buf = buf;
        _encoding = encoding != null ? encoding : Protocol.currentEncoding;
        _format = FormatType.CompactFormat;
    }

    /**
     * Resets this output stream. This method allows the stream to be reused, to avoid creating
     * unnecessary garbage.
     */
    public void reset() {
        _buf.reset();
        clear();
    }

    /**
     * Releases any data retained by encapsulations. The {@link #reset} method internally calls
     * <code>
     * clear</code>.
     */
    public void clear() {
        if (_encapsStack != null) {
            assert (_encapsStack.next == null);
            _encapsStack.next = _encapsCache;
            _encapsCache = _encapsStack;
            _encapsCache.reset();
            _encapsStack = null;
        }
    }

    /**
     * Indicates that marshaling is finished.
     *
     * @return The byte sequence containing the encoded data.
     */
    public byte[] finished() {
        Buffer buf = prepareWrite();
        byte[] result = new byte[buf.b.limit()];
        buf.b.get(result);
        return result;
    }

    /**
     * Swaps the contents of one stream with another.
     *
     * @param other The other stream.
     */
    public void swap(OutputStream other) {
        Buffer tmpBuf = other._buf;
        other._buf = _buf;
        _buf = tmpBuf;

        EncodingVersion tmpEncoding = other._encoding;
        other._encoding = _encoding;
        _encoding = tmpEncoding;

        //
        // Swap is never called for streams that have encapsulations being written. However,
        // encapsulations might still be set in case marshaling failed. We just reset the
        // encapsulations if there are still some set.
        //
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
    public void resize(int sz) {
        _buf.resize(sz, false);
        _buf.position(sz);
    }

    /**
     * Prepares the internal data buffer to be written to a socket.
     *
     * @return The internal buffer.
     */
    public Buffer prepareWrite() {
        _buf.limit(_buf.size());
        _buf.position(0);
        return _buf;
    }

    /**
     * Retrieves the internal data buffer.
     *
     * @return The buffer.
     */
    public Buffer getBuffer() {
        return _buf;
    }

    /**
     * Marks the start of a class instance.
     *
     * @param data Preserved slices for this instance, or null.
     */
    public void startValue(SlicedData data) {
        assert (_encapsStack != null && _encapsStack.encoder != null);
        _encapsStack.encoder.startInstance(SliceType.ValueSlice, data);
    }

    /** Marks the end of a class instance. */
    public void endValue() {
        assert (_encapsStack != null && _encapsStack.encoder != null);
        _encapsStack.encoder.endInstance();
    }

    /** Marks the start of a user exception. */
    public void startException() {
        assert (_encapsStack != null && _encapsStack.encoder != null);
        _encapsStack.encoder.startInstance(SliceType.ExceptionSlice, null);
    }

    /** Marks the end of a user exception. */
    public void endException() {
        assert (_encapsStack != null && _encapsStack.encoder != null);
        _encapsStack.encoder.endInstance();
    }

    /** Writes the start of an encapsulation to the stream. */
    public void startEncapsulation() {
        //
        // If no encoding version is specified, use the current write encapsulation encoding version
        // if there's a current write encapsulation, otherwise, use the stream encoding version.
        //

        if (_encapsStack != null) {
            startEncapsulation(_encapsStack.encoding, _encapsStack.format);
        } else {
            startEncapsulation(_encoding, null);
        }
    }

    /**
     * Writes the start of an encapsulation to the stream.
     *
     * @param encoding The encoding version of the encapsulation.
     * @param format Specify the compact or sliced format, or null.
     */
    public void startEncapsulation(EncodingVersion encoding, FormatType format) {
        Protocol.checkSupportedEncoding(encoding);

        Encaps curr = _encapsCache;
        if (curr != null) {
            curr.reset();
            _encapsCache = _encapsCache.next;
        } else {
            curr = new Encaps();
        }
        curr.next = _encapsStack;
        _encapsStack = curr;

        _encapsStack.format = format;
        _encapsStack.setEncoding(encoding);
        _encapsStack.start = _buf.size();

        writeInt(0); // Placeholder for the encapsulation length.
        _encapsStack.encoding.ice_writeMembers(this);
    }

    /** Ends the previous encapsulation. */
    public void endEncapsulation() {
        assert (_encapsStack != null);

        // Size includes size and version.
        int start = _encapsStack.start;
        int sz = _buf.size() - start;
        _buf.b.putInt(start, sz);

        Encaps curr = _encapsStack;
        _encapsStack = curr.next;
        curr.next = _encapsCache;
        _encapsCache = curr;
        _encapsCache.reset();
    }

    /**
     * Writes an empty encapsulation using the given encoding version.
     *
     * @param encoding The desired encoding version.
     */
    public void writeEmptyEncapsulation(EncodingVersion encoding) {
        Protocol.checkSupportedEncoding(encoding);
        writeInt(6); // Size
        encoding.ice_writeMembers(this);
    }

    /**
     * Writes a pre-encoded encapsulation.
     *
     * @param v The encapsulation data.
     */
    public void writeEncapsulation(byte[] v) {
        if (v.length < 6) {
            throw new MarshalException(
                "A byte sequence with " + v.length + " bytes is not a valid encapsulation.");
        }
        expand(v.length);
        _buf.b.put(v);
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
     * Marks the start of a new slice for a class instance or user exception.
     *
     * @param typeId The Slice type ID corresponding to this slice.
     * @param compactId The Slice compact type ID corresponding to this slice or -1 if no compact ID
     *     is defined for the type ID.
     * @param last True if this is the last slice, false otherwise.
     */
    public void startSlice(String typeId, int compactId, boolean last) {
        assert (_encapsStack != null && _encapsStack.encoder != null);
        _encapsStack.encoder.startSlice(typeId, compactId, last);
    }

    /** Marks the end of a slice for a class instance or user exception. */
    public void endSlice() {
        assert (_encapsStack != null && _encapsStack.encoder != null);
        _encapsStack.encoder.endSlice();
    }

    /**
     * Writes the state of Slice classes whose index was previously written with {@link #writeValue}
     * to the stream.
     */
    public void writePendingValues() {
        if (_encapsStack != null && _encapsStack.encoder != null) {
            _encapsStack.encoder.writePendingValues();
        } else if (_encapsStack != null
            ? _encapsStack.encoding_1_0
            : _encoding.equals(Util.Encoding_1_0)) {
            //
            // If using the 1.0 encoding and no instances were written, we still write an empty
            // sequence for pending instances if requested (i.e.: if this is called).
            //
            // This is required by the 1.0 encoding, even if no instances are written we do marshal
            // an empty sequence if marshaled data types use classes.
            //
            writeSize(0);
        }
    }

    /**
     * Writes a size to the stream.
     *
     * @param v The size to write.
     */
    public void writeSize(int v) {
        if (v > 254) {
            expand(5);
            _buf.b.put((byte) -1);
            _buf.b.putInt(v);
        } else {
            expand(1);
            _buf.b.put((byte) v);
        }
    }

    /**
     * Returns the current position and allocates four bytes for a fixed-length (32-bit) size value.
     *
     * @return The current position.
     */
    public int startSize() {
        int pos = _buf.b.position();
        writeInt(0); // Placeholder for 32-bit size
        return pos;
    }

    /**
     * Computes the amount of data written since the previous call to startSize and writes that
     * value at the saved position.
     *
     * @param pos The saved position.
     */
    public void endSize(int pos) {
        assert (pos >= 0);
        rewriteInt(_buf.b.position() - pos - 4, pos);
    }

    /**
     * Writes a blob of bytes to the stream.
     *
     * @param v The byte array to be written. All of the bytes in the array are written.
     */
    public void writeBlob(byte[] v) {
        if (v == null) {
            return;
        }
        expand(v.length);
        _buf.b.put(v);
    }

    /**
     * Writes a blob of bytes to the stream.
     *
     * @param v The byte array to be written.
     * @param off The offset into the byte array from which to copy.
     * @param len The number of bytes from the byte array to copy.
     */
    public void writeBlob(byte[] v, int off, int len) {
        if (v == null) {
            return;
        }
        expand(len);
        _buf.b.put(v, off, len);
    }

    /**
     * Write the header information for an optional value.
     *
     * @param tag The numeric tag associated with the value.
     * @param format The optional format of the value.
     * @return True if the current encoding supports optionals, false otherwise.
     */
    public boolean writeOptional(int tag, OptionalFormat format) {
        assert (_encapsStack != null);
        if (_encapsStack.encoder != null) {
            return _encapsStack.encoder.writeOptional(tag, format);
        } else {
            return writeOptionalImpl(tag, format);
        }
    }

    /**
     * Writes a byte to the stream.
     *
     * @param v The byte to write to the stream.
     */
    public void writeByte(byte v) {
        expand(1);
        _buf.b.put(v);
    }

    /**
     * Writes an optional byte to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional byte to write to the stream.
     */
    public void writeByte(int tag, Optional<Byte> v) {
        if (v != null && v.isPresent()) {
            writeByte(tag, v.get());
        }
    }

    /**
     * Writes an optional byte to the stream.
     *
     * @param tag The optional tag.
     * @param v The byte to write to the stream.
     */
    public void writeByte(int tag, byte v) {
        if (writeOptional(tag, OptionalFormat.F1)) {
            writeByte(v);
        }
    }

    /**
     * Writes a byte to the stream at the given position. The current position of the stream is not
     * modified.
     *
     * @param v The byte to write to the stream.
     * @param dest The position at which to store the byte in the buffer.
     */
    public void rewriteByte(byte v, int dest) {
        _buf.b.put(dest, v);
    }

    /**
     * Writes a byte sequence to the stream.
     *
     * @param v The byte sequence to write to the stream. Passing <code>null</code> causes an empty
     *     sequence to be written to the stream.
     */
    public void writeByteSeq(byte[] v) {
        if (v == null) {
            writeSize(0);
        } else {
            writeSize(v.length);
            expand(v.length);
            _buf.b.put(v);
        }
    }

    /**
     * Writes an optional byte sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional byte sequence to write to the stream.
     */
    public void writeByteSeq(int tag, Optional<byte[]> v) {
        if (v != null && v.isPresent()) {
            writeByteSeq(tag, v.get());
        }
    }

    /**
     * Writes an optional byte sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The byte sequence to write to the stream.
     */
    public void writeByteSeq(int tag, byte[] v) {
        if (writeOptional(tag, OptionalFormat.VSize)) {
            writeByteSeq(v);
        }
    }

    /**
     * Writes the remaining contents of the byte buffer as a byte sequence to the stream.
     *
     * @param v The byte buffer to write to the stream.
     */
    public void writeByteBuffer(ByteBuffer v) {
        if (v == null || v.remaining() == 0) {
            writeSize(0);
        } else {
            writeSize(v.remaining());
            expand(v.remaining());
            _buf.b.put(v);
        }
    }

    /**
     * Writes a serializable Java object to the stream.
     *
     * @param o The serializable object to write.
     */
    public void writeSerializable(Serializable o) {
        if (o == null) {
            writeSize(0);
            return;
        }
        try {
            var w = new OutputStreamWrapper(this);
            ObjectOutputStream out = new ObjectOutputStream(w);
            out.writeObject(o);
            out.close();
            w.close();
        } catch (Exception ex) {
            throw new MarshalException("cannot serialize object: " + ex);
        }
    }

    /**
     * Writes an optional serializable Java object to the stream.
     *
     * @param <T> The serializable class.
     * @param tag The optional tag.
     * @param v The optional serializable object to write.
     */
    public <T extends Serializable> void writeSerializable(
            int tag, Optional<T> v) {
        if (v != null && v.isPresent()) {
            writeSerializable(tag, v.get());
        }
    }

    /**
     * Writes an optional serializable Java object to the stream.
     *
     * @param tag The optional tag.
     * @param v The serializable object to write.
     */
    public void writeSerializable(int tag, Serializable v) {
        if (writeOptional(tag, OptionalFormat.VSize)) {
            writeSerializable(v);
        }
    }

    /**
     * Writes a boolean to the stream.
     *
     * @param v The boolean to write to the stream.
     */
    public void writeBool(boolean v) {
        expand(1);
        _buf.b.put(v ? (byte) 1 : (byte) 0);
    }

    /**
     * Writes an optional boolean to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional boolean to write to the stream.
     */
    public void writeBool(int tag, Optional<Boolean> v) {
        if (v != null && v.isPresent()) {
            writeBool(tag, v.get());
        }
    }

    /**
     * Writes an optional boolean to the stream.
     *
     * @param tag The optional tag.
     * @param v The boolean to write to the stream.
     */
    public void writeBool(int tag, boolean v) {
        if (writeOptional(tag, OptionalFormat.F1)) {
            writeBool(v);
        }
    }

    /**
     * Writes a boolean to the stream at the given position. The current position of the stream is
     * not modified.
     *
     * @param v The boolean to write to the stream.
     * @param dest The position at which to store the boolean in the buffer.
     */
    public void rewriteBool(boolean v, int dest) {
        _buf.b.put(dest, v ? (byte) 1 : (byte) 0);
    }

    /**
     * Writes a boolean sequence to the stream.
     *
     * @param v The boolean sequence to write to the stream. Passing <code>null</code> causes an
     *     empty sequence to be written to the stream.
     */
    public void writeBoolSeq(boolean[] v) {
        if (v == null) {
            writeSize(0);
        } else {
            writeSize(v.length);
            expand(v.length);
            for (boolean b : v) {
                _buf.b.put(b ? (byte) 1 : (byte) 0);
            }
        }
    }

    /**
     * Writes an optional boolean sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional boolean sequence to write to the stream.
     */
    public void writeBoolSeq(int tag, Optional<boolean[]> v) {
        if (v != null && v.isPresent()) {
            writeBoolSeq(tag, v.get());
        }
    }

    /**
     * Writes an optional boolean sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The boolean sequence to write to the stream.
     */
    public void writeBoolSeq(int tag, boolean[] v) {
        if (writeOptional(tag, OptionalFormat.VSize)) {
            writeBoolSeq(v);
        }
    }

    /**
     * Writes a short to the stream.
     *
     * @param v The short to write to the stream.
     */
    public void writeShort(short v) {
        expand(2);
        _buf.b.putShort(v);
    }

    /**
     * Writes an optional short to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional short to write to the stream.
     */
    public void writeShort(int tag, Optional<Short> v) {
        if (v != null && v.isPresent()) {
            writeShort(tag, v.get());
        }
    }

    /**
     * Writes an optional short to the stream.
     *
     * @param tag The optional tag.
     * @param v The short to write to the stream.
     */
    public void writeShort(int tag, short v) {
        if (writeOptional(tag, OptionalFormat.F2)) {
            writeShort(v);
        }
    }

    /**
     * Writes a short sequence to the stream.
     *
     * @param v The short sequence to write to the stream. Passing <code>null</code> causes an empty
     *     sequence to be written to the stream.
     */
    public void writeShortSeq(short[] v) {
        if (v == null) {
            writeSize(0);
        } else {
            writeSize(v.length);
            expand(v.length * 2);
            ShortBuffer shortBuf = _buf.b.asShortBuffer();
            shortBuf.put(v);
            _buf.position(_buf.b.position() + v.length * 2);
        }
    }

    /**
     * Writes an optional short sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional short sequence to write to the stream.
     */
    public void writeShortSeq(int tag, Optional<short[]> v) {
        if (v != null && v.isPresent()) {
            writeShortSeq(tag, v.get());
        }
    }

    /**
     * Writes an optional short sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The short sequence to write to the stream.
     */
    public void writeShortSeq(int tag, short[] v) {
        if (writeOptional(tag, OptionalFormat.VSize)) {
            writeSize(v == null || v.length == 0 ? 1 : v.length * 2 + (v.length > 254 ? 5 : 1));
            writeShortSeq(v);
        }
    }

    /**
     * Writes the remaining contents of the short buffer as a short sequence to the stream.
     *
     * @param v The short buffer to write to the stream.
     */
    public void writeShortBuffer(ShortBuffer v) {
        if (v == null || v.remaining() == 0) {
            writeSize(0);
        } else {
            int sz = v.remaining();
            writeSize(sz);
            expand(sz * 2);

            ShortBuffer shortBuf = _buf.b.asShortBuffer();
            shortBuf.put(v);
            _buf.position(_buf.b.position() + sz * 2);
        }
    }

    /**
     * Writes an int to the stream.
     *
     * @param v The int to write to the stream.
     */
    public void writeInt(int v) {
        expand(4);
        _buf.b.putInt(v);
    }

    /**
     * Writes an optional int to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional int to write to the stream.
     */
    public void writeInt(int tag, OptionalInt v) {
        if (v != null && v.isPresent()) {
            writeInt(tag, v.getAsInt());
        }
    }

    /**
     * Writes an optional int to the stream.
     *
     * @param tag The optional tag.
     * @param v The int to write to the stream.
     */
    public void writeInt(int tag, int v) {
        if (writeOptional(tag, OptionalFormat.F4)) {
            writeInt(v);
        }
    }

    /**
     * Writes an int to the stream at the given position. The current position of the stream is not
     * modified.
     *
     * @param v The int to write to the stream.
     * @param dest The position at which to store the int in the buffer.
     */
    public void rewriteInt(int v, int dest) {
        _buf.b.putInt(dest, v);
    }

    /**
     * Writes an int sequence to the stream.
     *
     * @param v The int sequence to write to the stream. Passing <code>null</code> causes an empty
     *     sequence to be written to the stream.
     */
    public void writeIntSeq(int[] v) {
        if (v == null) {
            writeSize(0);
        } else {
            writeSize(v.length);
            expand(v.length * 4);
            IntBuffer intBuf = _buf.b.asIntBuffer();
            intBuf.put(v);
            _buf.position(_buf.b.position() + v.length * 4);
        }
    }

    /**
     * Writes an optional int sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional int sequence to write to the stream.
     */
    public void writeIntSeq(int tag, Optional<int[]> v) {
        if (v != null && v.isPresent()) {
            writeIntSeq(tag, v.get());
        }
    }

    /**
     * Writes an optional int sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The int sequence to write to the stream.
     */
    public void writeIntSeq(int tag, int[] v) {
        if (writeOptional(tag, OptionalFormat.VSize)) {
            writeSize(v == null || v.length == 0 ? 1 : v.length * 4 + (v.length > 254 ? 5 : 1));
            writeIntSeq(v);
        }
    }

    /**
     * Writes the remaining contents of the int buffer as an int sequence to the stream.
     *
     * @param v The int buffer to write to the stream.
     */
    public void writeIntBuffer(IntBuffer v) {
        if (v == null || v.remaining() == 0) {
            writeSize(0);
        } else {
            int sz = v.remaining();
            writeSize(sz);
            expand(sz * 4);

            IntBuffer intBuf = _buf.b.asIntBuffer();
            intBuf.put(v);
            _buf.position(_buf.b.position() + sz * 4);
        }
    }

    /**
     * Writes a long to the stream.
     *
     * @param v The long to write to the stream.
     */
    public void writeLong(long v) {
        expand(8);
        _buf.b.putLong(v);
    }

    /**
     * Writes an optional long to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional long to write to the stream.
     */
    public void writeLong(int tag, OptionalLong v) {
        if (v != null && v.isPresent()) {
            writeLong(tag, v.getAsLong());
        }
    }

    /**
     * Writes an optional long to the stream.
     *
     * @param tag The optional tag.
     * @param v The long to write to the stream.
     */
    public void writeLong(int tag, long v) {
        if (writeOptional(tag, OptionalFormat.F8)) {
            writeLong(v);
        }
    }

    /**
     * Writes a long sequence to the stream.
     *
     * @param v The long sequence to write to the stream. Passing <code>null</code> causes an empty
     *     sequence to be written to the stream.
     */
    public void writeLongSeq(long[] v) {
        if (v == null) {
            writeSize(0);
        } else {
            writeSize(v.length);
            expand(v.length * 8);
            LongBuffer longBuf = _buf.b.asLongBuffer();
            longBuf.put(v);
            _buf.position(_buf.b.position() + v.length * 8);
        }
    }

    /**
     * Writes an optional long sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional long sequence to write to the stream.
     */
    public void writeLongSeq(int tag, Optional<long[]> v) {
        if (v != null && v.isPresent()) {
            writeLongSeq(tag, v.get());
        }
    }

    /**
     * Writes an optional long sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The long sequence to write to the stream.
     */
    public void writeLongSeq(int tag, long[] v) {
        if (writeOptional(tag, OptionalFormat.VSize)) {
            writeSize(v == null || v.length == 0 ? 1 : v.length * 8 + (v.length > 254 ? 5 : 1));
            writeLongSeq(v);
        }
    }

    /**
     * Writes the remaining contents of the long buffer as a long sequence to the stream.
     *
     * @param v The long buffer to write to the stream.
     */
    public void writeLongBuffer(LongBuffer v) {
        if (v == null || v.remaining() == 0) {
            writeSize(0);
        } else {
            int sz = v.remaining();
            writeSize(sz);
            expand(sz * 8);

            LongBuffer longBuf = _buf.b.asLongBuffer();
            longBuf.put(v);
            _buf.position(_buf.b.position() + sz * 8);
        }
    }

    /**
     * Writes a float to the stream.
     *
     * @param v The float to write to the stream.
     */
    public void writeFloat(float v) {
        expand(4);
        _buf.b.putFloat(v);
    }

    /**
     * Writes an optional float to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional float to write to the stream.
     */
    public void writeFloat(int tag, Optional<Float> v) {
        if (v != null && v.isPresent()) {
            writeFloat(tag, v.get());
        }
    }

    /**
     * Writes an optional float to the stream.
     *
     * @param tag The optional tag.
     * @param v The float to write to the stream.
     */
    public void writeFloat(int tag, float v) {
        if (writeOptional(tag, OptionalFormat.F4)) {
            writeFloat(v);
        }
    }

    /**
     * Writes a float sequence to the stream.
     *
     * @param v The float sequence to write to the stream. Passing <code>null</code> causes an empty
     *     sequence to be written to the stream.
     */
    public void writeFloatSeq(float[] v) {
        if (v == null) {
            writeSize(0);
        } else {
            writeSize(v.length);
            expand(v.length * 4);
            FloatBuffer floatBuf = _buf.b.asFloatBuffer();
            floatBuf.put(v);
            _buf.position(_buf.b.position() + v.length * 4);
        }
    }

    /**
     * Writes an optional float sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional float sequence to write to the stream.
     */
    public void writeFloatSeq(int tag, Optional<float[]> v) {
        if (v != null && v.isPresent()) {
            writeFloatSeq(tag, v.get());
        }
    }

    /**
     * Writes an optional float sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The float sequence to write to the stream.
     */
    public void writeFloatSeq(int tag, float[] v) {
        if (writeOptional(tag, OptionalFormat.VSize)) {
            writeSize(v == null || v.length == 0 ? 1 : v.length * 4 + (v.length > 254 ? 5 : 1));
            writeFloatSeq(v);
        }
    }

    /**
     * Writes the remaining contents of the float buffer as a float sequence to the stream.
     *
     * @param v The float buffer to write to the stream.
     */
    public void writeFloatBuffer(FloatBuffer v) {
        if (v == null || v.remaining() == 0) {
            writeSize(0);
        } else {
            int sz = v.remaining();
            writeSize(sz);
            expand(sz * 4);

            FloatBuffer floatBuf = _buf.b.asFloatBuffer();
            floatBuf.put(v);
            _buf.position(_buf.b.position() + sz * 4);
        }
    }

    /**
     * Writes a double to the stream.
     *
     * @param v The double to write to the stream.
     */
    public void writeDouble(double v) {
        expand(8);
        _buf.b.putDouble(v);
    }

    /**
     * Writes an optional double to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional double to write to the stream.
     */
    public void writeDouble(int tag, OptionalDouble v) {
        if (v != null && v.isPresent()) {
            writeDouble(tag, v.getAsDouble());
        }
    }

    /**
     * Writes an optional double to the stream.
     *
     * @param tag The optional tag.
     * @param v The double to write to the stream.
     */
    public void writeDouble(int tag, double v) {
        if (writeOptional(tag, OptionalFormat.F8)) {
            writeDouble(v);
        }
    }

    /**
     * Writes a double sequence to the stream.
     *
     * @param v The double sequence to write to the stream. Passing <code>null</code> causes an
     *     empty sequence to be written to the stream.
     */
    public void writeDoubleSeq(double[] v) {
        if (v == null) {
            writeSize(0);
        } else {
            writeSize(v.length);
            expand(v.length * 8);
            DoubleBuffer doubleBuf = _buf.b.asDoubleBuffer();
            doubleBuf.put(v);
            _buf.position(_buf.b.position() + v.length * 8);
        }
    }

    /**
     * Writes an optional double sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional double sequence to write to the stream.
     */
    public void writeDoubleSeq(int tag, Optional<double[]> v) {
        if (v != null && v.isPresent()) {
            writeDoubleSeq(tag, v.get());
        }
    }

    /**
     * Writes an optional double sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The double sequence to write to the stream.
     */
    public void writeDoubleSeq(int tag, double[] v) {
        if (writeOptional(tag, OptionalFormat.VSize)) {
            writeSize(v == null || v.length == 0 ? 1 : v.length * 8 + (v.length > 254 ? 5 : 1));
            writeDoubleSeq(v);
        }
    }

    /**
     * Writes the remaining contents of the double buffer as a double sequence to the stream.
     *
     * @param v The double buffer to write to the stream.
     */
    public void writeDoubleBuffer(DoubleBuffer v) {
        if (v == null || v.remaining() == 0) {
            writeSize(0);
        } else {
            int sz = v.remaining();
            writeSize(sz);
            expand(sz * 8);

            DoubleBuffer doubleBuf = _buf.b.asDoubleBuffer();
            doubleBuf.put(v);
            _buf.position(_buf.b.position() + sz * 8);
        }
    }

    static final Charset _utf8 = Charset.forName("UTF8");
    private CharsetEncoder _charEncoder;

    /**
     * Writes a string to the stream.
     *
     * @param v The string to write to the stream. Passing <code>null</code> causes an empty string
     *     to be written to the stream.
     */
    public void writeString(String v) {
        if (v == null) {
            writeSize(0);
        } else {
            final int len = v.length();
            if (len > 0) {
                if (_stringBytes == null || len > _stringBytes.length) {
                    _stringBytes = new byte[len];
                }
                if (_stringChars == null || len > _stringChars.length) {
                    _stringChars = new char[len];
                }
                //
                // If the string contains only 7-bit characters, it's more efficient
                // to perform the conversion to UTF-8 manually.
                //
                v.getChars(0, len, _stringChars, 0);
                for (int i = 0; i < len; i++) {
                    if (_stringChars[i] > (char) 127) {
                        //
                        // Found a multibyte character.
                        //
                        if (_charEncoder == null) {
                            _charEncoder = _utf8.newEncoder();
                        }
                        ByteBuffer b = null;
                        try {
                            b = _charEncoder.encode(CharBuffer.wrap(_stringChars, 0, len));
                        } catch (CharacterCodingException ex) {
                            throw new MarshalException("failed to encode multibyte character", ex);
                        }
                        writeSize(b.limit());
                        expand(b.limit());
                        _buf.b.put(b);
                        return;
                    }
                    _stringBytes[i] = (byte) _stringChars[i];
                }
                writeSize(len);
                expand(len);
                _buf.b.put(_stringBytes, 0, len);
            } else {
                writeSize(0);
            }
        }
    }

    /**
     * Writes an optional string to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional string to write to the stream.
     */
    public void writeString(int tag, Optional<String> v) {
        if (v != null && v.isPresent()) {
            writeString(tag, v.get());
        }
    }

    /**
     * Writes an optional string to the stream.
     *
     * @param tag The optional tag.
     * @param v The string to write to the stream.
     */
    public void writeString(int tag, String v) {
        if (writeOptional(tag, OptionalFormat.VSize)) {
            writeString(v);
        }
    }

    /**
     * Writes a string sequence to the stream.
     *
     * @param v The string sequence to write to the stream. Passing <code>null</code> causes an
     *     empty sequence to be written to the stream.
     */
    public void writeStringSeq(String[] v) {
        if (v == null) {
            writeSize(0);
        } else {
            writeSize(v.length);
            for (String e : v) {
                writeString(e);
            }
        }
    }

    /**
     * Writes an optional string sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional string sequence to write to the stream.
     */
    public void writeStringSeq(int tag, Optional<String[]> v) {
        if (v != null && v.isPresent()) {
            writeStringSeq(tag, v.get());
        }
    }

    /**
     * Writes an optional string sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The string sequence to write to the stream.
     */
    public void writeStringSeq(int tag, String[] v) {
        if (writeOptional(tag, OptionalFormat.FSize)) {
            int pos = startSize();
            writeStringSeq(v);
            endSize(pos);
        }
    }

    /**
     * Writes a proxy to the stream.
     *
     * @param v The proxy to write.
     */
    public void writeProxy(ObjectPrx v) {
        if (v != null) {
            v._write(this);
        } else {
            Identity ident = new Identity();
            ident.ice_writeMembers(this);
        }
    }

    /**
     * Writes an optional proxy to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional proxy to write to the stream.
     */
    public <Prx extends ObjectPrx> void writeProxy(int tag, Optional<Prx> v) {
        if (v != null && v.isPresent()) {
            writeProxy(tag, v.get());
        }
    }

    /**
     * Writes an optional proxy to the stream.
     *
     * @param tag The optional tag.
     * @param v The proxy to write to the stream.
     */
    public void writeProxy(int tag, ObjectPrx v) {
        if (writeOptional(tag, OptionalFormat.FSize)) {
            int pos = startSize();
            writeProxy(v);
            endSize(pos);
        }
    }

    /**
     * Write an enumerated value.
     *
     * @param v The enumerator.
     * @param maxValue The maximum enumerator value in the definition.
     */
    public void writeEnum(int v, int maxValue) {
        if (isEncoding_1_0()) {
            if (maxValue < 127) {
                writeByte((byte) v);
            } else if (maxValue < 32767) {
                writeShort((short) v);
            } else {
                writeInt(v);
            }
        } else {
            writeSize(v);
        }
    }

    /**
     * Writes a Slice value to the stream.
     *
     * @param v The value to write. This method writes the index of an instance; the state of the
     *     value is written once {@link #writePendingValues} is called.
     */
    public void writeValue(Value v) {
        initEncaps();
        _encapsStack.encoder.writeValue(v);
    }

    /**
     * Writes a user exception to the stream.
     *
     * @param v The user exception to write.
     */
    public void writeException(UserException v) {
        initEncaps();
        // Exceptions are always encoded with the sliced format.
        _encapsStack.format = FormatType.SlicedFormat;
        _encapsStack.encoder.writeException(v);
    }

    private boolean writeOptionalImpl(int tag, OptionalFormat format) {
        if (isEncoding_1_0()) {
            return false; // Optional members aren't supported with the 1.0 encoding.
        }

        int v = format.value();
        if (tag < 30) {
            v |= tag << 3;
            writeByte((byte) v);
        } else {
            v |= 0x0F0; // tag = 30
            writeByte((byte) v);
            writeSize(tag);
        }
        return true;
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
     * @return True if no data has been written yet, false otherwise.
     */
    public boolean isEmpty() {
        return _buf.empty();
    }

    /**
     * Expand the stream to accept more data.
     *
     * @param n The number of bytes to accommodate in the stream.
     */
    public void expand(int n) {
        _buf.expand(n);
    }

    private Buffer _buf;
    private FormatType _format;
    private byte[] _stringBytes; // Reusable array for string operations.
    private char[] _stringChars; // Reusable array for string operations.

    private enum SliceType {
        NoSlice,
        ValueSlice,
        ExceptionSlice
    }

    private abstract static class EncapsEncoder {
        protected EncapsEncoder(OutputStream stream, Encaps encaps) {
            _stream = stream;
            _encaps = encaps;
            _typeIdIndex = 0;
            _marshaledMap = new IdentityHashMap<>();
        }

        abstract void writeValue(Value v);

        abstract void writeException(UserException v);

        abstract void startInstance(SliceType type, SlicedData data);

        abstract void endInstance();

        abstract void startSlice(String typeId, int compactId, boolean last);

        abstract void endSlice();

        boolean writeOptional(int tag, OptionalFormat format) {
            return false;
        }

        void writePendingValues() {}

        protected int registerTypeId(String typeId) {
            if (_typeIdMap == null) // Lazy initialization
                {
                    _typeIdMap = new TreeMap<>();
                }

            Integer p = _typeIdMap.get(typeId);
            if (p != null) {
                return p;
            } else {
                _typeIdMap.put(typeId, ++_typeIdIndex);
                return -1;
            }
        }

        protected final OutputStream _stream;
        protected final Encaps _encaps;

        // Encapsulation attributes for instance marshaling.
        protected final IdentityHashMap<Value, Integer> _marshaledMap;
        private TreeMap<String, Integer> _typeIdMap;
        private int _typeIdIndex;
    }

    private static final class EncapsEncoder10 extends EncapsEncoder {
        EncapsEncoder10(OutputStream stream, Encaps encaps) {
            super(stream, encaps);
            _sliceType = SliceType.NoSlice;
            _valueIdIndex = 0;
            _toBeMarshaledMap = new IdentityHashMap<>();
        }

        @Override
        void writeValue(Value v) {
            //
            // Object references are encoded as a negative integer in 1.0.
            //
            if (v != null) {
                _stream.writeInt(-registerValue(v));
            } else {
                _stream.writeInt(0);
            }
        }

        @Override
        void writeException(UserException v) {
            //
            // User exception with the 1.0 encoding start with a boolean flag that indicates whether
            // or not the exception uses classes.
            //
            // This allows reading the pending instances even if some part of the exception was
            // sliced.
            //
            boolean usesClasses = v._usesClasses();
            _stream.writeBool(usesClasses);
            v._write(_stream);
            if (usesClasses) {
                writePendingValues();
            }
        }

        @Override
        void startInstance(SliceType sliceType, SlicedData sliceData) {
            _sliceType = sliceType;
        }

        @Override
        void endInstance() {
            if (_sliceType == SliceType.ValueSlice) {
                //
                // Write the Object slice.
                //
                startSlice(Value.ice_staticId(), -1, true);
                _stream.writeSize(0); // For compatibility with the old AFM.
                endSlice();
            }
            _sliceType = SliceType.NoSlice;
        }

        @Override
        void startSlice(String typeId, int compactId, boolean last) {
            //
            // For instance slices, encode a boolean to indicate how the type ID
            // is encoded and the type ID either as a string or index. For exception slices, always
            // encode the type ID as a string.
            //
            if (_sliceType == SliceType.ValueSlice) {
                int index = registerTypeId(typeId);
                if (index < 0) {
                    _stream.writeBool(false);
                    _stream.writeString(typeId);
                } else {
                    _stream.writeBool(true);
                    _stream.writeSize(index);
                }
            } else {
                _stream.writeString(typeId);
            }

            _stream.writeInt(0); // Placeholder for the slice length.

            _writeSlice = _stream.pos();
        }

        @Override
        void endSlice() {
            //
            // Write the slice length.
            //
            final int sz = _stream.pos() - _writeSlice + 4;
            _stream.rewriteInt(sz, _writeSlice - 4);
        }

        @Override
        void writePendingValues() {
            while (!_toBeMarshaledMap.isEmpty()) {
                //
                // Consider the to be marshaled instances as marshaled now, this is necessary to
                // avoid adding again the "to be
                // marshaled instances" into _toBeMarshaledMap while writing
                // instances.
                //
                _marshaledMap.putAll(_toBeMarshaledMap);

                IdentityHashMap<Value, Integer> savedMap = _toBeMarshaledMap;
                _toBeMarshaledMap = new IdentityHashMap<>();
                _stream.writeSize(savedMap.size());
                for (Map.Entry<Value, Integer> p : savedMap.entrySet()) {
                    //
                    // Ask the instance to marshal itself. Any new class instances that are
                    // triggered by the classes marshaled are added to toBeMarshaledMap.
                    //
                    _stream.writeInt(p.getValue().intValue());

                    p.getKey().ice_preMarshal();
                    p.getKey()._iceWrite(_stream);
                }
            }
            _stream.writeSize(
                0); // Zero marker indicates end of sequence of sequences of instances.
        }

        private int registerValue(Value v) {
            assert (v != null);

            //
            // Look for this instance in the to-be-marshaled map.
            //
            Integer p = _toBeMarshaledMap.get(v);
            if (p != null) {
                return p.intValue();
            }

            //
            // Didn't find it, try the marshaled map next.
            //
            p = _marshaledMap.get(v);
            if (p != null) {
                return p.intValue();
            }

            //
            // We haven't seen this instance previously, create a new
            // index, and insert it into the to-be-marshaled map.
            //
            _toBeMarshaledMap.put(v, ++_valueIdIndex);
            return _valueIdIndex;
        }

        // Instance attributes
        private SliceType _sliceType;

        // Slice attributes
        private int _writeSlice; // Position of the slice data members

        // Encapsulation attributes for instance marshaling.
        private int _valueIdIndex;
        private IdentityHashMap<Value, Integer> _toBeMarshaledMap;
    }

    private static final class EncapsEncoder11 extends EncapsEncoder {
        EncapsEncoder11(OutputStream stream, Encaps encaps) {
            super(stream, encaps);
            _current = null;
            _valueIdIndex = 1;
        }

        @Override
        void writeValue(Value v) {
            if (v == null) {
                _stream.writeSize(0);
            } else if (_current != null && _encaps.format == FormatType.SlicedFormat) {
                if (_current.indirectionTable == null) // Lazy initialization
                    {
                        _current.indirectionTable = new ArrayList<>();
                        _current.indirectionMap = new IdentityHashMap<>();
                    }

                //
                // If writing an instance within a slice and using the sliced format, write an index
                // from the instance indirection table. The indirect instance table is encoded at
                // the end of each slice and is always read (even if the Slice is unknown).
                //
                Integer index = _current.indirectionMap.get(v);
                if (index == null) {
                    _current.indirectionTable.add(v);
                    final int idx =
                        _current.indirectionTable
                            .size(); // Position + 1 (0 is reserved for nil)
                    _current.indirectionMap.put(v, idx);
                    _stream.writeSize(idx);
                } else {
                    _stream.writeSize(index.intValue());
                }
            } else {
                writeInstance(v); // Write the instance or a reference if already marshaled.
            }
        }

        @Override
        void writeException(UserException v) {
            v._write(_stream);
        }

        @Override
        void startInstance(SliceType sliceType, SlicedData data) {
            if (_current == null) {
                _current = new InstanceData(null);
            } else {
                _current = _current.next == null ? new InstanceData(_current) : _current.next;
            }
            _current.sliceType = sliceType;
            _current.firstSlice = true;

            if (data != null) {
                writeSlicedData(data);
            }
        }

        @Override
        void endInstance() {
            _current = _current.previous;
        }

        @Override
        void startSlice(String typeId, int compactId, boolean last) {
            assert ((_current.indirectionTable == null || _current.indirectionTable.isEmpty())
                && (_current.indirectionMap == null || _current.indirectionMap.isEmpty()));

            _current.sliceFlagsPos = _stream.pos();

            _current.sliceFlags = (byte) 0;
            if (_encaps.format == FormatType.SlicedFormat) {
                // Encode the slice size if using the sliced format.
                _current.sliceFlags |= Protocol.FLAG_HAS_SLICE_SIZE;
            }
            if (last) {
                _current.sliceFlags |= Protocol.FLAG_IS_LAST_SLICE; // This is the last slice.
            }

            _stream.writeByte((byte) 0); // Placeholder for the slice flags

            //
            // For instance slices, encode the flag and the type ID either as a string or index. For
            // exception slices, always encode the type ID a string.
            //
            if (_current.sliceType == SliceType.ValueSlice) {
                //
                // Encode the type ID (only in the first slice for the compact encoding).
                //
                if (_encaps.format == FormatType.SlicedFormat || _current.firstSlice) {
                    if (compactId != -1) {
                        _current.sliceFlags |= Protocol.FLAG_HAS_TYPE_ID_COMPACT;
                        _stream.writeSize(compactId);
                    } else {
                        int index = registerTypeId(typeId);
                        if (index < 0) {
                            _current.sliceFlags |= Protocol.FLAG_HAS_TYPE_ID_STRING;
                            _stream.writeString(typeId);
                        } else {
                            _current.sliceFlags |= Protocol.FLAG_HAS_TYPE_ID_INDEX;
                            _stream.writeSize(index);
                        }
                    }
                }
            } else {
                _stream.writeString(typeId);
            }

            if ((_current.sliceFlags & Protocol.FLAG_HAS_SLICE_SIZE) != 0) {
                _stream.writeInt(0); // Placeholder for the slice length.
            }

            _current.writeSlice = _stream.pos();
            _current.firstSlice = false;
        }

        @Override
        void endSlice() {
            //
            // Write the optional member end marker if some optional members were encoded. Note that
            // the optional members are encoded before the indirection table and are included in the
            // slice size.
            //
            if ((_current.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS) != 0) {
                _stream.writeByte((byte) Protocol.OPTIONAL_END_MARKER);
            }

            //
            // Write the slice length if necessary.
            //
            if ((_current.sliceFlags & Protocol.FLAG_HAS_SLICE_SIZE) != 0) {
                final int sz = _stream.pos() - _current.writeSlice + 4;
                _stream.rewriteInt(sz, _current.writeSlice - 4);
            }

            //
            // Only write the indirection table if it contains entries.
            //
            if (_current.indirectionTable != null && !_current.indirectionTable.isEmpty()) {
                assert (_encaps.format == FormatType.SlicedFormat);
                _current.sliceFlags |= Protocol.FLAG_HAS_INDIRECTION_TABLE;

                //
                // Write the indirection instance table.
                //
                _stream.writeSize(_current.indirectionTable.size());
                for (Value v : _current.indirectionTable) {
                    writeInstance(v);
                }
                _current.indirectionTable.clear();
                _current.indirectionMap.clear();
            }

            //
            // Finally, update the slice flags.
            //
            _stream.rewriteByte(_current.sliceFlags, _current.sliceFlagsPos);
        }

        @Override
        boolean writeOptional(int tag, OptionalFormat format) {
            if (_current == null) {
                return _stream.writeOptionalImpl(tag, format);
            } else {
                if (_stream.writeOptionalImpl(tag, format)) {
                    _current.sliceFlags |= Protocol.FLAG_HAS_OPTIONAL_MEMBERS;
                    return true;
                } else {
                    return false;
                }
            }
        }

        private void writeSlicedData(SlicedData slicedData) {
            assert (slicedData != null);

            //
            // We only remarshal preserved slices if we are using the sliced
            // format. Otherwise, we ignore the preserved slices, which
            // essentially "slices" the instance into the most-derived type
            // known by the sender.
            //
            if (_encaps.format != FormatType.SlicedFormat) {
                return;
            }

            for (SliceInfo info : slicedData.slices) {
                startSlice(info.typeId, info.compactId, info.isLastSlice);

                //
                // Write the bytes associated with this slice.
                //
                _stream.writeBlob(info.bytes);

                if (info.hasOptionalMembers) {
                    _current.sliceFlags |= Protocol.FLAG_HAS_OPTIONAL_MEMBERS;
                }

                //
                // Make sure to also re-write the instance indirection table.
                //
                if (info.instances != null && info.instances.length > 0) {
                    if (_current.indirectionTable == null) // Lazy initialization
                        {
                            _current.indirectionTable = new ArrayList<>();
                            _current.indirectionMap = new IdentityHashMap<>();
                        }
                    for (Value o : info.instances) {
                        _current.indirectionTable.add(o);
                    }
                }

                endSlice();
            }
        }

        private void writeInstance(Value v) {
            assert (v != null);

            //
            // If the instance was already marshaled, just write it's ID.
            //
            Integer p = _marshaledMap.get(v);
            if (p != null) {
                _stream.writeSize(p);
                return;
            }

            //
            // We haven't seen this instance previously, create a new ID,
            // insert it into the marshaled map, and write the instance.
            //
            _marshaledMap.put(v, ++_valueIdIndex);

            v.ice_preMarshal();
            _stream.writeSize(1); // Class instance marker.
            v._iceWrite(_stream);
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
            boolean firstSlice;

            // Slice attributes
            byte sliceFlags;
            int writeSlice; // Position of the slice data members
            int sliceFlagsPos; // Position of the slice flags
            List<Value> indirectionTable;
            IdentityHashMap<Value, Integer> indirectionMap;

            final InstanceData previous;
            InstanceData next;
        }

        private InstanceData _current;

        private int _valueIdIndex; // The ID of the next instance to marhsal
    }

    private static final class Encaps {
        void reset() {
            encoder = null;
        }

        void setEncoding(EncodingVersion encoding) {
            this.encoding = encoding;
            encoding_1_0 = encoding.equals(Util.Encoding_1_0);
        }

        int start;
        FormatType format;
        EncodingVersion encoding;
        boolean encoding_1_0;

        EncapsEncoder encoder;

        Encaps next;
    }

    //
    // The encoding version to use when there's no encapsulation to read from or write to. This is
    // for example used to read message headers or when the user is using the streaming API with no
    // encapsulation.
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
            }

        if (_encapsStack.format == null) {
            _encapsStack.format = _format;
        }

        if (_encapsStack.encoder == null) // Lazy initialization.
            {
                if (_encapsStack.encoding_1_0) {
                    _encapsStack.encoder = new EncapsEncoder10(this, _encapsStack);
                } else {
                    _encapsStack.encoder = new EncapsEncoder11(this, _encapsStack);
                }
            }
    }

    /**
     * @hidden
     */
    @FunctionalInterface
    public static interface Marshaler {
        void marshal(OutputStream ostr);
    }
}
