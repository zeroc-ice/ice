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

        Encaps enc = new Encaps();
        enc.encoding = 0;
        enc.start = 0;
        _encapsStack.add(enc);
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

        java.util.LinkedList tmpStack = other._encapsStack;
        other._encapsStack = _encapsStack;
        _encapsStack = tmpStack;
    }

    private static final int MAX = 1024 * 1024; // TODO: Configurable

    public void
    resize(int total)
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
        _buf.limit(total);
        _limit = total;
    }

    /* TODO - Remove?
    public void
    reserve(int total)
    {
        if (total > MAX)
        {
            throw new Ice.MemoryLimitException();
        }
        _buf.reserve(total);
    }
    */

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
        Encaps enc = new Encaps();
        enc.encoding = 0;
        enc.start = _buf.position();
        _encapsStack.add(enc);
    }

    public void
    endWriteEncaps()
    {
        Encaps enc = (Encaps)_encapsStack.removeLast();
        final int sz = _buf.position() - enc.start;
        _buf.putInt(enc.start - 4, sz);
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
        Encaps enc = new Encaps();
        enc.encoding = (byte)encoding;
        enc.start = _buf.position();
        _encapsStack.add(enc);
    }

    public void
    endReadEncaps()
    {
        Encaps enc = (Encaps)_encapsStack.removeLast();
        int sz = _buf.getInt(enc.start - 4);
        try
        {
            _buf.position(enc.start + sz);
        }
        catch (IllegalArgumentException ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public void
    checkReadEncaps()
    {
        Encaps enc = (Encaps)_encapsStack.getLast();
        int sz = _buf.getInt(enc.start - 4);
        if (sz != _buf.position() - enc.start)
        {
            throw new Ice.EncapsulationException();
        }
    }

    public int
    getReadEncapsSize()
    {
        Encaps enc = (Encaps)_encapsStack.getLast();
        return _buf.getInt(enc.start - 4);
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
        Encaps enc = (Encaps)_encapsStack.getLast();
        Integer pos = null;
        if (enc.stringsWritten != null) // Lazy creation
        {
            pos = (Integer)enc.stringsWritten.get(v);
        }
        if (pos != null)
        {
            writeInt(pos.intValue());
        }
        else
        {
            final int len = v.length();
            writeInt(len);
            if (len > 0)
            {
                if (enc.stringsWritten == null)
                {
                    enc.stringsWritten = new java.util.HashMap();
                }
                int num = enc.stringsWritten.size();
                enc.stringsWritten.put(v, new Integer(-(num + 1)));
                final char[] arr = v.toCharArray();
                expand(len);
                for (int i = 0; i < len; i++)
                {
                    _buf.put((byte)arr[i]);
                }
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

        if (len < 0)
        {
            Encaps enc = (Encaps)_encapsStack.getLast();
            if (enc.stringsRead == null || // Lazy creation
                -(len + 1) >= enc.stringsRead.size())
            {
                throw new Ice.IllegalIndirectionException();
            }
            return (String)enc.stringsRead.get(-(len + 1));
        }
        else
        {
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
                    Encaps enc = (Encaps)_encapsStack.getLast();
                    if (enc.stringsRead == null)
                    {
                        enc.stringsRead = new java.util.ArrayList(10);
                    }
                    enc.stringsRead.add(v);
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
    writeWString(String v)
    {
        Encaps enc = (Encaps)_encapsStack.getLast();
        Integer pos = null;
        if (enc.wstringsWritten != null) // Lazy creation
        {
            pos = (Integer)enc.wstringsWritten.get(v);
        }
        if (pos != null)
        {
            writeInt(pos.intValue());
        }
        else
        {
            final int len = v.length();
            writeInt(len);
            if (len > 0)
            {
                if (enc.wstringsWritten == null)
                {
                    enc.wstringsWritten = new java.util.HashMap();
                }
                int num = enc.wstringsWritten.size();
                enc.wstringsWritten.put(v, new Integer(-(num + 1)));
                final int sz = len * 2;
                expand(sz);
                java.nio.CharBuffer charBuf = _buf.asCharBuffer();
                charBuf.put(v);
                _buf.position(_buf.position() + sz);
            }
        }
    }

    public void
    writeWStringSeq(String[] v)
    {
        writeInt(v.length);
        for (int i = 0; i < v.length; i++)
        {
            writeWString(v[i]);
        }
    }

    public String
    readWString()
    {
        final int len = readInt();

        if (len < 0)
        {
            Encaps enc = (Encaps)_encapsStack.getLast();
            if (enc.wstringsRead == null || // Lazy creation
                -(len + 1) >= enc.wstringsRead.size())
            {
                throw new Ice.IllegalIndirectionException();
            }
            return (String)enc.wstringsRead.get(-(len + 1));
        }
        else
        {
            if (len == 0)
            {
                return "";
            }
            else
            {
                try
                {
                    char[] arr = new char[len];
                    java.nio.CharBuffer charBuf = _buf.asCharBuffer();
                    charBuf.get(arr);
                    String v = new String(arr);
                    Encaps enc = (Encaps)_encapsStack.getLast();
                    if (enc.wstringsRead == null)
                    {
                        enc.wstringsRead = new java.util.ArrayList(10);
                    }
                    enc.wstringsRead.add(v);
                    _buf.position(_buf.position() + len * 2);
                    return v;
                }
                catch (java.nio.BufferUnderflowException ex)
                {
                    throw new Ice.UnmarshalOutOfBoundsException();
                }
            }
        }
    }

    public String[]
    readWStringSeq()
    {
        final int sz = readInt();
        // Don't use resize(sz) or reserve(sz) here, as it cannot be
        // checked whether sz is a reasonable value
        String[] v = new String[sz];
        for (int i = 0; i < sz; i++)
        {
            v[i] = readWString();
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
        Encaps enc = (Encaps)_encapsStack.getLast();
        Integer pos = null;
        if (enc.objectsWritten != null) // Lazy creation
        {
            pos = (Integer)enc.objectsWritten.get(v);
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
                if (enc.objectsWritten == null)
                {
                    enc.objectsWritten = new java.util.HashMap();
                }
                int num = enc.objectsWritten.size();
                enc.objectsWritten.put(v, new Integer(num));
                writeString(v.__getClassIds()[0]);
                v.__write(this);
            }
            else
            {
                writeString("");
            }
        }
    }

    //
    // TODO: Can we eliminate the need for a Holder?
    //
    public boolean
    readObject(String signatureType, Ice.ObjectHolder v)
    {
        final int pos = readInt();

        if (pos >= 0)
        {
            Encaps enc = (Encaps)_encapsStack.getLast();
            if (enc.objectsRead == null || // Lazy creation
                pos >= enc.objectsRead.size())
            {
                throw new Ice.IllegalIndirectionException();
            }
            v.value = (Ice.Object)enc.objectsRead.get(pos);
            return true;
        }
        else
        {
            String id = readString();

            if (id.length() == 0)
            {
                v.value = null;
                return true;
            }
            else if (id.equals("::Ice::Object"))
            {
                v.value = new Ice.Object();
                readObject(v.value);
                return true;
            }
            else
            {
                Encaps enc = (Encaps)_encapsStack.getLast();
                if (enc.objectsRead == null)
                {
                    enc.objectsRead = new java.util.ArrayList(10);
                }

                Ice.ObjectFactory factory =
                    _instance.servantFactoryManager().find(id);
                if (factory != null)
                {
                    v.value = factory.create(id);
                    if (v.value != null)
                    {
                        readObject(v.value);
                        return true;
                    }
                }

                if (id.equals(signatureType))
                {
                    return false;
                }

                throw new Ice.NoObjectFactoryException();
            }
        }
    }

    public void
    readObject(Ice.Object v)
    {
        assert(v != null);
        Encaps enc = (Encaps)_encapsStack.getLast();
        if (enc.objectsRead == null) // Lazy creation
        {
            enc.objectsRead = new java.util.ArrayList(10);
        }
        enc.objectsRead.add(v);
        v.__read(this);
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

    private static class Encaps
    {
        int start;
        byte encoding;
        java.util.ArrayList stringsRead;
        java.util.HashMap stringsWritten;
        java.util.ArrayList wstringsRead;
        java.util.HashMap wstringsWritten;
        java.util.ArrayList objectsRead;
        java.util.HashMap objectsWritten;
    }

    private java.util.LinkedList _encapsStack = new java.util.LinkedList();
}
