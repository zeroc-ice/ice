// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class ByteBuffer
{
    public
    ByteBuffer()
    {
	_order = BIG_ENDIAN;
    }

    public static final int BIG_ENDIAN = 0;
    public static final int LITTLE_ENDIAN = 1;

    public int
    order()
    {
	return _order;
    }

    public ByteBuffer
    order(int bo)
    {
	_order = bo;
	return this;
    }

    public static ByteBuffer
    allocate(int capacity)
    {
	if(capacity < 0)
	{
	    throw new IllegalArgumentException("capacity must be non-negative");
	}
	ByteBuffer ret = new ByteBuffer();
	ret._position = 0;
	ret._limit = capacity;
	ret._capacity = capacity;
	ret._bytes = new byte[capacity];
	return ret;
    }

    public int
    position()
    {
	return _position;
    }

    public ByteBuffer
    position(int pos)
    {
	if(pos < 0)
	{
	    throw new IllegalArgumentException("position must be non-negative");
	}
	if(pos > _limit)
	{
	    throw new IllegalArgumentException("position must be less than limit");
	}
	_position = pos;
	return this;
    }
    
    public int
    limit()
    {
	return _limit;
    }

    public ByteBuffer
    limit(int newLimit)
    {
	if(newLimit < 0)
	{
	    throw new IllegalArgumentException("limit must be non-negative");
	}
	if(newLimit > _capacity)
	{
	    throw new IllegalArgumentException("limit must be less than capacity");
	}
	_limit = newLimit;
	return this;
    }

    public void
    clear()
    {
	_position = 0;
	_limit = _capacity;
    }

    public int
    remaining()
    {
	return _limit - _position;
    }

    public boolean
    hasRemaining()
    {
	return _position < _limit;
    }

    public int
    capacity()
    {
	return _capacity;
    }

    public byte[]
    array()
    {
	return _bytes;
    }

    public ByteBuffer
    put(ByteBuffer buf)
    {
	int len = buf.remaining();
	checkOverflow(len);
	System.arraycopy(buf._bytes, buf._position, _bytes, _position, len);
	_position += len;
	return this;
    }

    public byte
    get()
    {
	checkUnderflow(1);
	return _bytes[_position++];
    }

    public ByteBuffer
    get(byte[] b)
    {
	return get(b, 0, b.length);
    }

    public ByteBuffer
    get(byte[] b, int offset, int length)
    {
	if(offset < 0)
	{
	    throw new IllegalArgumentException("offset must be non-negative");
	}
	if(offset + length > b.length)
	{
	    throw new IllegalArgumentException("insufficient room beyond given offset in destination array");
	}
	checkUnderflow(length);
	System.arraycopy(_bytes, _position, b, offset, length);
	_position += length;
	return this;
    }

    public ByteBuffer
    put(byte b)
    {
	checkOverflow(1);
	_bytes[_position++] = b;
	return this;
    }

    public ByteBuffer
    put(byte[] b)
    {
	return put(b, 0, b.length);
    }

    public ByteBuffer
    put(byte[] b, int offset, int length)
    {
	if(offset < 0)
	{
	    throw new IllegalArgumentException("offset must be non-negative");
	}
	if(offset + length > b.length)
	{
	    throw new IllegalArgumentException("insufficient data beyond given offset in source array");
	}
	checkOverflow(length);
	System.arraycopy(b, offset, _bytes, _position, length);
	_position += length;
	return this;
    }

    public short
    getShort()
    {
	checkUnderflow(2);
	int high, low;
	if(_order == BIG_ENDIAN)
	{
	    high = _bytes[_position++] & 0xff;
	    low = _bytes[_position++] & 0xff;
	}
	else
	{
	    low = _bytes[_position++] & 0xff;
	    high = _bytes[_position++] & 0xff;
	}
	return (short)(high << 8 | low);
    }

    public void
    getShortSeq(short[] seq)
    {
	checkUnderflow(seq.length * 2);
	if(_order == BIG_ENDIAN)
	{
	    for(int i = 0; i < seq.length; ++i)
	    {
		int high = _bytes[_position++] & 0xff;
		int low = _bytes[_position++] & 0xff;
		seq[i] = (short)(high << 8 | low);
	    }
	}
	else
	{
	    for(int i = 0; i < seq.length; ++i)
	    {
	        int low = _bytes[_position++] & 0xff;
	        int high = _bytes[_position++] & 0xff;
	        seq[i] = (short)(high << 8 | low);
	    }
	}
    }

    public ByteBuffer
    putShort(short val)
    {
	checkOverflow(2);
	if(_order == BIG_ENDIAN)
	{
	    _bytes[_position++] = (byte)(val >>> 8);
	    _bytes[_position++] = (byte)val;
	}
	else
	{
	    _bytes[_position++] = (byte)val;
	    _bytes[_position++] = (byte)(val >>> 8);
	}
	return this;
    }

    public ByteBuffer
    putShortSeq(short[] seq)
    {
	checkOverflow(seq.length * 2);
	if(_order == BIG_ENDIAN)
	{
	    for(int i = 0; i < seq.length; ++i)
	    {
		_bytes[_position++] = (byte)(seq[i] >>> 8);
		_bytes[_position++] = (byte)seq[i];
	    }
	}
	else
	{
	    for(int i = 0; i < seq.length; ++i)
	    {
		_bytes[_position++] = (byte)seq[i];
		_bytes[_position++] = (byte)(seq[i] >>> 8);
	    }
	}
	return this;
    }

    public int
    getInt()
    {
	checkUnderflow(4);	
	int ret = 0;
	if(_order == BIG_ENDIAN)
	{
	    for(int shift = 24; shift >= 0; shift -= 8)
	    {
		ret |= (int)(_bytes[_position++] & 0xff) << shift;
	    }
	}
	else
	{
	    for(int shift = 0; shift < 32; shift += 8)
	    {
		ret |= (int)(_bytes[_position++] & 0xff) << shift;
	    }
	}
	return ret;
    }

    public void
    getIntSeq(int[] seq)
    {
	checkUnderflow(seq.length * 4);
	if(_order == BIG_ENDIAN)
	{
	    for(int i = 0; i < seq.length; ++i)
	    {
		int val = 0;
		for(int shift = 24; shift >= 0; shift -= 8)
		{
		    val |= (int)(_bytes[_position++] & 0xff) << shift;
		}
		seq[i] = val;
	    }
	}
	else
	{
	    for(int i = 0; i < seq.length; ++i)
	    {
		int val = 0;
		for(int shift = 0; shift < 32; shift += 8)
		{
		    val |= (int)(_bytes[_position++] & 0xff) << shift;
		}
		seq[i] = val;
	    }
	}
    }

    public ByteBuffer
    putInt(int val)
    {
	putInt(_position, val);
	_position += 4;
	return this;
    }

    public ByteBuffer
    putInt(int pos, int val)
    {
	if(pos < 0)
	{
	    throw new IllegalArgumentException("position must be non-negative");
	}
	if(pos + 4 > _limit)
	{
	    throw new IllegalArgumentException("position must be less than limit - 4");
	}
	if(_order == BIG_ENDIAN)
	{
	    for(int shift = 24; shift >= 0; shift -= 8)
	    {
		_bytes[pos++] = (byte)((val >> shift) & 0xff);
	    }
	}
	else
	{
	    for(int shift = 0; shift < 32; shift += 8)
	    {
		_bytes[pos++] = (byte)((val >> shift) & 0xff);
	    }
	}
	return this;
    }

    public ByteBuffer
    putIntSeq(int[] seq)
    {
	checkOverflow(seq.length * 4);
	if(_order == BIG_ENDIAN)
	{
	    for(int i = 0; i < seq.length; ++i)
	    {
		int val = seq[i];
		for(int shift = 24; shift >= 0; shift -= 8)
		{
		    _bytes[_position++] = (byte)((val >> shift) & 0xff);
		}
	    }
	}
	else
	{
	    for(int i = 0; i < seq.length; ++i)
	    {
		int val = seq[i];
		for(int shift = 0; shift < 32; shift += 8)
		{
		    _bytes[_position++] = (byte)((val >> shift) & 0xff);
		}
	    }
	}
	return this;
    }

    public long
    getLong()
    {
	checkUnderflow(8);	
	long ret = 0;
	if(_order == BIG_ENDIAN)
	{
	    for(int shift = 56; shift >= 0; shift -= 8)
	    {
		ret |= (long)(_bytes[_position++] & 0xff) << shift;
	    }
	}
	else
	{
	    for(int shift = 0; shift < 64; shift += 8)
	    {
		ret |= (long)(_bytes[_position++] & 0xff) << shift;
	    }
	}
	return ret;
    }

    public void
    getLongSeq(long[] seq)
    {
	checkUnderflow(seq.length * 8);
	for(int i = 0; i < seq.length; ++i)
	{
	    if(_order == BIG_ENDIAN)
	    {
		long val = 0;
		for(int shift = 56; shift >= 0; shift -= 8)
		{
		    val |= (long)(_bytes[_position++] & 0xff) << shift;
		}
		seq[i] = val;
	    }
	    else
	    {
		long val = 0;
		for(int shift = 0; shift < 64; shift += 8)
		{
		    val |= (long)(_bytes[_position++] & 0xff) << shift;
		}
		seq[i] = val;
	    }
	}
    }

    public ByteBuffer
    putLong(long val)
    {
	checkOverflow(8);
	if(_order == BIG_ENDIAN)
	{
	    for(int shift = 56; shift >= 0; shift -= 8)
	    {
		_bytes[_position++] = (byte)((val >> shift) & 0xff);
	    }
	}
	else
	{
	    for(int shift = 0; shift < 64; shift += 8)
	    {
		_bytes[_position++] = (byte)((val >> shift) & 0xff);
	    }
	}
	return this;
    }

    public ByteBuffer
    putLongSeq(long[] seq)
    {
	checkOverflow(seq.length * 8);
	if(_order == BIG_ENDIAN)
	{
	    for(int i = 0; i < seq.length; ++i)
	    {
		long val = seq[i];
		for(int shift = 56; shift >= 0; shift -= 8)
		{
		    _bytes[_position++] = (byte)((val >> shift) & 0xff);
		}
	    }
	}
	else
	{
	    for(int i = 0; i < seq.length; ++i)
	    {
		long val = seq[i];
		for(int shift = 0; shift < 64; shift += 8)
		{
		    _bytes[_position++] = (byte)((val >> shift) & 0xff);
		}
	    }
	}
	return this;
    }

    public float
    getFloat()
    {
	checkUnderflow(4);	
	int bits = 0;
	if(_order == BIG_ENDIAN)
	{
	    for(int shift = 24; shift >= 0; shift -= 8)
	    {
		bits |= (int)(_bytes[_position++] & 0xff) << shift;
	    }
	}
	else
	{
	    for(int shift = 0; shift < 32; shift += 8)
	    {
		bits |= (int)(_bytes[_position++] & 0xff) << shift;
	    }
	}
	return Float.intBitsToFloat(bits);
    }

    public void
    getFloatSeq(float[] seq)
    {
	checkUnderflow(seq.length * 4);
	if(_order == BIG_ENDIAN)
	{
	    for(int i = 0; i < seq.length; ++i)
	    {
		int bits = 0;
		for(int shift = 24; shift >= 0; shift -= 8)
		{
		    bits |= (int)(_bytes[_position++] & 0xff) << shift;
		}
		seq[i] = Float.intBitsToFloat(bits);
	    }
	}
	else
	{
	    for(int i = 0; i < seq.length; ++i)
	    {
		int bits = 0;
		for(int shift = 0; shift < 32; shift += 8)
		{
		    bits |= (int)(_bytes[_position++] & 0xff) << shift;
		}
		seq[i] = Float.intBitsToFloat(bits);
	    }
	}
    }

    public ByteBuffer
    putFloat(float val)
    {
	checkOverflow(4);
	int bits = Float.floatToIntBits(val);
	if(_order == BIG_ENDIAN)
	{
	    for(int shift = 24; shift >= 0; shift -= 8)
	    {
		_bytes[_position++] = (byte)((bits >> shift) & 0xff);
	    }
	}
	else
	{
	    for(int shift = 0; shift < 32; shift += 8)
	    {
		_bytes[_position++] = (byte)((bits >> shift) & 0xff);
	    }
	}
	return this;
    }

    public ByteBuffer
    putFloatSeq(float[] seq)
    {
	checkOverflow(seq.length * 4);
	if(_order == BIG_ENDIAN)
	{
	    for(int i = 0; i < seq.length; ++i)
	    {
		int bits = Float.floatToIntBits(seq[i]);
		for(int shift = 24; shift >= 0; shift -= 8)
		{
		    _bytes[_position++] = (byte)((bits >> shift) & 0xff);
		}
	    }
	}
	else
	{
	    for(int i = 0; i < seq.length; ++i)
	    {
		int bits = Float.floatToIntBits(seq[i]);
		for(int shift = 0; shift < 32; shift += 8)
		{
		    _bytes[_position++] = (byte)((bits >> shift) & 0xff);
		}
	    }
	}
	return this;
    }

    public double
    getDouble()
    {
	checkUnderflow(8);	
	long bits = 0;
	if(_order == BIG_ENDIAN)
	{
	    for(int shift = 56; shift >= 0; shift -= 8)
	    {
		bits |= (long)(_bytes[_position++] & 0xff) << shift;
	    }
	}
	else
	{
	    for(int shift = 0; shift < 64; shift += 8)
	    {
		bits |= (long)(_bytes[_position++] & 0xff) << shift;
	    }
	}
	return Double.longBitsToDouble(bits);
    }

    public void
    getDoubleSeq(double[] seq)
    {
	checkUnderflow(seq.length * 8);
	if(_order == BIG_ENDIAN)
	{
	    for(int i = 0; i < seq.length; ++i)
	    {
		long bits = 0;
		for(int shift = 56; shift >= 0; shift -= 8)
		{
		    bits |= (long)(_bytes[_position++] & 0xff) << shift;
		}
		seq[i] = Double.longBitsToDouble(bits);
	    }
	}
	else
	{
	    for(int i = 0; i < seq.length; ++i)
	    {
		long bits = 0;
		for(int shift = 0; shift < 64; shift += 8)
		{
		    bits |= (long)(_bytes[_position++] & 0xff) << shift;
		}
		seq[i] = Double.longBitsToDouble(bits);
	    }
	}
    }

    public ByteBuffer
    putDouble(double val)
    {
	checkOverflow(8);
	long bits = Double.doubleToLongBits(val);
	if(_order == BIG_ENDIAN)
	{
	    for(int shift = 56; shift >= 0; shift -= 8)
	    {
		_bytes[_position++] = (byte)((bits >> shift) & 0xff);
	    }
	}
	else
	{
	    for(int shift = 0; shift < 64; shift += 8)
	    {
		_bytes[_position++] = (byte)((bits >> shift) & 0xff);
	    }
	}
	return this;
    }

    public ByteBuffer
    putDoubleSeq(double[] seq)
    {
	checkOverflow(seq.length * 8);
	if(_order == BIG_ENDIAN)
	{
	    for(int i = 0; i < seq.length; ++i)
	    {
		long bits = Double.doubleToLongBits(seq[i]);
		for(int shift = 56; shift >= 0; shift -= 8)
		{
		    _bytes[_position++] = (byte)((bits >> shift) & 0xff);
		}
	    }
	}
	else
	{
	    for(int i = 0; i < seq.length; ++i)
	    {
		long bits = Double.doubleToLongBits(seq[i]);
		for(int shift = 0; shift < 64; shift += 8)
		{
		    _bytes[_position++] = (byte)((bits >> shift) & 0xff);
		}
	    }
	}
	return this;
    }

    byte[]
    rawBytes()
    {
	return _bytes;
    }

    byte[]
    rawBytes(int offset, int len)
    {
	if(offset + len > _limit)
	{
	    throw new UnderflowException();
	}
	byte[] rc = new byte[len];
	System.arraycopy(_bytes, offset, rc, 0, len);
	return rc;
    }

    private void
    checkUnderflow(int size)
    {
	if(_position + size > _limit)
	{
	    throw new UnderflowException();
	}
    }

    private void
    checkOverflow(int size)
    {
	if(_position + size > _limit)
	{
	    throw new OverflowException();
	}
    }

    public static class UnderflowException extends RuntimeException
    {
	public UnderflowException()
	{
	    super("buffer underflow");
	}
    }

    public static class OverflowException extends RuntimeException
    {
	public OverflowException()
	{
	    super("buffer overflow");
	}
    }

    private int _position;
    private int _limit;
    private int _capacity;
    private byte[] _bytes;
    private int _order;
}
