// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// *********************************************************************

namespace IceInternal
{
using System;

public class ByteBuffer
{

    public ByteBuffer()
    {
        _order = ByteOrder.BIG_ENDIAN;
    }

    public enum ByteOrder { BIG_ENDIAN, LITTLE_ENDIAN };
    
    public static ByteOrder nativeOrder()
    {
        return NO._o;
    }

    public ByteOrder order()
    {
        return _order;
    }

    public ByteBuffer order(ByteOrder bo)
    {
        _order = bo;
	return this;
    }

    public static ByteBuffer
    allocate(int capacity)
    {
	if(capacity < 0)
	{
	    throw new ArgumentOutOfRangeException("capacity", capacity, "capacity must be non-negative");
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
	    throw new ArgumentOutOfRangeException("pos", pos, "position must be non-negative");
	}
	if(pos > _limit)
	{
	    throw new ArgumentOutOfRangeException("pos", pos, "position must be less than limit");
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
	    throw new ArgumentOutOfRangeException("newLimit", newLimit, "limit must be non-negative");
	}
	if(newLimit > _capacity)
	{
	    throw new ArgumentOutOfRangeException("newLimit", newLimit, "limit must be less than capacity");
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

    public bool
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
    toArray()
    {
	byte[] rc = new byte[_limit - _position];
	Buffer.BlockCopy(_bytes, 0, rc, 0, _limit - _position);
	return rc;
    }

    public ByteBuffer
    put(ByteBuffer buf)
    {
	Buffer.BlockCopy(buf._bytes, buf._position, _bytes, _position, buf._limit);
	return this;
    }

    public byte
    get()
    {
	checkUnderflow(1);
	return Buffer.GetByte(_bytes, _position++);
    }

    public ByteBuffer
    get(byte[] b)
    {
	return get(b, 0, Buffer.ByteLength(b));
    }

    public ByteBuffer
    get(byte[] b, int offset, int length)
    {
	if(offset < 0)
	{
	    throw new ArgumentOutOfRangeException("offset", offset, "offset must be non-negative");
	}
	if(offset > _limit)
	{
	    throw new ArgumentOutOfRangeException("offset", offset, "offset must be less than buffer limit");
	}
	if(offset + length > _limit)
	{
	    throw new InvalidOperationException("buffer underflow");
	}
	Buffer.BlockCopy(_bytes, _position, b, 0, length);
	_position += length;
	return this;
    }

    public ByteBuffer
    put(byte b)
    {
	checkOverflow(1);
	Buffer.SetByte(_bytes, _position++, b);
	return this;
    }

    public ByteBuffer
    put(byte[] b)
    {
	return put(b, _position, Buffer.ByteLength(b));
    }

    public ByteBuffer
    put(byte[]b, int offset, int length)
    {
	if(offset < 0)
	{
	    throw new ArgumentOutOfRangeException("offset", offset, "offset must be non-negative");
	}
	if(offset > _limit)
	{
	    throw new ArgumentOutOfRangeException("offset", offset, "offset must be less than buffer limit");
	}
	if(offset + length > _limit)
	{
	    throw new InvalidOperationException("buffer overflow");
	}
	Buffer.BlockCopy(b, 0, _bytes, offset, length);
	_position += length;
	return this;
    }

    public bool
    getBool()
    {
	return get() == 1;
    }

    public void
    getBoolSeq(bool[] seq)
    {
	int len = Buffer.ByteLength(seq);
	checkUnderflow(len);
	Buffer.BlockCopy(_bytes, _position, seq, 0, len);
	_position += len;
    }

    public ByteBuffer
    putBool(bool b)
    {
	return put(b ? (byte)1 : (byte)0);
    }

    public ByteBuffer
    putBoolSeq(bool[] seq)
    {
	int len = Buffer.ByteLength(seq);
	checkOverflow(len);
	Buffer.BlockCopy(seq, 0, _bytes, _position, len);
	_position += len;
	return this;
    }

    public unsafe short
    getShort()
    {
	checkUnderflow(2);
	short ret;
	if(NO._o == _order)
	{
	    fixed(byte* p = &_bytes[_position])
	    {
		ret = *((short*)p);
	    }
	}
	else
	{
	    byte* p = (byte*)&ret;
	    *p++ = Buffer.GetByte(_bytes, _position + 1);
	    *p = Buffer.GetByte(_bytes, _position);
	}
	_position += 2;
	return ret;
    }

    public unsafe void
    getShortSeq(short[] seq)
    {
	int len = Buffer.ByteLength(seq);
	checkUnderflow(len);
	if(NO._o == _order)
	{
	    Buffer.BlockCopy(_bytes, _position, seq, 0, len);
	}
	else
	{
	    for(int i = 0; i < seq.Length; ++i)
	    {
		fixed(short* p = &seq[i])
		{
		    int index = i * 2;
		    byte* q = (byte*)p;
		    *q++ = Buffer.GetByte(_bytes, index + 1);
		    *q = Buffer.GetByte(_bytes, index);
		}
	    }
	}
	_position += len;
    }

    public unsafe ByteBuffer
    putShort(short val)
    {
	checkOverflow(2);
	if(NO._o == _order)
	{
	    fixed(byte* p = &_bytes[_position])
	    {
		*((short*)p) = val;
	    }
	    _position += 2;
	}
	else
	{
	    byte* p = (byte*)&val;
	    Buffer.SetByte(_bytes, _position++, *(p + 1));
	    Buffer.SetByte(_bytes, _position++, *p);
	}
	return this;
    }

    public unsafe ByteBuffer
    putShortSeq(short[] seq)
    {
	int len = Buffer.ByteLength(seq);
	checkOverflow(len);
	if(NO._o == _order)
	{
	    Buffer.BlockCopy(seq, 0, _bytes, _position, len);
	    _position += len;
	}
	else
	{
	    for(int i = 0; i < seq.Length; ++i)
	    {
	        int index = i * 2;
		Buffer.SetByte(_bytes, _position++, Buffer.GetByte(seq, index + 1));
		Buffer.SetByte(_bytes, _position++, Buffer.GetByte(seq, index));
	    }
	}
	return this;
    }

    public unsafe int
    getInt()
    {
	checkUnderflow(4);	
	int ret;
	if(NO._o == _order)
	{
	    fixed(byte* p = &_bytes[_position])
	    {
		ret = *((int*)p);
	    }
	}
	else
	{
	    byte* p = (byte*)&ret;
	    *p++ = Buffer.GetByte(_bytes, _position + 3);
	    *p++ = Buffer.GetByte(_bytes, _position + 2);
	    *p++ = Buffer.GetByte(_bytes, _position + 1);
	    *p = Buffer.GetByte(_bytes, _position);
	}
	_position += 4;
	return ret;
    }

    public unsafe void
    getIntSeq(int[] seq)
    {
	int len = Buffer.ByteLength(seq);
	checkUnderflow(len);
	if(NO._o == _order)
	{
	    Buffer.BlockCopy(_bytes, _position, seq, 0, len);
	}
	else
	{
	    for(int i = 0; i < seq.Length; ++i)
	    {
		fixed(int* p = &seq[i])
		{
		    int index = i * 4;
		    byte* q = (byte*)p;
		    *q++ = Buffer.GetByte(_bytes, index + 3);
		    *q++ = Buffer.GetByte(_bytes, index + 2);
		    *q++ = Buffer.GetByte(_bytes, index + 1);
		    *q = Buffer.GetByte(_bytes, index);
		}
	    }
	}
	_position += len;
    }

    public ByteBuffer
    putInt(int val)
    {
	return putInt(_position, val);
    }

    public unsafe ByteBuffer
    putInt(int pos, int val)
    {
	if(pos < 0)
	{
	    throw new ArgumentOutOfRangeException("pos", pos, "position must be non-negative");
	}
	if(pos + 4 > _limit)
	{
	    throw new ArgumentOutOfRangeException("pos", pos, "position must be less than limit - 4");
	}
	if(NO._o == _order)
	{
	    fixed(byte* p = &_bytes[pos])
	    {
		*((int*)p) = val;
	    }	
	    _position += 4;
	}
	else
	{
	    byte* p = (byte*)&val;
	    Buffer.SetByte(_bytes, _position++, *(p + 3));
	    Buffer.SetByte(_bytes, _position++, *(p + 2));
	    Buffer.SetByte(_bytes, _position++, *(p + 1));
	    Buffer.SetByte(_bytes, _position++, *p);
	}
	return this;
    }

    public ByteBuffer
    putIntSeq(int[] seq)
    {
	int len = Buffer.ByteLength(seq);
	checkOverflow(len);
	if(NO._o == _order)
	{
	    Buffer.BlockCopy(seq, 0, _bytes, _position, len);
	    _position += len;
	}
	else
	{
	    for(int i = 0; i < seq.Length; ++i)
	    {
	        int index = i * 4;
		Buffer.SetByte(_bytes, _position++, Buffer.GetByte(seq, index + 3));
		Buffer.SetByte(_bytes, _position++, Buffer.GetByte(seq, index + 2));
		Buffer.SetByte(_bytes, _position++, Buffer.GetByte(seq, index + 1));
		Buffer.SetByte(_bytes, _position++, Buffer.GetByte(seq, index));
	    }
	}
	return this;
    }

    public unsafe long
    getLong()
    {
	checkUnderflow(8);	
	long ret;
	if(NO._o == _order)
	{
	    fixed(byte* p = &_bytes[_position])
	    {
		ret = *((long*)p);
	    }
	}
	else
	{
	    byte* p = (byte*)&ret;
	    *p++ = Buffer.GetByte(_bytes, _position + 7);
	    *p++ = Buffer.GetByte(_bytes, _position + 6);
	    *p++ = Buffer.GetByte(_bytes, _position + 5);
	    *p++ = Buffer.GetByte(_bytes, _position + 4);
	    *p++ = Buffer.GetByte(_bytes, _position + 3);
	    *p++ = Buffer.GetByte(_bytes, _position + 2);
	    *p++ = Buffer.GetByte(_bytes, _position + 1);
	    *p = Buffer.GetByte(_bytes, _position);
	}
	_position += 8;
	return ret;
    }

    public unsafe void
    getLongSeq(long[] seq)
    {
	int len = Buffer.ByteLength(seq);
	checkUnderflow(len);
	if(NO._o == _order)
	{
	    Buffer.BlockCopy(_bytes, _position, seq, 0, len);
	}
	else
	{
	    for(int i = 0; i < seq.Length; ++i)
	    {
		fixed(long* p = &seq[i])
		{
		    int index = i * 8;
		    byte* q = (byte*)p;
		    *q++ = Buffer.GetByte(_bytes, index + 7);
		    *q++ = Buffer.GetByte(_bytes, index + 6);
		    *q++ = Buffer.GetByte(_bytes, index + 5);
		    *q++ = Buffer.GetByte(_bytes, index + 4);
		    *q++ = Buffer.GetByte(_bytes, index + 3);
		    *q++ = Buffer.GetByte(_bytes, index + 2);
		    *q++ = Buffer.GetByte(_bytes, index + 1);
		    *q = Buffer.GetByte(_bytes, index);
		}
	    }
	}
	_position += len;
    }

    public unsafe ByteBuffer
    putLong(long val)
    {
	checkOverflow(8);
	if(NO._o == _order)
	{
	    fixed(byte* p = &_bytes[_position])
	    {
		*((long*)p) = val;
	    }
	    _position += 8;
	}
	else
	{
	    byte* p = (byte*)&val;
	    Buffer.SetByte(_bytes, _position++, *(p + 7));
	    Buffer.SetByte(_bytes, _position++, *(p + 6));
	    Buffer.SetByte(_bytes, _position++, *(p + 5));
	    Buffer.SetByte(_bytes, _position++, *(p + 4));
	    Buffer.SetByte(_bytes, _position++, *(p + 3));
	    Buffer.SetByte(_bytes, _position++, *(p + 2));
	    Buffer.SetByte(_bytes, _position++, *(p + 1));
	    Buffer.SetByte(_bytes, _position++, *p);
	}
	return this;
    }

    public ByteBuffer
    putLongSeq(long[] seq)
    {
	int len = Buffer.ByteLength(seq);
	checkOverflow(len);
	if(NO._o == _order)
	{
	    Buffer.BlockCopy(seq, 0, _bytes, _position, len);
	    _position += len;
	}
	else
	{
	    for(int i = 0; i < seq.Length; ++i)
	    {
		int index = i * 8;
		Buffer.SetByte(_bytes, _position++, Buffer.GetByte(seq, index + 7));
		Buffer.SetByte(_bytes, _position++, Buffer.GetByte(seq, index + 6));
		Buffer.SetByte(_bytes, _position++, Buffer.GetByte(seq, index + 5));
		Buffer.SetByte(_bytes, _position++, Buffer.GetByte(seq, index + 4));
		Buffer.SetByte(_bytes, _position++, Buffer.GetByte(seq, index + 3));
		Buffer.SetByte(_bytes, _position++, Buffer.GetByte(seq, index + 2));
		Buffer.SetByte(_bytes, _position++, Buffer.GetByte(seq, index + 1));
		Buffer.SetByte(_bytes, _position++, Buffer.GetByte(seq, index));
	    }
	}
	return this;
    }

    public unsafe float
    getFloat()
    {
	checkUnderflow(4);	
	float ret;
	if(NO._o == _order)
	{
	    fixed(byte* p = &_bytes[_position])
	    {
		ret = *((float*)p);
	    }
	}
	else
	{
	    byte* p = (byte*)&ret;
	    *p++ = Buffer.GetByte(_bytes, _position + 3);
	    *p++ = Buffer.GetByte(_bytes, _position + 2);
	    *p++ = Buffer.GetByte(_bytes, _position + 1);
	    *p = Buffer.GetByte(_bytes, _position);
	}
	_position += 4;
	return ret;
    }

    public unsafe void
    getFloatSeq(float[] seq)
    {
	int len = Buffer.ByteLength(seq);
	checkUnderflow(len);
	if(NO._o == _order)
	{
	    Buffer.BlockCopy(_bytes, _position, seq, 0, len);
	}
	else
	{
	    for(int i = 0; i < seq.Length; ++i)
	    {
		fixed(float* p = &seq[i])
		{
		    int index = i * 4;
		    byte* q = (byte*)p;
		    *q++ = Buffer.GetByte(_bytes, index + 3);
		    *q++ = Buffer.GetByte(_bytes, index + 2);
		    *q++ = Buffer.GetByte(_bytes, index + 1);
		    *q = Buffer.GetByte(_bytes, index);
		}
	    }
	}
	_position += len;
    }

    public unsafe ByteBuffer
    putFloat(float val)
    {
	checkOverflow(4);
	if(NO._o == _order)
	{
	    fixed(byte* p = &_bytes[_position])
	    {
		*((float*)p) = val;
	    }
	    _position += 2;
	}
	else
	{
	    byte* p = (byte*)&val;
	    Buffer.SetByte(_bytes, _position++, *(p + 3));
	    Buffer.SetByte(_bytes, _position++, *(p + 2));
	    Buffer.SetByte(_bytes, _position++, *(p + 1));
	    Buffer.SetByte(_bytes, _position++, *p);
	}
	return this;
    }

    public ByteBuffer
    putFloatSeq(float[] seq)
    {
	int len = Buffer.ByteLength(seq);
	checkOverflow(len);
	if(NO._o == _order)
	{
	    Buffer.BlockCopy(seq, 0, _bytes, _position, len);
	    _position += len;
	}
	else
	{
	    for(int i = 0; i < seq.Length; ++i)
	    {
		int index = i * 4;
		Buffer.SetByte(_bytes, _position++, Buffer.GetByte(seq, index + 3));
		Buffer.SetByte(_bytes, _position++, Buffer.GetByte(seq, index + 2));
		Buffer.SetByte(_bytes, _position++, Buffer.GetByte(seq, index + 1));
		Buffer.SetByte(_bytes, _position++, Buffer.GetByte(seq, index));
	    }
	}
	return this;
    }

    public unsafe double
    getDouble()
    {
	checkUnderflow(8);	
	double ret;
	if(NO._o == _order)
	{
	    fixed(byte* p = &_bytes[_position])
	    {
		ret = *((double*)p);
	    }
	}
	else
	{
	    byte* p = (byte*)&ret;
	    *p++ = Buffer.GetByte(_bytes, _position + 7);
	    *p++ = Buffer.GetByte(_bytes, _position + 6);
	    *p++ = Buffer.GetByte(_bytes, _position + 5);
	    *p++ = Buffer.GetByte(_bytes, _position + 4);
	    *p++ = Buffer.GetByte(_bytes, _position + 3);
	    *p++ = Buffer.GetByte(_bytes, _position + 2);
	    *p++ = Buffer.GetByte(_bytes, _position + 1);
	    *p = Buffer.GetByte(_bytes, _position);
	}
	_position += 8;
	return ret;
    }

    public unsafe void
    getDoubleSeq(double[] seq)
    {
	int len = Buffer.ByteLength(seq);
	checkUnderflow(len);
	if(NO._o == _order)
	{
	    Buffer.BlockCopy(_bytes, _position, seq, 0, len);
	}
	else
	{
	    for(int i = 0; i < seq.Length; ++i)
	    {
		fixed(double* p = &seq[i])
		{
		    int index = i * 8;
		    byte* q = (byte*)p;
		    *q++ = Buffer.GetByte(_bytes, index + 7);
		    *q++ = Buffer.GetByte(_bytes, index + 6);
		    *q++ = Buffer.GetByte(_bytes, index + 5);
		    *q++ = Buffer.GetByte(_bytes, index + 4);
		    *q++ = Buffer.GetByte(_bytes, index + 3);
		    *q++ = Buffer.GetByte(_bytes, index + 2);
		    *q++ = Buffer.GetByte(_bytes, index + 1);
		    *q = Buffer.GetByte(_bytes, index);
		}
	    }
	}
	_position += len;
    }

    public unsafe ByteBuffer
    putDouble(double val)
    {
	checkOverflow(8);
	if(NO._o == _order)
	{
	    fixed(byte* p = &_bytes[_position])
	    {
		*((double*)p) = val;
	    }
	    _position += 8;
	}
	else
	{
	    byte* p = (byte*)&val;
	    Buffer.SetByte(_bytes, _position++, *(p + 7));
	    Buffer.SetByte(_bytes, _position++, *(p + 6));
	    Buffer.SetByte(_bytes, _position++, *(p + 5));
	    Buffer.SetByte(_bytes, _position++, *(p + 4));
	    Buffer.SetByte(_bytes, _position++, *(p + 3));
	    Buffer.SetByte(_bytes, _position++, *(p + 2));
	    Buffer.SetByte(_bytes, _position++, *(p + 1));
	    Buffer.SetByte(_bytes, _position++, *p);
	}
	return this;
    }

    public ByteBuffer
    putDoubleSeq(double[] seq)
    {
	int len = Buffer.ByteLength(seq);
	checkOverflow(len);
	if(NO._o == _order)
	{
	    Buffer.BlockCopy(seq, 0, _bytes, _position, len);
	    _position += len;
	}
	else
	{
	    for(int i = 0; i < seq.Length; ++i)
	    {
	        int index = i * 8;
		Buffer.SetByte(_bytes, _position++, Buffer.GetByte(seq, index + 7));
		Buffer.SetByte(_bytes, _position++, Buffer.GetByte(seq, index + 6));
		Buffer.SetByte(_bytes, _position++, Buffer.GetByte(seq, index + 5));
		Buffer.SetByte(_bytes, _position++, Buffer.GetByte(seq, index + 4));
		Buffer.SetByte(_bytes, _position++, Buffer.GetByte(seq, index + 3));
		Buffer.SetByte(_bytes, _position++, Buffer.GetByte(seq, index + 2));
		Buffer.SetByte(_bytes, _position++, Buffer.GetByte(seq, index + 1));
		Buffer.SetByte(_bytes, _position++, Buffer.GetByte(seq, index));
	    }
	}
	return this;
    }

    private void
    checkUnderflow(int size)
    {
	if(_position + size > _limit)
	{
	    throw new InvalidOperationException("buffer underflow");
	}
    }

    private void
    checkOverflow(int size)
    {
	if(_position + size > _limit)
	{
	    throw new InvalidOperationException("buffer overflow");
	}
    }

    private int _position;
    private int _limit;
    private int _capacity;
    private byte[] _bytes;
    private ByteOrder _order;

    private class NO // Native Order
    {
        static NO()
	{
	    byte[] b = BitConverter.GetBytes((int)1);
	    _o = b[0] == 1 ? ByteOrder.LITTLE_ENDIAN : ByteOrder.BIG_ENDIAN;
	}
	internal static readonly ByteOrder _o;
    }
}

}
