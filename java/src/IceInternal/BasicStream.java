// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class BasicStream
{
    public
    BasicStream(Instance instance)
    {
        initialize(instance, false);
    }

    public
    BasicStream(Instance instance, boolean unlimited)
    {
        initialize(instance, unlimited);
    }

    private void
    initialize(Instance instance, boolean unlimited)
    {
        _instance = instance;
        _buf = new Buffer(_instance.messageSizeMax());
        _closure = null;
        _unlimited = unlimited;

        _readEncapsStack = null;
        _writeEncapsStack = null;
        _readEncapsCache = null;
        _writeEncapsCache = null;
        
        _traceSlicing = -1;

        _sliceObjects = true;

        _messageSizeMax = _instance.messageSizeMax(); // Cached for efficiency.

        _seqDataStack = null;
        _objectList = null;
    }

    //
    // This function allows this object to be reused, rather than
    // reallocated.
    //
    public void
    reset()
    {
        _buf.reset();

        if(_readEncapsStack != null)
        {
            assert(_readEncapsStack.next == null);
            _readEncapsStack.next = _readEncapsCache;
            _readEncapsCache = _readEncapsStack;
            _readEncapsStack = null;
            _readEncapsCache.reset();
        }

        if(_objectList != null)
        {
            _objectList.clear();
        }

        _sliceObjects = true;
    }

    public void
    clear()
    {
        _readEncapsStack = null;
        _writeEncapsStack = null;
        _seqDataStack = null;
        
        if(_objectList != null)
        {
            _objectList.clear();
        }
        _objectList = null;
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

        Object tmpClosure = other._closure;
        other._closure = _closure;
        _closure = tmpClosure;

        Buffer tmpBuf = other._buf;
        other._buf = _buf;
        _buf = tmpBuf;

        ReadEncaps tmpRead = other._readEncapsStack;
        other._readEncapsStack = _readEncapsStack;
        _readEncapsStack = tmpRead;

        tmpRead = other._readEncapsCache;
        other._readEncapsCache = _readEncapsCache;
        _readEncapsCache = tmpRead;

        WriteEncaps tmpWrite = other._writeEncapsStack;
        other._writeEncapsStack = _writeEncapsStack;
        _writeEncapsStack = tmpWrite;

        tmpWrite = other._writeEncapsCache;
        other._writeEncapsCache = _writeEncapsCache;
        _writeEncapsCache = tmpWrite;

        int tmpReadSlice = other._readSlice;
        other._readSlice = _readSlice;
        _readSlice = tmpReadSlice;

        int tmpWriteSlice = other._writeSlice;
        other._writeSlice = _writeSlice;
        _writeSlice = tmpWriteSlice;

        SeqData tmpSeqDataStack = other._seqDataStack;
        other._seqDataStack = _seqDataStack;
        _seqDataStack = tmpSeqDataStack;

        java.util.ArrayList<Ice.Object> tmpObjectList = other._objectList;
        other._objectList = _objectList;
        _objectList = tmpObjectList;

        boolean tmpUnlimited = other._unlimited;
        other._unlimited = _unlimited;
        _unlimited = tmpUnlimited;
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

    //
    // startSeq() and endSeq() sanity-check sequence sizes during
    // unmarshaling and prevent malicious messages with incorrect
    // sequence sizes from causing the receiver to use up all
    // available memory by allocating sequences with an impossibly
    // large number of elements.
    //
    // The code generator inserts calls to startSeq() and endSeq()
    // around the code to unmarshal a sequence. startSeq() is called
    // immediately after reading the sequence size, and endSeq() is
    // called after reading the final element of a sequence.
    //
    // For sequences that contain constructed types that, in turn,
    // contain sequences, the code generator also inserts a call to
    // endElement() after unmarshaling each element.
    //
    // startSeq() is passed the unmarshaled element count, plus the
    // minimum size (in bytes) occupied by the sequence's element
    // type. numElements * minSize is the smallest possible number of
    // bytes that the sequence will occupy on the wire.
    //
    // Every time startSeq() is called, it pushes the element count
    // and the minimum size on a stack. Every time endSeq() is called,
    // it pops the stack.
    //
    // For an ordinary sequence (one that does not (recursively)
    // contain nested sequences), numElements * minSize must be less
    // than the number of bytes remaining in the stream.
    //
    // For a sequence that is nested within some other sequence, there
    // must be enough bytes remaining in the stream for this sequence
    // (numElements + minSize), plus the sum of the bytes required by
    // the remaining elements of all the enclosing sequences.
    //
    // For the enclosing sequences, numElements - 1 is the number of
    // elements for which unmarshaling has not started yet. (The call
    // to endElement() in the generated code decrements that number
    // whenever a sequence element is unmarshaled.)
    //
    // For sequence that variable-length elements, checkSeq() is
    // called whenever an element is unmarshaled. checkSeq() also
    // checks whether the stream has a sufficient number of bytes
    // remaining.  This means that, for messages with bogus sequence
    // sizes, unmarshaling is aborted at the earliest possible point.
    //

    public void
    startSeq(int numElements, int minSize)
    {
        if(numElements == 0) // Optimization to avoid pushing a useless stack frame.
        {
            return;
        }

        //
        // Push the current sequence details on the stack.
        //
        SeqData sd = new SeqData(numElements, minSize);
        sd.previous = _seqDataStack;
        _seqDataStack = sd;

        int bytesLeft = _buf.b.remaining();
        if(_seqDataStack.previous == null) // Outermost sequence
        {
            //
            // The sequence must fit within the message.
            //
            if(numElements * minSize > bytesLeft) 
            {
                throw new Ice.UnmarshalOutOfBoundsException();
            }
        }
        else // Nested sequence
        {
            checkSeq(bytesLeft);
        }
    }

    //
    // Check, given the number of elements requested for this
    // sequence, that this sequence, plus the sum of the sizes of the
    // remaining number of elements of all enclosing sequences, would
    // still fit within the message.
    //
    public void
    checkSeq()
    {
        checkSeq(_buf.b.remaining());
    }

    public void
    checkSeq(int bytesLeft)
    {
        int size = 0;
        SeqData sd = _seqDataStack;
        do
        {
            size += (sd.numElements - 1) * sd.minSize;
            sd = sd.previous;
        }
        while(sd != null);

        if(size > bytesLeft)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public void
    checkFixedSeq(int numElements, int elemSize)
    {
        int bytesLeft = _buf.b.remaining();
        if(_seqDataStack == null) // Outermost sequence
        {
            //
            // The sequence must fit within the message.
            //
            if(numElements * elemSize > bytesLeft) 
            {
                throw new Ice.UnmarshalOutOfBoundsException();
            }
        }
        else // Nested sequence
        {
            checkSeq(bytesLeft - numElements * elemSize);
        }
    }

    public void
    endSeq(int sz)
    {
        if(sz == 0) // Pop only if something was pushed previously.
        {
            return;
        }

        //
        // Pop the sequence stack.
        //
        SeqData oldSeqData = _seqDataStack;
        assert(oldSeqData != null);
        _seqDataStack = oldSeqData.previous;
    }

    public void
    endElement()
    {
        assert(_seqDataStack != null);
        --_seqDataStack.numElements;
    }

    final private static byte[] _encapsBlob =
    {
        0, 0, 0, 0, // Placeholder for the encapsulation length.
        Protocol.encodingMajor,
        Protocol.encodingMinor
    };

    public void
    startWriteEncaps()
    {
        {
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
        }

        _writeEncapsStack.start = _buf.size();
        writeBlob(_encapsBlob);
    }

    public void
    endWriteEncaps()
    {
        assert(_writeEncapsStack != null);
        int start = _writeEncapsStack.start;
        int sz = _buf.size() - start; // Size includes size and version.
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
    startReadEncaps()
    {
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
        }

        _readEncapsStack.start = _buf.b.position();

        //
        // I don't use readSize() and writeSize() for encapsulations,
        // because when creating an encapsulation, I must know in
        // advance how many bytes the size information will require in
        // the data stream. If I use an Int, it is always 4 bytes. For
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

        byte eMajor = readByte();
        byte eMinor = readByte();
        if(eMajor != Protocol.encodingMajor || eMinor > Protocol.encodingMinor)
        {
            Ice.UnsupportedEncodingException e = new Ice.UnsupportedEncodingException();
            e.badMajor = eMajor < 0 ? eMajor + 256 : eMajor;
            e.badMinor = eMinor < 0 ? eMinor + 256 : eMinor;
            e.major = Protocol.encodingMajor;
            e.minor = Protocol.encodingMinor;
            throw e;
        }
        // _readEncapsStack.encodingMajor = eMajor; // Currently unused
        // _readEncapsStack.encodingMinor = eMinor; // Currently unused
    }

    public void
    endReadEncaps()
    {
        assert(_readEncapsStack != null);
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
    skipEmptyEncaps()
    {
        int sz = readInt();
        if(sz < 6)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }

        if(sz != 6)
        {
            throw new Ice.EncapsulationException();
        }

        try
        {
            _buf.b.position(_buf.b.position() + 2);
        }
        catch(IllegalArgumentException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
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

    public int
    getReadEncapsSize()
    {
        assert(_readEncapsStack != null);
        return _readEncapsStack.sz - 6;
    }

    public void
    skipEncaps()
    {
        int sz = readInt();
        if(sz < 6)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        try
        {
            _buf.b.position(_buf.b.position() + sz - 4);
        }
        catch(IllegalArgumentException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public void
    startWriteSlice()
    {
        writeInt(0); // Placeholder for the slice length.
        _writeSlice = _buf.size();
    }

    public void endWriteSlice()
    {
        final int sz = _buf.size() - _writeSlice + 4;
        _buf.b.putInt(_writeSlice - 4, sz);
    }

    public void startReadSlice()
    {
        int sz = readInt();
        if(sz < 4)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        _readSlice = _buf.b.position();
    }

    public void endReadSlice()
    {
    }

    public void skipSlice()
    {
        int sz = readInt();
        if(sz < 4)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        try
        {
            _buf.b.position(_buf.b.position() + sz - 4);
        }
        catch(IllegalArgumentException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
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
                return (int)(b < 0 ? b + 256 : b);
            }
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public void
    writeTypeId(String id)
    {
        if(_writeEncapsStack == null || _writeEncapsStack.typeIdMap == null)
        {
            //
            // writeObject() must be called first.
            //
            throw new Ice.MarshalException("type ids require an encapsulation");
        }

        Integer index = _writeEncapsStack.typeIdMap.get(id);
        if(index != null)
        {
            writeBool(true);
            writeSize(index.intValue());
        }
        else
        {
            index = Integer.valueOf(++_writeEncapsStack.typeIdIndex);
            _writeEncapsStack.typeIdMap.put(id, index);
            writeBool(false);
            writeString(id);
        }
    }

    public String
    readTypeId()
    {
        if(_readEncapsStack == null || _readEncapsStack.typeIdMap == null)
        {
            //
            // readObject() must be called first.
            //
            throw new Ice.MarshalException("type ids require an encapsulation");
        }

        String id;
        Integer index;
        final boolean isIndex = readBool();
        if(isIndex)
        {
            index = Integer.valueOf(readSize());
            id = _readEncapsStack.typeIdMap.get(index);
            if(id == null)
            {
                throw new Ice.UnmarshalOutOfBoundsException();
            }
        }
        else
        {
            id = readString();
            index = Integer.valueOf(++_readEncapsStack.typeIdIndex);
            _readEncapsStack.typeIdMap.put(index, id);
        }
        return id;
    }

    public void
    writeBlob(byte[] v)
    {
        expand(v.length);
        _buf.b.put(v);
    }

    public void
    writeBlob(byte[] v, int off, int len)
    {
        expand(len);
        _buf.b.put(v, off, len);
    }

    public byte[]
    readBlob(int sz)
    {
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

    public void
    writeByte(byte v)
    {
        expand(1);
        _buf.b.put(v);
    }

    public void
    writeByte(byte v, int end)
    {
        if(v < 0 || v >= end)
        {
            throw new Ice.MarshalException("enumerator out of range");
        }
        writeByte(v);
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

    public byte
    readByte(int end)
    {
        byte v = readByte();
        if(v < 0 || v >= end)
        {
            throw new Ice.MarshalException("enumerator out of range");
        }
        return v;
    }

    public byte[]
    readByteSeq()
    {
        try
        {
            final int sz = readSize();
            checkFixedSeq(sz, 1);
            byte[] v = new byte[sz];
            _buf.b.get(v);
            return v;
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public java.io.Serializable
    readSerializable()
    {
        int sz = readSize();
        if (sz == 0)
        {
            return null;
        }
        checkFixedSeq(sz, 1);
        try
        {
            InputStreamWrapper w = new InputStreamWrapper(sz, this);
            ObjectInputStream in = new ObjectInputStream(w);
            return (java.io.Serializable)in.readObject();
        }
        catch(java.lang.Exception ex)
        {
            Ice.MarshalException e = new Ice.MarshalException("cannot deserialize object");
            e.initCause(ex);
            throw e;
        }
    }

    public void
    writeBool(boolean v)
    {
        expand(1);
        _buf.b.put(v ? (byte)1 : (byte)0);
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

    public boolean[]
    readBoolSeq()
    {
        try
        {
            final int sz = readSize();
            checkFixedSeq(sz, 1);
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
    writeShort(short v)
    {
        expand(2);
        _buf.b.putShort(v);
    }

    public void
    writeShort(short v, int end)
    {
        if(v < 0 || v >= end)
        {
            throw new Ice.MarshalException("enumerator out of range");
        }
        writeShort(v);
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

    public short
    readShort(int end)
    {
        short v = readShort();
        if(v < 0 || v >= end)
        {
            throw new Ice.MarshalException("enumerator out of range");
        }
        return v;
    }

    public short[]
    readShortSeq()
    {
        try
        {
            final int sz = readSize();
            checkFixedSeq(sz, 2);
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
    writeInt(int v)
    {
        expand(4);
        _buf.b.putInt(v);
    }

    public void
    writeInt(int v, int end)
    {
        if(v < 0 || v >= end)
        {
            throw new Ice.MarshalException("enumerator out of range");
        }
        writeInt(v);
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

    public int
    readInt(int end)
    {
        int v = readInt();
        if(v < 0 || v >= end)
        {
            throw new Ice.MarshalException("enumerator out of range");
        }
        return v;
    }

    public int[]
    readIntSeq()
    {
        try
        {
            final int sz = readSize();
            checkFixedSeq(sz, 4);
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
    writeLong(long v)
    {
        expand(8);
        _buf.b.putLong(v);
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

    public long[]
    readLongSeq()
    {
        try
        {
            final int sz = readSize();
            checkFixedSeq(sz, 8);
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
    writeFloat(float v)
    {
        expand(4);
        _buf.b.putFloat(v);
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

    public float[]
    readFloatSeq()
    {
        try
        {
            final int sz = readSize();
            checkFixedSeq(sz, 4);
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
    writeDouble(double v)
    {
        expand(8);
        _buf.b.putDouble(v);
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

    public double[]
    readDoubleSeq()
    {
        try
        {
            final int sz = readSize();
            checkFixedSeq(sz, 8);
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
                            Ice.MarshalException e = new Ice.MarshalException();
                            e.initCause(ex);
                            throw e;
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

    public String[]
    readStringSeq()
    {
        final int sz = readSize();
        startSeq(sz, 1);
        String[] v = new String[sz];
        for(int i = 0; i < sz; i++)
        {
            v[i] = readString();
            checkSeq();
            endElement();
        }
        endSeq(sz);
        return v;
    }

    public void
    writeProxy(Ice.ObjectPrx v)
    {
        _instance.proxyFactory().proxyToStream(v, this);
    }

    public Ice.ObjectPrx
    readProxy()
    {
        return _instance.proxyFactory().streamToProxy(this);
    }

    public void
    writeObject(Ice.Object v)
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
        }

        if(_writeEncapsStack.toBeMarshaledMap == null)  // Lazy initialization
        {
            _writeEncapsStack.toBeMarshaledMap = new java.util.IdentityHashMap<Ice.Object, Integer>();
            _writeEncapsStack.marshaledMap = new java.util.IdentityHashMap<Ice.Object, Integer>();
            _writeEncapsStack.typeIdMap = new java.util.TreeMap<String, Integer>();
        }
        if(v != null)
        {
            //
            // Look for this instance in the to-be-marshaled map.
            //
            Integer p = _writeEncapsStack.toBeMarshaledMap.get(v);
            if(p == null)
            {
                //
                // Didn't find it, try the marshaled map next.
                //
                Integer q = _writeEncapsStack.marshaledMap.get(v);
                if(q == null)
                {
                    //
                    // We haven't seen this instance previously,
                    // create a new index, and insert it into the
                    // to-be-marshaled map.
                    //
                    q = Integer.valueOf(++_writeEncapsStack.writeIndex);
                    _writeEncapsStack.toBeMarshaledMap.put(v, q);
                }
                p = q;
            }
            writeInt(-p.intValue());
        }
        else
        {
            writeInt(0); // Write null reference
        }
    }

    public void
    readObject(Patcher patcher)
    {
        Ice.Object v = null;

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
        }

        if(_readEncapsStack.patchMap == null) // Lazy initialization
        {
            _readEncapsStack.patchMap = new java.util.TreeMap<Integer, java.util.LinkedList<Patcher> >();
            _readEncapsStack.unmarshaledMap = new java.util.TreeMap<Integer, Ice.Object>();
            _readEncapsStack.typeIdMap = new java.util.TreeMap<Integer, String>();
        }

        int index = readInt();

        if(patcher != null)
        {
            if(index == 0)
            {
                patcher.patch(null);
                return;
            }

            if(index < 0)
            {
                Integer i = Integer.valueOf(-index);
                java.util.LinkedList<Patcher> patchlist = _readEncapsStack.patchMap.get(i);
                if(patchlist == null)
                {
                    //
                    // We have no outstanding instances to be patched for
                    // this index, so make a new entry in the patch map.
                    //
                    patchlist = new java.util.LinkedList<Patcher>();
                    _readEncapsStack.patchMap.put(i, patchlist);
                }
                //
                // Append a patcher for this instance and see if we can
                // patch the instance. (The instance may have been
                // unmarshaled previously.)
                //
                patchlist.add(patcher);
                patchReferences(null, i);
                return;
            }
        }
        if(index < 0)
        {
            throw new Ice.MarshalException("Invalid class instance index");
        }

        String mostDerivedId = readTypeId();
        String id = mostDerivedId;

        while(true)
        {
            //
            // If we slice all the way down to Ice::Object, we throw
            // because Ice::Object is abstract.
            //
            if(id.equals(Ice.ObjectImpl.ice_staticId()))
            {
                throw new Ice.NoObjectFactoryException("", mostDerivedId);
            }

            //
            // Try to find a factory registered for the specific type.
            //
            Ice.ObjectFactory userFactory = _instance.servantFactoryManager().find(id);
            if(userFactory != null)
            {
                v = userFactory.create(id);
            }

            //
            // If that fails, invoke the default factory if one has
            // been registered.
            //
            if(v == null)
            {
                userFactory = _instance.servantFactoryManager().find("");
                if(userFactory != null)
                {
                    v = userFactory.create(id);
                }
            }

            //
            // Last chance: check the table of static factories (i.e.,
            // automatically generated factories for concrete
            // classes).
            //
            if(v == null)
            {
                userFactory = loadObjectFactory(id);
                if(userFactory != null)
                {
                    v = userFactory.create(id);
                }
            }

            if(v == null)
            {
                if(_sliceObjects)
                {
                    //
                    // Performance sensitive, so we use lazy
                    // initialization for tracing.
                    //
                    if(_traceSlicing == -1)
                    {
                        _traceSlicing = _instance.traceLevels().slicing;
                        _slicingCat = _instance.traceLevels().slicingCat;
                    }
                    if(_traceSlicing > 0)
                    {
                        TraceUtil.traceSlicing("class", id, _slicingCat, _instance.initializationData().logger);
                    }
                    skipSlice(); // Slice off this derived part -- we don't understand it.
                    id = readTypeId(); // Read next id for next iteration.
                    continue;
                }
                else
                {
                    Ice.NoObjectFactoryException ex = new Ice.NoObjectFactoryException();
                    ex.type = id;
                    throw ex;
                }
            }

            Integer i = Integer.valueOf(index);
            _readEncapsStack.unmarshaledMap.put(i, v);

            //
            // Record each object instance so that readPendingObjects
            // can invoke ice_postUnmarshal after all objects have
            // been unmarshaled.
            //
            if(_objectList == null)
            {
                _objectList = new java.util.ArrayList<Ice.Object>();
            }
            _objectList.add(v);

            v.__read(this, false);
            patchReferences(i, null);
            return;
        }
    }

    public void
    writeUserException(Ice.UserException v)
    {
        writeBool(v.__usesClasses());
        v.__write(this);
        if(v.__usesClasses())
        {
            writePendingObjects();
        }
    }

    public void
    throwException()
        throws Ice.UserException
    {
        boolean usesClasses = readBool();

        String id = readString();
        final String origId = id;

        for(;;)
        {
            //
            // Look for a factory for this ID.
            //
            UserExceptionFactory factory = getUserExceptionFactory(id);

            if(factory != null)
            {
                //
                // Got factory -- get the factory to instantiate the
                // exception, initialize the exception members, and
                // throw the exception.
                //
                try
                {
                    factory.createAndThrow();
                }
                catch(Ice.UserException ex)
                {
                    ex.__read(this, false);
                    if(usesClasses)
                    {
                        readPendingObjects();
                    }
                    throw ex;
                }
            }
            else
            {
                //
                // Performance sensitive, so we use lazy
                // initialization for tracing.
                //
                if(_traceSlicing == -1)
                {
                    _traceSlicing = _instance.traceLevels().slicing;
                    _slicingCat = _instance.traceLevels().slicingCat;
                }
                if(_traceSlicing > 0)
                {
                    TraceUtil.traceSlicing("exception", id, _slicingCat, _instance.initializationData().logger);
                }

                skipSlice(); // Slice off what we don't understand.

                try
                {
                    id = readString(); // Read type id for next slice.
                }
                catch(Ice.UnmarshalOutOfBoundsException ex)
                {
                    //
                    // When readString raises this exception it means we've seen the last slice,
                    // so we set the reason member to a more helpful message.
                    //
                    Ice.UnmarshalOutOfBoundsException e = new Ice.UnmarshalOutOfBoundsException();
                    e.reason = "unknown exception type `" + origId + "'";
                    e.initCause(ex);
                    throw e;
                }
            }
        }

        //
        // The only way out of the loop above is to find an exception
        // for which the receiver has a factory. If this does not
        // happen, sender and receiver disagree about the Slice
        // definitions they use. In that case, the receiver will
        // eventually fail to read another type ID and throw a
        // MarshalException.
        //
    }

    public void
    writePendingObjects()
    {
        if(_writeEncapsStack != null && _writeEncapsStack.toBeMarshaledMap != null)
        {
            while(_writeEncapsStack.toBeMarshaledMap.size() > 0)
            {
                java.util.IdentityHashMap<Ice.Object, Integer> savedMap =
                    new java.util.IdentityHashMap<Ice.Object, Integer>(_writeEncapsStack.toBeMarshaledMap);
                writeSize(savedMap.size());
                for(java.util.Map.Entry<Ice.Object, Integer> p : savedMap.entrySet())
                {
                    //
                    // Add an instance from the old to-be-marshaled
                    // map to the marshaled map and then ask the
                    // instance to marshal itself. Any new class
                    // instances that are triggered by the classes
                    // marshaled are added to toBeMarshaledMap.
                    //
                    _writeEncapsStack.marshaledMap.put(p.getKey(), p.getValue());
                    writeInstance(p.getKey(), p.getValue());
                }
            
                //
                // We have marshaled all the instances for this pass,
                // substract what we have marshaled from the
                // toBeMarshaledMap.
                //
                for(Ice.Object p : savedMap.keySet())
                {
                    _writeEncapsStack.toBeMarshaledMap.remove(p);
                }
            }
        }
        writeSize(0); // Zero marker indicates end of sequence of sequences of instances.
    }

    public void
    readPendingObjects()
    {
        int num;
        do
        {
            num = readSize();
            for(int k = num; k > 0; --k)
            {
                readObject(null);
            }
        }
        while(num > 0);

        if(_readEncapsStack != null && _readEncapsStack.patchMap != null && _readEncapsStack.patchMap.size() != 0)
        {
            //
            // If any entries remain in the patch map, the sender has sent an index for an object, but failed
            // to supply the object.
            //
            throw new Ice.MarshalException("Index for class received, but no instance");
        }

        //
        // Iterate over unmarshaledMap and invoke ice_postUnmarshal on
        // each object. We must do this after all objects in this
        // encapsulation have been unmarshaled in order to ensure that
        // any object data members have been properly patched.
        //
        if(_objectList != null)
        {
            for(Ice.Object obj : _objectList)
            {
                try
                {
                    obj.ice_postUnmarshal();
                }
                catch(java.lang.Exception ex)
                {
                    java.io.StringWriter sw = new java.io.StringWriter();
                    java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                    IceUtilInternal.OutputBase out = new IceUtilInternal.OutputBase(pw);
                    out.setUseTab(false);
                    out.print("exception raised by ice_postUnmarshal:\n");
                    ex.printStackTrace(pw);
                    pw.flush();
                    _instance.initializationData().logger.warning(sw.toString());
                }
            }
        }
    }

    public void
    sliceObjects(boolean b)
    {
        _sliceObjects = b;
    }

    void
    writeInstance(Ice.Object v, Integer index)
    {
        writeInt(index.intValue());
        try
        {
            v.ice_preMarshal();
        }
        catch(java.lang.Exception ex)
        {
            java.io.StringWriter sw = new java.io.StringWriter();
            java.io.PrintWriter pw = new java.io.PrintWriter(sw);
            IceUtilInternal.OutputBase out = new IceUtilInternal.OutputBase(pw);
            out.setUseTab(false);
            out.print("exception raised by ice_preMarshal:\n");
            ex.printStackTrace(pw);
            pw.flush();
            _instance.initializationData().logger.warning(sw.toString());
        }
        v.__write(this);
    }

    void
    patchReferences(Integer instanceIndex, Integer patchIndex)
    {
        //
        // Called whenever we have unmarshaled a new instance or an
        // index.  The instanceIndex is the index of the instance just
        // unmarshaled and patchIndex is the index just
        // unmarshaled. (Exactly one of the two parameters must be
        // null.) Patch any pointers in the patch map with the new
        // address.
        //
        assert((instanceIndex != null && patchIndex == null) || (instanceIndex == null && patchIndex != null));

        java.util.LinkedList<Patcher> patchlist;
        Ice.Object v;
        if(instanceIndex != null)
        {
            //
            // We have just unmarshaled an instance -- check if
            // something needs patching for that instance.
            //
            patchlist = _readEncapsStack.patchMap.get(instanceIndex);
            if(patchlist == null)
            {
                return; // We don't have anything to patch for the instance just unmarshaled
            }
            v = _readEncapsStack.unmarshaledMap.get(instanceIndex);
            patchIndex = instanceIndex;
        }
        else
        {
            //
            // We have just unmarshaled an index -- check if we have
            // unmarshaled the instance for that index yet.
            //
            v = _readEncapsStack.unmarshaledMap.get(patchIndex);
            if(v == null)
            {
               return; // We haven't unmarshaled the instance for this index yet
            }
            patchlist = _readEncapsStack.patchMap.get(patchIndex);
        }
        assert(patchlist != null && patchlist.size() > 0);
        assert(v != null);

        //
        // Patch all references that refer to the instance.
        //
        for(Patcher p : patchlist)
        {
            try
            {
                p.patch(v);
            }
            catch(ClassCastException ex)
            {
                Ice.NoObjectFactoryException nof = new Ice.NoObjectFactoryException();
                nof.type = p.type();
                nof.initCause(ex);
                throw nof;
            }
        }

        //
        // Clear out the patch map for that index -- there is nothing
        // left to patch for that index for the time being.
        //
        _readEncapsStack.patchMap.remove(patchIndex);
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
            Ice.CompressionException e = new Ice.CompressionException();
            e.reason = "bzip2 compression failure";
            e.initCause(ex);
            throw e;
        }

        //
        // Don't bother if the compressed data is larger than the
        // uncompressed data.
        //
        if(compressedLen >= uncompressedLen)
        {
            return null;
        }

        BasicStream cstream = new BasicStream(_instance);
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
            _buf.b.get(compressed);
        }

        BasicStream ucStream = new BasicStream(_instance);
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
            Ice.CompressionException e = new Ice.CompressionException();
            e.reason = "bzip2 uncompression failure";
            e.initCause(ex);
            throw e;
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

    private static final class DynamicObjectFactory implements Ice.ObjectFactory
    {
        DynamicObjectFactory(Class<?> c)
        {
            _class = c;
        }

        public Ice.Object
        create(String type)
        {
            try
            {
                return (Ice.Object)_class.newInstance();
            }
            catch(java.lang.Exception ex)
            {
                Ice.SyscallException e = new Ice.SyscallException();
                e.initCause(ex);
                throw e;
            }
        }

        public void
        destroy()
        {
        }

        private Class<?> _class;
    }

    private Ice.ObjectFactory
    loadObjectFactory(String id)
    {
        Ice.ObjectFactory factory = null;

        try
        {
            Class<?> c = findClass(id);
            if(c != null)
            {
                Ice.ObjectFactory dynamicFactory = new DynamicObjectFactory(c);
                //
                // We will try to install the dynamic factory, but
                // another thread may install a factory first.
                //
                while(factory == null)
                {
                    try
                    {
                        _instance.servantFactoryManager().add(dynamicFactory, id);
                        factory = dynamicFactory;
                    }
                    catch(Ice.AlreadyRegisteredException ex)
                    {
                        //
                        // Another thread already installed the
                        // factory, so try to obtain it. It's possible
                        // (but unlikely) that the factory will have
                        // already been removed, in which case the
                        // return value will be null and the while
                        // loop will attempt to install the dynamic
                        // factory again.
                        //
                        factory = _instance.servantFactoryManager().find(id);
                    }
                }
            }
        }
        catch(LinkageError ex)
        {
            Ice.NoObjectFactoryException e = new Ice.NoObjectFactoryException();
            e.type = id;
            e.initCause(ex);
            throw e;
        }

        return factory;
    }

    private static final class DynamicUserExceptionFactory
        implements UserExceptionFactory
    {
        DynamicUserExceptionFactory(Class<?> c)
        {
            _class = c;
        }

        public void
        createAndThrow()
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
                Ice.SyscallException e = new Ice.SyscallException();
                e.initCause(ex);
                throw e;
            }
        }

        public void
        destroy()
        {
        }

        private Class<?> _class;
    }

    private UserExceptionFactory
    getUserExceptionFactory(String id)
    {
        UserExceptionFactory factory = null;

        synchronized(_factoryMutex)
        {
            factory = _exceptionFactories.get(id);
        }

        if(factory == null)
        {
            try
            {
                Class<?> c = findClass(id);
                if(c != null)
                {
                    factory = new DynamicUserExceptionFactory(c);
                }
            }
            catch(LinkageError ex)
            {
                Ice.MarshalException e = new Ice.MarshalException();
                e.initCause(ex);
                throw e;
            }

            if(factory != null)
            {
                synchronized(_factoryMutex)
                {
                    _exceptionFactories.put(id, factory);
                }
            }
        }

        return factory;
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
        String className = typeToClass(id);
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

        return c;
    }

    private Class<?>
    getConcreteClass(String className)
        throws LinkageError
    {
        Class<?> c = Util.findClass(className);

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
            throw new Ice.MarshalException();
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

    private static final class ReadEncaps
    {
        int start;
        int sz;

        // byte encodingMajor; // Currently unused
        // byte encodingMinor; // Currently unused

        java.util.TreeMap<Integer, java.util.LinkedList<Patcher> > patchMap;
        java.util.TreeMap<Integer, Ice.Object> unmarshaledMap;
        int typeIdIndex;
        java.util.TreeMap<Integer, String> typeIdMap;
        ReadEncaps next;

        void
        reset()
        {
            if(patchMap != null)
            {
                patchMap.clear();
                unmarshaledMap.clear();
                typeIdIndex = 0;
                typeIdMap.clear();
            }
        }
    }

    private static final class WriteEncaps
    {
        int start;

        int writeIndex;
        java.util.IdentityHashMap<Ice.Object, Integer> toBeMarshaledMap;
        java.util.IdentityHashMap<Ice.Object, Integer> marshaledMap;
        int typeIdIndex;
        java.util.TreeMap<String, Integer> typeIdMap;
        WriteEncaps next;

        void
        reset()
        {
            if(toBeMarshaledMap != null)
            {
                writeIndex = 0;
                toBeMarshaledMap.clear();
                marshaledMap.clear();
                typeIdIndex = 0;
                typeIdMap.clear();
            }
        }
    }

    private ReadEncaps _readEncapsStack;
    private WriteEncaps _writeEncapsStack;
    private ReadEncaps _readEncapsCache;
    private WriteEncaps _writeEncapsCache;

    private int _readSlice;
    private int _writeSlice;

    private int _traceSlicing;
    private String _slicingCat;

    private boolean _sliceObjects;

    private int _messageSizeMax;
    private boolean _unlimited;

    private static final class SeqData
    {
        public SeqData(int numElements, int minSize)
        {
            this.numElements = numElements;
            this.minSize = minSize;
        }

        public int numElements;
        public int minSize;
        public SeqData previous;
    }
    SeqData _seqDataStack;

    private java.util.ArrayList<Ice.Object> _objectList;

    private static java.util.HashMap<String, UserExceptionFactory> _exceptionFactories =
        new java.util.HashMap<String, UserExceptionFactory>();
    private static java.lang.Object _factoryMutex = new java.lang.Object(); // Protects _exceptionFactories.

    public static boolean
    compressible()
    {
        return _bzInputStreamCtor != null && _bzOutputStreamCtor != null;
    }

    private static java.lang.reflect.Constructor<?> _bzInputStreamCtor;
    private static java.lang.reflect.Constructor<?> _bzOutputStreamCtor;
    static
    {
        try
        {
            Class<?> cls;
            Class<?>[] types = new Class<?>[1];
            cls = Util.findClass("org.apache.tools.bzip2.CBZip2InputStream");
            if(cls != null)
            {
                types[0] = java.io.InputStream.class;
                _bzInputStreamCtor = cls.getDeclaredConstructor(types);
            }
            cls = Util.findClass("org.apache.tools.bzip2.CBZip2OutputStream");
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
}
