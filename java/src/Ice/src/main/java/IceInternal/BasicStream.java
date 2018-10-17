// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

import java.io.IOException;

public class BasicStream
{
    public
    BasicStream(Instance instance, Ice.EncodingVersion encoding)
    {
        this(instance, encoding, instance.cacheMessageBuffers() > 1);
    }

    public
    BasicStream(Instance instance, Ice.EncodingVersion encoding, boolean direct)
    {
        initialize(instance, encoding);
        _buf = new Buffer(direct);
    }

    public
    BasicStream(Instance instance, Ice.EncodingVersion encoding, byte[] data)
    {
        initialize(instance, encoding);
        _buf = new Buffer(data);
    }

    public
    BasicStream(Instance instance, Ice.EncodingVersion encoding, java.nio.ByteBuffer data)
    {
        initialize(instance, encoding);
        _buf = new Buffer(data);
    }

    private void
    initialize(Instance instance, Ice.EncodingVersion encoding)
    {
        _instance = instance;
        _closure = null;
        _encoding = encoding;

        _readEncapsStack = null;
        _writeEncapsStack = null;
        _readEncapsCache = null;
        _writeEncapsCache = null;

        _sliceObjects = true;

        _startSeq = -1;
    }

    //
    // This function allows this object to be reused, rather than
    // reallocated.
    //
    public void
    reset()
    {
        _buf.reset();
        clear();
    }

    public void
    clear()
    {
        if(_readEncapsStack != null)
        {
            assert(_readEncapsStack.next == null);
            _readEncapsStack.next = _readEncapsCache;
            _readEncapsCache = _readEncapsStack;
            _readEncapsCache.reset();
            _readEncapsStack = null;
        }

        if(_writeEncapsStack != null)
        {
            assert(_writeEncapsStack.next == null);
            _writeEncapsStack.next = _writeEncapsCache;
            _writeEncapsCache = _writeEncapsStack;
            _writeEncapsCache.reset();
            _writeEncapsStack = null;
        }

        _startSeq = -1;

        _sliceObjects = true;
    }

    public Instance
    instance()
    {
        return _instance;
    }

    public Object
    closure()
    {
        return _closure;
    }

    public Object
    closure(Object p)
    {
        Object prev = _closure;
        _closure = p;
        return prev;
    }

    public void
    swap(BasicStream other)
    {
        assert(_instance == other._instance);

        Buffer tmpBuf = other._buf;
        other._buf = _buf;
        _buf = tmpBuf;

        Object tmpClosure = other._closure;
        other._closure = _closure;
        _closure = tmpClosure;

        //
        // Swap is never called for BasicStreams that have encapsulations being read/write. However,
        // encapsulations might still be set in case marshalling or un-marshalling failed. We just
        // reset the encapsulations if there are still some set.
        //
        resetEncaps();
        other.resetEncaps();

        int tmpStartSeq = other._startSeq;
        other._startSeq = _startSeq;
        _startSeq = tmpStartSeq;

        int tmpMinSeqSize = other._minSeqSize;
        other._minSeqSize = _minSeqSize;
        _minSeqSize = tmpMinSeqSize;
    }

    public void
    resetEncaps()
    {
        _readEncapsStack = null;
        _writeEncapsStack = null;
    }

    public void
    resize(int sz, boolean reading)
    {
        _buf.resize(sz, reading);
        _buf.b.position(sz);
    }

    public Buffer
    prepareWrite()
    {
        _buf.b.limit(_buf.size());
        _buf.b.position(0);
        return _buf;
    }

    public Buffer
    getBuffer()
    {
        return _buf;
    }

    public void
    startWriteObject(Ice.SlicedData data)
    {
        assert(_writeEncapsStack != null && _writeEncapsStack.encoder != null);
        _writeEncapsStack.encoder.startInstance(SliceType.ObjectSlice, data);
    }

    public void
    endWriteObject()
    {
        assert(_writeEncapsStack != null && _writeEncapsStack.encoder != null);
        _writeEncapsStack.encoder.endInstance();
    }

    public void
    startReadObject()
    {
        assert(_readEncapsStack != null && _readEncapsStack.decoder != null);
        _readEncapsStack.decoder.startInstance(SliceType.ObjectSlice);
    }

    public Ice.SlicedData
    endReadObject(boolean preserve)
    {
        assert(_readEncapsStack != null && _readEncapsStack.decoder != null);
        return _readEncapsStack.decoder.endInstance(preserve);
    }

    public void
    startWriteException(Ice.SlicedData data)
    {
        assert(_writeEncapsStack != null && _writeEncapsStack.encoder != null);
        _writeEncapsStack.encoder.startInstance(SliceType.ExceptionSlice, data);
    }

    public void
    endWriteException()
    {
        assert(_writeEncapsStack != null && _writeEncapsStack.encoder != null);
        _writeEncapsStack.encoder.endInstance();
    }

    public void
    startReadException()
    {
        assert(_readEncapsStack != null && _readEncapsStack.decoder != null);
        _readEncapsStack.decoder.startInstance(SliceType.ExceptionSlice);
    }

    public Ice.SlicedData
    endReadException(boolean preserve)
    {
        assert(_readEncapsStack != null && _readEncapsStack.decoder != null);
        return _readEncapsStack.decoder.endInstance(preserve);
    }

    public void
    startWriteEncaps()
    {
        //
        // If no encoding version is specified, use the current write
        // encapsulation encoding version if there's a current write
        // encapsulation, otherwise, use the stream encoding version.
        //

        if(_writeEncapsStack != null)
        {
            startWriteEncaps(_writeEncapsStack.encoding, _writeEncapsStack.format);
        }
        else
        {
            startWriteEncaps(_encoding, Ice.FormatType.DefaultFormat);
        }
    }

    public void
    startWriteEncaps(Ice.EncodingVersion encoding, Ice.FormatType format)
    {
        Protocol.checkSupportedEncoding(encoding);

        WriteEncaps curr = _writeEncapsCache;
        if(curr != null)
        {
            curr.reset();
            _writeEncapsCache = _writeEncapsCache.next;
        }
        else
        {
            curr = new WriteEncaps();
        }
        curr.next = _writeEncapsStack;
        _writeEncapsStack = curr;

        _writeEncapsStack.format = format;
        _writeEncapsStack.setEncoding(encoding);
        _writeEncapsStack.start = _buf.size();

        writeInt(0); // Placeholder for the encapsulation length.
        _writeEncapsStack.encoding.__write(this);
    }

    public void
    endWriteEncaps()
    {
        assert(_writeEncapsStack != null);

        // Size includes size and version.
        int start = _writeEncapsStack.start;
        int sz = _buf.size() - start;
        _buf.b.putInt(start, sz);

        WriteEncaps curr = _writeEncapsStack;
        _writeEncapsStack = curr.next;
        curr.next = _writeEncapsCache;
        _writeEncapsCache = curr;
        _writeEncapsCache.reset();
    }

    public void
    endWriteEncapsChecked() // Used by public stream API.
    {
        if(_writeEncapsStack == null)
        {
            throw new Ice.EncapsulationException("not in an encapsulation");
        }
        endWriteEncaps();
    }

    public void
    writeEmptyEncaps(Ice.EncodingVersion encoding)
    {
        Protocol.checkSupportedEncoding(encoding);
        writeInt(6); // Size
        encoding.__write(this);
    }

    public void
    writeEncaps(byte[] v)
    {
        if(v.length < 6)
        {
            throw new Ice.EncapsulationException();
        }
        expand(v.length);
        _buf.b.put(v);
    }

    public Ice.EncodingVersion
    getWriteEncoding()
    {
        return _writeEncapsStack != null ? _writeEncapsStack.encoding : _encoding;
    }

    public Ice.EncodingVersion
    startReadEncaps()
    {
        ReadEncaps curr = _readEncapsCache;
        if(curr != null)
        {
            curr.reset();
            _readEncapsCache = _readEncapsCache.next;
        }
        else
        {
            curr = new ReadEncaps();
        }
        curr.next = _readEncapsStack;
        _readEncapsStack = curr;

        _readEncapsStack.start = _buf.b.position();

        //
        // I don't use readSize() and writeSize() for encapsulations,
        // because when creating an encapsulation, I must know in advance
        // how many bytes the size information will require in the data
        // stream. If I use an Int, it is always 4 bytes. For
        // readSize()/writeSize(), it could be 1 or 5 bytes.
        //
        int sz = readInt();
        if(sz < 6)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        if(sz - 4 > _buf.b.remaining())
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        _readEncapsStack.sz = sz;

        Ice.EncodingVersion encoding = new Ice.EncodingVersion();
        encoding.__read(this);
        Protocol.checkSupportedEncoding(encoding); // Make sure the encoding is supported.
        _readEncapsStack.setEncoding(encoding);

        return encoding;
    }

    public void
    endReadEncaps()
    {
        assert(_readEncapsStack != null);

        if(!_readEncapsStack.encoding_1_0)
        {
            skipOpts();
            if(_buf.b.position() != _readEncapsStack.start + _readEncapsStack.sz)
            {
                throw new Ice.EncapsulationException();
            }
        }
        else if(_buf.b.position() != _readEncapsStack.start + _readEncapsStack.sz)
        {
            if(_buf.b.position() + 1 != _readEncapsStack.start + _readEncapsStack.sz)
            {
                throw new Ice.EncapsulationException();
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
                throw new Ice.UnmarshalOutOfBoundsException();
            }
        }

        ReadEncaps curr = _readEncapsStack;
        _readEncapsStack = curr.next;
        curr.next = _readEncapsCache;
        _readEncapsCache = curr;
        _readEncapsCache.reset();
    }

    public Ice.EncodingVersion
    skipEmptyEncaps()
    {
        int sz = readInt();
        if(sz < 6)
        {
            throw new Ice.EncapsulationException();
        }
        if(sz - 4 > _buf.b.remaining())
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }

