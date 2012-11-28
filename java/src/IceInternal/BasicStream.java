// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class BasicStream
{
    public
    BasicStream(Instance instance, Ice.EncodingVersion encoding)
    {
        this(instance, encoding, false);
    }

    public
    BasicStream(Instance instance, Ice.EncodingVersion encoding, boolean unlimited)
    {
        initialize(instance, encoding, unlimited, instance.cacheMessageBuffers() > 1);
    }

    public
    BasicStream(Instance instance, Ice.EncodingVersion encoding, boolean unlimited, boolean direct)
    {
        initialize(instance, encoding, unlimited, direct);
    }

    private void
    initialize(Instance instance, Ice.EncodingVersion encoding, boolean unlimited, boolean direct)
    {
        _instance = instance;
        _buf = new Buffer(_instance.messageSizeMax(), direct);
        _closure = null;
        _encoding = encoding;

        _readEncapsStack = null;
        _writeEncapsStack = null;
        _readEncapsCache = null;
        _writeEncapsCache = null;

        _sliceObjects = true;

        _messageSizeMax = _instance.messageSizeMax(); // Cached for efficiency.
        _unlimited = unlimited;

        _startSeq = -1;
        _sizePos = -1;
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

        boolean tmpUnlimited = other._unlimited;
        other._unlimited = _unlimited;
        _unlimited = tmpUnlimited;

        int tmpStartSeq = other._startSeq;
        other._startSeq = _startSeq;
        _startSeq = tmpStartSeq;

        int tmpMinSeqSize = other._minSeqSize;
        other._minSeqSize = _minSeqSize;
        _minSeqSize = tmpMinSeqSize;

        int tmpSizePos = other._sizePos;
        other._sizePos = _sizePos;
        _sizePos = tmpSizePos;
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
        //
        // Check memory limit if stream is not unlimited.
        //
        if(!_unlimited && sz > _messageSizeMax)
        {
            Ex.throwMemoryLimitException(sz, _messageSizeMax);
        }

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
        _writeEncapsStack.encoder.startObject(data);
    }

    public void
    endWriteObject()
    {
        assert(_writeEncapsStack != null && _writeEncapsStack.encoder != null);
        _writeEncapsStack.encoder.endObject();
    }

    public void
    startReadObject()
    {
        assert(_readEncapsStack != null && _readEncapsStack.decoder != null);
        _readEncapsStack.decoder.startObject();
    }

    public Ice.SlicedData
    endReadObject(boolean preserve)
    {
        assert(_readEncapsStack != null && _readEncapsStack.decoder != null);
        return _readEncapsStack.decoder.endObject(preserve);
    }

    public void
    startWriteException(Ice.SlicedData data)
    {
        assert(_writeEncapsStack != null && _writeEncapsStack.encoder != null);
        _writeEncapsStack.encoder.startException(data);
    }

    public void
    endWriteException()
    {
        assert(_writeEncapsStack != null && _writeEncapsStack.encoder != null);
        _writeEncapsStack.encoder.endException();
    }

    public void
    startReadException()
    {
        assert(_readEncapsStack != null && _readEncapsStack.decoder != null);
        _readEncapsStack.decoder.startException();
    }

    public Ice.SlicedData
    endReadException(boolean preserve)
    {
        assert(_readEncapsStack != null && _readEncapsStack.decoder != null);
        return _readEncapsStack.decoder.endException(preserve);
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

        if(_writeEncapsStack.encoder != null)
        {
            _writeEncapsStack.encoder.writePendingObjects();
        }

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

        if(_readEncapsStack.decoder != null)
        {
            _readEncapsStack.decoder.readPendingObjects();
        }
        else if(_buf.b.position() < _readEncapsStack.start + _readEncapsStack.sz && !_readEncapsStack.encoding_1_0)
        {
            //
            // Read remaining encapsulation optionals. This returns
            // true if the optionals end with the end marker. The end
            // marker indicates that there are more to read from the
            // encapsulation: object instances. In this case, don't
            // bother reading the objects, just skip to the end of the
            // encapsulation.
            //
            if(skipOpts())
            {
                _buf.b.position(_readEncapsStack.start + _readEncapsStack.sz);
            }
        }

        if(_buf.b.position() != _readEncapsStack.start + _readEncapsStack.sz)
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

    public void
    skipEmptyEncaps(Ice.EncodingVersion encoding)
    {
        int sz = readInt();
        if(sz != 6)
        {
            throw new Ice.EncapsulationException();
        }

        final int pos = _buf.b.position();
        if(pos + 2 > _buf.size())
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }

        if(encoding != null)
        {
            encoding.__read(this);
        }
        else
        {
            _buf.b.position(pos + 2);
        }
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
    startWriteSlice(String typeId, boolean last)
    {
        assert(_writeEncapsStack != null && _writeEncapsStack.encoder != null);
        _writeEncapsStack.encoder.startSlice(typeId, last);
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
            _readEncapsStack.decoder = null;
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
            _writeEncapsStack.encoder = null;
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

    public void
    writeSizeSeq(java.util.List<Integer> v)
    {
        writeSize(v.size());
        for(Integer n : v)
        {
            writeSize(n);
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
                return (int)(b < 0 ? b + 256 : b);
            }
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public int[]
    readSizeSeq()
    {
        int sz = readSize();
        int[] v = new int[sz];

        if(sz > 0)
        {
            for(int n = 0; n < sz; ++n)
            {
                v[n] = readSize();
            }
        }

        return v;
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

    public void
    startSize()
    {
        _sizePos = _buf.b.position();
        writeInt(0); // Placeholder for 32-bit size
    }

    public void
    endSize()
    {
        assert(_sizePos >= 0);
        rewriteInt(_buf.b.position() - _sizePos - 4, _sizePos);
        _sizePos = -1;
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

    public java.io.Serializable
    readSerializable()
    {
        int sz = readAndCheckSeqSize(1);
        if (sz == 0)
        {
            return null;
        }
        try
        {
            InputStreamWrapper w = new InputStreamWrapper(sz, this);
            ObjectInputStream in = new ObjectInputStream(_instance, w);
            return (java.io.Serializable)in.readObject();
        }
        catch(java.lang.Exception ex)
        {
            throw new Ice.MarshalException("cannot deserialize object", ex);
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
            startSize();
            writeStringSeq(v);
            endSize();
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
            startSize();
            writeProxy(v);
            endSize();
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
        if(writeOpt(tag, Ice.OptionalFormat.Size))
        {
            writeObject(v);
        }
    }

    public void
    readObject(Patcher patcher)
    {
        assert(patcher != null);
        initReadEncaps();
        _readEncapsStack.decoder.readObject(patcher);
    }

    public void
    readObject(int tag, Ice.Optional<Ice.Object> v)
    {
        if(readOpt(tag, Ice.OptionalFormat.Size))
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

        //
        // Reset the encoder, the writing of the exception wrote
        // pending objects if any.
        //
        _writeEncapsStack.encoder = null;
    }

    public void
    throwException(UserExceptionFactory factory)
        throws Ice.UserException
    {
        initReadEncaps();
        try
        {
            _readEncapsStack.decoder.throwException(factory);
        }
        catch(Ice.UserException ex)
        {
            _readEncapsStack.decoder = null;
            throw ex;
        }
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

        int tag = 0;
        Ice.OptionalFormat format;
        do
        {
            if(_buf.b.position() >= _readEncapsStack.start + _readEncapsStack.sz)
            {
                return false; // End of encapsulation also indicates end of optionals.
            }

            final byte b = readByte();
            final int v = b < 0 ? (int)b + 256 : b;
            format = Ice.OptionalFormat.valueOf(v & 0x07); // First 3 bits.
            tag = v >> 3;
            if(tag == 31)
            {
                tag = readSize();
            }
        }
        while(format != Ice.OptionalFormat.EndMarker && tag < readTag && skipOpt(format)); // Skip optional data members

        if(format == Ice.OptionalFormat.EndMarker || tag > readTag)
        {
            //
            // Rewind the stream to correctly read the next optional data
            // member tag & format next time.
            //
            int offset = tag < 31 ? 1 : (tag < 255 ? 2 : 6);
            _buf.b.position(_buf.b.position() - offset);
            return false; // No optional data members with the requested tag.
        }

        assert(readTag == tag);
        if(format != expectedFormat)
        {
            String msg = "invalid optional data member `" + tag + "': unexpected format";
            throw new Ice.MarshalException(msg);
        }

        //
        // We have an optional data member with the requested tag and
        // format.
        //
        return true;
    }

    public boolean
    writeOptImpl(int tag, Ice.OptionalFormat format)
    {
        if(isWriteEncoding_1_0())
        {
            return false; // Optional members aren't supported with the 1.0 encoding.
        }

        int v = format.value();
        if(tag < 31)
        {
            v |= tag << 3;
            writeByte((byte)v);
        }
        else
        {
            v |= 0x0F8; // tag = 31
            writeByte((byte)v);
            writeSize(tag);
        }
        return true;
    }

    public boolean
    skipOpt(Ice.OptionalFormat format)
    {
        int sz;
        switch(format)
        {
        case F1:
        {
            sz = 1;
            break;
        }
        case F2:
        {
            sz = 2;
            break;
        }
        case F4:
        {
            sz = 4;
            break;
        }
        case F8:
        {
            sz = 8;
            break;
        }
        case Size:
        {
            skipSize();
            return true;
        }
        case VSize:
        {
            sz = readSize();
            break;
        }
        case FSize:
        {
            sz = readInt();
            break;
        }
        default:
        {
            return false;
        }
        }
        skip(sz);
        return true;
    }

    public boolean
    skipOpts()
    {
        //
        // Skip remaining un-read optional members.
        //
        Ice.OptionalFormat format;
        do
        {
            if(_buf.b.position() >= _readEncapsStack.start + _readEncapsStack.sz)
            {
                return false; // End of encapsulation also indicates end of optionals.
            }

            final byte b = readByte();
            final int v = b < 0 ? (int)b + 256 : b;
            format = Ice.OptionalFormat.valueOf(v & 0x07); // Read first 3 bits.
            if((v >> 3) == 31)
            {
                skipSize();
            }
        }
        while(skipOpt(format));
        assert(format == Ice.OptionalFormat.EndMarker);
        return true;
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

        public void
        close()
            throws java.io.IOException
        {
        }

        public void
        flush()
            throws java.io.IOException
        {
        }

        public void
        write(byte[] b)
            throws java.io.IOException
        {
            assert(_data.length - _pos >= b.length);
            System.arraycopy(b, 0, _data, _pos, b.length);
            _pos += b.length;
        }

        public void
        write(byte[] b, int off, int len)
            throws java.io.IOException
        {
            assert(_data.length - _pos >= len);
            System.arraycopy(b, off, _data, _pos, len);
            _pos += len;
        }

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
            //
            // For interoperability with the bzip2 C library, we insert the magic bytes
            // 'B', 'Z' before invoking the Java implementation.
            //
            bos.write((int)'B');
            bos.write((int)'Z');
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
    uncompress(int headerSize)
    {
        assert(compressible());

        pos(headerSize);
        int uncompressedSize = readInt();
        if(uncompressedSize <= headerSize)
        {
            throw new Ice.IllegalMessageSizeException();
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
        if(!_unlimited && _buf.b != null && _buf.b.position() + n > _messageSizeMax)
        {
            Ex.throwMemoryLimitException(_buf.b.position() + n, _messageSizeMax);
        }
        _buf.expand(n);
    }

    private Ice.Object
    createObject(String id)
    {
        Ice.Object obj = null;

        try
        {
            Class<?> c = findClass(id);
            if(c != null)
            {
                obj = (Ice.Object)c.newInstance();
            }
        }
        catch(java.lang.Exception ex)
        {
            throw new Ice.NoObjectFactoryException("no object factory", id, ex);
        }

        return obj;
    }

    private static final class DynamicUserExceptionFactory
        implements UserExceptionFactory
    {
        DynamicUserExceptionFactory(Class<?> c)
        {
            _class = c;
        }

        public void
        createAndThrow(String typeId)
            throws Ice.UserException
        {
            try
            {
                throw (Ice.UserException)_class.newInstance();
            }
            catch(Ice.UserException ex)
            {
                throw ex;
            }
            catch(java.lang.Exception ex)
            {
                throw new Ice.SyscallException(ex);
            }
        }

        public void
        destroy()
        {
        }

        private Class<?> _class;
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

    //
    // Optional data member format.
    //
    private static final int MemberFormatF1           = 0;
    private static final int MemberFormatF2           = 1;
    private static final int MemberFormatF4           = 2;
    private static final int MemberFormatF8           = 3;
    private static final int MemberFormatVSize        = 4;
    private static final int MemberFormatFSize        = 5;
    private static final int MemberFormatReserved     = 6;
    private static final int MemberFormatEndMarker    = 7;

    private Instance _instance;
    private Buffer _buf;
    private Object _closure;
    private byte[] _stringBytes; // Reusable array for reading strings.
    private char[] _stringChars; // Reusable array for reading strings.

    private enum SliceType { NoSlice, ObjectSlice, ExceptionSlice }

    private static final class EncapsDecoder
    {
        EncapsDecoder(BasicStream stream, ReadEncaps encaps, boolean sliceObjects)
        {
            _stream = stream;
            _encaps = encaps;
            _sliceObjects = sliceObjects;
            _traceSlicing = -1;
            _sliceType = SliceType.NoSlice;
            _typeIdIndex = 0;
            _slices = new java.util.ArrayList<Ice.SliceInfo>();
            _indirectionTables = new java.util.ArrayList<int[]>();
            _indirectPatchList = new java.util.ArrayList<IndirectPatchEntry>();
            _patchMap = new java.util.TreeMap<Integer, java.util.LinkedList<Patcher> >();
            _unmarshaledMap = new java.util.TreeMap<Integer, Ice.Object>();
            _typeIdMap = new java.util.TreeMap<Integer, String>();
        }

        void readObject(Patcher patcher)
        {
            int index = 0;
            if(_encaps.encoding_1_0)
            {
                //
                // Object references are encoded as a negative integer in 1.0.
                //
                index = _stream.readInt();
                if(index > 0)
                {
                    throw new Ice.MarshalException("invalid object id");
                }
                index = -index;
            }
            else
            {
                //
                // Later versions use a size.
                //
                index = _stream.readSize();
                if(index < 0)
                {
                    throw new Ice.MarshalException("invalid object id");
                }
            }

            if(index == 0)
            {
                patcher.patch(null);
            }
            else if(_sliceType != SliceType.NoSlice && (_sliceFlags & FLAG_HAS_INDIRECTION_TABLE) != 0)
            {
                //
                // Maintain a list of indirect references. Note that the indirect index
                // starts at 1, so we decrement it by one to derive an index into
                // the indirection table that we'll read at the end of the slice.
                //
                IndirectPatchEntry e = new IndirectPatchEntry();
                e.index = index - 1;
                e.patcher = patcher;
                _indirectPatchList.add(e);
            }
            else
            {
                addPatchEntry(index, patcher);
            }
        }

        void throwException(UserExceptionFactory factory)
            throws Ice.UserException
        {
            assert(_sliceType == SliceType.NoSlice);

            //
            // User exception with the 1.0 encoding start with a boolean flag
            // that indicates whether or not the exception has classes.
            //
            // This allows reading the pending objects even if some part of
            // the exception was sliced. With encoding > 1.0, we don't need
            // this, each slice indirect patch table indicates the presence of
            // objects.
            //
            boolean usesClasses;
            if(_encaps.encoding_1_0)
            {
                usesClasses = _stream.readBool();
            }
            else
            {
                usesClasses = true; // Always call readPendingObjects.
            }

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
                // Performance sensitive, so we use lazy initialization for
                // tracing.
                //
                if(_traceSlicing == -1)
                {
                    _traceSlicing = _stream.instance().traceLevels().slicing;
                    _slicingCat = _stream.instance().traceLevels().slicingCat;
                }
                if(_traceSlicing > 0)
                {
                    TraceUtil.traceSlicing("exception", _typeId, _slicingCat,
                                           _stream.instance().initializationData().logger);
                }

                //
                // Slice off what we don't understand.
                //
                skipSlice();

                if((_sliceFlags & FLAG_IS_LAST_SLICE) != 0)
                {
                    if(mostDerivedId.length() > 2 && mostDerivedId.charAt(0) == ':' && mostDerivedId.charAt(1) == ':')
                    {
                        throw new Ice.UnknownUserException(mostDerivedId.substring(2));
                    }
                    else
                    {
                        throw new Ice.UnknownUserException(mostDerivedId);
                    }
                }

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
                    if(_encaps.encoding_1_0)
                    {
                        // Set the reason member to a more helpful message.
                        ex.reason = "unknown exception type `" + mostDerivedId + "'";
                    }
                    throw ex;
                }
            }
        }

        void startObject()
        {
            assert(_sliceType == SliceType.ObjectSlice);
            _skipFirstSlice = true;
        }

        Ice.SlicedData endObject(boolean preserve)
        {
            if(_encaps.encoding_1_0)
            {
                //
                // Read the Ice::Object slice.
                //
                startSlice();

                //
                // For compatibility with the old AFM.
                //
                int sz = _stream.readSize();
                if(sz != 0)
                {
                    throw new Ice.MarshalException("invalid Object slice");
                }

                endSlice();
            }

            _sliceType = SliceType.NoSlice;
            Ice.SlicedData slicedData = null;
            if(preserve)
            {
                slicedData = readSlicedData();
            }
            _slices.clear();
            _indirectionTables.clear();
            return slicedData;
        }

        void startException()
        {
            assert(_sliceType == SliceType.ExceptionSlice);
            _skipFirstSlice = true;
        }

        Ice.SlicedData endException(boolean preserve)
        {
            _sliceType = SliceType.NoSlice;
            Ice.SlicedData slicedData = null;
            if(preserve)
            {
                slicedData = readSlicedData();
            }
            _slices.clear();
            _indirectionTables.clear();
            return slicedData;
        }

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
            // Read the slice flags. For the 1.0 encoding there's no flag but
            // just a boolean for object slices. The boolean indicates whether
            // or not the type ID is encoded as a string or as an index.
            //
            if(_encaps.encoding_1_0)
            {
                _sliceFlags = FLAG_HAS_SLICE_SIZE;
                if(_sliceType == SliceType.ObjectSlice) // For exceptions, the type ID is always encoded as a string
                {
                    boolean isIndex = _stream.readBool();
                    _sliceFlags |= isIndex ? FLAG_HAS_TYPE_ID_INDEX : FLAG_HAS_TYPE_ID_STRING;
                }
            }
            else
            {
                _sliceFlags = _stream.readByte();
            }

            //
            // Read the type ID, for object slices the type ID is encoded as a
            // string or as an index, for exceptions it's always encoded as a
            // string.
            //
            if(_sliceType == SliceType.ObjectSlice)
            {
                if((_sliceFlags & FLAG_HAS_TYPE_ID_INDEX) != 0)
                {
                    int index = _stream.readSize();
                    _typeId = _typeIdMap.get(index);
                    if(_typeId == null)
                    {
                        throw new Ice.UnmarshalOutOfBoundsException();
                    }
                }
                else if((_sliceFlags & FLAG_HAS_TYPE_ID_STRING) != 0)
                {
                    _typeId = _stream.readString();
                    _typeIdMap.put(++_typeIdIndex, _typeId);
                }
                else
                {
                    // Only the most derived slice encodes the type ID for the
                    // compact format.
                    _typeId = "";
                }
            }
            else
            {
                _typeId = _stream.readString();
            }

            //
            // Read the slice size if necessary.
            //
            if((_sliceFlags & FLAG_HAS_SLICE_SIZE) != 0)
            {
                _sliceSize = _stream.readInt();
                if(_sliceSize < 4)
                {
                    throw new Ice.UnmarshalOutOfBoundsException();
                }
            }
            else
            {
                _sliceSize = 0;
            }

            //
            // Reset the indirect patch list for this new slice.
            //
            _indirectPatchList.clear();
            return _typeId;
        }

        void endSlice()
        {
            if((_sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS) != 0)
            {
                _stream.skipOpts();
            }

            //
            // Read the indirection table if one is present and transform the
            // indirect patch list into patch entries with direct references.
            //
            if((_sliceFlags & FLAG_HAS_INDIRECTION_TABLE) != 0)
            {
                //
                // The table is written as a sequence<size> to conserve space.
                //
                int[] indirectionTable = _stream.readSizeSeq();

                //
                // Sanity checks. If there are optional members, it's possible
                // that not all object references were read if they are from
                // unknown optional data members.
                //
                if(indirectionTable.length == 0 && !_indirectPatchList.isEmpty())
                {
                    throw new Ice.MarshalException("empty indirection table");
                }
                else if(indirectionTable.length > 0 && _indirectPatchList.isEmpty() &&
                        (_sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS) == 0)
                {
                    throw new Ice.MarshalException("no references to indirection table");
                }

                //
                // Convert indirect references into direct references.
                //
                for(IndirectPatchEntry e : _indirectPatchList)
                {
                    assert(e.index >= 0);
                    if(e.index >= indirectionTable.length)
                    {
                        throw new Ice.MarshalException("indirection out of range");
                    }
                    final int id = indirectionTable[e.index];
                    if(id <= 0)
                    {
                        //
                        // Entries in the table must be positive, just like a regular object reference.
                        //
                        throw new Ice.MarshalException("invalid id in object indirection table");
                    }
                    addPatchEntry(id, e.patcher);
                }
            }
        }

        void skipSlice()
        {
            int start = _stream.pos();

            if((_sliceFlags & FLAG_HAS_SLICE_SIZE) != 0)
            {
                assert(_sliceSize >= 4);
                _stream.skip(_sliceSize - 4);
            }
            else
            {
                if(_sliceType == SliceType.ObjectSlice)
                {
                    throw new Ice.NoObjectFactoryException(
                        "compact format prevents slicing (the sender should use the sliced format instead)", 
                        _typeId);
                }
                else
                {
                    throw new Ice.UnknownUserException(_typeId.substring(2));
                }
            }

            if(!_encaps.encoding_1_0)
            {
                //
                // Preserve this slice.
                //
                Ice.SliceInfo info = new Ice.SliceInfo();
                info.typeId = _typeId;
                info.hasOptionalMembers = (_sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS) != 0;
                info.isLastSlice = (_sliceFlags & FLAG_IS_LAST_SLICE) != 0;
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
                _slices.add(info);

                if((_sliceFlags & FLAG_HAS_INDIRECTION_TABLE) != 0)
                {
                    //
                    // Read the indirection table, which is written as a sequence<size> to conserve space.
                    //
                    _indirectionTables.add(_stream.readSizeSeq());
                }
                else
                {
                    _indirectionTables.add(new int[0]);
                }
            }
        }

        boolean readOpt(int readTag, Ice.OptionalFormat expectedFormat)
        {
            if(_sliceType == SliceType.NoSlice)
            {
                return _stream.readOptImpl(readTag, expectedFormat);
            }
            else if((_sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS) != 0)
            {
                return _stream.readOptImpl(readTag, expectedFormat);
            }
            return false;
        }

        void readPendingObjects()
        {
            //
            // With the 1.0 encoding, we read pending objects if the marshaled
            // data uses classes. Otherwise, only read pending objects if some
            // non-nil references were read.
            //
            if(!_encaps.encoding_1_0)
            {
                if(_patchMap.isEmpty())
                {
                    return;
                }
                else
                {
                    //
                    // Read unread encapsulation optionals before reading the
                    // pending objects.
                    //
                    _stream.skipOpts();
                }
            }

            int num;
            java.util.ArrayList<Ice.Object> objectList = new java.util.ArrayList<Ice.Object>();
            do
            {
                num = _stream.readSize();
                for(int k = num; k > 0; --k)
                {
                    objectList.add(readInstance());
                }
            }
            while(num > 0);

            if(!_patchMap.isEmpty())
            {
                //
                // If any entries remain in the patch map, the sender has sent an index for an object, but failed
                // to supply the object.
                //
                throw new Ice.MarshalException("index for class received, but no instance");
            }

            //
            // Iterate over the object list and invoke ice_postUnmarshal on
            // each object.  We must do this after all objects have been
            // unmarshaled in order to ensure that any object data members
            // have been properly patched.
            //
            for(Ice.Object p : objectList)
            {
                try
                {
                    p.ice_postUnmarshal();
                }
                catch(java.lang.Exception ex)
                {
                    String s = "exception raised by ice_postUnmarshal:\n" + Ex.toString(ex);
                    _stream.instance().initializationData().logger.warning("exception raised by ice_postUnmarshal:\n");
                }
            }
        }

        private Ice.Object readInstance()
        {
            int index;
            if(_encaps.encoding_1_0)
            {
                index = _stream.readInt();
            }
            else
            {
                index = _stream.readSize();
            }

            Ice.Object v = null;
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
            ObjectFactoryManager servantFactoryManager = _stream.instance().servantFactoryManager();
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

                //
                // Try to find a factory registered for the specific type.
                //
                Ice.ObjectFactory userFactory = servantFactoryManager.find(_typeId);
                if(userFactory != null)
                {
                    v = userFactory.create(_typeId);
                }

                //
                // If that fails, invoke the default factory if one has been
                // registered.
                //
                if(v == null)
                {
                    userFactory = servantFactoryManager.find("");
                    if(userFactory != null)
                    {
                        v = userFactory.create(_typeId);
                    }
                }

                //
                // Last chance: try to instantiate the class dynamically.
                //
                if(v == null)
                {
                    v = _stream.createObject(_typeId);
                }

                //
                // We found a factory, we get out of this loop.
                //
                if(v != null)
                {
                    break;
                }

                //
                // Performance sensitive, so we use lazy initialization for tracing.
                //
                if(_traceSlicing == -1)
                {
                    _traceSlicing = _stream.instance().traceLevels().slicing;
                    _slicingCat = _stream.instance().traceLevels().slicingCat;
                }
                if(_traceSlicing > 0)
                {
                    TraceUtil.traceSlicing("class", _typeId, _slicingCat,
                                           _stream.instance().initializationData().logger);
                }

                //
                // If object slicing is disabled, stop un-marshalling.
                //
                if(!_sliceObjects)
                {
                    throw new Ice.NoObjectFactoryException("object slicing is disabled", _typeId);
                }

                //
                // Slice off what we don't understand.
                //
                skipSlice();

                //
                // If this is the last slice, keep the object as an opaque
                // UnknownSlicedData object.
                //
                if((_sliceFlags & FLAG_IS_LAST_SLICE) != 0)
                {
                    v = new Ice.UnknownSlicedObject(mostDerivedId);
                    break;
                }

                startSlice(); // Read next Slice header for next iteration.
            }

            //
            // Add the object to the map of un-marshalled objects, this must
            // be done before reading the objects (for circular references).
            //
            _unmarshaledMap.put(index, v);

            //
            // Read the object.
            //
            v.__read(_stream);

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

            return v;
        }

        private void addPatchEntry(int index, Patcher patcher)
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

            //
            // Add patch entry if the object isn't un-marshalled yet, the
            // smart pointer will be patched when the instance is
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

        private Ice.SlicedData readSlicedData()
        {
            if(_slices.isEmpty()) // No preserved slices.
            {
                return null;
            }

            //
            // The _indirectionTables member holds the indirection table for each slice
            // in _slices.
            //
            assert(_slices.size() == _indirectionTables.size());

            for(int n = 0; n < _slices.size(); ++n)
            {
                //
                // We use the "objects" list in SliceInfo to hold references to the target
                // objects. Note however that we may not have actually read these objects
                // yet, so they need to be treated just like we had read the object references
                // directly (i.e., we add them to the patch list).
                //
                // Another important note: the SlicedData object that we return here must
                // not be destroyed before readPendingObjects is called, otherwise the
                // patch references will refer to invalid addresses.
                //
                final int[] table = _indirectionTables.get(n);
                Ice.SliceInfo info = _slices.get(n);
                info.objects = new Ice.Object[table.length];
                for(int j = 0; j < table.length; ++j)
                {
                    if(table[j] <= 0)
                    {
                        throw new Ice.MarshalException("invalid id in object indirection table");
                    }
                    SequencePatcher patcher = new SequencePatcher(info.objects, Ice.Object.class,
                                                                  Ice.ObjectImpl.ice_staticId(), j);
                    addPatchEntry(table[j], patcher);
                }
            }

            Ice.SliceInfo[] arr = new Ice.SliceInfo[_slices.size()];
            _slices.toArray(arr);
            return new Ice.SlicedData(arr);
        }

        private boolean skipOpt(int format)
        {
            int sz;
            switch(format)
            {
            case MemberFormatF1:
            {
                sz = 1;
                break;
            }
            case MemberFormatF2:
            {
                sz = 2;
                break;
            }
            case MemberFormatF4:
            {
                sz = 4;
                break;
            }
            case MemberFormatF8:
            {
                sz = 8;
                break;
            }
            case MemberFormatVSize:
            {
                sz = _stream.readSize();
                break;
            }
            case MemberFormatFSize:
            {
                sz = _stream.readInt();
                break;
            }
            default:
            {
                return false;
            }
            }

            int pos = _stream.pos();
            if(pos + sz > _stream.size())
            {
                throw new Ice.UnmarshalOutOfBoundsException();
            }
            _stream.pos(pos + sz);
            return true;
        }

        private final BasicStream _stream;
        private final ReadEncaps _encaps;
        private final boolean _sliceObjects;

        private int _traceSlicing;
        private String _slicingCat;

        // Object/exception attributes
        private SliceType _sliceType;
        private boolean _skipFirstSlice;
        private java.util.ArrayList<Ice.SliceInfo> _slices;     // Preserved slices.
        private java.util.ArrayList<int[]> _indirectionTables;

        // Slice attributes
        private byte _sliceFlags;
        private int _sliceSize;
        private String _typeId;

        private static final class IndirectPatchEntry
        {
            int index;
            Patcher patcher;
        }
        private java.util.ArrayList<IndirectPatchEntry> _indirectPatchList;

        // Encapsulation attributes for object un-marshalling
        private java.util.TreeMap<Integer, java.util.LinkedList<Patcher> > _patchMap;
        private java.util.TreeMap<Integer, Ice.Object> _unmarshaledMap;
        private java.util.TreeMap<Integer, String> _typeIdMap;
        private int _typeIdIndex;
    }

    private static final class EncapsEncoder
    {
        EncapsEncoder(BasicStream stream, WriteEncaps encaps)
        {
            _stream = stream;
            _encaps = encaps;
            _sliceType = SliceType.NoSlice;
            _objectIdIndex = 0;
            _typeIdIndex = 0;
            _indirectionTable = new java.util.ArrayList<Integer>();
            _indirectionMap = new java.util.TreeMap<Integer, Integer>();
            _toBeMarshaledMap = new java.util.IdentityHashMap<Ice.Object, Integer>();
            _marshaledMap = new java.util.IdentityHashMap<Ice.Object, Integer>();
            _typeIdMap = new java.util.TreeMap<String, Integer>();
        }

        void writeObject(Ice.Object v)
        {
            if(v != null)
            {
                //
                // Register the object.
                //
                int index = registerObject(v);

                if(_encaps.encoding_1_0)
                {
                    //
                    // Object references are encoded as a negative integer in 1.0.
                    //
                    _stream.writeInt(-index);
                }
                else if(_sliceType != SliceType.NoSlice && _encaps.format == Ice.FormatType.SlicedFormat)
                {
                    //
                    // An object reference that appears inside a slice of an
                    // object or exception encoded as a positive non-zero
                    // index into a per-slice indirection table.
                    //
                    // We use _indirectionMap to keep track of the object
                    // references in the current slice; it maps the object
                    // reference to the position in the indirection list. Note
                    // that the position is offset by one (e.g., the first
                    // position = 1).
                    //
                    Integer p = _indirectionMap.get(index);
                    if(p == null)
                    {
                        _indirectionTable.add(index);
                        int sz = _indirectionTable.size(); // Position + 1
                        _indirectionMap.put(index, sz);
                        _stream.writeSize(sz);
                    }
                    else
                    {
                        _stream.writeSize(p.intValue());
                    }
                }
                else
                {
                    _stream.writeSize(index);
                }
            }
            else
            {
                //
                // Write nil reference.
                //
                if(_encaps.encoding_1_0)
                {
                    _stream.writeInt(0);
                }
                else
                {
                    _stream.writeSize(0);
                }
            }
        }

        void writeUserException(Ice.UserException v)
        {
            //
            // User exception with the 1.0 encoding start with a boolean
            // flag that indicates whether or not the exception uses
            // classes. 
            //
            // This allows reading the pending objects even if some part of
            // the exception was sliced. With encoding > 1.0, we don't need
            // this, each slice indirect patch table indicates the presence of
            // objects.
            //
            boolean usesClasses;
            if(_encaps.encoding_1_0)
            {
                usesClasses = v.__usesClasses();
                _stream.writeBool(usesClasses);
            }
            else
            {
                usesClasses = true; // Always call writePendingObjects
            }
            v.__write(_stream);
            if(usesClasses)
            {
                writePendingObjects();
            }
        }

        void startObject(Ice.SlicedData data)
        {
            _sliceType = SliceType.ObjectSlice;
            _firstSlice = true;
            if(data != null)
            {
                writeSlicedData(data);
            }
        }

        void endObject()
        {
            if(_encaps.encoding_1_0)
            {
                //
                // Write the Object slice.
                //
                startSlice(Ice.ObjectImpl.ice_staticId(), true);
                _stream.writeSize(0); // For compatibility with the old AFM.
                endSlice();
            }
            _sliceType = SliceType.NoSlice;
        }

        void startException(Ice.SlicedData data)
        {
            _sliceType = SliceType.ExceptionSlice;
            _firstSlice = true;
            if(data != null)
            {
                writeSlicedData(data);
            }
        }

        void endException()
        {
            _sliceType = SliceType.NoSlice;
        }

        void startSlice(String typeId, boolean last)
        {
            assert(_indirectionTable.isEmpty() && _indirectionMap.isEmpty());
            _sliceFlags = (byte)0;
            _sliceFlagsPos = _stream.pos();

            //
            // Encode the slice size for the old encoding and if using the
            // sliced format.
            //
            if(_encaps.encoding_1_0 || _encaps.format == Ice.FormatType.SlicedFormat)
            {
                _sliceFlags |= FLAG_HAS_SLICE_SIZE;
            }

            //
            // This is the last slice.
            //
            if(last)
            {
                _sliceFlags |= FLAG_IS_LAST_SLICE;
            }

            //
            // For object slices, encode the flag and the type ID either as a
            // string or index. For exception slices, don't encode slice flags
            // for the old encoding and always encode the type ID a string.
            //
            if(_sliceType == SliceType.ObjectSlice)
            {
                _stream.writeByte((byte)0); // Placeholder for the slice flags

                //
                // Encode the type ID (only in the first slice for the compact
                // encoding).
                //
                if(_encaps.format == Ice.FormatType.SlicedFormat || _encaps.encoding_1_0 || _firstSlice)
                {
                    //
                    // If the type ID has already been seen, write the index
                    // of the type ID, otherwise allocate a new type ID and
                    // write the string.
                    //
                    Integer p = _typeIdMap.get(typeId);
                    if(p != null)
                    {
                        _sliceFlags |= FLAG_HAS_TYPE_ID_INDEX;
                        _stream.writeSize(p.intValue());
                    }
                    else
                    {
                        _sliceFlags |= FLAG_HAS_TYPE_ID_STRING;
                        _typeIdMap.put(typeId, ++_typeIdIndex);
                        _stream.writeString(typeId);
                    }
                }
            }
            else
            {
                if(!_encaps.encoding_1_0)
                {
                    _stream.writeByte((byte)0); // Placeholder for the slice flags
                }
                _stream.writeString(typeId);
            }

            if((_sliceFlags & FLAG_HAS_SLICE_SIZE) != 0)
            {
                _stream.writeInt(0); // Placeholder for the slice length.
            }

            _writeSlice = _stream.pos();
            _firstSlice = false;
        }

        void endSlice()
        {
            //
            // Write the optional member end marker if some optional members
            // were encoded. Note that the optional members are encoded before
            // the indirection table and are included in the slice size.
            //
            if((_sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS) != 0)
            {
                assert(!_encaps.encoding_1_0);
                _stream.writeByte((byte)Ice.OptionalFormat.EndMarker.value());
            }

            //
            // Write the slice length if necessary.
            //
            if((_sliceFlags & FLAG_HAS_SLICE_SIZE) != 0)
            {
                final int sz = _stream.pos() - _writeSlice + 4;
                _stream.rewriteInt(sz, _writeSlice - 4);
            }

            //
            // Only write the indirection table if it contains entries.
            //
            if(!_indirectionTable.isEmpty())
            {
                assert(!_encaps.encoding_1_0);
                assert(_encaps.format == Ice.FormatType.SlicedFormat);
                _sliceFlags |= FLAG_HAS_INDIRECTION_TABLE;

                //
                // Write the indirection table as a sequence<size> to conserve space.
                //
                _stream.writeSizeSeq(_indirectionTable);

                _indirectionTable.clear();
                _indirectionMap.clear();
            }

            //
            // Finally, update the slice flags (or the object slice has index
            // type ID boolean for the 1.0 encoding)
            //
            if(_encaps.encoding_1_0)
            {
                if(_sliceType == SliceType.ObjectSlice) // No flags for 1.0 exception slices.
                {
                    _stream.rewriteBool((_sliceFlags & FLAG_HAS_TYPE_ID_INDEX) != 0, _sliceFlagsPos);
                }
            }
            else
            {
                _stream.rewriteByte(_sliceFlags, _sliceFlagsPos);
            }
        }

        boolean writeOpt(int tag, Ice.OptionalFormat format)
        {
            if(_sliceType == SliceType.NoSlice)
            {
                return _stream.writeOptImpl(tag, format);
            }
            else
            {
                if(_stream.writeOptImpl(tag, format))
                {
                    _sliceFlags |= FLAG_HAS_OPTIONAL_MEMBERS;
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }

        void writePendingObjects()
        {
            //
            // With the 1.0 encoding, write pending objects if the marshalled
            // data uses classes. Otherwise with encoding > 1.0, only write
            // pending objects if some non-nil references were written.
            //
            if(!_encaps.encoding_1_0)
            {
                if(_toBeMarshaledMap.isEmpty())
                {
                    return;
                }
                else
                {
                    //
                    // Write end marker for encapsulation optionals before encoding
                    // the pending objects.
                    //
                    _stream.writeByte((byte)Ice.OptionalFormat.EndMarker.value());
                }
            }

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
                    if(_encaps.encoding_1_0)
                    {
                        _stream.writeInt(p.getValue().intValue());
                    }
                    else
                    {
                        _stream.writeSize(p.getValue().intValue());
                    }

                    try
                    {
                        p.getKey().ice_preMarshal();
                    }
                    catch(java.lang.Exception ex)
                    {
                        String s = "exception raised by ice_preUnmarshal:\n" + Ex.toString(ex);
                        _stream.instance().initializationData().logger.warning(
                            "exception raised by ice_preUnmarshal:\n");
                    }

                    p.getKey().__write(_stream);
                }
            }
            _stream.writeSize(0); // Zero marker indicates end of sequence of sequences of instances.
        }

        private void writeSlicedData(Ice.SlicedData slicedData)
        {
            assert(slicedData != null);

            //
            // We only remarshal preserved slices if the target encoding is > 1.0 and we are
            // using the sliced format. Otherwise, we ignore the preserved slices, which
            // essentially "slices" the object into the most-derived type known by the sender.
            //
            if(_encaps.encoding_1_0 || _encaps.format != Ice.FormatType.SlicedFormat)
            {
                return;
            }

            for(int n = 0; n < slicedData.slices.length; ++n)
            {
                Ice.SliceInfo info = slicedData.slices[n];
                startSlice(info.typeId, info.isLastSlice);

                //
                // Write the bytes associated with this slice.
                //
                _stream.writeBlob(info.bytes);

                if(info.hasOptionalMembers)
                {
                    _sliceFlags |= FLAG_HAS_OPTIONAL_MEMBERS;
                }

                //
                // Assemble and write the indirection table. The table must have the same order
                // as the list of objects.
                //
                for(int j = 0; j < info.objects.length; ++j)
                {
                    _indirectionTable.add(registerObject(info.objects[j]));
                }

                endSlice();
            }
        }

        private int registerObject(Ice.Object v)
        {
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

        private final BasicStream _stream;
        private final WriteEncaps _encaps;

        // Object/exception attributes
        private SliceType _sliceType;
        private boolean _firstSlice;

        // Slice attributes
        private byte _sliceFlags;
        private int _writeSlice;        // Position of the slice data members
        private int _sliceFlagsPos;     // Position of the slice flags
        private java.util.ArrayList<Integer> _indirectionTable;
        private java.util.TreeMap<Integer, Integer> _indirectionMap;

        // Encapsulation attributes for object marshalling.
        private int _objectIdIndex;
        private java.util.IdentityHashMap<Ice.Object, Integer> _toBeMarshaledMap;
        private java.util.IdentityHashMap<Ice.Object, Integer> _marshaledMap;
        private java.util.TreeMap<String, Integer> _typeIdMap;
        private int _typeIdIndex;
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
            _readEncapsStack.decoder = new EncapsDecoder(this, _readEncapsStack, _sliceObjects);
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
            _writeEncapsStack.encoder = new EncapsEncoder(this, _writeEncapsStack);
        }
    }

    private boolean _sliceObjects;

    private int _messageSizeMax;
    private boolean _unlimited;

    private int _startSeq;
    private int _minSeqSize;

    private int _sizePos;

    private static final byte FLAG_HAS_TYPE_ID_STRING       = (byte)(1<<0);
    private static final byte FLAG_HAS_TYPE_ID_INDEX        = (byte)(1<<1);
    private static final byte FLAG_HAS_OPTIONAL_MEMBERS     = (byte)(1<<2);
    private static final byte FLAG_HAS_INDIRECTION_TABLE    = (byte)(1<<3);
    private static final byte FLAG_HAS_SLICE_SIZE           = (byte)(1<<4);
    private static final byte FLAG_IS_LAST_SLICE            = (byte)(1<<5);

    private static boolean _checkedBZip2 = false;
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
            }
            catch(Exception ex)
            {
                // Ignore - bzip2 compression not available.
            }
        }
        return _bzInputStreamCtor != null && _bzOutputStreamCtor != null;
    }
}
