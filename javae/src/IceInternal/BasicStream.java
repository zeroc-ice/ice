// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class BasicStream
{
    public
    BasicStream(IceInternal.Instance instance)
    {
        initialize(instance, false);
    }

    public
    BasicStream(IceInternal.Instance instance, boolean unlimited)
    {
        initialize(instance, unlimited);
    }

    private void
    initialize(IceInternal.Instance instance, boolean unlimited)
    {
        _instance = instance;
        _unlimited = unlimited;
        allocate(1500);
        _capacity = _buf.capacity();
        _limit = 0;
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_buf.limit() == _capacity);
	}

        _readEncapsStack = null;
        _writeEncapsStack = null;

	_messageSizeMax = _instance.messageSizeMax(); // Cached for efficiency.

	_seqDataStack = null;

	_shrinkCounter = 0;
    }

    //
    // This function allows this object to be reused, rather than reallocated.
    //
    public void
    reset()
    {
	if(_limit > 0 && _limit * 2 < _capacity)
	{
	    //
	    // If twice the size of the stream is less than the capacity
	    // for more than two consecutive times, we shrink the buffer.
	    // This is to avoid holding on too much memory if it's not 
	    // needed anymore.
	    //
	    if(++_shrinkCounter > 2)
	    {
 	        allocate(_limit);
 	        _capacity = _buf.capacity();
 		_shrinkCounter = 0;
	    }
	}
	else
	{
	    _shrinkCounter = 0;
	}
        _limit = 0;
        _buf.limit(_capacity);
        _buf.position(0);

        _readEncapsStack = null;
    }

    public IceInternal.Instance
    instance()
    {
        return _instance;
    }

    public void
    swap(BasicStream other)
    {
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_instance == other._instance);
	}

        ByteBuffer tmpBuf = other._buf;
        other._buf = _buf;
        _buf = tmpBuf;

        int tmpCapacity = other._capacity;
        other._capacity = _capacity;
        _capacity = tmpCapacity;

        int tmpLimit = other._limit;
        other._limit = _limit;
        _limit = tmpLimit;

        ReadEncaps tmpRead = other._readEncapsStack;
        other._readEncapsStack = _readEncapsStack;
        _readEncapsStack = tmpRead;

        WriteEncaps tmpWrite = other._writeEncapsStack;
        other._writeEncapsStack = _writeEncapsStack;
        _writeEncapsStack = tmpWrite;

	int tmpReadSlice = other._readSlice;
	other._readSlice = _readSlice;
	_readSlice = tmpReadSlice;

	int tmpWriteSlice = other._writeSlice;
	other._writeSlice = _writeSlice;
	_writeSlice = tmpWriteSlice;

	SeqData tmpSeqDataStack = other._seqDataStack;
	other._seqDataStack = _seqDataStack;
	_seqDataStack = tmpSeqDataStack;

	int tmpShrinkCounter = other._shrinkCounter;
	other._shrinkCounter = _shrinkCounter;
	_shrinkCounter = tmpShrinkCounter;

        boolean tmpUnlimited = other._unlimited;
        other._unlimited = _unlimited;
        _unlimited = tmpUnlimited;
    }

    public void
    resize(int total, boolean reading)
    {
        if(!_unlimited && total > _messageSizeMax)
        {
            throw new Ice.MemoryLimitException();
        }
        if(total > _capacity)
        {
            final int cap2 = _capacity << 1;
            int newCapacity = cap2 > total ? cap2 : total;
            _buf.limit(_limit);
            reallocate(newCapacity);
            _capacity = _buf.capacity();
        }
        //
        // If this stream is used for reading, then we want to set the
        // buffer's limit to the new total size. If this buffer is
        // used for writing, then we must set the buffer's limit to
        // the buffer's capacity.
        //
        if(reading)
        {
            _buf.limit(total);
        }
        else
        {
            _buf.limit(_capacity);
        }
        _buf.position(total);
        _limit = total;
    }

    public ByteBuffer
    prepareRead()
    {
        return _buf;
    }

    public ByteBuffer
    prepareWrite()
    {
        _buf.limit(_limit);
        _buf.position(0);
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

	int bytesLeft = _buf.remaining();
	if(_seqDataStack.previous == null) // Outermost sequence
	{
	    //
	    // The sequence must fit within the message.
	    //
	    if(numElements * minSize > bytesLeft) 
	    {
		Ice.Util.throwUnmarshalOutOfBoundsException();
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
	checkSeq(_buf.remaining());
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
	    Ice.Util.throwUnmarshalOutOfBoundsException();
	}
    }

    public void
    checkFixedSeq(int numElements, int elemSize)
    {
	int bytesLeft = _buf.remaining();
	if(_seqDataStack == null) // Outermost sequence
	{
	    //
	    // The sequence must fit within the message.
	    //
	    if(numElements * elemSize > bytesLeft) 
	    {
		Ice.Util.throwUnmarshalOutOfBoundsException();
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
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(oldSeqData != null);
	}
	_seqDataStack = oldSeqData.previous;
    }

    public void
    endElement()
    {
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_seqDataStack != null);
	}
	--_seqDataStack.numElements;
    }

    public void
    startWriteEncaps()
    {
	WriteEncaps curr = new WriteEncaps();
	curr.next = _writeEncapsStack;
	_writeEncapsStack = curr;

        _writeEncapsStack.start = _buf.position();
        writeInt(0); // Placeholder for the encapsulation length.
        writeByte(Protocol.encodingMajor);
        writeByte(Protocol.encodingMinor);
    }

    public void
    endWriteEncaps()
    {
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_writeEncapsStack != null);
	}
        int start = _writeEncapsStack.start;
        int sz = _buf.position() - start; // Size includes size and version.
	_buf.putInt(start, sz);

	_writeEncapsStack = _writeEncapsStack.next;
    }

    public void
    startReadEncaps()
    {
	ReadEncaps curr = new ReadEncaps();
	curr.next = _readEncapsStack;
	_readEncapsStack = curr;

        _readEncapsStack.start = _buf.position();
	
	//
	// I don't use readSize() and writeSize() for encapsulations,
	// because when creating an encapsulation, I must know in
	// advance how many bytes the size information will require in
	// the data stream. If I use an Int, it is always 4 bytes. For
	// readSize()/writeSize(), it could be 1 or 5 bytes.
	//
        int sz = readInt();
	if(sz < 0)
	{
	    Ice.Util.throwNegativeSizeException();
	}

	if(sz - 4 > _buf.limit())
	{
	    Ice.Util.throwUnmarshalOutOfBoundsException();
	}
	_readEncapsStack.sz = sz;

        byte eMajor = readByte();
        byte eMinor = readByte();
	if(eMajor != Protocol.encodingMajor || eMinor > Protocol.encodingMinor)
	{
            Ice.Util.throwUnsupportedEncodingException(eMajor, eMinor);
	}
        _readEncapsStack.encodingMajor = eMajor;
        _readEncapsStack.encodingMinor = eMinor;
    }

    public void
    endReadEncaps()
    {
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_readEncapsStack != null);
	}
        int start = _readEncapsStack.start;
        int sz = _readEncapsStack.sz;
        try
        {
            _buf.position(start + sz);
        }
        catch(IllegalArgumentException ex)
        {
            Ice.Util.throwUnmarshalOutOfBoundsException();
        }

	_readEncapsStack = _readEncapsStack.next;
    }

    public int
    getReadEncapsSize()
    {
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_readEncapsStack != null);
	}
	return _readEncapsStack.sz - 6;
    }

    public void
    skipEncaps()
    {
        int sz = readInt();
	if(sz < 0)
	{
	    Ice.Util.throwNegativeSizeException();
	}
        try
        {
            _buf.position(_buf.position() + sz - 4);
        }
        catch(IllegalArgumentException ex)
        {
            Ice.Util.throwUnmarshalOutOfBoundsException();
        }
    }

    public void
    startWriteSlice()
    {
        writeInt(0); // Placeholder for the slice length.
        _writeSlice = _buf.position();
    }

    public void endWriteSlice()
    {
        final int sz = _buf.position() - _writeSlice + 4;
        _buf.putInt(_writeSlice - 4, sz);
    }

    public void startReadSlice()
    {
        int sz = readInt();
	if(sz < 0)
	{
	    Ice.Util.throwNegativeSizeException();
	}
        _readSlice = _buf.position();
    }

    public void endReadSlice()
    {
    }

    public void skipSlice()
    {
        int sz = readInt();
	if(sz < 0)
	{
	    Ice.Util.throwNegativeSizeException();
	}
        try
        {
            _buf.position(_buf.position() + sz - 4);
        }
        catch(IllegalArgumentException ex)
        {
            Ice.Util.throwUnmarshalOutOfBoundsException();
        }
    }

    public void
    writeSize(int v)
    {
	if(v > 254)
	{
	    expand(5);
	    _buf.put((byte)-1);
	    _buf.putInt(v);
	}
	else
	{
	    expand(1);
	    _buf.put((byte)v);
	}
    }

    public int
    readSize()
    {
        try
        {
	    byte b = _buf.get();
	    if(b == -1)
	    {
		int v = _buf.getInt();
		if(v < 0)
		{
		    Ice.Util.throwNegativeSizeException();
		}
		return v;
	    }
	    else
	    {
		return (int)(b < 0 ? b + 256 : b);
	    }
        }
        catch(ByteBuffer.UnderflowException ex)
        {
            Ice.Util.throwUnmarshalOutOfBoundsException();
            return 0; // Keep the compiler happy.
        }
    }

    public void
    writeBlob(byte[] v)
    {
        expand(v.length);
        _buf.put(v);
    }

    public void
    writeBlob(byte[] v, int off, int len)
    {
        expand(len);
        _buf.put(v, off, len);
    }

    public byte[]
    readBlob(int sz)
    {
        byte[] v = new byte[sz];
        try
        {
            _buf.get(v);
            return v;
        }
        catch(ByteBuffer.UnderflowException ex)
        {
            Ice.Util.throwUnmarshalOutOfBoundsException();
            return new byte[0]; // Keep the compiler happy.
        }
    }

    public void
    writeByte(byte v)
    {
        expand(1);
        _buf.put(v);
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
	    _buf.put(v);
	}
    }

    public byte
    readByte()
    {
        try
        {
            return _buf.get();
        }
        catch(ByteBuffer.UnderflowException ex)
        {
            Ice.Util.throwUnmarshalOutOfBoundsException();
            return 0; // Keep the compiler happy.
        }
    }

    public byte[]
    readByteSeq()
    {
	    
        try
        {
            final int sz = readSize();
	    checkFixedSeq(sz, 1);
            byte[] v = new byte[sz];
            _buf.get(v);
            return v;
        }
        catch(ByteBuffer.UnderflowException ex)
        {
            Ice.Util.throwUnmarshalOutOfBoundsException();
            return new byte[0]; // Keep the compiler happy.
        }
    }

    public void
    writeBool(boolean v)
    {
        expand(1);
        _buf.put(v ? (byte)1 : (byte)0);
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
	    for(int i = 0; i < v.length; i++)
	    {
		_buf.put(v[i] ? (byte)1 : (byte)0);
	    }
	}
    }

    public boolean
    readBool()
    {
        try
        {
            return _buf.get() == 1;
        }
        catch(ByteBuffer.UnderflowException ex)
        {
            Ice.Util.throwUnmarshalOutOfBoundsException();
            return false; // Keep the compiler happy.
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
                v[i] = _buf.get() == 1;
            }
            return v;
        }
        catch(ByteBuffer.UnderflowException ex)
        {
            Ice.Util.throwUnmarshalOutOfBoundsException();
            return new boolean[0]; // Keep the compiler happy.
        }
    }

    public void
    writeShort(short v)
    {
        expand(2);
        _buf.putShort(v);
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
	    _buf.putShortSeq(v);
	}
    }

    public short
    readShort()
    {
        try
        {
            return _buf.getShort();
        }
        catch(ByteBuffer.UnderflowException ex)
        {
            Ice.Util.throwUnmarshalOutOfBoundsException();
            return 0; // Keep the compiler happy.
        }
    }

    public short[]
    readShortSeq()
    {
        try
        {
            final int sz = readSize();
	    checkFixedSeq(sz, 2);
            short[] v = new short[sz];
            _buf.getShortSeq(v);
            return v;
        }
        catch(ByteBuffer.UnderflowException ex)
        {
            Ice.Util.throwUnmarshalOutOfBoundsException();
            return new short[0];
        }
    }

    public void
    writeInt(int v)
    {
        expand(4);
        _buf.putInt(v);
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
	    _buf.putIntSeq(v);
	}
    }

    public int
    readInt()
    {
        try
        {
            return _buf.getInt();
        }
        catch(ByteBuffer.UnderflowException ex)
        {
            Ice.Util.throwUnmarshalOutOfBoundsException();
            return 0; // Keep the compiler happy.
        }
    }

    public int[]
    readIntSeq()
    {
        try
        {
            final int sz = readSize();
	    checkFixedSeq(sz, 4);
            int[] v = new int[sz];
            _buf.getIntSeq(v);
            return v;
        }
        catch(ByteBuffer.UnderflowException ex)
        {
            Ice.Util.throwUnmarshalOutOfBoundsException();
            return new int[0]; // Keep the compiler happy.
        }
    }

    public void
    writeLong(long v)
    {
        expand(8);
        _buf.putLong(v);
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
	    _buf.putLongSeq(v);
	}
    }

    public long
    readLong()
    {
        try
        {
            return _buf.getLong();
        }
        catch(ByteBuffer.UnderflowException ex)
        {
            Ice.Util.throwUnmarshalOutOfBoundsException();
            return 0; // Keep the compiler happy.
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
            _buf.getLongSeq(v);
            return v;
        }
        catch(ByteBuffer.UnderflowException ex)
        {
            Ice.Util.throwUnmarshalOutOfBoundsException();
            return new long[0]; // Keep the compiler happy.
        }
    }

    public void
    writeFloat(float v)
    {
        expand(4);
        _buf.putFloat(v);
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
	    _buf.putFloatSeq(v);
	}
    }

    public float
    readFloat()
    {
        try
        {
            return _buf.getFloat();
        }
        catch(ByteBuffer.UnderflowException ex)
        {
            Ice.Util.throwUnmarshalOutOfBoundsException();
            return 0.0f; // Kee the compiler happy.
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
            _buf.getFloatSeq(v);
            return v;
        }
        catch(ByteBuffer.UnderflowException ex)
        {
            Ice.Util.throwUnmarshalOutOfBoundsException();
            return new float[0]; // Keep the compiler happy.
        }
    }

    public void
    writeDouble(double v)
    {
        expand(8);
        _buf.putDouble(v);
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
	    _buf.putDoubleSeq(v);
	}
    }

    public double
    readDouble()
    {
        try
        {
            return _buf.getDouble();
        }
        catch(ByteBuffer.UnderflowException ex)
        {
            Ice.Util.throwUnmarshalOutOfBoundsException();
            return 0.0; // Keep the compiler happy.
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
            _buf.getDoubleSeq(v);
            return v;
        }
        catch(ByteBuffer.UnderflowException ex)
        {
            Ice.Util.throwUnmarshalOutOfBoundsException();
            return new double[0]; // Keep the compiler happy.
        }
    }

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
		byte[] arr = v.getBytes();
		writeSize(arr.length);
		expand(arr.length);
		_buf.put(arr);
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
	    for(int i = 0; i < v.length; i++)
	    {
		writeString(v[i]);
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
                // excessive garbage
                //
                if(_stringBytes == null || len > _stringBytes.length)
                {
                    _stringBytes = new byte[len];
                    _stringChars = new char[len];
                }
                _buf.get(_stringBytes, 0, len);

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
			// behavior of this constructor is undefined. It
			// would be better to explicitly decode using
			// java.nio.charset.CharsetDecoder and to throw
			// MarshalException if the string won't decode.
			//
                        return new String(_stringBytes, 0, len);
                    }
                    else
                    {
                        _stringChars[i] = (char)_stringBytes[i];
                    }
                }
                return new String(_stringChars, 0, len);
            }
            catch(ByteBuffer.UnderflowException ex)
            {
                Ice.Util.throwUnmarshalOutOfBoundsException();
                return "";
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
    writeUserException(Ice.UserException v)
    {
        writeBool(false); // uses classes
	v.__write(this);
    }

    public void
    throwException()
        throws Ice.UserException
    {
        boolean usesClasses = readBool();

        String id = readString();

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
		    throw ex;
		}
	    }
	    else
	    {
	        skipSlice(); // Slice off what we don't understand.
		id = readString(); // Read type id for next slice.
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

    public int
    pos()
    {
        return _buf.position();
    }

    public void
    pos(int n)
    {
        _buf.position(n);
    }

    public int
    size()
    {
        return _limit;
    }

    public boolean
    isEmpty()
    {
        return _limit == 0;
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
	    if(IceUtil.Debug.ASSERT)
	    {
		IceUtil.Debug.Assert(_data.length - _pos >= b.length);
	    }
	    System.arraycopy(b, 0, _data, _pos, b.length);
	    _pos += b.length;
	}

	public void
	write(byte[] b, int off, int len)
	    throws java.io.IOException
	{
	    if(IceUtil.Debug.ASSERT)
	    {
		IceUtil.Debug.Assert(_data.length - _pos >= len);
	    }
	    System.arraycopy(b, off, _data, _pos, len);
	    _pos += len;
	}

	public void
	write(int b)
	    throws java.io.IOException
	{
	    if(IceUtil.Debug.ASSERT)
	    {
		IceUtil.Debug.Assert(_data.length - _pos >= 1);
	    }
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

    private void
    expand(int size)
    {
        if(_buf.position() == _limit)
        {
            int oldLimit = _limit;
            _limit += size;
            if(!_unlimited && _limit > _messageSizeMax)
            {
                throw new Ice.MemoryLimitException();
            }
            if(_limit > _capacity)
            {
                final int cap2 = _capacity << 1;
                int newCapacity = cap2 > _limit ? cap2 : _limit;
                _buf.limit(oldLimit);
                int pos = _buf.position();
                _buf.position(0);
                reallocate(newCapacity);
		if(IceUtil.Debug.ASSERT)
		{
		    IceUtil.Debug.Assert(_buf != null);
		}
                _capacity = _buf.capacity();
                _buf.limit(_capacity);
                _buf.position(pos);
            }
        }
    }

    private static final class DynamicUserExceptionFactory
        implements UserExceptionFactory
    {
        DynamicUserExceptionFactory(Class c)
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

        private Class _class;
    }

    private UserExceptionFactory
    getUserExceptionFactory(String id)
    {
        UserExceptionFactory factory = null;

	synchronized(_factoryMutex)
	{
	    factory = (UserExceptionFactory)_exceptionFactories.get(id);
	}

        if(factory == null)
        {
	    /*
	      TODO- LinkageError is not available in CLDC. The try/catch
	      block has no meaning unless a replacement for checking if
	      a class is instantiable or not.
	      
            try
            {
                Class c = findClass(id);
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
	    */

	    Class c = findClass(id);
	    if(c != null)
	    {
		factory = new DynamicUserExceptionFactory(c);
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

    private Class
    findClass(String id)
    {
        Class c = null;

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

    private Class
    getConcreteClass(String className)
    {
        try
        {
            Class c = Class.forName(className);
            //
            // Ensure the class is instantiable.
	    //
	    // TODO- Need to check for abstract classes. CLDC 
	    // currently doesn't provide sufficient APIs for checking
	    //
	    if(!c.isInterface())
	    {
		return c;
	    }
        }
        catch(ClassNotFoundException ex)
        {
            // Ignore
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
        boolean found = IceUtil.Arrays.search(keywordList, name) >= 0;
        return found ? "_" + name : name;
    }

    private String
    typeToClass(String id)
    {
        if(!id.startsWith("::"))
        {
            throw new Ice.MarshalException();
        }

        StringBuffer buf = new StringBuffer(id.length());

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

    private void
    allocate(int size)
    {
	ByteBuffer buf = null;
	try
	{
	    buf = ByteBuffer.allocate(size);
	}
	catch(OutOfMemoryError ex)
	{
	    Ice.MarshalException e = new Ice.MarshalException();
	    e.reason = "OutOfMemoryError occurred while allocating a ByteBuffer";
	    e.initCause(ex);
	    throw e;
	}
	buf.order(ByteBuffer.LITTLE_ENDIAN);
	_buf = buf;
    }

    private void
    reallocate(int size)
    {
        //
	// Limit buffer size to MessageSizeMax
	//
        if(!_unlimited)
        {
	    size = size > _messageSizeMax ? _messageSizeMax : size;
        }

	ByteBuffer old = _buf;
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(old != null);
	}

	allocate(size);
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_buf != null);
	}

	old.position(0);
	_buf.put(old);
    }

    private IceInternal.Instance _instance;
    private ByteBuffer _buf;
    private int _capacity; // Cache capacity to avoid excessive method calls.
    private int _limit; // Cache limit to avoid excessive method calls.
    private byte[] _stringBytes; // Reusable array for reading strings.
    private char[] _stringChars; // Reusable array for reading strings.

    private static final class ReadEncaps
    {
        int start;
	int sz;

        byte encodingMajor;
        byte encodingMinor;

        ReadEncaps next;
    }

    private static final class WriteEncaps
    {
        int start;

        WriteEncaps next;
    }

    private ReadEncaps _readEncapsStack;
    private WriteEncaps _writeEncapsStack;

    private int _readSlice;
    private int _writeSlice;

    private int _messageSizeMax;
    private boolean _unlimited;
    private int _shrinkCounter;

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

    private static java.util.Hashtable _exceptionFactories = new java.util.Hashtable();
    private static java.lang.Object _factoryMutex = new java.lang.Object(); // Protects _exceptionFactories.
}