        Ice.EncodingVersion encoding = new Ice.EncodingVersion();
        encoding.__read(this);
        if(encoding.equals(Ice.Util.Encoding_1_0))
        {
            if(sz != 6)
            {
                throw new Ice.EncapsulationException();
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

    public void
    endReadEncapsChecked() // Used by public stream API.
    {
        if(_readEncapsStack == null)
        {
            throw new Ice.EncapsulationException("not in an encapsulation");
        }
        endReadEncaps();
    }

    public byte[]
    readEncaps(Ice.EncodingVersion encoding)
    {
        int sz = readInt();
        if(sz < 6)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }

        if(sz - 4 > _buf.b.remaining())
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }

        if(encoding != null)
        {
            encoding.__read(this);
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
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public Ice.EncodingVersion
    getReadEncoding()
    {
        return _readEncapsStack != null ? _readEncapsStack.encoding : _encoding;
    }

    public int
    getReadEncapsSize()
    {
        assert(_readEncapsStack != null);
        return _readEncapsStack.sz - 6;
    }

    public Ice.EncodingVersion
    skipEncaps()
    {
        int sz = readInt();
        if(sz < 6)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        Ice.EncodingVersion encoding = new Ice.EncodingVersion();
        encoding.__read(this);
        try
        {
            _buf.b.position(_buf.b.position() + sz - 6);
        }
        catch(IllegalArgumentException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        return encoding;
    }

    public void
    startWriteSlice(String typeId, int compactId, boolean last)
    {
        assert(_writeEncapsStack != null && _writeEncapsStack.encoder != null);
        _writeEncapsStack.encoder.startSlice(typeId, compactId, last);
    }

    public void
    endWriteSlice()
    {
        assert(_writeEncapsStack != null && _writeEncapsStack.encoder != null);
        _writeEncapsStack.encoder.endSlice();
    }

    public String
    startReadSlice() // Returns type ID of next slice
    {
        assert(_readEncapsStack != null && _readEncapsStack.decoder != null);
        return _readEncapsStack.decoder.startSlice();
    }

    public void
    endReadSlice()
    {
        assert(_readEncapsStack != null && _readEncapsStack.decoder != null);
        _readEncapsStack.decoder.endSlice();
    }

    public void
    skipSlice()
    {
        assert(_readEncapsStack != null && _readEncapsStack.decoder != null);
        _readEncapsStack.decoder.skipSlice();
    }

    public void
    readPendingObjects()
    {
        if(_readEncapsStack != null && _readEncapsStack.decoder != null)
        {
            _readEncapsStack.decoder.readPendingObjects();
        }
        else if(_readEncapsStack != null ? _readEncapsStack.encoding_1_0 : _encoding.equals(Ice.Util.Encoding_1_0))
        {
            //
            // If using the 1.0 encoding and no objects were read, we
            // still read an empty sequence of pending objects if
            // requested (i.e.: if this is called).
            //
            // This is required by the 1.0 encoding, even if no objects
            // are written we do marshal an empty sequence if marshaled
            // data types use classes.
            //
            skipSize();
        }
    }

    public void
    writePendingObjects()
    {
        if(_writeEncapsStack != null && _writeEncapsStack.encoder != null)
        {
            _writeEncapsStack.encoder.writePendingObjects();
        }
        else if(_writeEncapsStack != null ? _writeEncapsStack.encoding_1_0 : _encoding.equals(Ice.Util.Encoding_1_0))
        {
            //
            // If using the 1.0 encoding and no objects were written, we
            // still write an empty sequence for pending objects if
            // requested (i.e.: if this is called).
            //
            // This is required by the 1.0 encoding, even if no objects
            // are written we do marshal an empty sequence if marshaled
            // data types use classes.
            //
            writeSize(0);
        }
    }

    public void
    writeSize(int v)
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

    public int
    readSize()
    {
        try
        {
            byte b = _buf.b.get();
            if(b == -1)
            {
                int v = _buf.b.getInt();
                if(v < 0)
                {
                    throw new Ice.UnmarshalOutOfBoundsException();
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
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public int
    readAndCheckSeqSize(int minSize)
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
            throw new Ice.UnmarshalOutOfBoundsException();
        }

        return sz;
    }

    public int
    startSize()
    {
        int pos = _buf.b.position();
        writeInt(0); // Placeholder for 32-bit size
    return pos;
    }

    public void
    endSize(int pos)
    {
        assert(pos >= 0);
        rewriteInt(_buf.b.position() - pos - 4, pos);
    }

    public void
    writeBlob(byte[] v)
    {
        if(v == null)
        {
            return;
        }
        expand(v.length);
        _buf.b.put(v);
    }

    public void
    writeBlob(byte[] v, int off, int len)
    {
        if(v == null)
        {
            return;
        }
        expand(len);
        _buf.b.put(v, off, len);
    }

    public byte[]
    readBlob(int sz)
    {
        if(_buf.b.remaining() < sz)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        byte[] v = new byte[sz];
        try
        {
            _buf.b.get(v);
            return v;
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    // Read/write format and tag for optionals
    public boolean
    writeOpt(int tag, Ice.OptionalFormat format)
    {
        assert(_writeEncapsStack != null);
        if(_writeEncapsStack.encoder != null)
        {
            return _writeEncapsStack.encoder.writeOpt(tag, format);
        }
        else
        {
            return writeOptImpl(tag, format);
        }
    }

    public boolean
    readOpt(int tag, Ice.OptionalFormat expectedFormat)
    {
        assert(_readEncapsStack != null);
        if(_readEncapsStack.decoder != null)
        {
            return _readEncapsStack.decoder.readOpt(tag, expectedFormat);
        }
        else
        {
            return readOptImpl(tag, expectedFormat);
        }
    }

    public void
    writeByte(byte v)
    {
        expand(1);
        _buf.b.put(v);
    }

    public void
    writeByte(int tag, Ice.ByteOptional v)
    {
        if(v != null && v.isSet())
        {
            writeByte(tag, v.get());
        }
    }

    public void
    writeByte(int tag, byte v)
    {
        if(writeOpt(tag, Ice.OptionalFormat.F1))
        {
            writeByte(v);
        }
    }

    public void
    rewriteByte(byte v, int dest)
    {
        _buf.b.put(dest, v);
    }

    public void
    writeByteSeq(byte[] v)
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

    public void
    writeByteSeq(int tag, Ice.Optional<byte[]> v)
    {
        if(v != null && v.isSet())
        {
            writeByteSeq(tag, v.get());
        }
    }

    public void
    writeByteSeq(int tag, byte[] v)
    {
        if(writeOpt(tag, Ice.OptionalFormat.VSize))
        {
            writeByteSeq(v);
        }
    }

    public void
    writeByteBuffer(java.nio.ByteBuffer v)
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

    public void
    writeSerializable(java.io.Serializable o)
    {
        if(o == null)
        {
            writeSize(0);
            return;
        }
        try
        {
            OutputStreamWrapper w = new OutputStreamWrapper(this);
            java.io.ObjectOutputStream out = new java.io.ObjectOutputStream(w);
            out.writeObject(o);
            out.close();
            w.close();
        }
        catch(java.lang.Exception ex)
        {
            throw new Ice.MarshalException("cannot serialize object: " + ex);
        }
    }

    public byte
    readByte()
    {
        try
        {
            return _buf.b.get();
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public void
    readByte(int tag, Ice.ByteOptional v)
    {
        if(readOpt(tag, Ice.OptionalFormat.F1))
        {
            v.set(readByte());
        }
        else
        {
            v.clear();
        }
    }

    public byte[]
    readByteSeq()
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
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public void
    readByteSeq(int tag, Ice.Optional<byte[]> v)
    {
        if(readOpt(tag, Ice.OptionalFormat.VSize))
        {
            v.set(readByteSeq());
        }
        else
        {
            v.clear();
        }
    }

    public java.nio.ByteBuffer
    readByteBuffer()
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
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public java.io.Serializable
    readSerializable()
    {
        int sz = readAndCheckSeqSize(1);
        if (sz == 0)
        {
            return null;
        }
        ObjectInputStream in = null;
        try
        {
            InputStreamWrapper w = new InputStreamWrapper(sz, this);
            in = new ObjectInputStream(_instance, w);
            return (java.io.Serializable)in.readObject();
        }
        catch(java.lang.Exception ex)
        {
            throw new Ice.MarshalException("cannot deserialize object", ex);
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
                    throw new Ice.MarshalException("cannot deserialize object", ex);
                }
            }
        }
    }

    public void
    writeBool(boolean v)
    {
        expand(1);
        _buf.b.put(v ? (byte)1 : (byte)0);
    }

    public void
    writeBool(int tag, Ice.BooleanOptional v)
    {
        if(v != null && v.isSet())
        {
            writeBool(tag, v.get());
        }
    }

    public void
    writeBool(int tag, boolean v)
    {
        if(writeOpt(tag, Ice.OptionalFormat.F1))
        {
            writeBool(v);
        }
    }

    public void
    rewriteBool(boolean v, int dest)
    {
        _buf.b.put(dest, v ? (byte)1 : (byte)0);
    }

    public void
    writeBoolSeq(boolean[] v)
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

    public void
    writeBoolSeq(int tag, Ice.Optional<boolean[]> v)
    {
        if(v != null && v.isSet())
        {
            writeBoolSeq(tag, v.get());
        }
    }

    public void
    writeBoolSeq(int tag, boolean[] v)
    {
        if(writeOpt(tag, Ice.OptionalFormat.VSize))
        {
            writeBoolSeq(v);
        }
    }

    public boolean
    readBool()
    {
        try
        {
            return _buf.b.get() == 1;
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public void
    readBool(int tag, Ice.BooleanOptional v)
    {
        if(readOpt(tag, Ice.OptionalFormat.F1))
        {
            v.set(readBool());
        }
        else
        {
            v.clear();
        }
    }

    public boolean[]
    readBoolSeq()
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
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public void
    readBoolSeq(int tag, Ice.Optional<boolean[]> v)
    {
        if(readOpt(tag, Ice.OptionalFormat.VSize))
        {
            v.set(readBoolSeq());
        }
        else
        {
            v.clear();
        }
    }

    public void
    writeShort(short v)
    {
        expand(2);
        _buf.b.putShort(v);
    }

    public void
    writeShort(int tag, Ice.ShortOptional v)
    {
        if(v != null && v.isSet())
        {
            writeShort(tag, v.get());
        }
    }

    public void
    writeShort(int tag, short v)
    {
        if(writeOpt(tag, Ice.OptionalFormat.F2))
        {
            writeShort(v);
        }
    }

    public void
    writeShortSeq(short[] v)
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

    public void
    writeShortSeq(int tag, Ice.Optional<short[]> v)
    {
        if(v != null && v.isSet())
        {
            writeShortSeq(tag, v.get());
        }
    }

    public void
    writeShortSeq(int tag, short[] v)
    {
        if(writeOpt(tag, Ice.OptionalFormat.VSize))
        {
            writeSize(v == null || v.length == 0 ? 1 : v.length * 2 + (v.length > 254 ? 5 : 1));
            writeShortSeq(v);
        }
    }

    public void
    writeShortBuffer(java.nio.ShortBuffer v)
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

    public short
    readShort()
    {
        try
        {
            return _buf.b.getShort();
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public void
    readShort(int tag, Ice.ShortOptional v)
    {
        if(readOpt(tag, Ice.OptionalFormat.F2))
        {
            v.set(readShort());
        }
        else
        {
            v.clear();
        }
    }

    public short[]
    readShortSeq()
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
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public void
    readShortSeq(int tag, Ice.Optional<short[]> v)
    {
        if(readOpt(tag, Ice.OptionalFormat.VSize))
        {
            skipSize();
            v.set(readShortSeq());
        }
        else
        {
            v.clear();
        }
    }

    public java.nio.ShortBuffer
    readShortBuffer()
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
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public void
    writeInt(int v)
    {
        expand(4);
        _buf.b.putInt(v);
    }

    public void
    writeInt(int tag, Ice.IntOptional v)
    {
        if(v != null && v.isSet())
        {
            writeInt(tag, v.get());
        }
    }

    public void
    writeInt(int tag, int v)
    {
        if(writeOpt(tag, Ice.OptionalFormat.F4))
        {
            writeInt(v);
        }
    }

    public void
    rewriteInt(int v, int dest)
    {
        _buf.b.putInt(dest, v);
    }

    public void
    writeIntSeq(int[] v)
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

    public void
    writeIntSeq(int tag, Ice.Optional<int[]> v)
    {
        if(v != null && v.isSet())
        {
            writeIntSeq(tag, v.get());
        }
    }

    public void
    writeIntSeq(int tag, int[] v)
    {
        if(writeOpt(tag, Ice.OptionalFormat.VSize))
        {
            writeSize(v == null || v.length == 0 ? 1 : v.length * 4 + (v.length > 254 ? 5 : 1));
            writeIntSeq(v);
        }
    }

    public void
    writeIntBuffer(java.nio.IntBuffer v)
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

    public int
    readInt()
    {
        try
        {
            return _buf.b.getInt();
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public void
    readInt(int tag, Ice.IntOptional v)
    {
        if(readOpt(tag, Ice.OptionalFormat.F4))
        {
            v.set(readInt());
        }
        else
        {
            v.clear();
        }
    }

    public int[]
    readIntSeq()
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
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public void
    readIntSeq(int tag, Ice.Optional<int[]> v)
    {
        if(readOpt(tag, Ice.OptionalFormat.VSize))
        {
            skipSize();
            v.set(readIntSeq());
        }
        else
        {
            v.clear();
        }
    }

    public java.nio.IntBuffer
    readIntBuffer()
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
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public void
    writeLong(long v)
    {
        expand(8);
        _buf.b.putLong(v);
    }

    public void
    writeLong(int tag, Ice.LongOptional v)
    {
        if(v != null && v.isSet())
        {
            writeLong(tag, v.get());
        }
    }

    public void
    writeLong(int tag, long v)
    {
        if(writeOpt(tag, Ice.OptionalFormat.F8))
        {
            writeLong(v);
        }
    }

    public void
    writeLongSeq(long[] v)
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

    public void
    writeLongSeq(int tag, Ice.Optional<long[]> v)
    {
        if(v != null && v.isSet())
        {
            writeLongSeq(tag, v.get());
        }
    }

    public void
    writeLongSeq(int tag, long[] v)
    {
        if(writeOpt(tag, Ice.OptionalFormat.VSize))
        {
            writeSize(v == null || v.length == 0 ? 1 : v.length * 8 + (v.length > 254 ? 5 : 1));
            writeLongSeq(v);
        }
    }

    public void
    writeLongBuffer(java.nio.LongBuffer v)
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

    public long
    readLong()
    {
        try
        {
            return _buf.b.getLong();
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public void
    readLong(int tag, Ice.LongOptional v)
    {
        if(readOpt(tag, Ice.OptionalFormat.F8))
        {
            v.set(readLong());
        }
        else
        {
            v.clear();
        }
    }

    public long[]
    readLongSeq()
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
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public void
    readLongSeq(int tag, Ice.Optional<long[]> v)
    {
        if(readOpt(tag, Ice.OptionalFormat.VSize))
        {
            skipSize();
            v.set(readLongSeq());
        }
        else
        {
            v.clear();
        }
    }

    public java.nio.LongBuffer
    readLongBuffer()
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
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public void
    writeFloat(float v)
    {
        expand(4);
        _buf.b.putFloat(v);
    }

    public void
    writeFloat(int tag, Ice.FloatOptional v)
    {
        if(v != null && v.isSet())
        {
            writeFloat(tag, v.get());
        }
    }

    public void
    writeFloat(int tag, float v)
    {
        if(writeOpt(tag, Ice.OptionalFormat.F4))
        {
            writeFloat(v);
        }
    }

    public void
    writeFloatSeq(float[] v)
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

    public void
    writeFloatSeq(int tag, Ice.Optional<float[]> v)
    {
        if(v != null && v.isSet())
        {
            writeFloatSeq(tag, v.get());
        }
    }

    public void
    writeFloatSeq(int tag, float[] v)
    {
        if(writeOpt(tag, Ice.OptionalFormat.VSize))
        {
            writeSize(v == null || v.length == 0 ? 1 : v.length * 4 + (v.length > 254 ? 5 : 1));
            writeFloatSeq(v);
        }
    }

    public void
    writeFloatBuffer(java.nio.FloatBuffer v)
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

    public float
    readFloat()
    {
        try
        {
            return _buf.b.getFloat();
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public void
    readFloat(int tag, Ice.FloatOptional v)
    {
        if(readOpt(tag, Ice.OptionalFormat.F4))
        {
            v.set(readFloat());
        }
        else
        {
            v.clear();
        }
    }

    public float[]
    readFloatSeq()
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
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public void
    readFloatSeq(int tag, Ice.Optional<float[]> v)
    {
        if(readOpt(tag, Ice.OptionalFormat.VSize))
        {
            skipSize();
            v.set(readFloatSeq());
        }
        else
        {
            v.clear();
        }
    }

    public java.nio.FloatBuffer
    readFloatBuffer()
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
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public void
    writeDouble(double v)
    {
        expand(8);
        _buf.b.putDouble(v);
    }

    public void
    writeDouble(int tag, Ice.DoubleOptional v)
    {
        if(v != null && v.isSet())
        {
            writeDouble(tag, v.get());
        }
    }

    public void
    writeDouble(int tag, double v)
    {
        if(writeOpt(tag, Ice.OptionalFormat.F8))
        {
            writeDouble(v);
        }
    }

    public void
    writeDoubleSeq(double[] v)
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

    public void
    writeDoubleSeq(int tag, Ice.Optional<double[]> v)
    {
        if(v != null && v.isSet())
        {
            writeDoubleSeq(tag, v.get());
        }
    }

    public void
    writeDoubleSeq(int tag, double[] v)
    {
        if(writeOpt(tag, Ice.OptionalFormat.VSize))
        {
            writeSize(v == null || v.length == 0 ? 1 : v.length * 8 + (v.length > 254 ? 5 : 1));
            writeDoubleSeq(v);
        }
    }

    public void
    writeDoubleBuffer(java.nio.DoubleBuffer v)
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

    public double
    readDouble()
    {
        try
        {
            return _buf.b.getDouble();
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public void
    readDouble(int tag, Ice.DoubleOptional v)
    {
        if(readOpt(tag, Ice.OptionalFormat.F8))
        {
            v.set(readDouble());
        }
        else
        {
            v.clear();
        }
    }

    public double[]
    readDoubleSeq()
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
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public void
    readDoubleSeq(int tag, Ice.Optional<double[]> v)
    {
        if(readOpt(tag, Ice.OptionalFormat.VSize))
        {
            skipSize();
            v.set(readDoubleSeq());
        }
        else
        {
            v.clear();
        }
    }

    public java.nio.DoubleBuffer
    readDoubleBuffer()
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
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    final static java.nio.charset.Charset _utf8 = java.nio.charset.Charset.forName("UTF8");
    private java.nio.charset.CharsetEncoder _charEncoder = null;

    public void
    writeString(String v)
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
                            throw new Ice.MarshalException(ex);
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

    public void
    writeString(int tag, Ice.Optional<String> v)
    {
        if(v != null && v.isSet())
        {
            writeString(tag, v.get());
        }
    }

    public void
    writeString(int tag, String v)
    {
        if(writeOpt(tag, Ice.OptionalFormat.VSize))
        {
            writeString(v);
        }
    }

    public void
    writeStringSeq(String[] v)
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

    public void
    writeStringSeq(int tag, Ice.Optional<String[]> v)
    {
        if(v != null && v.isSet())
        {
            writeStringSeq(tag, v.get());
        }
    }

    public void
    writeStringSeq(int tag, String[] v)
    {
        if(writeOpt(tag, Ice.OptionalFormat.FSize))
        {
            int pos = startSize();
            writeStringSeq(v);
            endSize(pos);
        }
    }

    public String
    readString()
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
                throw new Ice.UnmarshalOutOfBoundsException();
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
                throw new Ice.UnmarshalOutOfBoundsException();
            }
        }
    }

    public void
    readString(int tag, Ice.Optional<String> v)
    {
        if(readOpt(tag, Ice.OptionalFormat.VSize))
        {
            v.set(readString());
        }
        else
        {
            v.clear();
        }
    }

    public String[]
    readStringSeq()
    {
        final int sz = readAndCheckSeqSize(1);
        String[] v = new String[sz];
        for(int i = 0; i < sz; i++)
        {
            v[i] = readString();
        }
        return v;
    }

    public void
    readStringSeq(int tag, Ice.Optional<String[]> v)
    {
        if(readOpt(tag, Ice.OptionalFormat.FSize))
        {
            skip(4);
            v.set(readStringSeq());
        }
        else
        {
            v.clear();
        }
    }

    public void
    writeProxy(Ice.ObjectPrx v)
    {
        _instance.proxyFactory().proxyToStream(v, this);
    }

    public void
    writeProxy(int tag, Ice.Optional<Ice.ObjectPrx> v)
    {
        if(v != null && v.isSet())
        {
            writeProxy(tag, v.get());
        }
    }

    public void
    writeProxy(int tag, Ice.ObjectPrx v)
    {
        if(writeOpt(tag, Ice.OptionalFormat.FSize))
        {
            int pos = startSize();
            writeProxy(v);
            endSize(pos);
        }
    }

    public Ice.ObjectPrx
    readProxy()
    {
        return _instance.proxyFactory().streamToProxy(this);
    }

    public void
    readProxy(int tag, Ice.Optional<Ice.ObjectPrx> v)
    {
        if(readOpt(tag, Ice.OptionalFormat.FSize))
        {
            skip(4);
            v.set(readProxy());
        }
        else
        {
            v.clear();
        }
    }

    public void
    writeEnum(int v, int maxValue)
    {
        if(isWriteEncoding_1_0())
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

    public int
    readEnum(int maxValue)
    {
        if(getReadEncoding().equals(Ice.Util.Encoding_1_0))
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

    public void
    writeObject(Ice.Object v)
    {
        initWriteEncaps();
        _writeEncapsStack.encoder.writeObject(v);
    }

    public <T extends Ice.Object> void
    writeObject(int tag, Ice.Optional<T> v)
    {
        if(v != null && v.isSet())
        {
            writeObject(tag, v.get());
        }
    }

    public void
    writeObject(int tag, Ice.Object v)
    {
        if(writeOpt(tag, Ice.OptionalFormat.Class))
        {
            writeObject(v);
        }
    }

    public void
    readObject(Patcher patcher)
    {
        initReadEncaps();
        _readEncapsStack.decoder.readObject(patcher);
    }

    public void
    readObject(int tag, Ice.Optional<Ice.Object> v)
    {
        if(readOpt(tag, Ice.OptionalFormat.Class))
        {
            Ice.OptionalObject opt = new Ice.OptionalObject(v, Ice.Object.class, Ice.ObjectImpl.ice_staticId());
            readObject(opt);
        }
        else
        {
            v.clear();
        }
    }

    public void
    writeUserException(Ice.UserException e)
    {
        initWriteEncaps();
        _writeEncapsStack.encoder.writeUserException(e);
    }

    public void
    throwException(UserExceptionFactory factory)
        throws Ice.UserException
    {
        initReadEncaps();
        _readEncapsStack.decoder.throwException(factory);
    }

    public void
    sliceObjects(boolean b)
    {
        _sliceObjects = b;
    }

    public boolean
    readOptImpl(int readTag, Ice.OptionalFormat expectedFormat)
    {
        if(isReadEncoding_1_0())
        {
            return false; // Optional members aren't supported with the 1.0 encoding.
        }

        while(true)
        {
            if(_buf.b.position() >= _readEncapsStack.start + _readEncapsStack.sz)
            {
                return false; // End of encapsulation also indicates end of optionals.
            }

            final byte b = readByte();
            final int v = b < 0 ? b + 256 : b;
            if(v == OPTIONAL_END_MARKER)
            {
                _buf.b.position(_buf.b.position() - 1); // Rewind.
                return false;
            }

            Ice.OptionalFormat format = Ice.OptionalFormat.valueOf(v & 0x07); // First 3 bits.
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
                skipOpt(format); // Skip optional data members
            }
            else
            {
                if(format != expectedFormat)
                {
                    throw new Ice.MarshalException("invalid optional data member `" + tag + "': unexpected format");
                }
                return true;
            }
        }
    }

    public boolean
    writeOptImpl(int tag, Ice.OptionalFormat format)
    {
        if(isWriteEncoding_1_0())
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

    public void
    skipOpt(Ice.OptionalFormat format)
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
            readObject(null);
            break;
        }
        }
    }

    public void
    skipOpts()
    {
        //
        // Skip remaining un-read optional members.
        //
        while(true)
        {
            if(_buf.b.position() >= _readEncapsStack.start + _readEncapsStack.sz)
            {
                return; // End of encapsulation also indicates end of optionals.
            }

            final byte b = readByte();
            final int v = b < 0 ? b + 256 : b;
            if(v == OPTIONAL_END_MARKER)
            {
                return;
            }

            Ice.OptionalFormat format = Ice.OptionalFormat.valueOf(v & 0x07); // Read first 3 bits.
            if((v >> 3) == 30)
            {
                skipSize();
            }
            skipOpt(format);
        }
    }

    public void
    skip(int size)
    {
        if(size > _buf.b.remaining())
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        _buf.b.position(_buf.b.position() + size);
    }

    public void
    skipSize()
    {
        byte b = readByte();
        if(b == -1)
        {
            skip(4);
        }
    }

    public int
    pos()
    {
        return _buf.b.position();
    }

    public void
    pos(int n)
    {
        _buf.b.position(n);
    }

    public int
    size()
    {
        return _buf.size();
    }

    public boolean
    isEmpty()
    {
        return _buf.empty();
    }

    private static class BufferedOutputStream extends java.io.OutputStream
    {
        BufferedOutputStream(byte[] data)
        {
            _data = data;
        }

        @Override
        public void
        close()
            throws java.io.IOException
        {
        }

        @Override
        public void
        flush()
            throws java.io.IOException
        {
        }

        @Override
        public void
        write(byte[] b)
            throws java.io.IOException
        {
            assert(_data.length - _pos >= b.length);
            System.arraycopy(b, 0, _data, _pos, b.length);
            _pos += b.length;
        }

        @Override
        public void
        write(byte[] b, int off, int len)
            throws java.io.IOException
        {
            assert(_data.length - _pos >= len);
            System.arraycopy(b, off, _data, _pos, len);
            _pos += len;
        }

        @Override
        public void
        write(int b)
            throws java.io.IOException
        {
            assert(_data.length - _pos >= 1);
            _data[_pos] = (byte)b;
            ++_pos;
        }

        int
        pos()
        {
            return _pos;
        }

        private byte[] _data;
        private int _pos;
    }

    public BasicStream
    compress(int headerSize, int compressionLevel)
    {
        assert(compressible());

        int uncompressedLen = size() - headerSize;
        int compressedLen = (int)(uncompressedLen * 1.01 + 600);
        byte[] compressed = new byte[compressedLen];

        byte[] data = null;
        int offset = 0;
        try
        {
            //
            // If the ByteBuffer is backed by an array then we can avoid
            // an extra copy by using the array directly.
            //
            data = _buf.b.array();
            offset = _buf.b.arrayOffset();
        }
        catch(Exception ex)
        {
            //
            // Otherwise, allocate an array to hold a copy of the uncompressed data.
            //
            data = new byte[size()];
            pos(0);
            _buf.b.get(data);
        }

        try
        {
            //
            // Compress the data using the class org.apache.tools.bzip2.CBZip2OutputStream.
            // Its constructor requires an OutputStream argument, therefore we pass the
            // compressed BasicStream in an OutputStream wrapper.
            //
            BufferedOutputStream bos = new BufferedOutputStream(compressed);
            if(_checkBZip2Magic)
            {
                //
                // For interoperability with the bzip2 C library, we insert the magic bytes
                // 'B', 'Z' before invoking the Java implementation.
                //
                bos.write('B');
                bos.write('Z');
            }
            java.lang.Object[] args = new java.lang.Object[]{ bos, Integer.valueOf(compressionLevel) };
            java.io.OutputStream os = (java.io.OutputStream)_bzOutputStreamCtor.newInstance(args);
            os.write(data, offset + headerSize, uncompressedLen);
            os.close();
            compressedLen = bos.pos();
        }
        catch(Exception ex)
        {
            throw new Ice.CompressionException("bzip2 compression failure", ex);
        }

        //
        // Don't bother if the compressed data is larger than the
        // uncompressed data.
        //
        if(compressedLen >= uncompressedLen)
        {
            return null;
        }

        BasicStream cstream = new BasicStream(_instance, _encoding);
        cstream.resize(headerSize + 4 + compressedLen, false);
        cstream.pos(0);

        //
        // Copy the header from the uncompressed stream to the
        // compressed one.
        //
        cstream._buf.b.put(data, offset, headerSize);

        //
        // Add the size of the uncompressed stream before the
        // message body.
        //
        cstream.writeInt(size());

        //
        // Add the compressed message body.
        //
        cstream._buf.b.put(compressed, 0, compressedLen);

        return cstream;
    }

    public BasicStream
    uncompress(int headerSize, int messageSizeMax)
    {
        assert(compressible());

        pos(headerSize);
        int uncompressedSize = readInt();
        if(uncompressedSize <= headerSize)
        {
            throw new Ice.IllegalMessageSizeException();
        }
        if(uncompressedSize > messageSizeMax)
        {
            IceInternal.Ex.throwMemoryLimitException(uncompressedSize, messageSizeMax);
        }

        int compressedLen = size() - headerSize - 4;

        byte[] compressed = null;
        int offset = 0;
        try
        {
            //
            // If the ByteBuffer is backed by an array then we can avoid
            // an extra copy by using the array directly.
            //
            compressed = _buf.b.array();
            offset = _buf.b.arrayOffset();
        }
        catch(Exception ex)
        {
            //
            // Otherwise, allocate an array to hold a copy of the compressed data.
            //
            compressed = new byte[size()];
            pos(0);
            _buf.b.get(compressed);
        }

        BasicStream ucStream = new BasicStream(_instance, _encoding);
        ucStream.resize(uncompressedSize, false);

        try
        {
            //
            // Uncompress the data using the class org.apache.tools.bzip2.CBZip2InputStream.
            // Its constructor requires an InputStream argument, therefore we pass the
            // compressed data in a ByteArrayInputStream.
            //
            java.io.ByteArrayInputStream bais =
                new java.io.ByteArrayInputStream(compressed, offset + headerSize + 4, compressedLen);
            if(_checkBZip2Magic)
            {
                //
                // For interoperability with the bzip2 C library, we insert the magic bytes
                // 'B', 'Z' during compression and therefore must extract them before we
                // invoke the Java implementation.
                //
                byte magicB = (byte)bais.read();
                byte magicZ = (byte)bais.read();
                if(magicB != (byte)'B' || magicZ != (byte)'Z')
                {
                    Ice.CompressionException e = new Ice.CompressionException();
                    e.reason = "bzip2 uncompression failure: invalid magic bytes";
                    throw e;
                }
            }
            java.lang.Object[] args = new java.lang.Object[]{ bais };
            java.io.InputStream is = (java.io.InputStream)_bzInputStreamCtor.newInstance(args);
            ucStream.pos(headerSize);
            byte[] arr = new byte[8 * 1024];
            int n;
            while((n = is.read(arr)) != -1)
            {
                ucStream.writeBlob(arr, 0, n);
            }
            is.close();
        }
        catch(Exception ex)
        {
            throw new Ice.CompressionException("bzip2 uncompression failure", ex);
        }

        //
        // Copy the header from the compressed stream to the uncompressed one.
        //
        ucStream.pos(0);
        ucStream._buf.b.put(compressed, offset, headerSize);

        return ucStream;
    }

    public void
    expand(int n)
    {
        _buf.expand(n);
    }

    private String
    getTypeId(int compactId)
    {
        String className = "IceCompactId.TypeId_" + Integer.toString(compactId);
        Class<?> c = getConcreteClass(className);
        if(c == null)
        {
            for(String pkg : _instance.getPackages())
            {
                c = getConcreteClass(pkg + "." + className);
                if(c != null)
                {
                    break;
                }
            }
        }
        if(c != null)
        {
            try
            {
                return (String)c.getField("typeId").get(null);
            }
            catch(Exception ex)
            {
                assert(false);
            }
        }
        return "";
    }

    private Ice.UserException
    createUserException(String id)
    {
        Ice.UserException userEx = null;

        try
        {
            Class<?> c = findClass(id);
            if(c != null)
            {
                userEx = (Ice.UserException)c.newInstance();
            }
        }
        catch(java.lang.Exception ex)
        {
            throw new Ice.MarshalException(ex);
        }

        return userEx;
    }

    private Class<?>
    findClass(String id)
        throws LinkageError
    {
        Class<?> c = null;

        //
        // To convert a Slice type id into a Java class, we do the following:
        //
        // 1. Convert the Slice type id into a classname (e.g., ::M::X -> M.X).
        // 2. If that fails, extract the top-level module (if any) from the type id
        //    and check for an Ice.Package property. If found, prepend the property
        //    value to the classname.
        // 3. If that fails, check for an Ice.Default.Package property. If found,
        //    prepend the property value to the classname.
        //
        String className = _instance.getClassForType(id);
        boolean addClass = false;
        if(className == null)
        {
            className = typeToClass(id);
            addClass = true;
        }

        c = getConcreteClass(className);
        if(c == null)
        {
            int pos = id.indexOf(':', 2);
            if(pos != -1)
            {
                String topLevelModule = id.substring(2, pos);
                String pkg = _instance.initializationData().properties.getProperty("Ice.Package." + topLevelModule);
                if(pkg.length() > 0)
                {
                    c = getConcreteClass(pkg + "." + className);
                }
            }
        }

        if(c == null)
        {
            String pkg = _instance.initializationData().properties.getProperty("Ice.Default.Package");
            if(pkg.length() > 0)
            {
                c = getConcreteClass(pkg + "." + className);
            }
        }

        if(c != null && addClass)
        {
            _instance.addClassForType(id, c.getName());
        }

        return c;
    }

    private Class<?>
    getConcreteClass(String className)
        throws LinkageError
    {
        Class<?> c = _instance.findClass(className);

        if(c != null)
        {
            //
            // Ensure the class is instantiable. The constants are
            // defined in the JVM specification (0x200 = interface,
            // 0x400 = abstract).
            //
            int modifiers = c.getModifiers();
            if((modifiers & 0x200) == 0 && (modifiers & 0x400) == 0)
            {
                return c;
            }
        }

        return null;
    }

    private static String
    fixKwd(String name)
    {
        //
        // Keyword list. *Must* be kept in alphabetical order. Note that checkedCast and uncheckedCast
        // are not Java keywords, but are in this list to prevent illegal code being generated if
        // someone defines Slice operations with that name.
        //
        final String[] keywordList =
        {
            "abstract", "assert", "boolean", "break", "byte", "case", "catch",
            "char", "checkedCast", "class", "clone", "const", "continue", "default", "do",
            "double", "else", "enum", "equals", "extends", "false", "final", "finalize",
            "finally", "float", "for", "getClass", "goto", "hashCode", "if",
            "implements", "import", "instanceof", "int", "interface", "long",
            "native", "new", "notify", "notifyAll", "null", "package", "private",
            "protected", "public", "return", "short", "static", "strictfp", "super", "switch",
            "synchronized", "this", "throw", "throws", "toString", "transient",
            "true", "try", "uncheckedCast", "void", "volatile", "wait", "while"
        };
        boolean found =  java.util.Arrays.binarySearch(keywordList, name) >= 0;
        return found ? "_" + name : name;
    }

    private String
    typeToClass(String id)
    {
        if(!id.startsWith("::"))
        {
            throw new Ice.MarshalException("expected type id but received `" + id + "'");
        }

        StringBuilder buf = new StringBuilder(id.length());

        int start = 2;
        boolean done = false;
        while(!done)
        {
            int end = id.indexOf(':', start);
            String s;
            if(end != -1)
            {
                s = id.substring(start, end);
                start = end + 2;
            }
            else
            {
                s = id.substring(start);
                done = true;
            }
            if(buf.length() > 0)
            {
                buf.append('.');
            }
            buf.append(fixKwd(s));
        }

        return buf.toString();
    }

    private Instance _instance;
    private Buffer _buf;
    private Object _closure;
    private byte[] _stringBytes; // Reusable array for reading strings.
    private char[] _stringChars; // Reusable array for reading strings.

    private enum SliceType { NoSlice, ObjectSlice, ExceptionSlice }

    abstract private static class EncapsDecoder
    {
        EncapsDecoder(BasicStream stream, boolean sliceObjects, ObjectFactoryManager f)
        {
            _stream = stream;
            _sliceObjects = sliceObjects;
            _servantFactoryManager = f;
            _typeIdIndex = 0;
            _unmarshaledMap = new java.util.TreeMap<Integer, Ice.Object>();
        }

        abstract void readObject(Patcher patcher);
        abstract void throwException(UserExceptionFactory factory)
            throws Ice.UserException;

        abstract void startInstance(SliceType type);
        abstract Ice.SlicedData endInstance(boolean preserve);
        abstract String startSlice();
        abstract void endSlice();
        abstract void skipSlice();

        boolean
        readOpt(int tag, Ice.OptionalFormat format)
        {
            return false;
        }

        void
        readPendingObjects()
        {
        }

        protected String
        readTypeId(boolean isIndex)
        {
            if(_typeIdMap == null) // Lazy initialization
            {
                _typeIdMap = new java.util.TreeMap<Integer, String>();
            }

            if(isIndex)
            {
                int index = _stream.readSize();
                String typeId = _typeIdMap.get(index);
                if(typeId == null)
                {
                    throw new Ice.UnmarshalOutOfBoundsException();
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

        protected Class<?>
        resolveClass(String typeId)
        {
            Class<?> cls = null;
            if(_typeIdCache == null)
            {
                _typeIdCache = new java.util.HashMap<String, Class<?> >(); // Lazy initialization.
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
                    cls = _stream.findClass(typeId);
                    _typeIdCache.put(typeId, cls != null ? cls : EncapsDecoder.class);
                }
                catch(java.lang.Exception ex)
                {
                    throw new Ice.NoObjectFactoryException("no object factory", typeId, ex);
                }
            }

            return cls;
        }

        protected Ice.Object
        newInstance(String typeId)
        {
            //
            // Try to find a factory registered for the specific type.
            //
            Ice.ObjectFactory userFactory = _servantFactoryManager.find(typeId);
            Ice.Object v = null;
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
                userFactory = _servantFactoryManager.find("");
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
                        v = (Ice.Object)cls.newInstance();
                    }
                    catch(java.lang.Exception ex)
                    {
                        throw new Ice.NoObjectFactoryException("no object factory", typeId, ex);
                    }
                }
            }

            return v;
        }

        protected void
        addPatchEntry(int index, Patcher patcher)
        {
            assert(index > 0);

            //
            // Check if already un-marshalled the object. If that's the case,
            // just patch the object smart pointer and we're done.
            //
            Ice.Object obj = _unmarshaledMap.get(index);
            if(obj != null)
            {
                patcher.patch(obj);
                return;
            }

            if(_patchMap == null) // Lazy initialization
            {
                _patchMap = new java.util.TreeMap<Integer, java.util.LinkedList<Patcher> >();
            }

            //
            // Add patch entry if the object isn't un-marshalled yet,
            // the smart pointer will be patched when the instance is
            // un-marshalled.
            //
            java.util.LinkedList<Patcher> l = _patchMap.get(index);
            if(l == null)
            {
                //
                // We have no outstanding instances to be patched for this
                // index, so make a new entry in the patch map.
                //
                l = new java.util.LinkedList<Patcher>();
                _patchMap.put(index, l);
            }

            //
            // Append a patch entry for this instance.
            //
            l.add(patcher);
        }

        protected void
        unmarshal(int index, Ice.Object v)
        {
            //
            // Add the object to the map of un-marshalled objects, this must
            // be done before reading the objects (for circular references).
            //
            _unmarshaledMap.put(index, v);

            //
            // Read the object.
            //
            v.__read(_stream);

            if(_patchMap != null)
            {
                //
                // Patch all instances now that the object is un-marshalled.
                //
                java.util.LinkedList<Patcher> l = _patchMap.get(index);
                if(l != null)
                {
                    assert(l.size() > 0);

                    //
                    // Patch all pointers that refer to the instance.
                    //
                    for(Patcher p : l)
                    {
                        p.patch(v);
                    }

                    //
                    // Clear out the patch map for that index -- there is nothing left
                    // to patch for that index for the time being.
                    //
                    _patchMap.remove(index);
                }
            }

            if((_patchMap == null || _patchMap.isEmpty()) && _objectList == null)
            {
                try
                {
                    v.ice_postUnmarshal();
                }
                catch(java.lang.Exception ex)
                {
                    String s = "exception raised by ice_postUnmarshal:\n" + Ex.toString(ex);
                    _stream.instance().initializationData().logger.warning(s);
                }
            }
            else
            {
                if(_objectList == null) // Lazy initialization
                {
                    _objectList = new java.util.ArrayList<Ice.Object>();
                }
                _objectList.add(v);

                if(_patchMap == null || _patchMap.isEmpty())
                {
                    //
                    // Iterate over the object list and invoke ice_postUnmarshal on
                    // each object.  We must do this after all objects have been
                    // unmarshaled in order to ensure that any object data members
                    // have been properly patched.
                    //
                    for(Ice.Object p : _objectList)
                    {
                        try
                        {
                            p.ice_postUnmarshal();
                        }
                        catch(java.lang.Exception ex)
                        {
                            String s = "exception raised by ice_postUnmarshal:\n" + Ex.toString(ex);
                            _stream.instance().initializationData().logger.warning(s);
                        }
                    }
                    _objectList.clear();
                }
            }
        }

        protected final BasicStream _stream;
        protected final boolean _sliceObjects;
        protected ObjectFactoryManager _servantFactoryManager;

        // Encapsulation attributes for object un-marshalling
        protected java.util.TreeMap<Integer, java.util.LinkedList<Patcher> > _patchMap;

        // Encapsulation attributes for object un-marshalling
        private java.util.TreeMap<Integer, Ice.Object> _unmarshaledMap;
        private java.util.TreeMap<Integer, String> _typeIdMap;
        private int _typeIdIndex;
        private java.util.List<Ice.Object> _objectList;

        private java.util.HashMap<String, Class<?> > _typeIdCache;
    }

    private static final class EncapsDecoder10 extends EncapsDecoder
    {
        EncapsDecoder10(BasicStream stream, boolean sliceObjects, ObjectFactoryManager f)
        {
            super(stream, sliceObjects, f);
            _sliceType = SliceType.NoSlice;
        }

        @Override
        void readObject(Patcher patcher)
        {
            assert(patcher != null);

            //
            // Object references are encoded as a negative integer in 1.0.
            //
            int index = _stream.readInt();
            if(index > 0)
            {
                throw new Ice.MarshalException("invalid object id");
            }
            index = -index;

            if(index == 0)
            {
                patcher.patch(null);
            }
            else
            {
                addPatchEntry(index, patcher);
            }
        }

        @Override
        void throwException(UserExceptionFactory factory)
            throws Ice.UserException
        {
            assert(_sliceType == SliceType.NoSlice);

            //
            // User exception with the 1.0 encoding start with a boolean flag
            // that indicates whether or not the exception has classes.
            //
            // This allows reading the pending objects even if some part of
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
                Ice.UserException userEx = null;

                //
                // Use a factory if one was provided.
                //
                if(factory != null)
                {
                    try
                    {
                        factory.createAndThrow(_typeId);
                    }
                    catch(Ice.UserException ex)
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
                    userEx.__read(_stream);
                    if(usesClasses)
                    {
                        readPendingObjects();
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
                catch(Ice.UnmarshalOutOfBoundsException ex)
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
        Ice.SlicedData endInstance(boolean preserve)
        {
            //
            // Read the Ice::Object slice.
            //
            if(_sliceType == SliceType.ObjectSlice)
            {
                startSlice();
                int sz = _stream.readSize(); // For compatibility with the old AFM.
                if(sz != 0)
                {
                    throw new Ice.MarshalException("invalid Object slice");
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
            // For objects, first read the type ID boolean which indicates
            // whether or not the type ID is encoded as a string or as an
            // index. For exceptions, the type ID is always encoded as a
            // string.
            //
            if(_sliceType == SliceType.ObjectSlice) // For exceptions, the type ID is always encoded as a string
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
                throw new Ice.UnmarshalOutOfBoundsException();
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
            if(_stream.instance().traceLevels().slicing > 0)
            {
                Ice.Logger logger = _stream.instance().initializationData().logger;
                if(_sliceType == SliceType.ObjectSlice)
                {
                    TraceUtil.traceSlicing("object", _typeId, _stream.instance().traceLevels().slicingCat, logger);
                }
                else
                {
                    TraceUtil.traceSlicing("exception", _typeId, _stream.instance().traceLevels().slicingCat, logger);
                }
            }

            assert(_sliceSize >= 4);
            _stream.skip(_sliceSize - 4);
        }

        @Override
        void readPendingObjects()
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
                throw new Ice.MarshalException("index for class received, but no instance");
            }
        }

        private void readInstance()
        {
            int index = _stream.readInt();

            if(index <= 0)
            {
                throw new Ice.MarshalException("invalid object id");
            }

            _sliceType = SliceType.ObjectSlice;
            _skipFirstSlice = false;

            //
            // Read the first slice header.
            //
            startSlice();
            final String mostDerivedId = _typeId;
            Ice.Object v = null;
            while(true)
            {
                //
                // For the 1.0 encoding, the type ID for the base Object class
                // marks the last slice.
                //
                if(_typeId.equals(Ice.ObjectImpl.ice_staticId()))
                {
                    throw new Ice.NoObjectFactoryException("", mostDerivedId);
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
                // If object slicing is disabled, stop un-marshalling.
                //
                if(!_sliceObjects)
                {
                    throw new Ice.NoObjectFactoryException("no object factory found and object slicing is disabled",
                                                           _typeId);
                }

                //
                // Slice off what we don't understand.
                //
                skipSlice();
                startSlice(); // Read next Slice header for next iteration.
            }

            //
            // Un-marshal the object and add-it to the map of un-marshaled objects.
            //
            unmarshal(index, v);
        }

        // Object/exception attributes
        private SliceType _sliceType;
        private boolean _skipFirstSlice;

        // Slice attributes
        private int _sliceSize;
        private String _typeId;
    }

    private static class EncapsDecoder11 extends EncapsDecoder
    {
        EncapsDecoder11(BasicStream stream, boolean sliceObjects, ObjectFactoryManager f)
        {
            super(stream, sliceObjects, f);
            _objectIdIndex = 1;
            _current = null;
        }

        @Override
        void readObject(Patcher patcher)
        {
            int index = _stream.readSize();
            if(index < 0)
            {
                throw new Ice.MarshalException("invalid object id");
            }
            else if(index == 0)
            {
                if(patcher != null)
                {
                    patcher.patch(null);
                }
            }
            else if(_current != null && (_current.sliceFlags & FLAG_HAS_INDIRECTION_TABLE) != 0)
            {
                //
                // When reading an object within a slice and there's an
                // indirect object table, always read an indirect reference
                // that points to an object from the indirect object table
                // marshaled at the end of the Slice.
                //
                // Maintain a list of indirect references. Note that the
                // indirect index starts at 1, so we decrement it by one to
                // derive an index into the indirection table that we'll read
                // at the end of the slice.
                //
                if(patcher != null)
                {
                    if(_current.indirectPatchList == null) // Lazy initialization
                    {
                        _current.indirectPatchList = new java.util.ArrayDeque<IndirectPatchEntry>();
                    }
                    IndirectPatchEntry e = new IndirectPatchEntry();
                    e.index = index - 1;
                    e.patcher = patcher;
                    _current.indirectPatchList.push(e);
                }
            }
            else
            {
                readInstance(index, patcher);
            }
        }

        @Override
        void throwException(UserExceptionFactory factory)
            throws Ice.UserException
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
                Ice.UserException userEx = null;

                //
                // Use a factory if one was provided.
                //
                if(factory != null)
                {
                    try
                    {
                        factory.createAndThrow(_current.typeId);
                    }
                    catch(Ice.UserException ex)
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
                    userEx.__read(_stream);
                    throw userEx;

                    // Never reached.
                }

                //
                // Slice off what we don't understand.
                //
                skipSlice();

                if((_current.sliceFlags & FLAG_IS_LAST_SLICE) != 0)
                {
                    if(mostDerivedId.startsWith("::"))
                    {
                        throw new Ice.UnknownUserException(mostDerivedId.substring(2));
                    }
                    else
                    {
                        throw new Ice.UnknownUserException(mostDerivedId);
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
        Ice.SlicedData endInstance(boolean preserve)
        {
            Ice.SlicedData slicedData = null;
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
            // Read the type ID, for object slices the type ID is encoded as a
            // string or as an index, for exceptions it's always encoded as a
            // string.
            //
            if(_current.sliceType == SliceType.ObjectSlice)
            {
                if((_current.sliceFlags & FLAG_HAS_TYPE_ID_COMPACT) == FLAG_HAS_TYPE_ID_COMPACT) // Must be checked 1st!
                {
                    _current.typeId = "";
                    _current.compactId = _stream.readSize();
                }
                else if((_current.sliceFlags & (FLAG_HAS_TYPE_ID_INDEX | FLAG_HAS_TYPE_ID_STRING)) != 0)
                {
                    _current.typeId = readTypeId((_current.sliceFlags & FLAG_HAS_TYPE_ID_INDEX) != 0);
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
            if((_current.sliceFlags & FLAG_HAS_SLICE_SIZE) != 0)
            {
                _current.sliceSize = _stream.readInt();
                if(_current.sliceSize < 4)
                {
                    throw new Ice.UnmarshalOutOfBoundsException();
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
            if((_current.sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS) != 0)
            {
                _stream.skipOpts();
            }

            //
            // Read the indirection table if one is present and transform the
            // indirect patch list into patch entries with direct references.
            //
            if((_current.sliceFlags & FLAG_HAS_INDIRECTION_TABLE) != 0)
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
                // that not all object references were read if they are from
                // unknown optional data members.
                //
                if(indirectionTable.length == 0)
                {
                    throw new Ice.MarshalException("empty indirection table");
                }
                if((_current.indirectPatchList == null || _current.indirectPatchList.isEmpty()) &&
                   (_current.sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS) == 0)
                {
                    throw new Ice.MarshalException("no references to indirection table");
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
                            throw new Ice.MarshalException("indirection out of range");
                        }
                        addPatchEntry(indirectionTable[e.index], e.patcher);
                    }
                    _current.indirectPatchList.clear();
                }
            }
        }

        @Override
        void skipSlice()
        {
            if(_stream.instance().traceLevels().slicing > 0)
            {
                Ice.Logger logger = _stream.instance().initializationData().logger;
                String slicingCat = _stream.instance().traceLevels().slicingCat;
                if(_current.sliceType == SliceType.ExceptionSlice)
                {
                    TraceUtil.traceSlicing("exception", _current.typeId, slicingCat, logger);
                }
                else
                {
                    TraceUtil.traceSlicing("object", _current.typeId, slicingCat, logger);
                }
            }

            int start = _stream.pos();

            if((_current.sliceFlags & FLAG_HAS_SLICE_SIZE) != 0)
            {
                assert(_current.sliceSize >= 4);
                _stream.skip(_current.sliceSize - 4);
            }
            else
            {
                if(_current.sliceType == SliceType.ObjectSlice)
                {
                    throw new Ice.NoObjectFactoryException("no object factory found and compact format prevents " +
                                                           "slicing (the sender should use the sliced format instead)",
                                                           _current.typeId);
                }
                else
                {
                    if(_current.typeId.startsWith("::"))
                    {
                        throw new Ice.UnknownUserException(_current.typeId.substring(2));
                    }
                    else
                    {
                        throw new Ice.UnknownUserException(_current.typeId);
                    }
                }
            }

            //
            // Preserve this slice.
            //
            Ice.SliceInfo info = new Ice.SliceInfo();
            info.typeId = _current.typeId;
            info.compactId = _current.compactId;
            info.hasOptionalMembers = (_current.sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS) != 0;
            info.isLastSlice = (_current.sliceFlags & FLAG_IS_LAST_SLICE) != 0;
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
                _current.slices = new java.util.ArrayList<Ice.SliceInfo>();
                _current.indirectionTables = new java.util.ArrayList<int[]>();
            }

            //
            // Read the indirect object table. We read the instances or their
            // IDs if the instance is a reference to an already un-marhsaled
            // object.
            //
            // The SliceInfo object sequence is initialized only if
            // readSlicedData is called.
            //

            if((_current.sliceFlags & FLAG_HAS_INDIRECTION_TABLE) != 0)
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
        boolean readOpt(int readTag, Ice.OptionalFormat expectedFormat)
        {
            if(_current == null)
            {
                return _stream.readOptImpl(readTag, expectedFormat);
            }
            else if((_current.sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS) != 0)
            {
                return _stream.readOptImpl(readTag, expectedFormat);
            }
            return false;
        }

        private int readInstance(int index, Patcher patcher)
        {
            assert(index > 0);

            if(index > 1)
            {
                if(patcher != null)
                {
                    addPatchEntry(index, patcher);
                }
                return index;
            }

            push(SliceType.ObjectSlice);

            //
            // Get the object ID before we start reading slices. If some
            // slices are skiped, the indirect object table are still read and
            // might read other objects.
            //
            index = ++_objectIdIndex;

            //
            // Read the first slice header.
            //
            startSlice();
            final String mostDerivedId = _current.typeId;
            Ice.Object v = null;
            final Ice.CompactIdResolver compactIdResolver = _stream.instance().initializationData().compactIdResolver;
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
                        _compactIdCache = new java.util.TreeMap<Integer, Class<?> >(); // Lazy initialization.
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
                                v = (Ice.Object)cls.newInstance();
                                updateCache = false;
                            }
                            catch(java.lang.Exception ex)
                            {
                                throw new Ice.NoObjectFactoryException("no object factory", "compact ID " +
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
                        if(compactIdResolver != null)
                        {
                            try
                            {
                                _current.typeId = compactIdResolver.resolve(_current.compactId);
                            }
                            catch(Ice.LocalException ex)
                            {
                                throw ex;
                            }
                            catch(Throwable ex)
                            {
                                throw new Ice.MarshalException("exception in CompactIdResolver for ID " +
                                                               _current.compactId, ex);
                            }
                        }

                        if(_current.typeId.isEmpty())
                        {
                            _current.typeId = _stream.getTypeId(_current.compactId);
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
                // If object slicing is disabled, stop un-marshalling.
                //
                if(!_sliceObjects)
                {
                    throw new Ice.NoObjectFactoryException("no object factory found and object slicing is disabled",
                                                           _current.typeId);
                }

                //
                // Slice off what we don't understand.
                //
                skipSlice();

                //
                // If this is the last slice, keep the object as an opaque
                // UnknownSlicedData object.
                //
                if((_current.sliceFlags & FLAG_IS_LAST_SLICE) != 0)
                {
                    //
                    // Provide a factory with an opportunity to supply the object.
                    // We pass the "::Ice::Object" ID to indicate that this is the
                    // last chance to preserve the object.
                    //
                    v = newInstance(Ice.ObjectImpl.ice_staticId());
                    if(v == null)
                    {
                        v = new Ice.UnknownSlicedObject(mostDerivedId);
                    }

                    break;
                }

                startSlice(); // Read next Slice header for next iteration.
            }

            //
            // Un-marshal the object
            //
            unmarshal(index, v);

            if(_current == null && _patchMap != null && !_patchMap.isEmpty())
            {
                //
                // If any entries remain in the patch map, the sender has sent an index for an object, but failed
                // to supply the object.
                //
                throw new Ice.MarshalException("index for class received, but no instance");
            }

            if(patcher != null)
            {
                patcher.patch(v);
            }
            return index;
        }

        private Ice.SlicedData readSlicedData()
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
                // We use the "objects" list in SliceInfo to hold references
                // to the target objects. Note that the objects might not have
                // been read yet in the case of a circular reference to an
                // enclosing object.
                //
                final int[] table = _current.indirectionTables.get(n);
                Ice.SliceInfo info = _current.slices.get(n);
                info.objects = new Ice.Object[table != null ? table.length : 0];
                for(int j = 0; j < info.objects.length; ++j)
                {
                    addPatchEntry(table[j], new SequencePatcher(info.objects, Ice.Object.class,
                                                                Ice.ObjectImpl.ice_staticId(), j));
                }
            }

            Ice.SliceInfo[] arr = new Ice.SliceInfo[_current.slices.size()];
            _current.slices.toArray(arr);
            return new Ice.SlicedData(arr);
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
            Patcher patcher;
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
            java.util.List<Ice.SliceInfo> slices;     // Preserved slices.
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

        private InstanceData _current;

        private int _objectIdIndex; // The ID of the next object to un-marshal.
        private java.util.TreeMap<Integer, Class<?> > _compactIdCache; // Cache of compact type IDs.
    }

    abstract private static class  EncapsEncoder
    {
        protected EncapsEncoder(BasicStream stream, WriteEncaps encaps)
        {
            _stream = stream;
            _encaps = encaps;
            _typeIdIndex = 0;
            _marshaledMap = new java.util.IdentityHashMap<Ice.Object, Integer>();
        }

        abstract void writeObject(Ice.Object v);
        abstract void writeUserException(Ice.UserException v);

        abstract void startInstance(SliceType type, Ice.SlicedData data);
        abstract void endInstance();
        abstract void startSlice(String typeId, int compactId, boolean last);
        abstract void endSlice();

        boolean writeOpt(int tag, Ice.OptionalFormat format)
        {
            return false;
        }

        void writePendingObjects()
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

        final protected BasicStream _stream;
        final protected WriteEncaps _encaps;

        // Encapsulation attributes for object marshalling.
        final protected java.util.IdentityHashMap<Ice.Object, Integer> _marshaledMap;

        // Encapsulation attributes for object marshalling.
        private java.util.TreeMap<String, Integer> _typeIdMap;
        private int _typeIdIndex;
    }

    private static final class EncapsEncoder10 extends EncapsEncoder
    {
        EncapsEncoder10(BasicStream stream, WriteEncaps encaps)
        {
            super(stream, encaps);
            _sliceType = SliceType.NoSlice;
            _objectIdIndex = 0;
            _toBeMarshaledMap = new java.util.IdentityHashMap<Ice.Object, Integer>();
        }

        @Override
        void writeObject(Ice.Object v)
        {
            //
            // Object references are encoded as a negative integer in 1.0.
            //
            if(v != null)
            {
                _stream.writeInt(-registerObject(v));
            }
            else
            {
                _stream.writeInt(0);
            }
        }

        @Override
        void writeUserException(Ice.UserException v)
        {
            //
            // User exception with the 1.0 encoding start with a boolean
            // flag that indicates whether or not the exception uses
            // classes.
            //
            // This allows reading the pending objects even if some part of
            // the exception was sliced.
            //
            boolean usesClasses = v.__usesClasses();
            _stream.writeBool(usesClasses);
            v.__write(_stream);
            if(usesClasses)
            {
                writePendingObjects();
            }
        }

        @Override
        void startInstance(SliceType sliceType, Ice.SlicedData sliceData)
        {
            _sliceType = sliceType;
        }

        @Override
        void endInstance()
        {
            if(_sliceType == SliceType.ObjectSlice)
            {
                //
                // Write the Object slice.
                //
                startSlice(Ice.ObjectImpl.ice_staticId(), -1, true);
                _stream.writeSize(0); // For compatibility with the old AFM.
                endSlice();
            }
            _sliceType = SliceType.NoSlice;
        }

        @Override
        void startSlice(String typeId, int compactId, boolean last)
        {
            //
            // For object slices, encode a boolean to indicate how the type ID
            // is encoded and the type ID either as a string or index. For
            // exception slices, always encode the type ID as a string.
            //
            if(_sliceType == SliceType.ObjectSlice)
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
        void writePendingObjects()
        {
            while(_toBeMarshaledMap.size() > 0)
            {
                //
                // Consider the to be marshalled objects as marshalled now,
                // this is necessary to avoid adding again the "to be
                // marshalled objects" into _toBeMarshaledMap while writing
                // objects.
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
                        String s = "exception raised by ice_preMarshal:\n" + Ex.toString(ex);
                        _stream.instance().initializationData().logger.warning(s);
                    }

                    p.getKey().__write(_stream);
                }
            }
            _stream.writeSize(0); // Zero marker indicates end of sequence of sequences of instances.
        }

        private int registerObject(Ice.Object v)
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
            _toBeMarshaledMap.put(v, ++_objectIdIndex);
            return _objectIdIndex;
        }

        // Instance attributes
        private SliceType _sliceType;

        // Slice attributes
        private int _writeSlice;        // Position of the slice data members

        // Encapsulation attributes for object marshalling.
        private int _objectIdIndex;
        private java.util.IdentityHashMap<Ice.Object, Integer> _toBeMarshaledMap;
    }

    private static final class EncapsEncoder11 extends EncapsEncoder
    {
        EncapsEncoder11(BasicStream stream, WriteEncaps encaps)
        {
            super(stream, encaps);
            _current = null;
            _objectIdIndex = 1;
        }

        @Override
        void writeObject(Ice.Object v)
        {
            if(v == null)
            {
                _stream.writeSize(0);
            }
            else if(_current != null && _encaps.format == Ice.FormatType.SlicedFormat)
            {
                if(_current.indirectionTable == null) // Lazy initialization
                {
                    _current.indirectionTable = new java.util.ArrayList<Ice.Object>();
                    _current.indirectionMap = new java.util.IdentityHashMap<Ice.Object, Integer>();
                }

                //
                // If writting an object within a slice and using the sliced
                // format, write an index from the object indirection
                // table. The indirect object table is encoded at the end of
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
        void writeUserException(Ice.UserException v)
        {
            v.__write(_stream);
        }

        @Override
        void startInstance(SliceType sliceType, Ice.SlicedData data)
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
            if(_encaps.format == Ice.FormatType.SlicedFormat)
            {
                _current.sliceFlags |= FLAG_HAS_SLICE_SIZE; // Encode the slice size if using the sliced format.
            }
            if(last)
            {
                _current.sliceFlags |= FLAG_IS_LAST_SLICE; // This is the last slice.
            }

            _stream.writeByte((byte)0); // Placeholder for the slice flags

            //
            // For object slices, encode the flag and the type ID either as a
            // string or index. For exception slices, always encode the type
            // ID a string.
            //
            if(_current.sliceType == SliceType.ObjectSlice)
            {
                //
                // Encode the type ID (only in the first slice for the compact
                // encoding).
                //
                if(_encaps.format == Ice.FormatType.SlicedFormat || _current.firstSlice)
                {
                    if(compactId >= 0)
                    {
                        _current.sliceFlags |= FLAG_HAS_TYPE_ID_COMPACT;
                        _stream.writeSize(compactId);
                    }
                    else
                    {
                        int index = registerTypeId(typeId);
                        if(index < 0)
                        {
                            _current.sliceFlags |= FLAG_HAS_TYPE_ID_STRING;
                            _stream.writeString(typeId);
                        }
                        else
                        {
                            _current.sliceFlags |= FLAG_HAS_TYPE_ID_INDEX;
                            _stream.writeSize(index);
                        }
                    }
                }
            }
            else
            {
                _stream.writeString(typeId);
            }

            if((_current.sliceFlags & FLAG_HAS_SLICE_SIZE) != 0)
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
            if((_current.sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS) != 0)
            {
                _stream.writeByte((byte)OPTIONAL_END_MARKER);
            }

            //
            // Write the slice length if necessary.
            //
            if((_current.sliceFlags & FLAG_HAS_SLICE_SIZE) != 0)
            {
                final int sz = _stream.pos() - _current.writeSlice + 4;
                _stream.rewriteInt(sz, _current.writeSlice - 4);
            }

            //
            // Only write the indirection table if it contains entries.
            //
            if(_current.indirectionTable != null && !_current.indirectionTable.isEmpty())
            {
                assert(_encaps.format == Ice.FormatType.SlicedFormat);
                _current.sliceFlags |= FLAG_HAS_INDIRECTION_TABLE;

                //
                // Write the indirection object table.
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
        boolean writeOpt(int tag, Ice.OptionalFormat format)
        {
            if(_current == null)
            {
                return _stream.writeOptImpl(tag, format);
            }
            else
            {
                if(_stream.writeOptImpl(tag, format))
                {
                    _current.sliceFlags |= FLAG_HAS_OPTIONAL_MEMBERS;
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }

        private void writeSlicedData(Ice.SlicedData slicedData)
        {
            assert(slicedData != null);

            //
            // We only remarshal preserved slices if we are using the sliced
            // format. Otherwise, we ignore the preserved slices, which
            // essentially "slices" the object into the most-derived type
            // known by the sender.
            //
            if(_encaps.format != Ice.FormatType.SlicedFormat)
            {
                return;
            }

            for(Ice.SliceInfo info : slicedData.slices)
            {
                startSlice(info.typeId, info.compactId, info.isLastSlice);

                //
                // Write the bytes associated with this slice.
                //
                _stream.writeBlob(info.bytes);

                if(info.hasOptionalMembers)
                {
                    _current.sliceFlags |= FLAG_HAS_OPTIONAL_MEMBERS;
                }

                //
                // Make sure to also re-write the object indirection table.
                //
                if(info.objects != null && info.objects.length > 0)
                {
                    if(_current.indirectionTable == null) // Lazy initialization
                    {
                        _current.indirectionTable = new java.util.ArrayList<Ice.Object>();
                        _current.indirectionMap = new java.util.IdentityHashMap<Ice.Object, Integer>();
                    }
                    for(Ice.Object o : info.objects)
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
            _marshaledMap.put(v, ++_objectIdIndex);

            try
            {
                v.ice_preMarshal();
            }
            catch(java.lang.Exception ex)
            {
                String s = "exception raised by ice_preMarshal:\n" + Ex.toString(ex);
                _stream.instance().initializationData().logger.warning(s);
            }

            _stream.writeSize(1); // Object instance marker.
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

        private int _objectIdIndex; // The ID of the next object to marhsal
    }

    private static final class ReadEncaps
    {
        void reset()
        {
            decoder = null;
        }

        void setEncoding(Ice.EncodingVersion encoding)
        {
            this.encoding = encoding;
            encoding_1_0 = encoding.equals(Ice.Util.Encoding_1_0);
        }

        int start;
        int sz;
        Ice.EncodingVersion encoding;
        boolean encoding_1_0;

        EncapsDecoder decoder;

        ReadEncaps next;
    }

    private static final class WriteEncaps
    {
        void reset()
        {
            encoder = null;
        }

        void setEncoding(Ice.EncodingVersion encoding)
        {
            this.encoding = encoding;
            encoding_1_0 = encoding.equals(Ice.Util.Encoding_1_0);
        }

        int start;
        Ice.FormatType format = Ice.FormatType.DefaultFormat;
        Ice.EncodingVersion encoding;
        boolean encoding_1_0;

        EncapsEncoder encoder;

        WriteEncaps next;
    }

    //
    // The encoding version to use when there's no encapsulation to
    // read from or write to. This is for example used to read message
    // headers or when the user is using the streaming API with no
    // encapsulation.
    //
    private Ice.EncodingVersion _encoding;

    private boolean isReadEncoding_1_0()
    {
        return _readEncapsStack != null ? _readEncapsStack.encoding_1_0 : _encoding.equals(Ice.Util.Encoding_1_0);
    }

    private boolean isWriteEncoding_1_0()
    {
        return _writeEncapsStack != null ? _writeEncapsStack.encoding_1_0 : _encoding.equals(Ice.Util.Encoding_1_0);
    }

    private ReadEncaps _readEncapsStack;
    private WriteEncaps _writeEncapsStack;
    private ReadEncaps _readEncapsCache;
    private WriteEncaps _writeEncapsCache;

    private void initReadEncaps()
    {
        if(_readEncapsStack == null) // Lazy initialization
        {
            _readEncapsStack = _readEncapsCache;
            if(_readEncapsStack != null)
            {
                _readEncapsCache = _readEncapsCache.next;
            }
            else
            {
                _readEncapsStack = new ReadEncaps();
            }
            _readEncapsStack.setEncoding(_encoding);
            _readEncapsStack.sz = _buf.b.limit();
        }

        if(_readEncapsStack.decoder == null) // Lazy initialization.
        {
            ObjectFactoryManager factoryManager = _instance.servantFactoryManager();
            if(_readEncapsStack.encoding_1_0)
            {
                _readEncapsStack.decoder = new EncapsDecoder10(this, _sliceObjects, factoryManager);
            }
            else
            {
                _readEncapsStack.decoder = new EncapsDecoder11(this, _sliceObjects, factoryManager);
            }
        }
    }

    private void initWriteEncaps()
    {
        if(_writeEncapsStack == null) // Lazy initialization
        {
            _writeEncapsStack = _writeEncapsCache;
            if(_writeEncapsStack != null)
            {
                _writeEncapsCache = _writeEncapsCache.next;
            }
            else
            {
                _writeEncapsStack = new WriteEncaps();
            }
            _writeEncapsStack.setEncoding(_encoding);
        }

        if(_writeEncapsStack.format == Ice.FormatType.DefaultFormat)
        {
            _writeEncapsStack.format = _instance.defaultsAndOverrides().defaultFormat;
        }

        if(_writeEncapsStack.encoder == null) // Lazy initialization.
        {
            if(_writeEncapsStack.encoding_1_0)
            {
                _writeEncapsStack.encoder = new EncapsEncoder10(this, _writeEncapsStack);
            }
            else
            {
                _writeEncapsStack.encoder = new EncapsEncoder11(this, _writeEncapsStack);
            }
        }
    }

    private boolean _sliceObjects;

    private int _startSeq;
    private int _minSeqSize;

    private static final int OPTIONAL_END_MARKER            = 0xFF;

    private static final byte FLAG_HAS_TYPE_ID_STRING       = (byte)(1<<0);
    private static final byte FLAG_HAS_TYPE_ID_INDEX        = (byte)(1<<1);
    private static final byte FLAG_HAS_TYPE_ID_COMPACT      = (byte)(1<<1 | 1<<0);
    private static final byte FLAG_HAS_OPTIONAL_MEMBERS     = (byte)(1<<2);
    private static final byte FLAG_HAS_INDIRECTION_TABLE    = (byte)(1<<3);
    private static final byte FLAG_HAS_SLICE_SIZE           = (byte)(1<<4);
    private static final byte FLAG_IS_LAST_SLICE            = (byte)(1<<5);

    private static boolean _checkedBZip2 = false;
    private static boolean _checkBZip2Magic = false;
    private static java.lang.reflect.Constructor<?> _bzInputStreamCtor;
    private static java.lang.reflect.Constructor<?> _bzOutputStreamCtor;

    public synchronized static boolean
    compressible()
    {
        //
        // Use lazy initialization when determining whether support for bzip2 compression
        // is available.
        //
        if(!_checkedBZip2)
        {
            _checkedBZip2 = true;
            try
            {
                Class<?> cls;
                Class<?>[] types = new Class<?>[1];
                cls = IceInternal.Util.findClass("org.apache.tools.bzip2.CBZip2InputStream", null);
                if(cls != null)
                {
                    types[0] = java.io.InputStream.class;
                    _bzInputStreamCtor = cls.getDeclaredConstructor(types);
                }
                cls = IceInternal.Util.findClass("org.apache.tools.bzip2.CBZip2OutputStream", null);
                if(cls != null)
                {
                    types = new Class[2];
                    types[0] = java.io.OutputStream.class;
                    types[1] = Integer.TYPE;
                    _bzOutputStreamCtor = cls.getDeclaredConstructor(types);
                }
                _checkBZip2Magic = true;
            }
            catch(Exception ex)
            {
                // Ignore - bzip2 compression not available.
            }
            if(_bzInputStreamCtor == null || _bzOutputStreamCtor == null)
            {
                try
                {
                    Class<?> cls;
                    Class<?>[] types = new Class<?>[1];
                    cls = IceInternal.Util.findClass(
                        "org.apache.commons.compress.compressors.bzip2.BZip2CompressorInputStream", null);
                    if(cls != null)
                    {
                        types[0] = java.io.InputStream.class;
                        _bzInputStreamCtor = cls.getDeclaredConstructor(types);
                    }
                    cls = IceInternal.Util.findClass(
                        "org.apache.commons.compress.compressors.bzip2.BZip2CompressorOutputStream", null);
                    if(cls != null)
                    {
                        types = new Class[2];
                        types[0] = java.io.OutputStream.class;
                        types[1] = Integer.TYPE;
                        _bzOutputStreamCtor = cls.getDeclaredConstructor(types);
                    }
                    _checkBZip2Magic = false;
                }
                catch(Exception ex)
                {
                    // Ignore - bzip2 compression not available.
                }
            }
        }
        return _bzInputStreamCtor != null && _bzOutputStreamCtor != null;
    }
}
