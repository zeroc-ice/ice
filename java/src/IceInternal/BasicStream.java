// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

public class BasicStream
{
    public
    BasicStream(IceInternal.Instance instance)
    {
        _instance = instance;
        _bufferManager = instance.bufferManager();
        _buf = _bufferManager.allocate(1500);
        assert(_buf != null);
        _capacity = _buf.capacity();
        _limit = 0;
        assert(_buf.limit() == _capacity);

        _readEncapsStack = null;
        _writeEncapsStack = null;
        _readEncapsCache = null;
        _writeEncapsCache = null;
    }

/*
 * Do NOT use a finalizer for BasicStream - this causes a
 * severe performance penalty!
 *
    protected void
    finalize()
        throws Throwable
    {
        if(_buf != null)
        {
            _bufferManager.reclaim(_buf);
        }

        super.finalize();
    }
*/

    public void
    reset()
    {
        _limit = 0;
        _buf.limit(_capacity);
        _buf.position(0);

        if(_readEncapsStack != null)
        {
            assert(_readEncapsStack.next == null);
            _readEncapsStack.next = _readEncapsCache;
            _readEncapsCache = _readEncapsStack;
            _readEncapsStack = null;
            if(_readEncapsCache.objectsRead != null)
            {
                _readEncapsCache.objectsRead.clear();
            }
        }
    }

    //
    // Must be called in order to reclaim the buffer
    //
    public void
    destroy()
    {
        _bufferManager.reclaim(_buf);
        _buf = null;
    }

    public IceInternal.Instance
    instance()
    {
        return _instance;
    }

    public void
    swap(BasicStream other)
    {
        assert(_instance == other._instance);

        java.nio.ByteBuffer tmpBuf = other._buf;
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
        tmpRead = other._readEncapsCache;
        other._readEncapsCache = _readEncapsCache;
        _readEncapsCache = tmpRead;
        WriteEncaps tmpWrite = other._writeEncapsStack;
        other._writeEncapsStack = _writeEncapsStack;
        _writeEncapsStack = tmpWrite;
        tmpWrite = other._writeEncapsCache;
        other._writeEncapsCache = _writeEncapsCache;
        _writeEncapsCache = tmpWrite;
    }

    private static final int MAX = 1024 * 1024; // TODO: Configurable

    public void
    resize(int total, boolean reading)
    {
        if(total > MAX)
        {
            throw new Ice.MemoryLimitException();
        }
        if(total > _capacity)
        {
            final int cap2 = _capacity << 1;
            int newCapacity = cap2 > total ? cap2 : total;
            _buf.limit(_limit);
            _buf.position(0);
            _buf = _bufferManager.reallocate(_buf, newCapacity);
            assert(_buf != null);
            _capacity = _buf.capacity();
        }
        //
        // If this stream is used for reading, then we want to set
        // the buffer's limit to the new total size. If this buffer
        // is used for writing, then we must set the buffer's limit
        // to the buffer's capacity.
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

    public java.nio.ByteBuffer
    prepareRead()
    {
        return _buf;
    }

    public java.nio.ByteBuffer
    prepareWrite()
    {
        _buf.limit(_limit);
        _buf.position(0);
        return _buf;
    }

    public void
    startWriteEncaps()
    {
        writeByte((byte)0); // Encoding
        writeInt(0); // Placeholder for the encapsulation length
        WriteEncaps curr = _writeEncapsCache;
        if(curr != null)
        {
            _writeEncapsCache = _writeEncapsCache.next;
        }
        else
        {
            curr = new WriteEncaps();
        }
        curr.encoding = 0;
        curr.start = _buf.position();
        curr.next = _writeEncapsStack;
        _writeEncapsStack = curr;
    }

    public void
    endWriteEncaps()
    {
        final WriteEncaps curr = _writeEncapsStack;
        assert(curr != null);
        final int start = curr.start;
        _writeEncapsStack = curr.next;
        curr.next = _writeEncapsCache;
        _writeEncapsCache = curr;
        if(_writeEncapsCache.objectsWritten != null)
        {
            _writeEncapsCache.objectsWritten.clear();
        }
        final int sz = _buf.position() - start;
        _buf.putInt(start - 4, sz);
    }

    public void
    startReadEncaps()
    {
        byte encoding = readByte();
        if(encoding != 0)
        {
            throw new Ice.UnsupportedEncodingException();
        }
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
	    throw new Ice.NegativeSizeException();
	}
        ReadEncaps curr = _readEncapsCache;
        if(curr != null)
        {
            assert(curr.objectsRead == null || curr.objectsRead.size() == 0);
            _readEncapsCache = _readEncapsCache.next;
        }
        else
        {
            curr = new ReadEncaps();
        }
        curr.encoding = encoding;
        curr.start = _buf.position();
        curr.next = _readEncapsStack;
        _readEncapsStack = curr;
    }

