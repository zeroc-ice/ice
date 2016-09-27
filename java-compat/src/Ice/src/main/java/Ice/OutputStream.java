// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public class OutputStream
{
    /**
     * Constructing an OutputStream without providing a communicator means the stream will
     * use the default encoding version, the default format for class encoding, and a
     * non-direct buffer. You can supply a communicator later by calling initialize().
     **/
    public OutputStream()
    {
        this(false);
    }

    /**
     * Constructing an OutputStream without providing a communicator means the stream will
     * use the default encoding version and the default format for class encoding.
     * You can supply a communicator later by calling initialize().
     *
     * @param direct Indicates whether to use a direct buffer.
     **/
    public OutputStream(boolean direct)
    {
        _buf = new IceInternal.Buffer(direct);
        _instance = null;
        _closure = null;
        _encoding = IceInternal.Protocol.currentEncoding;
        _format = FormatType.CompactFormat;
    }

    /**
     * This constructor uses the communicator's default encoding version.
     *
     * @param communicator The communicator to use when initializing the stream.
     **/
    public OutputStream(Communicator communicator)
    {
        assert(communicator != null);
        final IceInternal.Instance instance = IceInternal.Util.getInstance(communicator);
        initialize(instance, instance.defaultsAndOverrides().defaultEncoding, instance.cacheMessageBuffers() > 1);
    }

    /**
     * This constructor uses the communicator's default encoding version.
     *
     * @param communicator The communicator to use when initializing the stream.
     * @param direct Indicates whether to use a direct buffer.
     **/
    public OutputStream(Communicator communicator, boolean direct)
    {
        assert(communicator != null);
        final IceInternal.Instance instance = IceInternal.Util.getInstance(communicator);
        initialize(instance, instance.defaultsAndOverrides().defaultEncoding, direct);
    }

    /**
     * This constructor uses the given communicator and encoding version.
     *
     * @param communicator The communicator to use when initializing the stream.
     * @param encoding The desired Ice encoding version.
     **/
    public OutputStream(Communicator communicator, EncodingVersion encoding)
    {
        assert(communicator != null);
        final IceInternal.Instance instance = IceInternal.Util.getInstance(communicator);
        initialize(instance, encoding, instance.cacheMessageBuffers() > 1);
    }

    /**
     * This constructor uses the given communicator and encoding version.
     *
     * @param communicator The communicator to use when initializing the stream.
     * @param encoding The desired Ice encoding version.
     * @param direct Indicates whether to use a direct buffer.
     **/
    public OutputStream(Communicator communicator, EncodingVersion encoding, boolean direct)
    {
        assert(communicator != null);
        final IceInternal.Instance instance = IceInternal.Util.getInstance(communicator);
        initialize(instance, encoding, direct);
    }

    public OutputStream(IceInternal.Instance instance, EncodingVersion encoding)
    {
        initialize(instance, encoding, instance.cacheMessageBuffers() > 1);
    }

    public OutputStream(IceInternal.Instance instance, EncodingVersion encoding, boolean direct)
    {
        initialize(instance, encoding, direct);
    }

    public OutputStream(IceInternal.Instance instance, EncodingVersion encoding, IceInternal.Buffer buf, boolean adopt)
    {
        initialize(instance, encoding, new IceInternal.Buffer(buf, adopt));
    }

    /**
     * Initializes the stream to use the communicator's default encoding version and class
     * encoding format.
     *
     * @param communicator The communicator to use when initializing the stream.
     **/
    public void initialize(Communicator communicator)
    {
        assert(communicator != null);
        final IceInternal.Instance instance = IceInternal.Util.getInstance(communicator);
        initialize(instance, instance.defaultsAndOverrides().defaultEncoding, instance.cacheMessageBuffers() > 1);
    }

    /**
     * Initializes the stream to use the given encoding version and the communicator's
     * default class encoding format.
     *
     * @param communicator The communicator to use when initializing the stream.
     * @param encoding The desired Ice encoding version.
     **/
    public void initialize(Communicator communicator, EncodingVersion encoding)
    {
        assert(communicator != null);
        final IceInternal.Instance instance = IceInternal.Util.getInstance(communicator);
        initialize(instance, encoding, instance.cacheMessageBuffers() > 1);
    }

    private void initialize(IceInternal.Instance instance, EncodingVersion encoding, boolean direct)
    {
        initialize(instance, encoding, new IceInternal.Buffer(direct));
    }

    private void initialize(IceInternal.Instance instance, EncodingVersion encoding, IceInternal.Buffer buf)
    {
        assert(instance != null);

        _instance = instance;
        _buf = buf;
        _closure = null;
        _encoding = encoding;

        _format = _instance.defaultsAndOverrides().defaultFormat;

        _encapsStack = null;
        _encapsCache = null;
    }

    /**
     * Resets this output stream. This method allows the stream to be reused, to avoid creating
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
    }

    public IceInternal.Instance instance()
    {
        return _instance;
    }

    /**
     * Sets the encoding format for class and exception instances.
     *
     * @param fmt The encoding format.
     **/
    public void setFormat(FormatType fmt)
    {
        _format = fmt;
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

    /**
     * Indicates that marshaling is finished.
     *
     * @return The byte sequence containing the encoded data.
     **/
    public byte[] finished()
    {
        IceInternal.Buffer buf = prepareWrite();
        byte[] result = new byte[buf.b.limit()];
        buf.b.get(result);
        return result;
    }

    /**
     * Swaps the contents of one stream with another.
     *
     * @param other The other stream.
     **/
    public void swap(OutputStream other)
    {
        assert(_instance == other._instance);

        IceInternal.Buffer tmpBuf = other._buf;
        other._buf = _buf;
        _buf = tmpBuf;

        EncodingVersion tmpEncoding = other._encoding;
        other._encoding = _encoding;
        _encoding = tmpEncoding;

        Object tmpClosure = other._closure;
        other._closure = _closure;
        _closure = tmpClosure;

        //
        // Swap is never called for streams that have encapsulations being written. However,
        // encapsulations might still be set in case marshalling failed. We just
        // reset the encapsulations if there are still some set.
        //
        resetEncapsulation();
        other.resetEncapsulation();
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
        _buf.resize(sz, false);
        _buf.b.position(sz);
    }

    /**
     * Prepares the internal data buffer to be written to a socket.
     *
     * @return The internal buffer.
     **/
    public IceInternal.Buffer prepareWrite()
    {
        _buf.b.limit(_buf.size());
        _buf.b.position(0);
        return _buf;
    }

    /**
     * Retrieves the internal data buffer.
     *
     * @return The buffer.
     **/
    public IceInternal.Buffer getBuffer()
    {
        return _buf;
    }

    /**
     * Marks the start of a class instance.
     *
     * @param data Preserved slices for this instance, or null.
     **/
    public void startValue(SlicedData data)
    {
        assert(_encapsStack != null && _encapsStack.encoder != null);
        _encapsStack.encoder.startInstance(SliceType.ValueSlice, data);
    }

    /**
     * Marks the end of a class instance.
     **/
    public void endValue()
    {
        assert(_encapsStack != null && _encapsStack.encoder != null);
        _encapsStack.encoder.endInstance();
    }

    /**
     * Marks the start of a user exception.
     *
     * @param data Preserved slices for this exception, or null.
     **/
    public void startException(SlicedData data)
    {
        assert(_encapsStack != null && _encapsStack.encoder != null);
        _encapsStack.encoder.startInstance(SliceType.ExceptionSlice, data);
    }

    /**
     * Marks the end of a user exception.
     **/
    public void endException()
    {
        assert(_encapsStack != null && _encapsStack.encoder != null);
        _encapsStack.encoder.endInstance();
    }

    /**
     * Writes the start of an encapsulation to the stream.
     **/
    public void startEncapsulation()
    {
        //
        // If no encoding version is specified, use the current write
        // encapsulation encoding version if there's a current write
        // encapsulation, otherwise, use the stream encoding version.
        //

        if(_encapsStack != null)
        {
            startEncapsulation(_encapsStack.encoding, _encapsStack.format);
        }
        else
        {
            startEncapsulation(_encoding, FormatType.DefaultFormat);
        }
    }

    /**
     * Writes the start of an encapsulation to the stream.
     *
     * @param encoding The encoding version of the encapsulation.
     *
     * @param format Specify the compact or sliced format.
     *
     **/
    public void startEncapsulation(EncodingVersion encoding, FormatType format)
    {
        IceInternal.Protocol.checkSupportedEncoding(encoding);

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

        _encapsStack.format = format;
        _encapsStack.setEncoding(encoding);
        _encapsStack.start = _buf.size();

        writeInt(0); // Placeholder for the encapsulation length.
        _encapsStack.encoding.__write(this);
    }

    /**
     * Ends the previous encapsulation.
     **/
    public void endEncapsulation()
    {
        assert(_encapsStack != null);

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
     **/
    public void writeEmptyEncapsulation(EncodingVersion encoding)
    {
        IceInternal.Protocol.checkSupportedEncoding(encoding);
        writeInt(6); // Size
        encoding.__write(this);
    }

    /**
     * Writes a pre-encoded encapsulation.
     *
     * @param v The encapsulation data.
     **/
    public void writeEncapsulation(byte[] v)
    {
        if(v.length < 6)
        {
            throw new EncapsulationException();
        }
        expand(v.length);
        _buf.b.put(v);
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
     * Marks the start of a new slice for a class instance or user exception.
     *
     * @param typeId The Slice type ID corresponding to this slice.

     * @param compactId The Slice compact type ID corresponding to
     *        this slice or -1 if no compact ID is defined for the
     *        type ID.

     * @param last True if this is the last slice, false otherwise.
     **/
    public void startSlice(String typeId, int compactId, boolean last)
    {
        assert(_encapsStack != null && _encapsStack.encoder != null);
        _encapsStack.encoder.startSlice(typeId, compactId, last);
    }

    /**
     * Marks the end of a slice for a class instance or user exception.
     **/
    public void endSlice()
    {
        assert(_encapsStack != null && _encapsStack.encoder != null);
        _encapsStack.encoder.endSlice();
    }

    /**
     * Writes the state of Slice classes whose index was previously
     * written with {@link #writeValue} to the stream.
     **/
    public void writePendingValues()
    {
        if(_encapsStack != null && _encapsStack.encoder != null)
        {
            _encapsStack.encoder.writePendingValues();
        }
        else if(_encapsStack != null ? _encapsStack.encoding_1_0 : _encoding.equals(Util.Encoding_1_0))
        {
            //
            // If using the 1.0 encoding and no instances were written, we
            // still write an empty sequence for pending instances if
            // requested (i.e.: if this is called).
            //
            // This is required by the 1.0 encoding, even if no instances
            // are written we do marshal an empty sequence if marshaled
            // data types use classes.
            //
            writeSize(0);
        }
    }

    /**
     * Writes a size to the stream.
     *
     * @param v The size to write.
     **/
    public void writeSize(int v)
    {
        if(v > 254)
        {
            expand(5);
            _buf.b.put((byte)-1);
            _buf.b.putInt(v);
        }
        else
        {
            expand(1);
            _buf.b.put((byte)v);
        }
    }

    /**
     * Returns the current position and allocates four bytes for a fixed-length (32-bit)
     * size value.
     *
     * @return The current position.
     **/
    public int startSize()
    {
        int pos = _buf.b.position();
        writeInt(0); // Placeholder for 32-bit size
        return pos;
    }

    /**
     * Computes the amount of data written since the previous call to startSize and
     * writes that value at the saved position.
     *
     * @param pos The saved position.
     **/
    public void endSize(int pos)
    {
        assert(pos >= 0);
        rewriteInt(_buf.b.position() - pos - 4, pos);
    }

    /**
     * Writes a blob of bytes to the stream.
     *
     * @param v The byte array to be written. All of the bytes in the array are written.
     **/
    public void writeBlob(byte[] v)
    {
        if(v == null)
        {
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
     **/
    public void writeBlob(byte[] v, int off, int len)
    {
        if(v == null)
        {
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
     **/
    public boolean writeOptional(int tag, OptionalFormat format)
    {
        assert(_encapsStack != null);
        if(_encapsStack.encoder != null)
        {
            return _encapsStack.encoder.writeOptional(tag, format);
        }
        else
        {
            return writeOptionalImpl(tag, format);
        }
    }

    /**
     * Writes a byte to the stream.
     *
     * @param v The byte to write to the stream.
     **/
    public void writeByte(byte v)
    {
        expand(1);
        _buf.b.put(v);
    }

    /**
     * Writes an optional byte to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional byte to write to the stream.
     **/
    public void writeByte(int tag, ByteOptional v)
    {
        if(v != null && v.isSet())
        {
            writeByte(tag, v.get());
        }
    }

    /**
     * Writes an optional byte to the stream.
     *
     * @param tag The optional tag.
     * @param v The byte to write to the stream.
     **/
    public void writeByte(int tag, byte v)
    {
        if(writeOptional(tag, OptionalFormat.F1))
        {
            writeByte(v);
        }
    }

    /**
     * Writes a byte to the stream at the given position. The current position of the stream is not modified.
     *
     * @param v The byte to write to the stream.
     * @param dest The position at which to store the byte in the buffer.
     **/
    public void rewriteByte(byte v, int dest)
    {
        _buf.b.put(dest, v);
    }

    /**
     * Writes a byte sequence to the stream.
     *
     * @param v The byte sequence to write to the stream.
     * Passing <code>null</code> causes an empty sequence to be written to the stream.
     **/
    public void writeByteSeq(byte[] v)
    {
        if(v == null)
        {
            writeSize(0);
        }
        else
        {
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
     **/
    public void writeByteSeq(int tag, Optional<byte[]> v)
    {
        if(v != null && v.isSet())
        {
            writeByteSeq(tag, v.get());
        }
    }

    /**
     * Writes an optional byte sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The byte sequence to write to the stream.
     **/
    public void writeByteSeq(int tag, byte[] v)
    {
        if(writeOptional(tag, OptionalFormat.VSize))
        {
            writeByteSeq(v);
        }
    }

    /**
     * Writes the remaining contents of the byte buffer as a byte sequence to the stream.
     *
     * @param v The byte buffer to write to the stream.
     **/
    public void writeByteBuffer(java.nio.ByteBuffer v)
    {
        if(v == null || v.remaining() == 0)
        {
            writeSize(0);
        }
        else
        {
            writeSize(v.remaining());
            expand(v.remaining());
            _buf.b.put(v);
        }
    }

    /**
     * Writes a serializable Java object to the stream.
     *
     * @param o The serializable object to write.
     **/
    public void writeSerializable(java.io.Serializable o)
    {
        if(o == null)
        {
            writeSize(0);
            return;
        }
        try
        {
            IceInternal.OutputStreamWrapper w = new IceInternal.OutputStreamWrapper(this);
            java.io.ObjectOutputStream out = new java.io.ObjectOutputStream(w);
            out.writeObject(o);
            out.close();
            w.close();
        }
        catch(java.lang.Exception ex)
        {
            throw new MarshalException("cannot serialize object: " + ex);
        }
    }

    /**
     * Writes a boolean to the stream.
     *
     * @param v The boolean to write to the stream.
     **/
    public void writeBool(boolean v)
    {
        expand(1);
        _buf.b.put(v ? (byte)1 : (byte)0);
    }

    /**
     * Writes an optional boolean to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional boolean to write to the stream.
     **/
    public void writeBool(int tag, BooleanOptional v)
    {
        if(v != null && v.isSet())
        {
            writeBool(tag, v.get());
        }
    }

    /**
     * Writes an optional boolean to the stream.
     *
     * @param tag The optional tag.
     * @param v The boolean to write to the stream.
     **/
    public void writeBool(int tag, boolean v)
    {
        if(writeOptional(tag, OptionalFormat.F1))
        {
            writeBool(v);
        }
    }

    /**
     * Writes a boolean to the stream at the given position. The current position of the stream is not modified.
     *
     * @param v The boolean to write to the stream.
     * @param dest The position at which to store the boolean in the buffer.
     **/
    public void rewriteBool(boolean v, int dest)
    {
        _buf.b.put(dest, v ? (byte)1 : (byte)0);
    }

    /**
     * Writes a boolean sequence to the stream.
     *
     * @param v The boolean sequence to write to the stream.
     * Passing <code>null</code> causes an empty sequence to be written to the stream.
     **/
    public void writeBoolSeq(boolean[] v)
    {
        if(v == null)
        {
            writeSize(0);
        }
        else
        {
            writeSize(v.length);
            expand(v.length);
            for(boolean b : v)
            {
                _buf.b.put(b ? (byte)1 : (byte)0);
            }
        }
    }

    /**
     * Writes an optional boolean sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional boolean sequence to write to the stream.
     **/
    public void writeBoolSeq(int tag, Optional<boolean[]> v)
    {
        if(v != null && v.isSet())
        {
            writeBoolSeq(tag, v.get());
        }
    }

    /**
     * Writes an optional boolean sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The boolean sequence to write to the stream.
     **/
    public void writeBoolSeq(int tag, boolean[] v)
    {
        if(writeOptional(tag, OptionalFormat.VSize))
        {
            writeBoolSeq(v);
        }
    }

    /**
     * Writes a short to the stream.
     *
     * @param v The short to write to the stream.
     **/
    public void writeShort(short v)
    {
        expand(2);
        _buf.b.putShort(v);
    }

    /**
     * Writes an optional short to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional short to write to the stream.
     **/
    public void writeShort(int tag, ShortOptional v)
    {
        if(v != null && v.isSet())
        {
            writeShort(tag, v.get());
        }
    }

    /**
     * Writes an optional short to the stream.
     *
     * @param tag The optional tag.
     * @param v The short to write to the stream.
     **/
    public void writeShort(int tag, short v)
    {
        if(writeOptional(tag, OptionalFormat.F2))
        {
            writeShort(v);
        }
    }

    /**
     * Writes a short sequence to the stream.
     *
     * @param v The short sequence to write to the stream.
     * Passing <code>null</code> causes an empty sequence to be written to the stream.
     **/
    public void writeShortSeq(short[] v)
    {
        if(v == null)
        {
            writeSize(0);
        }
        else
        {
            writeSize(v.length);
            expand(v.length * 2);
            java.nio.ShortBuffer shortBuf = _buf.b.asShortBuffer();
            shortBuf.put(v);
            _buf.b.position(_buf.b.position() + v.length * 2);
        }
    }

    /**
     * Writes an optional short sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional short sequence to write to the stream.
     **/
    public void writeShortSeq(int tag, Optional<short[]> v)
    {
        if(v != null && v.isSet())
        {
            writeShortSeq(tag, v.get());
        }
    }

    /**
     * Writes an optional short sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The short sequence to write to the stream.
     **/
    public void writeShortSeq(int tag, short[] v)
    {
        if(writeOptional(tag, OptionalFormat.VSize))
        {
            writeSize(v == null || v.length == 0 ? 1 : v.length * 2 + (v.length > 254 ? 5 : 1));
            writeShortSeq(v);
        }
    }

    /**
     * Writes the remaining contents of the short buffer as a short sequence to the stream.
     *
     * @param v The short buffer to write to the stream.
     **/
    public void writeShortBuffer(java.nio.ShortBuffer v)
    {
        if(v == null || v.remaining() == 0)
        {
            writeSize(0);
        }
        else
        {
            int sz = v.remaining();
            writeSize(sz);
            expand(sz * 2);

            java.nio.ShortBuffer shortBuf = _buf.b.asShortBuffer();
            shortBuf.put(v);
            _buf.b.position(_buf.b.position() + sz * 2);
        }
    }

    /**
     * Writes an int to the stream.
     *
     * @param v The int to write to the stream.
     **/
    public void writeInt(int v)
    {
        expand(4);
        _buf.b.putInt(v);
    }

    /**
     * Writes an optional int to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional int to write to the stream.
     **/
    public void writeInt(int tag, IntOptional v)
    {
        if(v != null && v.isSet())
        {
            writeInt(tag, v.get());
        }
    }

    /**
     * Writes an optional int to the stream.
     *
     * @param tag The optional tag.
     * @param v The int to write to the stream.
     **/
    public void writeInt(int tag, int v)
    {
        if(writeOptional(tag, OptionalFormat.F4))
        {
            writeInt(v);
        }
    }

    /**
     * Writes an int to the stream at the given position. The current position of the stream is not modified.
     *
     * @param v The int to write to the stream.
     * @param dest The position at which to store the int in the buffer.
     **/
    public void rewriteInt(int v, int dest)
    {
        _buf.b.putInt(dest, v);
    }

    /**
     * Writes an int sequence to the stream.
     *
     * @param v The int sequence to write to the stream.
     * Passing <code>null</code> causes an empty sequence to be written to the stream.
     **/
    public void writeIntSeq(int[] v)
    {
        if(v == null)
        {
            writeSize(0);
        }
        else
        {
            writeSize(v.length);
            expand(v.length * 4);
            java.nio.IntBuffer intBuf = _buf.b.asIntBuffer();
            intBuf.put(v);
            _buf.b.position(_buf.b.position() + v.length * 4);
        }
    }

    /**
     * Writes an optional int sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional int sequence to write to the stream.
     **/
    public void writeIntSeq(int tag, Optional<int[]> v)
    {
        if(v != null && v.isSet())
        {
            writeIntSeq(tag, v.get());
        }
    }

    /**
     * Writes an optional int sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The int sequence to write to the stream.
     **/
    public void writeIntSeq(int tag, int[] v)
    {
        if(writeOptional(tag, OptionalFormat.VSize))
        {
            writeSize(v == null || v.length == 0 ? 1 : v.length * 4 + (v.length > 254 ? 5 : 1));
            writeIntSeq(v);
        }
    }

    /**
     * Writes the remaining contents of the int buffer as an int sequence to the stream.
     *
     * @param v The int buffer to write to the stream.
     **/
    public void writeIntBuffer(java.nio.IntBuffer v)
    {
        if(v == null || v.remaining() == 0)
        {
            writeSize(0);
        }
        else
        {
            int sz = v.remaining();
            writeSize(sz);
            expand(sz * 4);

            java.nio.IntBuffer intBuf = _buf.b.asIntBuffer();
            intBuf.put(v);
            _buf.b.position(_buf.b.position() + sz * 4);
        }
    }

    /**
     * Writes a long to the stream.
     *
     * @param v The long to write to the stream.
     **/
    public void writeLong(long v)
    {
        expand(8);
        _buf.b.putLong(v);
    }

    /**
     * Writes an optional long to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional long to write to the stream.
     **/
    public void writeLong(int tag, LongOptional v)
    {
        if(v != null && v.isSet())
        {
            writeLong(tag, v.get());
        }
    }

    /**
     * Writes an optional long to the stream.
     *
     * @param tag The optional tag.
     * @param v The long to write to the stream.
     **/
    public void writeLong(int tag, long v)
    {
        if(writeOptional(tag, OptionalFormat.F8))
        {
            writeLong(v);
        }
    }

    /**
     * Writes a long sequence to the stream.
     *
     * @param v The long sequence to write to the stream.
     * Passing <code>null</code> causes an empty sequence to be written to the stream.
     **/
    public void writeLongSeq(long[] v)
    {
        if(v == null)
        {
            writeSize(0);
        }
        else
        {
            writeSize(v.length);
            expand(v.length * 8);
            java.nio.LongBuffer longBuf = _buf.b.asLongBuffer();
            longBuf.put(v);
            _buf.b.position(_buf.b.position() + v.length * 8);
        }
    }

    /**
     * Writes an optional long sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional long sequence to write to the stream.
     **/
    public void writeLongSeq(int tag, Optional<long[]> v)
    {
        if(v != null && v.isSet())
        {
            writeLongSeq(tag, v.get());
        }
    }

    /**
     * Writes an optional long sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The long sequence to write to the stream.
     **/
    public void writeLongSeq(int tag, long[] v)
    {
        if(writeOptional(tag, OptionalFormat.VSize))
        {
            writeSize(v == null || v.length == 0 ? 1 : v.length * 8 + (v.length > 254 ? 5 : 1));
            writeLongSeq(v);
        }
    }

    /**
     * Writes the remaining contents of the long buffer as a long sequence to the stream.
     *
     * @param v The long buffer to write to the stream.
     **/
    public void writeLongBuffer(java.nio.LongBuffer v)
    {
        if(v == null || v.remaining() == 0)
        {
            writeSize(0);
        }
        else
        {
            int sz = v.remaining();
            writeSize(sz);
            expand(sz * 8);

            java.nio.LongBuffer longBuf = _buf.b.asLongBuffer();
            longBuf.put(v);
            _buf.b.position(_buf.b.position() + sz * 8);
        }
    }

    /**
     * Writes a float to the stream.
     *
     * @param v The float to write to the stream.
     **/
    public void writeFloat(float v)
    {
        expand(4);
        _buf.b.putFloat(v);
    }

    /**
     * Writes an optional float to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional float to write to the stream.
     **/
    public void writeFloat(int tag, FloatOptional v)
    {
        if(v != null && v.isSet())
        {
            writeFloat(tag, v.get());
        }
    }

    /**
     * Writes an optional float to the stream.
     *
     * @param tag The optional tag.
     * @param v The float to write to the stream.
     **/
    public void writeFloat(int tag, float v)
    {
        if(writeOptional(tag, OptionalFormat.F4))
        {
            writeFloat(v);
        }
    }

    /**
     * Writes a float sequence to the stream.
     *
     * @param v The float sequence to write to the stream.
     * Passing <code>null</code> causes an empty sequence to be written to the stream.
     **/
    public void writeFloatSeq(float[] v)
    {
        if(v == null)
        {
            writeSize(0);
        }
        else
        {
            writeSize(v.length);
            expand(v.length * 4);
            java.nio.FloatBuffer floatBuf = _buf.b.asFloatBuffer();
            floatBuf.put(v);
            _buf.b.position(_buf.b.position() + v.length * 4);
        }
    }

    /**
     * Writes an optional float sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional float sequence to write to the stream.
     **/
    public void writeFloatSeq(int tag, Optional<float[]> v)
    {
        if(v != null && v.isSet())
        {
            writeFloatSeq(tag, v.get());
        }
    }

    /**
     * Writes an optional float sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The float sequence to write to the stream.
     **/
    public void writeFloatSeq(int tag, float[] v)
    {
        if(writeOptional(tag, OptionalFormat.VSize))
        {
            writeSize(v == null || v.length == 0 ? 1 : v.length * 4 + (v.length > 254 ? 5 : 1));
            writeFloatSeq(v);
        }
    }

    /**
     * Writes the remaining contents of the float buffer as a float sequence to the stream.
     *
     * @param v The float buffer to write to the stream.
     **/
    public void writeFloatBuffer(java.nio.FloatBuffer v)
    {
        if(v == null || v.remaining() == 0)
        {
            writeSize(0);
        }
        else
        {
            int sz = v.remaining();
            writeSize(sz);
            expand(sz * 4);

            java.nio.FloatBuffer floatBuf = _buf.b.asFloatBuffer();
            floatBuf.put(v);
            _buf.b.position(_buf.b.position() + sz * 4);
        }
    }

    /**
     * Writes a double to the stream.
     *
     * @param v The double to write to the stream.
     **/
    public void writeDouble(double v)
    {
        expand(8);
        _buf.b.putDouble(v);
    }

    /**
     * Writes an optional double to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional double to write to the stream.
     **/
    public void writeDouble(int tag, DoubleOptional v)
    {
        if(v != null && v.isSet())
        {
            writeDouble(tag, v.get());
        }
    }

    /**
     * Writes an optional double to the stream.
     *
     * @param tag The optional tag.
     * @param v The double to write to the stream.
     **/
    public void writeDouble(int tag, double v)
    {
        if(writeOptional(tag, OptionalFormat.F8))
        {
            writeDouble(v);
        }
    }

    /**
     * Writes a double sequence to the stream.
     *
     * @param v The double sequence to write to the stream.
     * Passing <code>null</code> causes an empty sequence to be written to the stream.
     **/
    public void writeDoubleSeq(double[] v)
    {
        if(v == null)
        {
            writeSize(0);
        }
        else
        {
            writeSize(v.length);
            expand(v.length * 8);
            java.nio.DoubleBuffer doubleBuf = _buf.b.asDoubleBuffer();
            doubleBuf.put(v);
            _buf.b.position(_buf.b.position() + v.length * 8);
        }
    }

    /**
     * Writes an optional double sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional double sequence to write to the stream.
     **/
    public void writeDoubleSeq(int tag, Optional<double[]> v)
    {
        if(v != null && v.isSet())
        {
            writeDoubleSeq(tag, v.get());
        }
    }

    /**
     * Writes an optional double sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The double sequence to write to the stream.
     **/
    public void writeDoubleSeq(int tag, double[] v)
    {
        if(writeOptional(tag, OptionalFormat.VSize))
        {
            writeSize(v == null || v.length == 0 ? 1 : v.length * 8 + (v.length > 254 ? 5 : 1));
            writeDoubleSeq(v);
        }
    }

    /**
     * Writes the remaining contents of the double buffer as a double sequence to the stream.
     *
     * @param v The double buffer to write to the stream.
     **/
    public void writeDoubleBuffer(java.nio.DoubleBuffer v)
    {
        if(v == null || v.remaining() == 0)
        {
            writeSize(0);
        }
        else
        {
            int sz = v.remaining();
            writeSize(sz);
            expand(sz * 8);

            java.nio.DoubleBuffer doubleBuf = _buf.b.asDoubleBuffer();
            doubleBuf.put(v);
            _buf.b.position(_buf.b.position() + sz * 8);
        }
    }

    final static java.nio.charset.Charset _utf8 = java.nio.charset.Charset.forName("UTF8");
    private java.nio.charset.CharsetEncoder _charEncoder = null;

    /**
     * Writes a string to the stream.
     *
     * @param v The string to write to the stream. Passing <code>null</code> causes
     * an empty string to be written to the stream.
     **/
    public void writeString(String v)
    {
        if(v == null)
        {
            writeSize(0);
        }
        else
        {
            final int len = v.length();
            if(len > 0)
            {
                if(_stringBytes == null || len > _stringBytes.length)
                {
                    _stringBytes = new byte[len];
                }
                if(_stringChars == null || len > _stringChars.length)
                {
                    _stringChars = new char[len];
                }
                //
                // If the string contains only 7-bit characters, it's more efficient
                // to perform the conversion to UTF-8 manually.
                //
                v.getChars(0, len, _stringChars, 0);
                for(int i = 0; i < len; ++i)
                {
                    if(_stringChars[i] > (char)127)
                    {
                        //
                        // Found a multibyte character.
                        //
                        if(_charEncoder == null)
                        {
                            _charEncoder = _utf8.newEncoder();
                        }
                        java.nio.ByteBuffer b = null;
                        try
                        {
                            b = _charEncoder.encode(java.nio.CharBuffer.wrap(_stringChars, 0, len));
                        }
                        catch(java.nio.charset.CharacterCodingException ex)
                        {
                            throw new MarshalException(ex);
                        }
                        writeSize(b.limit());
                        expand(b.limit());
                        _buf.b.put(b);
                        return;
                    }
                    _stringBytes[i] = (byte)_stringChars[i];
                }
                writeSize(len);
                expand(len);
                _buf.b.put(_stringBytes, 0, len);
            }
            else
            {
                writeSize(0);
            }
        }
    }

    /**
     * Writes an optional string to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional string to write to the stream.
     **/
    public void writeString(int tag, Optional<String> v)
    {
        if(v != null && v.isSet())
        {
            writeString(tag, v.get());
        }
    }

    /**
     * Writes an optional string to the stream.
     *
     * @param tag The optional tag.
     * @param v The string to write to the stream.
     **/
    public void writeString(int tag, String v)
    {
        if(writeOptional(tag, OptionalFormat.VSize))
        {
            writeString(v);
        }
    }

    /**
     * Writes a string sequence to the stream.
     *
     * @param v The string sequence to write to the stream.
     * Passing <code>null</code> causes an empty sequence to be written to the stream.
     **/
    public void writeStringSeq(String[] v)
    {
        if(v == null)
        {
            writeSize(0);
        }
        else
        {
            writeSize(v.length);
            for(String e : v)
            {
                writeString(e);
            }
        }
    }

    /**
     * Writes an optional string sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional string sequence to write to the stream.
     **/
    public void writeStringSeq(int tag, Optional<String[]> v)
    {
        if(v != null && v.isSet())
        {
            writeStringSeq(tag, v.get());
        }
    }

    /**
     * Writes an optional string sequence to the stream.
     *
     * @param tag The optional tag.
     * @param v The string sequence to write to the stream.
     **/
    public void writeStringSeq(int tag, String[] v)
    {
        if(writeOptional(tag, OptionalFormat.FSize))
        {
            int pos = startSize();
            writeStringSeq(v);
            endSize(pos);
        }
    }

    /**
     * Writes a proxy to the stream.
     *
     * @param v The proxy to write.
     **/
    public void writeProxy(ObjectPrx v)
    {
        if(v != null)
        {
            v.__write(this);
        }
        else
        {
            Identity ident = new Identity();
            ident.__write(this);
        }
    }

    /**
     * Writes an optional proxy to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional proxy to write to the stream.
     **/
    public void writeProxy(int tag, Optional<ObjectPrx> v)
    {
        if(v != null && v.isSet())
        {
            writeProxy(tag, v.get());
        }
    }

    /**
     * Writes an optional proxy to the stream.
     *
     * @param tag The optional tag.
     * @param v The proxy to write to the stream.
     **/
    public void writeProxy(int tag, ObjectPrx v)
    {
        if(writeOptional(tag, OptionalFormat.FSize))
        {
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
     **/
    public void writeEnum(int v, int maxValue)
    {
        if(isEncoding_1_0())
        {
            if(maxValue < 127)
            {
                writeByte((byte)v);
            }
            else if(maxValue < 32767)
            {
                writeShort((short)v);
            }
            else
            {
                writeInt(v);
            }
        }
        else
        {
            writeSize(v);
        }
    }

    /**
     * Writes a Slice value to the stream.
     *
     * @param v The value to write. This method writes the index of an instance; the state of the value is
     * written once {@link #writePendingValues} is called.
     **/
    public void writeValue(Ice.Object v)
    {
        initEncaps();
        _encapsStack.encoder.writeValue(v);
    }

    /**
     * Writes an optional value to the stream.
     *
     * @param tag The optional tag.
     * @param v The optional value to write to the stream.
     * @param <T> The type of the optional value.
     **/
    public <T extends Ice.Object> void writeValue(int tag, Optional<T> v)
    {
        if(v != null && v.isSet())
        {
            writeValue(tag, v.get());
        }
    }

    /**
     * Writes an optional value to the stream.
     *
     * @param tag The optional tag.
     * @param v The value to write to the stream.
     **/
    public void writeValue(int tag, Ice.Object v)
    {
        if(writeOptional(tag, OptionalFormat.Class))
        {
            writeValue(v);
        }
    }

    /**
     * Writes a user exception to the stream.
     *
     * @param v The user exception to write.
     **/
    public void writeException(UserException v)
    {
        initEncaps();
        _encapsStack.encoder.writeException(v);
    }

    private boolean writeOptionalImpl(int tag, OptionalFormat format)
    {
        if(isEncoding_1_0())
        {
            return false; // Optional members aren't supported with the 1.0 encoding.
        }

        int v = format.value();
        if(tag < 30)
        {
            v |= tag << 3;
            writeByte((byte)v);
        }
        else
        {
            v |= 0x0F0; // tag = 30
            writeByte((byte)v);
            writeSize(tag);
        }
        return true;
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
     * @return True if no data has been written yet, false otherwise.
     **/
    public boolean isEmpty()
    {
        return _buf.empty();
    }

    /**
     * Expand the stream to accept more data.
     *
     * @param n The number of bytes to accommodate in the stream.
     **/
    public void expand(int n)
    {
        _buf.expand(n);
    }

    private IceInternal.Instance _instance;
    private IceInternal.Buffer _buf;
    private Object _closure;
    private FormatType _format;
    private byte[] _stringBytes; // Reusable array for string operations.
    private char[] _stringChars; // Reusable array for string operations.

    private enum SliceType { NoSlice, ValueSlice, ExceptionSlice }

    abstract private static class EncapsEncoder
    {
        protected EncapsEncoder(OutputStream stream, Encaps encaps)
        {
            _stream = stream;
            _encaps = encaps;
            _typeIdIndex = 0;
            _marshaledMap = new java.util.IdentityHashMap<Ice.Object, Integer>();
        }

        abstract void writeValue(Ice.Object v);
        abstract void writeException(UserException v);

        abstract void startInstance(SliceType type, SlicedData data);
        abstract void endInstance();
        abstract void startSlice(String typeId, int compactId, boolean last);
        abstract void endSlice();

        boolean writeOptional(int tag, OptionalFormat format)
        {
            return false;
        }

        void writePendingValues()
        {
        }

        protected int registerTypeId(String typeId)
        {
            if(_typeIdMap == null) // Lazy initialization
            {
                _typeIdMap = new java.util.TreeMap<String, Integer>();
            }

            Integer p = _typeIdMap.get(typeId);
            if(p != null)
            {
                return p;
            }
            else
            {
                _typeIdMap.put(typeId, ++_typeIdIndex);
                return -1;
            }
        }

        final protected OutputStream _stream;
        final protected Encaps _encaps;

        // Encapsulation attributes for instance marshaling.
        final protected java.util.IdentityHashMap<Ice.Object, Integer> _marshaledMap;
        private java.util.TreeMap<String, Integer> _typeIdMap;
        private int _typeIdIndex;
    }

    private static final class EncapsEncoder10 extends EncapsEncoder
    {
        EncapsEncoder10(OutputStream stream, Encaps encaps)
        {
            super(stream, encaps);
            _sliceType = SliceType.NoSlice;
            _valueIdIndex = 0;
            _toBeMarshaledMap = new java.util.IdentityHashMap<Ice.Object, Integer>();
        }

        @Override
        void writeValue(Ice.Object v)
        {
            //
            // Object references are encoded as a negative integer in 1.0.
            //
            if(v != null)
            {
                _stream.writeInt(-registerValue(v));
            }
            else
            {
                _stream.writeInt(0);
            }
        }

        @Override
        void writeException(UserException v)
        {
            //
            // User exception with the 1.0 encoding start with a boolean
            // flag that indicates whether or not the exception uses
            // classes.
            //
            // This allows reading the pending instances even if some part of
            // the exception was sliced.
            //
            boolean usesClasses = v.__usesClasses();
            _stream.writeBool(usesClasses);
            v.__write(_stream);
            if(usesClasses)
            {
                writePendingValues();
            }
        }

        @Override
        void startInstance(SliceType sliceType, SlicedData sliceData)
        {
            _sliceType = sliceType;
        }

        @Override
        void endInstance()
        {
            if(_sliceType == SliceType.ValueSlice)
            {
                //
                // Write the Object slice.
                //
                startSlice(ObjectImpl.ice_staticId(), -1, true);
                _stream.writeSize(0); // For compatibility with the old AFM.
                endSlice();
            }
            _sliceType = SliceType.NoSlice;
        }

        @Override
        void startSlice(String typeId, int compactId, boolean last)
        {
            //
            // For instance slices, encode a boolean to indicate how the type ID
            // is encoded and the type ID either as a string or index. For
            // exception slices, always encode the type ID as a string.
            //
            if(_sliceType == SliceType.ValueSlice)
            {
                int index = registerTypeId(typeId);
                if(index < 0)
                {
                    _stream.writeBool(false);
                    _stream.writeString(typeId);
                }
                else
                {
                    _stream.writeBool(true);
                    _stream.writeSize(index);
                }
            }
            else
            {
                _stream.writeString(typeId);
            }

            _stream.writeInt(0); // Placeholder for the slice length.

            _writeSlice = _stream.pos();
        }

        @Override
        void endSlice()
        {
            //
            // Write the slice length.
            //
            final int sz = _stream.pos() - _writeSlice + 4;
            _stream.rewriteInt(sz, _writeSlice - 4);
        }

        @Override
        void writePendingValues()
        {
            while(_toBeMarshaledMap.size() > 0)
            {
                //
                // Consider the to be marshalled instances as marshaled now,
                // this is necessary to avoid adding again the "to be
                // marshaled instances" into _toBeMarshaledMap while writing
                // instances.
                //
                _marshaledMap.putAll(_toBeMarshaledMap);

                java.util.IdentityHashMap<Ice.Object, Integer> savedMap = _toBeMarshaledMap;
                _toBeMarshaledMap = new java.util.IdentityHashMap<Ice.Object, Integer>();
                _stream.writeSize(savedMap.size());
                for(java.util.Map.Entry<Ice.Object, Integer> p : savedMap.entrySet())
                {
                    //
                    // Ask the instance to marshal itself. Any new class
                    // instances that are triggered by the classes marshaled
                    // are added to toBeMarshaledMap.
                    //
                    _stream.writeInt(p.getValue().intValue());

                    try
                    {
                        p.getKey().ice_preMarshal();
                    }
                    catch(java.lang.Exception ex)
                    {
                        String s = "exception raised by ice_preMarshal:\n" + IceInternal.Ex.toString(ex);
                        _stream.instance().initializationData().logger.warning(s);
                    }

                    p.getKey().__write(_stream);
                }
            }
            _stream.writeSize(0); // Zero marker indicates end of sequence of sequences of instances.
        }

        private int registerValue(Ice.Object v)
        {
            assert(v != null);

            //
            // Look for this instance in the to-be-marshaled map.
            //
            Integer p = _toBeMarshaledMap.get(v);
            if(p != null)
            {
                return p.intValue();
            }

            //
            // Didn't find it, try the marshaled map next.
            //
            p = _marshaledMap.get(v);
            if(p != null)
            {
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
        private int _writeSlice;        // Position of the slice data members

        // Encapsulation attributes for instance marshaling.
        private int _valueIdIndex;
        private java.util.IdentityHashMap<Ice.Object, Integer> _toBeMarshaledMap;
    }

    private static final class EncapsEncoder11 extends EncapsEncoder
    {
        EncapsEncoder11(OutputStream stream, Encaps encaps)
        {
            super(stream, encaps);
            _current = null;
            _valueIdIndex = 1;
        }

        @Override
        void writeValue(Ice.Object v)
        {
            if(v == null)
            {
                _stream.writeSize(0);
            }
            else if(_current != null && _encaps.format == FormatType.SlicedFormat)
            {
                if(_current.indirectionTable == null) // Lazy initialization
                {
                    _current.indirectionTable = new java.util.ArrayList<Ice.Object>();
                    _current.indirectionMap = new java.util.IdentityHashMap<Ice.Object, Integer>();
                }

                //
                // If writing an instance within a slice and using the sliced
                // format, write an index from the instance indirection
                // table. The indirect instance table is encoded at the end of
                // each slice and is always read (even if the Slice is
                // unknown).
                //
                Integer index = _current.indirectionMap.get(v);
                if(index == null)
                {
                    _current.indirectionTable.add(v);
                    final int idx = _current.indirectionTable.size(); // Position + 1 (0 is reserved for nil)
                    _current.indirectionMap.put(v, idx);
                    _stream.writeSize(idx);
                }
                else
                {
                    _stream.writeSize(index.intValue());
                }
            }
            else
            {
                writeInstance(v); // Write the instance or a reference if already marshaled.
            }
        }

        @Override
        void writeException(UserException v)
        {
            v.__write(_stream);
        }

        @Override
        void startInstance(SliceType sliceType, SlicedData data)
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
            _current.firstSlice = true;

            if(data != null)
            {
                writeSlicedData(data);
            }
        }

        @Override
        void endInstance()
        {
            _current = _current.previous;
        }

        @Override
        void startSlice(String typeId, int compactId, boolean last)
        {
            assert((_current.indirectionTable == null || _current.indirectionTable.isEmpty()) &&
                   (_current.indirectionMap == null || _current.indirectionMap.isEmpty()));

            _current.sliceFlagsPos = _stream.pos();

            _current.sliceFlags = (byte)0;
            if(_encaps.format == FormatType.SlicedFormat)
            {
                // Encode the slice size if using the sliced format.
                _current.sliceFlags |= IceInternal.Protocol.FLAG_HAS_SLICE_SIZE;
            }
            if(last)
            {
                _current.sliceFlags |= IceInternal.Protocol.FLAG_IS_LAST_SLICE; // This is the last slice.
            }

            _stream.writeByte((byte)0); // Placeholder for the slice flags

            //
            // For instance slices, encode the flag and the type ID either as a
            // string or index. For exception slices, always encode the type
            // ID a string.
            //
            if(_current.sliceType == SliceType.ValueSlice)
            {
                //
                // Encode the type ID (only in the first slice for the compact
                // encoding).
                //
                if(_encaps.format == FormatType.SlicedFormat || _current.firstSlice)
                {
                    if(compactId >= 0)
                    {
                        _current.sliceFlags |= IceInternal.Protocol.FLAG_HAS_TYPE_ID_COMPACT;
                        _stream.writeSize(compactId);
                    }
                    else
                    {
                        int index = registerTypeId(typeId);
                        if(index < 0)
                        {
                            _current.sliceFlags |= IceInternal.Protocol.FLAG_HAS_TYPE_ID_STRING;
                            _stream.writeString(typeId);
                        }
                        else
                        {
                            _current.sliceFlags |= IceInternal.Protocol.FLAG_HAS_TYPE_ID_INDEX;
                            _stream.writeSize(index);
                        }
                    }
                }
            }
            else
            {
                _stream.writeString(typeId);
            }

            if((_current.sliceFlags & IceInternal.Protocol.FLAG_HAS_SLICE_SIZE) != 0)
            {
                _stream.writeInt(0); // Placeholder for the slice length.
            }

            _current.writeSlice = _stream.pos();
            _current.firstSlice = false;
        }

        @Override
        void endSlice()
        {
            //
            // Write the optional member end marker if some optional members
            // were encoded. Note that the optional members are encoded before
            // the indirection table and are included in the slice size.
            //
            if((_current.sliceFlags & IceInternal.Protocol.FLAG_HAS_OPTIONAL_MEMBERS) != 0)
            {
                _stream.writeByte((byte)IceInternal.Protocol.OPTIONAL_END_MARKER);
            }

            //
            // Write the slice length if necessary.
            //
            if((_current.sliceFlags & IceInternal.Protocol.FLAG_HAS_SLICE_SIZE) != 0)
            {
                final int sz = _stream.pos() - _current.writeSlice + 4;
                _stream.rewriteInt(sz, _current.writeSlice - 4);
            }

            //
            // Only write the indirection table if it contains entries.
            //
            if(_current.indirectionTable != null && !_current.indirectionTable.isEmpty())
            {
                assert(_encaps.format == FormatType.SlicedFormat);
                _current.sliceFlags |= IceInternal.Protocol.FLAG_HAS_INDIRECTION_TABLE;

                //
                // Write the indirection instance table.
                //
                _stream.writeSize(_current.indirectionTable.size());
                for(Ice.Object v : _current.indirectionTable)
                {
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
        boolean writeOptional(int tag, OptionalFormat format)
        {
            if(_current == null)
            {
                return _stream.writeOptionalImpl(tag, format);
            }
            else
            {
                if(_stream.writeOptionalImpl(tag, format))
                {
                    _current.sliceFlags |= IceInternal.Protocol.FLAG_HAS_OPTIONAL_MEMBERS;
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }

        private void writeSlicedData(SlicedData slicedData)
        {
            assert(slicedData != null);

            //
            // We only remarshal preserved slices if we are using the sliced
            // format. Otherwise, we ignore the preserved slices, which
            // essentially "slices" the instance into the most-derived type
            // known by the sender.
            //
            if(_encaps.format != FormatType.SlicedFormat)
            {
                return;
            }

            for(SliceInfo info : slicedData.slices)
            {
                startSlice(info.typeId, info.compactId, info.isLastSlice);

                //
                // Write the bytes associated with this slice.
                //
                _stream.writeBlob(info.bytes);

                if(info.hasOptionalMembers)
                {
                    _current.sliceFlags |= IceInternal.Protocol.FLAG_HAS_OPTIONAL_MEMBERS;
                }

                //
                // Make sure to also re-write the instance indirection table.
                //
                if(info.instances != null && info.instances.length > 0)
                {
                    if(_current.indirectionTable == null) // Lazy initialization
                    {
                        _current.indirectionTable = new java.util.ArrayList<Ice.Object>();
                        _current.indirectionMap = new java.util.IdentityHashMap<Ice.Object, Integer>();
                    }
                    for(Ice.Object o : info.instances)
                    {
                        _current.indirectionTable.add(o);
                    }
                }

                endSlice();
            }
        }

        private void writeInstance(Ice.Object v)
        {
            assert(v != null);

            //
            // If the instance was already marshaled, just write it's ID.
            //
            Integer p = _marshaledMap.get(v);
            if(p != null)
            {
                _stream.writeSize(p);
                return;
            }

            //
            // We haven't seen this instance previously, create a new ID,
            // insert it into the marshaled map, and write the instance.
            //
            _marshaledMap.put(v, ++_valueIdIndex);

            try
            {
                v.ice_preMarshal();
            }
            catch(java.lang.Exception ex)
            {
                String s = "exception raised by ice_preMarshal:\n" + IceInternal.Ex.toString(ex);
                _stream.instance().initializationData().logger.warning(s);
            }

            _stream.writeSize(1); // Class instance marker.
            v.__write(_stream);
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
            boolean firstSlice;

            // Slice attributes
            byte sliceFlags;
            int writeSlice;    // Position of the slice data members
            int sliceFlagsPos; // Position of the slice flags
            java.util.List<Ice.Object> indirectionTable;
            java.util.IdentityHashMap<Ice.Object, Integer> indirectionMap;

            final InstanceData previous;
            InstanceData next;
        }

        private InstanceData _current;

        private int _valueIdIndex; // The ID of the next instance to marhsal
    }

    private static final class Encaps
    {
        void reset()
        {
            encoder = null;
        }

        void setEncoding(EncodingVersion encoding)
        {
            this.encoding = encoding;
            encoding_1_0 = encoding.equals(Util.Encoding_1_0);
        }

        int start;
        FormatType format = FormatType.DefaultFormat;
        EncodingVersion encoding;
        boolean encoding_1_0;

        EncapsEncoder encoder;

        Encaps next;
    }

    //
    // The encoding version to use when there's no encapsulation to
    // read from or write to. This is for example used to read message
    // headers or when the user is using the streaming API with no
    // encapsulation.
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
        }

        if(_encapsStack.format == FormatType.DefaultFormat)
        {
            _encapsStack.format = _format;
        }

        if(_encapsStack.encoder == null) // Lazy initialization.
        {
            if(_encapsStack.encoding_1_0)
            {
                _encapsStack.encoder = new EncapsEncoder10(this, _encapsStack);
            }
            else
            {
                _encapsStack.encoder = new EncapsEncoder11(this, _encapsStack);
            }
        }
    }
}
