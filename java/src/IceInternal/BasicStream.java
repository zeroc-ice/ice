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

        Encaps enc = new Encaps();
        enc.start = 0;
        enc.encoding = 0;
        _encapsStack.add(enc);
    }

    protected void
    finalize()
        throws Throwable
    {
        //
        // No check for exactly one, because an error might have aborted
        // marshalling/unmarshalling
        //
        assert(_encapsStack.size() > 0);

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
        // TODO
    }

    private static final int MAX = 1024 * 1024; // TODO: Configurable

    public void
    resize(int total)
    {
        if (total > MAX)
        {
            throw new Ice.MemoryLimitException();
        }
        _buf.resize(total);
    }

    public void
    reserve(int total)
    {
        if (total > MAX)
        {
            throw new Ice.MemoryLimitException();
        }
        _buf.reserve(total);
    }

    public void
    startWriteEncaps()
    {
        writeInt(0); // Placeholder for the encapsulation length
        Encaps enc = new Encaps();
        enc.start = _buf.pos;
        enc.encoding = 0;
        _encapsStack.add(enc);
    }

    public void
    endWriteEncaps()
    {
        Encaps enc = (Encaps)_encapsStack.removeLast();
        int sz = _buf.pos - enc.start;
        int pos = enc.start - 4;
        _buf.data[pos++] = (byte)sz;
        _buf.data[pos++] = (byte)(sz >>> 8);
        _buf.data[pos++] = (byte)(sz >>> 16);
        _buf.data[pos++] = (byte)(sz >>> 24);
    }

    public void
    startReadEncaps()
    {
        int sz = readInt();
        Encaps enc = new Encaps();
        enc.start = _buf.pos;
        enc.encoding = readByte();
        if (enc.encoding != 0)
        {
            throw new Ice.UnsupportedEncodingException();
        }
        _encapsStack.add(enc);
    }

    public void
    endReadEncaps()
    {
        // TODO: Review
        Encaps enc = (Encaps)_encapsStack.removeLast();
        int save = _buf.pos;
        _buf.pos = enc.start - 4;
        int sz = readInt();
        _buf.pos = save;
        if (sz != _buf.pos - enc.start)
        {
            throw new Ice.EncapsulationException();
        }
    }

    public void
    skipEncaps()
    {
        int sz = readInt();
        _buf.pos += sz;
        if (_buf.pos > _buf.len)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    public void
    writeByte(byte v)
    {
        int pos = _buf.pos;
        _buf.pos++;
        if (_buf.pos > _buf.len)
        {
            resize(_buf.pos);
        }
        _buf.data[pos++] = v;
    }

    public void
    writeByteSeq(byte[] v)
    {
        int pos = _buf.pos;
        final int vlen = v.length;
        _buf.pos += 4 + vlen;
        if (_buf.pos > _buf.len)
        {
            resize(_buf.pos);
        }
        _buf.data[pos++] = (byte)vlen;
        _buf.data[pos++] = (byte)(vlen >>> 8);
        _buf.data[pos++] = (byte)(vlen >>> 16);
        _buf.data[pos++] = (byte)(vlen >>> 24);
        System.arraycopy(value, 0, _buf.data, pos, vlen);
        assert(pos == _buf.pos);
    }

    public byte
    readByte()
    {
        if (_buf.pos >= _buf.len)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        return _buf.data[_buf.pos++];
    }

    public byte[]
    readByteSeq()
    {
        final int sz = readInt();
        final int pos = _buf.pos;
        _buf.pos += sz;
        if (_buf.pos > _buf.len)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        byte[] v = new byte[sz];
        System.arraycopy(_buf.data, pos, v, 0, sz);
        return v;
    }

    public void
    writeBool(boolean v)
    {
        int pos = _buf.pos;
        _buf.pos++;
        if (_buf.pos > _buf.len)
        {
            resize(_buf.pos);
        }
        _buf.data[pos] = v ? (byte)1 : (byte)0;
    }

    public void
    writeBoolSeq(boolean[] v)
    {
        int pos = _buf.pos;
        final int vlen = v.length;
        _buf.pos += 4 + vlen;
        if (_buf.pos > _buf.len)
        {
            resize(_buf.pos);
        }
        _buf.data[pos++] = (byte)vlen;
        _buf.data[pos++] = (byte)(vlen >>> 8);
        _buf.data[pos++] = (byte)(vlen >>> 16);
        _buf.data[pos++] = (byte)(vlen >>> 24);
        for (int i = 0; i < vlen; i++)
        {
            _buf.data[pos++] = v[i] ? (byte)1 : (byte)0;
        }
        assert(pos == _buf.pos);
    }

    public boolean
    readBool()
    {
        if (_buf.pos >= _buf.len)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        return _buf.data[_buf.pos++] == 1;
    }

    public boolean[]
    readBoolSeq()
    {
        final int sz = readInt();
        int pos = _buf.pos;
        _buf.pos += sz;
        if (_buf.pos > _buf.len)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        boolean[] v = new boolean[sz];
        for (int i = 0; i < sz; i++)
        {
            v[i] = _buf.data[pos++] == 1;
        }
        return v;
    }

    public void
    writeShort(short v)
    {
        int pos = _buf.pos;
        _buf.pos += 2;
        if (_buf.pos > _buf.len)
        {
            resize(_buf.pos);
        }
        _buf.data[pos++] = (byte)v;
        _buf.data[pos]   = (byte)(v >>> 8);
    }

    public void
    writeShortSeq(short[] v)
    {
        int pos = _buf.pos;
        final int vlen = v.length;
        _buf.pos += 4 + (vlen * 2);
        if (_buf.pos > _buf.len)
        {
            resize(_buf.pos);
        }
        _buf.data[pos++] = (byte)vlen;
        _buf.data[pos++] = (byte)(vlen >>> 8);
        _buf.data[pos++] = (byte)(vlen >>> 16);
        _buf.data[pos++] = (byte)(vlen >>> 24);
        for (int i = 0; i < vlen; i++)
        {
            _buf.data[pos++] = (byte)v[i];
            _buf.data[pos++] = (byte)(v[i] >>> 8);
        }
        assert(pos == _buf.pos);
    }

    public short
    readShort()
    {
        int pos = _buf.pos;
        _buf.pos += 2;
        if (_buf.pos > _buf.len)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        return (short)((_buf.data[pos++] & 0xff) |
                       (_buf.data[pos] << 8));
    }

    public short[]
    readShortSeq()
    {
        int sz = readInt();
        int pos = _buf.pos;
        _buf.pos += sz * 2;
        if (_buf.pos > _buf.len)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        short[] v = new short[sz];
        for (int i = 0; i < sz; i++)
        {
            v[i] = (short)((_buf.data[pos++] & 0xff) |
                           (_buf.data[pos++] << 8));
        }
        return v;
    }

    public void
    writeInt(int v)
    {
        int pos = _buf.pos;
        _buf.pos += 4;
        if (_buf.pos > _buf.len)
        {
            resize(_buf.pos);
        }
        _buf.data[pos++] = (byte)v;
        _buf.data[pos++] = (byte)(v >>> 8);
        _buf.data[pos++] = (byte)(v >>> 16);
        _buf.data[pos]   = (byte)(v >>> 24);
    }

    public void
    writeIntSeq(int[] v)
    {
        int pos = _buf.pos;
        final int vlen = v.length;
        _buf.pos += 4 + (vlen * 4);
        if (_buf.pos > _buf.len)
        {
            resize(_buf.pos);
        }
        _buf.data[pos++] = (byte)vlen;
        _buf.data[pos++] = (byte)(vlen >>> 8);
        _buf.data[pos++] = (byte)(vlen >>> 16);
        _buf.data[pos++] = (byte)(vlen >>> 24);
        for (int i = 0; i < vlen; i++)
        {
            _buf.data[pos++] = (byte)v[i];
            _buf.data[pos++] = (byte)(v[i] >>> 8);
            _buf.data[pos++] = (byte)(v[i] >>> 16);
            _buf.data[pos++] = (byte)(v[i] >>> 24);
        }
        assert(pos == _buf.pos);
    }

    public int
    readInt()
    {
        int pos = _buf.pos;
        _buf.pos += 4;
        if (_buf.pos > _buf.len)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        return ((_buf.data[pos++] & 0xff) |
               ((_buf.data[pos++] << 8) & 0xff00) |
               ((_buf.data[pos++] << 16) & 0xff0000) |
               (_buf.data[pos]    << 24));
    }

    public int[]
    readIntSeq()
    {
        int sz = readInt();
        int pos = _buf.pos;
        _buf.pos += sz * 4;
        if (_buf.pos > _buf.len)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        int[] v = new int[sz];
        for (int i = 0; i < sz; i++)
        {
            v[i] = ((_buf.data[pos++] & 0xff) |
                   ((_buf.data[pos++] << 8) & 0xff00) |
                   ((_buf.data[pos++] << 16) & 0xff0000) |
                   (_buf.data[pos++]  << 24));
        }
        return v;
    }

    public void
    writeLong(long v)
    {
        int pos = _buf.pos;
        _buf.pos += 8;
        if (_buf.pos > _buf.len)
        {
            resize(_buf.pos);
        }
        _buf.data[pos++] = (byte)v;
        _buf.data[pos++] = (byte)(v >>> 8);
        _buf.data[pos++] = (byte)(v >>> 16);
        _buf.data[pos++] = (byte)(v >>> 24);
        _buf.data[pos++] = (byte)(v >>> 32);
        _buf.data[pos++] = (byte)(v >>> 40);
        _buf.data[pos++] = (byte)(v >>> 48);
        _buf.data[pos]   = (byte)(v >>> 56);
    }

    public void
    writeLongSeq(long[] v)
    {
        int pos = _buf.pos;
        final int vlen = v.length;
        _buf.pos += 4 + (vlen * 8);
        if (_buf.pos > _buf.len)
        {
            resize(_buf.pos);
        }
        _buf.data[pos++] = (byte)vlen;
        _buf.data[pos++] = (byte)(vlen >>> 8);
        _buf.data[pos++] = (byte)(vlen >>> 16);
        _buf.data[pos++] = (byte)(vlen >>> 24);
        for (int i = 0; i < vlen; i++)
        {
            _buf.data[pos++] = (byte)v[i];
            _buf.data[pos++] = (byte)(v[i] >>> 8);
            _buf.data[pos++] = (byte)(v[i] >>> 16);
            _buf.data[pos++] = (byte)(v[i] >>> 24);
            _buf.data[pos++] = (byte)(v[i] >>> 32);
            _buf.data[pos++] = (byte)(v[i] >>> 40);
            _buf.data[pos++] = (byte)(v[i] >>> 48);
            _buf.data[pos++] = (byte)(v[i] >>> 56);
        }
        assert(pos == _buf.pos);
    }

    public long
    readLong()
    {
        int pos = _buf.pos;
        _buf.pos += 8;
        if (_buf.pos > _buf.len)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        return ((long)_buf.data[pos++] & 0xffL) |
            (((long)_buf.data[pos++] << 8) & 0xff00L) |
            (((long)_buf.data[pos++] << 16) & 0xff0000L) |
            (((long)_buf.data[pos++] << 24) & 0xff000000L) |
            (((long)_buf.data[pos++] << 32) & 0xff00000000L) |
            (((long)_buf.data[pos++] << 40) & 0xff0000000000L) |
            (((long)_buf.data[pos++] << 48) & 0xff000000000000L) |
            ((long)_buf.data[pos]    << 56);
    }

    public long[]
    readLongSeq()
    {
        int sz = readInt();
        int pos = _buf.pos;
        _buf.pos += sz * 8;
        if (_buf.pos > _buf.len)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        long[] v = new long[sz];
        for (int i = 0; i < sz; i++)
        {
            v[i] = ((long)_buf.data[pos++] & 0xffL) |
                (((long)_buf.data[pos++] << 8) & 0xff00L) |
                (((long)_buf.data[pos++] << 16) & 0xff0000L) |
                (((long)_buf.data[pos++] << 24) & 0xff000000L) |
                (((long)_buf.data[pos++] << 32) & 0xff00000000L) |
                (((long)_buf.data[pos++] << 40) & 0xff0000000000L) |
                (((long)_buf.data[pos++] << 48) & 0xff000000000000L) |
                ((long)_buf.data[pos++]  << 56);
        }
        return v;
    }

    public void
    writeFloat(float v)
    {
        writeInt(Float.floatToIntBits(v));
    }

    public void
    writeFloatSeq(float[] v)
    {
        int pos = _buf.pos;
        final int vlen = v.length;
        _buf.pos += 4 + (vlen * 4);
        if (_buf.pos > _buf.len)
        {
            resize(_buf.pos);
        }
        _buf.data[pos++] = (byte)vlen;
        _buf.data[pos++] = (byte)(vlen >>> 8);
        _buf.data[pos++] = (byte)(vlen >>> 16);
        _buf.data[pos++] = (byte)(vlen >>> 24);
        for (int i = 0; i < vlen; i++)
        {
            int val = Float.floatToIntBits(v[i]);
            _buf.data[pos++] = (byte)val;
            _buf.data[pos++] = (byte)(val >>> 8);
            _buf.data[pos++] = (byte)(val >>> 16);
            _buf.data[pos++] = (byte)(val >>> 24);
        }
        assert(pos == _buf.pos);
    }

    public float
    readFloat()
    {
        return Float.intBitsToFloat(readInt());
    }

    public float[]
    readFloatSeq()
    {
        int sz = readInt();
        int pos = _buf.pos;
        _buf.pos += sz * 4;
        if (_buf.pos > _buf.len)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        int[] v = new int[sz];
        for (int i = 0; i < sz; i++)
        {
            int val = ((_buf.data[pos++] & 0xff) |
                      ((_buf.data[pos++] << 8) & 0xff00) |
                      ((_buf.data[pos++] << 16) & 0xff0000) |
                      (_buf.data[pos++]  << 24));
            v[i] = Float.intBitsToFloat(val);
        }
        return v;
    }

    public void
    writeDouble(double v)
    {
        writeLong(Double.doubleToLongBits(v));
    }

    public void
    writeDoubleSeq(double[] v)
    {
        int pos = _buf.pos;
        final int vlen = v.length;
        _buf.pos += 4 + (vlen * 8);
        if (_buf.pos > _buf.len)
        {
            resize(_buf.pos);
        }
        _buf.data[pos++] = (byte)vlen;
        _buf.data[pos++] = (byte)(vlen >>> 8);
        _buf.data[pos++] = (byte)(vlen >>> 16);
        _buf.data[pos++] = (byte)(vlen >>> 24);
        for (int i = 0; i < vlen; i++)
        {
            long val = Double.doubleToLongBits(v[i]);
            _buf.data[pos++] = (byte)val;
            _buf.data[pos++] = (byte)(val >>> 8);
            _buf.data[pos++] = (byte)(val >>> 16);
            _buf.data[pos++] = (byte)(val >>> 24);
            _buf.data[pos++] = (byte)(val >>> 32);
            _buf.data[pos++] = (byte)(val >>> 40);
            _buf.data[pos++] = (byte)(val >>> 48);
            _buf.data[pos++] = (byte)(val >>> 56);
        }
        assert(pos == _buf.pos);
    }

    public double
    readDouble()
    {
        return Double.longBitsToDouble(readLong());
    }

    public double[]
    readDoubleSeq()
    {
        int sz = readInt();
        int pos = _buf.pos;
        _buf.pos += sz * 8;
        if (_buf.pos > _buf.len)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        long[] v = new long[sz];
        for (int i = 0; i < sz; i++)
        {
            long val = ((long)_buf.data[pos++] & 0xffL) |
                       (((long)_buf.data[pos++] << 8) & 0xff00L) |
                       (((long)_buf.data[pos++] << 16) & 0xff0000L) |
                       (((long)_buf.data[pos++] << 24) & 0xff000000L) |
                       (((long)_buf.data[pos++] << 32) & 0xff00000000L) |
                       (((long)_buf.data[pos++] << 40) & 0xff0000000000L) |
                       (((long)_buf.data[pos++] << 48) & 0xff000000000000L) |
                       ((long)_buf.data[pos++]  << 56);
            v[i] = Double.longBitsToDouble(val);
        }
        return v;
    }

    public void
    writeString(String v)
    {
        Encaps enc = (Encaps)_encapsStack.getLast();
        Integer pos = null;
        if (enc.stringsWritten != null) // Lazy creation
        {
            pos = enc.stringsWritten.get(v);
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
                int pos = _buf.pos;
                _buf.pos += len;
                if (_buf.pos > _buf.len)
                {
                    resize(_buf.pos);
                }
                final char[] arr = v.toCharArray();
                for (int i = 0; i < len; i++)
                {
                    _buf.data[pos++] = (byte)arr[i];
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
                pos = _buf.pos;
                _buf.pos += len;
                if (_buf.pos > _buf.len)
                {
                    throw new Ice.UnmarshalOutOfBoundsException();
                }
                char[] arr = new char[len];
                for (int i = 0; i < len; i++)
                {
                    arr[i] = (char)_buf.data[pos++];
                }
                String v = new String(arr);
                Encaps enc = (Encaps)_encapsStack.getLast();
                if (enc.stringsRead == null)
                {
                    enc.stringsRead = new java.util.ArrayList(10);
                }
                enc.stringsRead.add(v);
                return v;
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
    }

    public void
    writeWString(String v)
    {
        Encaps enc = (Encaps)_encapsStack.getLast();
        Integer pos = null;
        if (enc.wstringsWritten != null) // Lazy creation
        {
            pos = enc.wstringsWritten.get(v);
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
                int pos = _buf.pos;
                _buf.pos += len;
                if (_buf.pos > _buf.len)
                {
                    resize(_buf.pos);
                }
                final char[] arr = v.toCharArray();
                for (int i = 0; i < len; i++)
                {
                    _buf.data[pos++] = (byte)v;
                    _buf.data[pos++] = (byte)(v >>> 8);
                }
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
                pos = _buf.pos;
                _buf.pos += len;
                if (_buf.pos > _buf.len)
                {
                    throw new Ice.UnmarshalOutOfBoundsException();
                }
                char[] arr = new char[len];
                for (int i = 0; i < len; i++)
                {
                    arr[i] = (char)(((char)_buf.data[pos++] & 0xff) |
                                    ((char)_buf.data[pos++] << 8));
                }
                String v = new String(arr);
                Encaps enc = (Encaps)_encapsStack.getLast();
                if (enc.wstringsRead == null)
                {
                    enc.wstringsRead = new java.util.ArrayList(10);
                }
                enc.wstringsRead.add(v);
                return v;
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
        // TODO
    }

    public Ice.Object
    readObject()
    {
        // TODO
    }

    private IceInternal.Instance _instance;
    private Buffer _buf = new Buffer();

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