    public void
    endReadEncaps()
    {
        final ReadEncaps curr = _readEncapsStack;
        assert(curr != null);
        final int start = curr.start;
        _readEncapsStack = curr.next;
        curr.next = _readEncapsCache;
        _readEncapsCache = curr;
        if(_readEncapsCache.objectsRead != null)
        {
            _readEncapsCache.objectsRead.clear();
        }
        final int sz = _buf.getInt(start - 4);
	if(sz < 0)
	{
	    throw new Ice.NegativeSizeException();
	}
        try
        {
            _buf.position(start + sz);
        }
        catch(IllegalArgumentException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public void
    checkReadEncaps()
    {
        assert(_readEncapsStack != null);
        final int sz = _buf.getInt(_readEncapsStack.start - 4);
	if(sz < 0)
	{
	    throw new Ice.NegativeSizeException();
	}
        if(sz != _buf.position() - _readEncapsStack.start)
        {
            throw new Ice.EncapsulationException();
        }
    }

    public int
    getReadEncapsSize()
    {
        assert(_readEncapsStack != null);
        int sz = _buf.getInt(_readEncapsStack.start - 4);
	if(sz < 0)
	{
	    throw new Ice.NegativeSizeException();
	}
	return sz;
    }

    public void
    skipEncaps()
    {
        byte encoding = readByte();
        if(encoding != 0)
        {
            throw new Ice.UnsupportedEncodingException();
        }
        int sz = readInt();
	if(sz < 0)
	{
	    throw new Ice.NegativeSizeException();
	}
        try
        {
            _buf.position(_buf.position() + sz);
        }
        catch(IllegalArgumentException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public void
    writeSize(int v)
    {
	if(v > 127)
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
	    if(b < 0)
	    {
		int v = _buf.getInt();
		if(v < 0)
		{
		    throw new Ice.NegativeSizeException();
		}
		return v;
	    }
	    else
	    {
		return (int)b;
	    }
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public void
    writeBlob(byte[] v)
    {
        expand(v.length);
        _buf.put(v);
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
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
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
        writeSize(v.length);
        expand(v.length);
        _buf.put(v);
    }

    public byte
    readByte()
    {
        try
        {
            return _buf.get();
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public byte[]
    readByteSeq()
    {
        try
        {
            final int sz = readSize();
            byte[] v = new byte[sz];
            _buf.get(v);
            return v;
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
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
        writeSize(v.length);
        expand(v.length);
        for(int i = 0; i < v.length; i++)
        {
            _buf.put(v[i] ? (byte)1 : (byte)0);
        }
    }

    public boolean
    readBool()
    {
        try
        {
            return _buf.get() == 1;
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
            boolean[] v = new boolean[sz];
            for(int i = 0; i < sz; i++)
            {
                v[i] = _buf.get() == 1;
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
        _buf.putShort(v);
    }

    public void
    writeShortSeq(short[] v)
    {
        writeSize(v.length);
        expand(v.length * 2);
        java.nio.ShortBuffer shortBuf = _buf.asShortBuffer();
        shortBuf.put(v);
        _buf.position(_buf.position() + v.length * 2);
    }

    public short
    readShort()
    {
        try
        {
            return _buf.getShort();
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public short[]
    readShortSeq()
    {
        try
        {
            final int sz = readSize();
            short[] v = new short[sz];
            java.nio.ShortBuffer shortBuf = _buf.asShortBuffer();
            shortBuf.get(v);
            _buf.position(_buf.position() + sz * 2);
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
        _buf.putInt(v);
    }

    public void
    writeIntSeq(int[] v)
    {
	writeSize(v.length);
        expand(v.length * 4);
        java.nio.IntBuffer intBuf = _buf.asIntBuffer();
        intBuf.put(v);
        _buf.position(_buf.position() + v.length * 4);
    }

    public int
    readInt()
    {
        try
        {
            return _buf.getInt();
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public int[]
    readIntSeq()
    {
        try
        {
            final int sz = readSize();
            int[] v = new int[sz];
            java.nio.IntBuffer intBuf = _buf.asIntBuffer();
            intBuf.get(v);
            _buf.position(_buf.position() + sz * 4);
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
        _buf.putLong(v);
    }

    public void
    writeLongSeq(long[] v)
    {
	writeSize(v.length);
        expand(v.length * 8);
        java.nio.LongBuffer longBuf = _buf.asLongBuffer();
        longBuf.put(v);
        _buf.position(_buf.position() + v.length * 8);
    }

    public long
    readLong()
    {
        try
        {
            return _buf.getLong();
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
            long[] v = new long[sz];
            java.nio.LongBuffer longBuf = _buf.asLongBuffer();
            longBuf.get(v);
            _buf.position(_buf.position() + sz * 8);
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
        _buf.putFloat(v);
    }

    public void
    writeFloatSeq(float[] v)
    {
	writeSize(v.length);
        expand(v.length * 4);
        java.nio.FloatBuffer floatBuf = _buf.asFloatBuffer();
        floatBuf.put(v);
        _buf.position(_buf.position() + v.length * 4);
    }

    public float
    readFloat()
    {
        try
        {
            return _buf.getFloat();
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
            float[] v = new float[sz];
            java.nio.FloatBuffer floatBuf = _buf.asFloatBuffer();
            floatBuf.get(v);
            _buf.position(_buf.position() + sz * 4);
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
        _buf.putDouble(v);
    }

    public void
    writeDoubleSeq(double[] v)
    {
	writeSize(v.length);
        expand(v.length * 8);
        java.nio.DoubleBuffer doubleBuf = _buf.asDoubleBuffer();
        doubleBuf.put(v);
        _buf.position(_buf.position() + v.length * 8);
    }

    public double
    readDouble()
    {
        try
        {
            return _buf.getDouble();
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
            double[] v = new double[sz];
            java.nio.DoubleBuffer doubleBuf = _buf.asDoubleBuffer();
            doubleBuf.get(v);
            _buf.position(_buf.position() + sz * 8);
            return v;
        }
        catch(java.nio.BufferUnderflowException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
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
                try
                {
                    byte[] arr = v.getBytes("UTF8");
                    writeSize(arr.length);
                    expand(arr.length);
                    _buf.put(arr);
                }
                catch(java.io.UnsupportedEncodingException ex)
                {
                    assert(false);
                }
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
        writeSize(v.length);
        for(int i = 0; i < v.length; i++)
        {
            writeString(v[i]);
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
                        // Multi-byte character found - we must
                        // use conversion
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
        // Don't use v.resize(sz) or v.reserve(sz) here, as it cannot be
        // checked whether sz is a reasonable value
        String[] v = new String[sz];
        for(int i = 0; i < sz; i++)
        {
            v[i] = readString();
        }
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

        Integer pos = null;
        if(_writeEncapsStack.objectsWritten != null) // Lazy creation
        {
            pos = (Integer)_writeEncapsStack.objectsWritten.get(v);
        }
        if(pos != null)
        {
            writeInt(pos.intValue());
        }
        else
        {
            writeInt(-1);

            if(v != null)
            {
                if(_writeEncapsStack.objectsWritten == null)
                {
                    _writeEncapsStack.objectsWritten = new java.util.IdentityHashMap();
                }
                int num = _writeEncapsStack.objectsWritten.size();
                _writeEncapsStack.objectsWritten.put(v, new Integer(num));
                writeString(v.ice_id(null));
                v.__write(this);
            }
            else
            {
                writeString("");
            }
        }
    }

    public Ice.Object
    readObject(String signatureType, Ice.ObjectFactory factory)
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

        final int pos = readInt();

        if(pos >= 0)
        {
            if(_readEncapsStack.objectsRead == null || // Lazy creation
                pos >= _readEncapsStack.objectsRead.size())
            {
                throw new Ice.IllegalIndirectionException();
            }
            v = (Ice.Object)_readEncapsStack.objectsRead.get(pos);
        }
        else
        {
            String id = readString();

            if(id.length() == 0)
            {
                return null;
            }
            else if(id.equals("::Ice::Object"))
            {
                v = new Ice.Object();
            }
            else
            {
                Ice.ObjectFactory userFactory = _instance.servantFactoryManager().find(id);
                if(userFactory != null)
                {
                    v = userFactory.create(id);
                }

                if(v == null && id.equals(signatureType))
                {
                    assert(factory != null);
                    v = factory.create(id);
                    assert(v != null);
                }

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
                    throw new Ice.NoObjectFactoryException();
                }
            }
            if(_readEncapsStack.objectsRead == null) // Lazy creation
            {
                _readEncapsStack.objectsRead = new java.util.ArrayList(10);
            }
            _readEncapsStack.objectsRead.add(v);
            v.__read(this);
        }

        return v;
    }

    public void
    writeUserException(Ice.UserException v)
    {
        writeString(v.__getExceptionIds()[0]);
        v.__write(this);
    }

    public int
    throwException(String[] ids)
        throws Ice.UserException
    {
        String id = readString();
        Ice.UserExceptionFactory factory = _instance.userExceptionFactoryManager().find(id);

        if(factory == null)
        {
            int pos = java.util.Arrays.binarySearch(ids, id);
            if(pos >= 0)
            {
                return pos;
            }
        }

        if(factory == null)
        {
            factory = loadUserExceptionFactory(id);
        }

        if(factory != null)
        {
            try
            {
                factory.createAndThrow(id);
            }
            catch(Ice.UserException ex)
            {
                String[] arr = ex.__getExceptionIds();
                for(int i = 0; !arr[i].equals("::Ice::UserException"); i++)
                {
                    if(java.util.Arrays.binarySearch(ids, arr[i]) >= 0)
                    {
                        ex.__read(this);
                        throw ex;
                    }
                }

                throw new Ice.UnknownUserException();
            }
        }

        throw new Ice.NoUserExceptionFactoryException();
    }

    int
    pos()
    {
        return _buf.position();
    }

    void
    pos(int n)
    {
        _buf.position(n);
    }

    int
    size()
    {
        return _limit;
    }

    private void
    expand(int size)
    {
        if(_buf.position() == _limit)
        {
            int oldLimit = _limit;
            _limit += size;
            if(_limit > MAX)
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
                _buf = _bufferManager.reallocate(_buf, newCapacity);
                assert(_buf != null);
                _capacity = _buf.capacity();
                _buf.limit(_capacity);
                _buf.position(pos);
            }
        }
    }

    private static final class DynamicObjectFactory implements Ice.ObjectFactory
    {
        DynamicObjectFactory(Class c)
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
            catch(Exception ex)
            {
                Ice.SystemException e = new Ice.SystemException();
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

    private Ice.ObjectFactory
    loadObjectFactory(String id)
    {
        Ice.ObjectFactory factory = null;

        try
        {
            Class c = Class.forName(typeToClass(id));
            //
            // Ensure the class is instantiable. The constants are
            // defined in the JVM specification (0x200 = interface,
            // 0x400 = abstract).
            //
            int modifiers = c.getModifiers();
            if((modifiers & 0x200) == 0 && (modifiers & 0x400) == 0)
            {
                factory = new DynamicObjectFactory(c);
                _instance.servantFactoryManager().add(factory, id);
            }
        }
        catch(ClassNotFoundException ex)
        {
            // Ignore
        }
        catch(Exception ex)
        {
            Ice.NoObjectFactoryException e = new Ice.NoObjectFactoryException();
            e.initCause(ex);
            throw e;
        }

        return factory;
    }

    private static final class DynamicUserExceptionFactory implements Ice.UserExceptionFactory
    {
        DynamicUserExceptionFactory(Class c)
        {
            _class = c;
        }

        public void
        createAndThrow(String type)
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
            catch(Exception ex)
            {
                Ice.SystemException e = new Ice.SystemException();
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

    private Ice.UserExceptionFactory
    loadUserExceptionFactory(String id)
    {
        Ice.UserExceptionFactory factory = null;

        try
        {
            Class c = Class.forName(typeToClass(id));
            //
            // Ensure the class is instantiable. The constants are
            // defined in the JVM specification (0x200 = interface,
            // 0x400 = abstract).
            //
            int modifiers = c.getModifiers();
            assert((modifiers & 0x200) == 0 && (modifiers & 0x400) == 0);
            factory = new DynamicUserExceptionFactory(c);
            _instance.userExceptionFactoryManager().add(factory, id);
        }
        catch(ClassNotFoundException ex)
        {
            // Ignore
        }
        catch(Exception ex)
        {
            Ice.NoUserExceptionFactoryException e = new Ice.NoUserExceptionFactoryException();
            e.initCause(ex);
            throw e;
        }

        return factory;
    }

    private String
    typeToClass(String id)
    {
        assert(id.startsWith("::"));
        return id.substring(2).replaceAll("::", ".");
    }

    private IceInternal.Instance _instance;
    private BufferManager _bufferManager;
    private java.nio.ByteBuffer _buf;
    private int _capacity; // Cache capacity to avoid excessive method calls
    private int _limit; // Cache limit to avoid excessive method calls
    private byte[] _stringBytes; // Reusable array for reading strings
    private char[] _stringChars; // Reusable array for reading strings

    private static final class ReadEncaps
    {
        int start;
        byte encoding;
        java.util.ArrayList objectsRead;
        ReadEncaps next;
    }

    private static final class WriteEncaps
    {
        int start;
        byte encoding;
        java.util.IdentityHashMap objectsWritten;
        WriteEncaps next;
    }

    private ReadEncaps _readEncapsStack;
    private WriteEncaps _writeEncapsStack;
    private ReadEncaps _readEncapsCache;
    private WriteEncaps _writeEncapsCache;
}
