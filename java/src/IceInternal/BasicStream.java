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
        _capacity = _buf.capacity();
        _limit = 0;
        assert(_buf.limit() == _capacity);

        _currentReadEncaps = null;
        _currentWriteEncaps = null;
    }

    //
    // Copy constructor only for special cases (e.g., tracing)
    //
    public
    BasicStream(BasicStream stream)
    {
        _instance = stream._instance;
        _bufferManager = _instance.bufferManager();
        //
        // NOTE: Using duplicate() is more efficient because the
        // new buffer shares the same underlying storage as the
        // old one. However, this means any modifications by
        // either buffer will be seen by both buffers.
        //
        _buf = stream._buf.duplicate();
        _buf.order(java.nio.ByteOrder.LITTLE_ENDIAN);
        _capacity = _buf.capacity();
        _limit = stream._limit;
        _buf.position(0);

        _currentReadEncaps = null;
        _currentWriteEncaps = null;
    }

    protected void
    finalize()
        throws Throwable
    {
        _bufferManager.reclaim(_buf);

        super.finalize();
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

        java.util.LinkedList tmpStack = other._readEncapsStack;
        other._readEncapsStack = _readEncapsStack;
        _readEncapsStack = tmpStack;
        tmpStack = other._writeEncapsStack;
        other._writeEncapsStack = _writeEncapsStack;
        _writeEncapsStack = tmpStack;
    }

    private static final int MAX = 1024 * 1024; // TODO: Configurable

    public void
    resize(int total, boolean reading)
    {
        if (total > MAX)
        {
            throw new Ice.MemoryLimitException();
        }
        if (total > _capacity)
        {
            final int cap2 = _capacity << 1;
            int newCapacity = cap2 > total ? cap2 : total;
            _buf = _bufferManager.reallocate(_buf, newCapacity);
            _capacity = _buf.capacity();
        }
        //
        // If this stream is used for reading, then we want to set
        // the buffer's limit to the new total size. If this buffer
        // is used for writing, then we must set the buffer's limit
        // to the buffer's capacity.
        //
        if (reading)
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

    java.nio.ByteBuffer
    prepareRead()
    {
        return _buf;
    }

    java.nio.ByteBuffer
    prepareWrite()
    {
        _buf.limit(_limit);
        _buf.position(0);
        return _buf;
    }

    public void
    startWriteEncaps()
    {
        writeInt(0); // Encoding
        writeInt(0); // Placeholder for the encapsulation length
        _currentWriteEncaps = new WriteEncaps();
        _currentWriteEncaps.encoding = 0;
        _currentWriteEncaps.start = _buf.position();
        _writeEncapsStack.add(_currentWriteEncaps);
    }

    public void
    endWriteEncaps()
    {
        assert(_currentWriteEncaps != null);
        final int start = _currentWriteEncaps.start;
        _writeEncapsStack.removeLast();
        if (_writeEncapsStack.isEmpty())
        {
            _currentWriteEncaps = null;
        }
        else
        {
            _currentWriteEncaps = (WriteEncaps)_writeEncapsStack.getLast();
        }
        final int sz = _buf.position() - start;
        _buf.putInt(start - 4, sz);
    }

    public void
    startReadEncaps()
    {
        int encoding = readInt();
        if (encoding != 0)
        {
            throw new Ice.UnsupportedEncodingException();
        }
        int sz = readInt();
        _currentReadEncaps = new ReadEncaps();
        _currentReadEncaps.encoding = (byte)encoding;
        _currentReadEncaps.start = _buf.position();
        _readEncapsStack.add(_currentReadEncaps);
    }

    public void
    endReadEncaps()
    {
        assert(_currentReadEncaps != null);
        final int start = _currentReadEncaps.start;
        _readEncapsStack.removeLast();
        if (_readEncapsStack.isEmpty())
        {
            _currentReadEncaps = null;
        }
        else
        {
            _currentReadEncaps = (ReadEncaps)_readEncapsStack.getLast();
        }
        final int sz = _buf.getInt(start - 4);
        try
        {
            _buf.position(start + sz);
        }
        catch (IllegalArgumentException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public void
    checkReadEncaps()
    {
        assert(_currentReadEncaps != null);
        final int sz = _buf.getInt(_currentReadEncaps.start - 4);
        if (sz != _buf.position() - _currentReadEncaps.start)
        {
            throw new Ice.EncapsulationException();
        }
    }

    public int
    getReadEncapsSize()
    {
        assert(_currentReadEncaps != null);
        return _buf.getInt(_currentReadEncaps.start - 4);
    }

    public void
    skipEncaps()
    {
        int encoding = readInt();
        if (encoding != 0)
        {
            throw new Ice.UnsupportedEncodingException();
        }
        int sz = readInt();
        try
        {
            _buf.position(_buf.position() + sz);
        }
        catch (IllegalArgumentException ex)
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
        catch (java.nio.BufferUnderflowException ex)
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
        expand(4 + v.length);
        _buf.putInt(v.length);
        _buf.put(v);
    }

    public byte
    readByte()
    {
        try
        {
            return _buf.get();
        }
        catch (java.nio.BufferUnderflowException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public byte[]
    readByteSeq()
    {
        try
        {
            final int sz = _buf.getInt();
            byte[] v = new byte[sz];
            _buf.get(v);
            return v;
        }
        catch (java.nio.BufferUnderflowException ex)
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
        expand(4 + v.length);
        _buf.putInt(v.length);
        for (int i = 0; i < v.length; i++)
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
        catch (java.nio.BufferUnderflowException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public boolean[]
    readBoolSeq()
    {
        try
        {
            final int sz = _buf.getInt();
            boolean[] v = new boolean[sz];
            for (int i = 0; i < sz; i++)
            {
                v[i] = _buf.get() == 1;
            }
            return v;
        }
        catch (java.nio.BufferUnderflowException ex)
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
        expand(4 + v.length * 2);
        _buf.putInt(v.length);

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
        catch (java.nio.BufferUnderflowException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public short[]
    readShortSeq()
    {
        try
        {
            final int sz = _buf.getInt();
            short[] v = new short[sz];
            java.nio.ShortBuffer shortBuf = _buf.asShortBuffer();
            shortBuf.get(v);
            _buf.position(_buf.position() + sz * 2);
            return v;
        }
        catch (java.nio.BufferUnderflowException ex)
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
        expand(4 + v.length * 4);
        _buf.putInt(v.length);

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
        catch (java.nio.BufferUnderflowException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public int[]
    readIntSeq()
    {
        try
        {
            final int sz = _buf.getInt();
            int[] v = new int[sz];
            java.nio.IntBuffer intBuf = _buf.asIntBuffer();
            intBuf.get(v);
            _buf.position(_buf.position() + sz * 4);
            return v;
        }
        catch (java.nio.BufferUnderflowException ex)
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
        expand(4 + v.length * 8);
        _buf.putInt(v.length);

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
        catch (java.nio.BufferUnderflowException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public long[]
    readLongSeq()
    {
        try
        {
            final int sz = _buf.getInt();
            long[] v = new long[sz];
            java.nio.LongBuffer longBuf = _buf.asLongBuffer();
            longBuf.get(v);
            _buf.position(_buf.position() + sz * 8);
            return v;
        }
        catch (java.nio.BufferUnderflowException ex)
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
        expand(4 + v.length * 4);
        _buf.putInt(v.length);

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
        catch (java.nio.BufferUnderflowException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public float[]
    readFloatSeq()
    {
        try
        {
            final int sz = _buf.getInt();
            float[] v = new float[sz];
            java.nio.FloatBuffer floatBuf = _buf.asFloatBuffer();
            floatBuf.get(v);
            _buf.position(_buf.position() + sz * 4);
            return v;
        }
        catch (java.nio.BufferUnderflowException ex)
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
        expand(4 + v.length * 8);
        _buf.putInt(v.length);

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
        catch (java.nio.BufferUnderflowException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public double[]
    readDoubleSeq()
    {
        try
        {
            final int sz = _buf.getInt();
            double[] v = new double[sz];
            java.nio.DoubleBuffer doubleBuf = _buf.asDoubleBuffer();
            doubleBuf.get(v);
            _buf.position(_buf.position() + sz * 8);
            return v;
        }
        catch (java.nio.BufferUnderflowException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public void
    writeString(String v)
    {
        final int len = v.length();
        writeInt(len);
        if (len > 0)
        {
            final char[] arr = v.toCharArray();
            expand(len);
            for (int i = 0; i < len; i++)
            {
                _buf.put((byte)arr[i]);
            }
        }
    }

    public void
    writeStringSeq(String[] v)
    {
        writeInt(v.length);
        for (int i = 0; i < v.length; i++)
        {
            writeString(v[i]);
        }
    }

    public String
    readString()
    {
        final int len = readInt();

        if (len == 0)
        {
            return "";
        }
        else
        {
            try
            {
                /* TODO: Performance review
                char[] arr = new char[len];
                for (int i = 0; i < len; i++)
                {
                    arr[i] = (char)_buf.get();
                }
                String v = new String(arr);
                */
                byte[] arr = new byte[len];
                _buf.get(arr);
                String v = new String(arr, "ISO-8859-1");
                return v;
            }
            catch (java.io.UnsupportedEncodingException ex)
            {
                assert(false);
                return "";
            }
            catch (java.nio.BufferUnderflowException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException();
            }
        }
    }

    public String[]
    readStringSeq()
    {
        final int sz = readInt();
        // Don't use v.resize(sz) or v.reserve(sz) here, as it cannot be
        // checked whether sz is a reasonable value
        String[] v = new String[sz];
        for (int i = 0; i < sz; i++)
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
        if (_currentWriteEncaps == null) // Lazy initialization
        {
            _currentWriteEncaps = new WriteEncaps();
            _writeEncapsStack.add(_currentWriteEncaps);
        }

        Integer pos = null;
        if (_currentWriteEncaps.objectsWritten != null) // Lazy creation
        {
            pos = (Integer)_currentWriteEncaps.objectsWritten.get(v);
        }
        if (pos != null)
        {
            writeInt(pos.intValue());
        }
        else
        {
            writeInt(-1);

            if (v != null)
            {
                if (_currentWriteEncaps.objectsWritten == null)
                {
                    _currentWriteEncaps.objectsWritten =
                        new java.util.IdentityHashMap();
                }
                int num = _currentWriteEncaps.objectsWritten.size();
                _currentWriteEncaps.objectsWritten.put(v, new Integer(num));
                writeString(v.__getClassIds()[0]);
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

        if (_currentReadEncaps == null) // Lazy initialization
        {
            _currentReadEncaps = new ReadEncaps();
            _readEncapsStack.add(_currentReadEncaps);
        }

        final int pos = readInt();

        if (pos >= 0)
        {
            if (_currentReadEncaps.objectsRead == null || // Lazy creation
                pos >= _currentReadEncaps.objectsRead.size())
            {
                throw new Ice.IllegalIndirectionException();
            }
            v = (Ice.Object)_currentReadEncaps.objectsRead.get(pos);
        }
        else
        {
            String id = readString();

            if (id.length() == 0)
            {
                return null;
            }
            else if (id.equals("::Ice::Object"))
            {
                v = new Ice.Object();
            }
            else
            {
                Ice.ObjectFactory userFactory = _instance.servantFactoryManager().find(id);
                if (userFactory != null)
                {
                    v = userFactory.create(id);
                }

                if (v == null && id.equals(signatureType))
                {
                    assert(factory != null);
                    v = factory.create(id);
                    assert(v != null);
                }

                if (v == null)
                {
                    throw new Ice.NoObjectFactoryException();
                }
            }
            if (_currentReadEncaps.objectsRead == null) // Lazy creation
            {
                _currentReadEncaps.objectsRead = new java.util.ArrayList(10);
            }
            _currentReadEncaps.objectsRead.add(v);
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
        Ice.UserExceptionFactory factory =
            _instance.userExceptionFactoryManager().find(id);

        if (factory != null)
        {
            try
            {
                factory.createAndThrow(id);
            }
            catch (Ice.UserException ex)
            {
                String[] arr = ex.__getExceptionIds();
                for (int i = 0; !arr[i].equals("::Ice::UserException"); i++)
                {
                    if (java.util.Arrays.binarySearch(ids, arr[i]) >= 0)
                    {
                        ex.__read(this);
                        throw ex;
                    }
                }

                throw new Ice.UnknownUserException();
            }
        }

        int pos = java.util.Arrays.binarySearch(ids, id);
        if (pos >= 0)
        {
            return pos;
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
        if (_buf.position() == _limit)
        {
            _limit += size;
            if (_limit > _capacity)
            {
                final int cap2 = _capacity << 1;
                int newCapacity = cap2 > _limit ? cap2 : _limit;
                _buf = _bufferManager.reallocate(_buf, newCapacity);
                _capacity = _buf.capacity();
            }
        }
    }

    private IceInternal.Instance _instance;
    private BufferManager _bufferManager;
    private java.nio.ByteBuffer _buf;
    private int _capacity; // Cache capacity to avoid excessive method calls
    private int _limit; // Cache limit to avoid excessive method calls

    private static class ReadEncaps
    {
        int start;
        byte encoding;
        java.util.ArrayList objectsRead;
    }

    private static class WriteEncaps
    {
        int start;
        byte encoding;
        java.util.IdentityHashMap objectsWritten;
    }

    private java.util.LinkedList _readEncapsStack = new java.util.LinkedList();
    private java.util.LinkedList _writeEncapsStack = new java.util.LinkedList();
    private ReadEncaps _currentReadEncaps;
    private WriteEncaps _currentWriteEncaps;
}
