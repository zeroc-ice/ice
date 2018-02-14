// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Runtime.InteropServices;

namespace IceInternal
{
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

        public static ByteBuffer allocate(int capacity)
        {
            if(capacity < 0)
            {
                throwOutOfRange("capacity", capacity, "capacity must be non-negative");
            }
            ByteBuffer ret = new ByteBuffer();
            ret._position = 0;
            ret._limit = capacity;
            ret._capacity = capacity;
            ret._bytes = new byte[capacity];
            ret._valBytes = new ValBytes();
            return ret;
        }

        public static ByteBuffer wrap(byte[] bytes)
        {
            ByteBuffer ret = new ByteBuffer();
            ret._position = 0;
            ret._limit = bytes.Length;
            ret._capacity = bytes.Length;
            ret._bytes = bytes;
            ret._valBytes = new ValBytes();
            return ret;
        }

        public int position()
        {
            return _position;
        }

        public ByteBuffer position(int pos)
        {
            if(pos < 0)
            {
                throwOutOfRange("pos", pos, "position must be non-negative");
            }
            if(pos > _limit)
            {
                throwOutOfRange("pos", pos, "position must be less than limit");
            }
            _position = pos;
            return this;
        }

        public int limit()
        {
            return _limit;
        }

        public ByteBuffer limit(int newLimit)
        {
            if(newLimit < 0)
            {
                throwOutOfRange("newLimit", newLimit, "limit must be non-negative");
            }
            if(newLimit > _capacity)
            {
                throwOutOfRange("newLimit", newLimit, "limit must be less than capacity");
            }
            _limit = newLimit;
            return this;
        }

        public void clear()
        {
            _position = 0;
            _limit = _capacity;
        }

        public void flip()
        {
            _limit = _position;
            _position = 0;
        }

        public void compact()
        {
            if(_position < _limit)
            {
                int n = _limit - _position;
                System.Buffer.BlockCopy(_bytes, _position, _bytes, 0, n);
                _position = n;
            }
            else
            {
                _position = 0;
            }
            _limit = _capacity;
        }

        public int remaining()
        {
            return _limit - _position;
        }

        public bool hasRemaining()
        {
            return _position < _limit;
        }

        public int capacity()
        {
            return _capacity;
        }

        public byte[] toArray()
        {
            int len = remaining();
            byte[] rc = new byte[len];
            System.Buffer.BlockCopy(_bytes, 0, rc, 0, len);
            return rc;
        }

        public byte[] toArray(int startIndex, int length)
        {
            if(startIndex < 0)
            {
                throwOutOfRange("startIndex", startIndex, "startIndex must be non-negative");
            }
            if(startIndex >= _position)
            {
                throwOutOfRange("startIndex", startIndex, "startIndex must be less than position");
            }
            if(length < 0)
            {
                throwOutOfRange("length", length, "length must be non-negative");
            }
            if(startIndex + length > _position)
            {
                throw new ArgumentException("startIndex + length must not exceed end mark of buffer");
            }
            byte[] rc = new byte[length];
            System.Buffer.BlockCopy(_bytes, startIndex, rc, 0, length);
            return rc;
        }

        public ByteBuffer put(ByteBuffer buf)
        {
            int len = buf.remaining();
            checkOverflow(len);
            System.Buffer.BlockCopy(buf._bytes, buf._position, _bytes, _position, len);
            _position += len;
            return this;
        }

        public byte get()
        {
            checkUnderflow(1);
            return System.Buffer.GetByte(_bytes, _position++);
        }

        public byte get(int pos)
        {
            return System.Buffer.GetByte(_bytes, pos);
        }

        public ByteBuffer get(byte[] b)
        {
            return get(b, 0, System.Buffer.ByteLength(b));
        }

        public ByteBuffer get(byte[] b, int offset, int length)
        {
            if(offset < 0)
            {
                throwOutOfRange("offset", offset, "offset must be non-negative");
            }
            if(offset + length > System.Buffer.ByteLength(b))
            {
                throwOutOfRange("length", length, "insufficient room beyond given offset in destination array");
            }
            checkUnderflow(length);
            System.Buffer.BlockCopy(_bytes, _position, b, offset, length);
            _position += length;
            return this;
        }

        public ByteBuffer put(byte b)
        {
            checkOverflow(1);
            System.Buffer.SetByte(_bytes, _position++, b);
            return this;
        }

        public ByteBuffer put(int pos, byte b)
        {
            System.Buffer.SetByte(_bytes, pos, b);
            return this;
        }

        public ByteBuffer put(byte[] b)
        {
            return put(b, 0, System.Buffer.ByteLength(b));
        }

        public ByteBuffer put(byte[] b, int offset, int length)
        {
            if(offset < 0)
            {
                throwOutOfRange("offset", offset, "offset must be non-negative");
            }
            if(offset + length > System.Buffer.ByteLength(b))
            {
                throwOutOfRange("length", length, "insufficient data beyond given offset in source array");
            }
            if(length > 0)
            {
                checkOverflow(length);
                System.Buffer.BlockCopy(b, offset, _bytes, _position, length);
                _position += length;
            }
            return this;
        }

        public bool getBool()
        {
            return get() == 1;
        }

        public void getBoolSeq(bool[] seq)
        {
            int len = System.Buffer.ByteLength(seq);
            checkUnderflow(len);
            System.Buffer.BlockCopy(_bytes, _position, seq, 0, len);
            _position += len;
        }

        public ByteBuffer putBool(bool b)
        {
            return put(b ? (byte)1 : (byte)0);
        }

        public ByteBuffer putBoolSeq(bool[] seq)
        {
            int len = System.Buffer.ByteLength(seq);
            checkOverflow(len);
            System.Buffer.BlockCopy(seq, 0, _bytes, _position, len);
            _position += len;
            return this;
        }

        [StructLayout(LayoutKind.Explicit)]
        struct ValBytes
        {
            [FieldOffset(0)]
            public short shortVal;

            [FieldOffset(0)]
            public int intVal;

            [FieldOffset(0)]
            public long longVal;

            [FieldOffset(0)]
            public float floatVal;

            [FieldOffset(0)]
            public double doubleVal;

            [FieldOffset(0)]
            public byte b0;
            [FieldOffset(1)]
            public byte b1;
            [FieldOffset(2)]
            public byte b2;
            [FieldOffset(3)]
            public byte b3;
            [FieldOffset(4)]
            public byte b4;
            [FieldOffset(5)]
            public byte b5;
            [FieldOffset(6)]
            public byte b6;
            [FieldOffset(7)]
            public byte b7;
        }

        public short getShort()
        {
            short v = getShort(_position);
            _position += 2;
            return v;
        }

#if !MANAGED
        unsafe
#endif
        public short getShort(int pos)
        {
            checkUnderflow(pos, 2);
            if(NO._o == _order)
            {
#if !MANAGED
                fixed(byte* p = &_bytes[pos])
                {
                    _valBytes.shortVal = *((short*)p);
                }
#else
                _valBytes.b0 = _bytes[pos];
                _valBytes.b1 = _bytes[pos + 1];
#endif
            }
            else
            {
                _valBytes.b1 = _bytes[pos];
                _valBytes.b0 = _bytes[pos + 1];
            }
            return _valBytes.shortVal;
        }

        public void getShortSeq(short[] seq)
        {
            int len = System.Buffer.ByteLength(seq);
            checkUnderflow(len);
            if(NO._o == _order)
            {
                System.Buffer.BlockCopy(_bytes, _position, seq, 0, len);
            }
            else
            {
                for(int i = 0; i < seq.Length; ++i)
                {
                    int index = _position + i * 2;
                    _valBytes.b1 = _bytes[index];
                    _valBytes.b0 = _bytes[index + 1];
                    seq[i] = _valBytes.shortVal;
                }
            }
            _position += len;
        }

#if !MANAGED
        unsafe
#endif
        public ByteBuffer putShort(short val)
        {
            checkOverflow(2);
            _valBytes.shortVal = val;
            if(NO._o == _order)
            {
#if !MANAGED
                fixed(byte* p = &_bytes[_position])
                {
                    *((short*)p) = _valBytes.shortVal;
                }
#else
                _bytes[_position] = _valBytes.b0;
                _bytes[_position + 1] = _valBytes.b1;
#endif
            }
            else
            {
                _bytes[_position + 1] = _valBytes.b0;
                _bytes[_position] = _valBytes.b1;
            }
            _position += 2;
            return this;
        }

        public ByteBuffer putShortSeq(short[] seq)
        {
            int len = System.Buffer.ByteLength(seq);
            checkOverflow(len);
            if(NO._o == _order)
            {
                System.Buffer.BlockCopy(seq, 0, _bytes, _position, len);
            }
            else
            {
                for(int i = 0; i < seq.Length; ++i)
                {
                    int index = _position + i * 2;
                    _valBytes.shortVal = seq[i];
                    _bytes[index + 1] = _valBytes.b0;
                    _bytes[index] = _valBytes.b1;
                }
            }
            _position += len;
            return this;
        }

#if !MANAGED
        unsafe
#endif
        public int getInt()
        {
            checkUnderflow(4);
            if(NO._o == _order)
            {
#if !MANAGED
                fixed(byte* p = &_bytes[_position])
                {
                    _valBytes.intVal = *((int*)p);
                }
#else
                _valBytes.b0 = _bytes[_position];
                _valBytes.b1 = _bytes[_position + 1];
                _valBytes.b2 = _bytes[_position + 2];
                _valBytes.b3 = _bytes[_position + 3];
#endif
            }
            else
            {
                _valBytes.b3 = _bytes[_position];
                _valBytes.b2 = _bytes[_position + 1];
                _valBytes.b1 = _bytes[_position + 2];
                _valBytes.b0 = _bytes[_position + 3];
            }
            _position += 4;
            return _valBytes.intVal;
        }

        public void getIntSeq(int[] seq)
        {
            int len = System.Buffer.ByteLength(seq);
            checkUnderflow(len);
            if(NO._o == _order)
            {
                System.Buffer.BlockCopy(_bytes, _position, seq, 0, len);
            }
            else
            {
                for(int i = 0; i < seq.Length; ++i)
                {
                    int index = _position + i * 4;
                    _valBytes.b3 = _bytes[index];
                    _valBytes.b2 = _bytes[index + 1];
                    _valBytes.b1 = _bytes[index + 2];
                    _valBytes.b0 = _bytes[index + 3];
                    seq[i] = _valBytes.intVal;
                }
            }
            _position += len;
        }

        public ByteBuffer putInt(int val)
        {
            putInt(_position, val);
            _position += 4;
            return this;
        }

#if !MANAGED
        unsafe
#endif
        public ByteBuffer putInt(int pos, int val)
        {
            if(pos < 0)
            {
                throwOutOfRange("pos", pos, "position must be non-negative");
            }
            if(pos + 4 > _limit)
            {
                throwOutOfRange("pos", pos, "position must be less than limit - 4");
            }
            _valBytes.intVal = val;
            if(NO._o == _order)
            {
#if !MANAGED
                fixed(byte* p = &_bytes[pos])
                {
                    *((int*)p) = _valBytes.intVal;
                }
#else
                _bytes[pos] = _valBytes.b0;
                _bytes[pos + 1] = _valBytes.b1;
                _bytes[pos + 2] = _valBytes.b2;
                _bytes[pos + 3] = _valBytes.b3;
#endif
            }
            else
            {
                _bytes[pos + 3] = _valBytes.b0;
                _bytes[pos + 2] = _valBytes.b1;
                _bytes[pos + 1] = _valBytes.b2;
                _bytes[pos] = _valBytes.b3;
            }
            return this;
        }

        public ByteBuffer putIntSeq(int[] seq)
        {
            int len = System.Buffer.ByteLength(seq);
            checkOverflow(len);
            if(NO._o == _order)
            {
                System.Buffer.BlockCopy(seq, 0, _bytes, _position, len);
            }
            else
            {
                for(int i = 0; i < seq.Length; ++i)
                {
                    int index = _position + i * 4;
                    _valBytes.intVal = seq[i];
                    _bytes[index + 3] = _valBytes.b0;
                    _bytes[index + 2] = _valBytes.b1;
                    _bytes[index + 1] = _valBytes.b2;
                    _bytes[index] = _valBytes.b3;
                }
            }
            _position += len;
            return this;
        }

        public long getLong()
        {
            long v = getLong(_position);
            _position += 8;
            return v;
        }

#if !MANAGED
        unsafe
#endif
        public long getLong(int pos)
        {
            checkUnderflow(pos, 8);
            if(NO._o == _order)
            {
#if !MANAGED
                fixed(byte* p = &_bytes[pos])
                {
                    _valBytes.longVal = *((long*)p);
                }
#else
                _valBytes.b0 = _bytes[pos];
                _valBytes.b1 = _bytes[pos + 1];
                _valBytes.b2 = _bytes[pos + 2];
                _valBytes.b3 = _bytes[pos + 3];
                _valBytes.b4 = _bytes[pos + 4];
                _valBytes.b5 = _bytes[pos + 5];
                _valBytes.b6 = _bytes[pos + 6];
                _valBytes.b7 = _bytes[pos + 7];
#endif
            }
            else
            {
                _valBytes.b7 = _bytes[pos];
                _valBytes.b6 = _bytes[pos + 1];
                _valBytes.b5 = _bytes[pos + 2];
                _valBytes.b4 = _bytes[pos + 3];
                _valBytes.b3 = _bytes[pos + 4];
                _valBytes.b2 = _bytes[pos + 5];
                _valBytes.b1 = _bytes[pos + 6];
                _valBytes.b0 = _bytes[pos + 7];
            }
            return _valBytes.longVal;
        }

        public void getLongSeq(long[] seq)
        {
            int len = System.Buffer.ByteLength(seq);
            checkUnderflow(len);
            if(NO._o == _order)
            {
                System.Buffer.BlockCopy(_bytes, _position, seq, 0, len);
            }
            else
            {
                for(int i = 0; i < seq.Length; ++i)
                {
                    int index = _position + i * 8;
                    _valBytes.b7 = _bytes[index];
                    _valBytes.b6 = _bytes[index + 1];
                    _valBytes.b5 = _bytes[index + 2];
                    _valBytes.b4 = _bytes[index + 3];
                    _valBytes.b3 = _bytes[index + 4];
                    _valBytes.b2 = _bytes[index + 5];
                    _valBytes.b1 = _bytes[index + 6];
                    _valBytes.b0 = _bytes[index + 7];
                    seq[i] = _valBytes.longVal;
                }
            }
            _position += len;
        }

#if !MANAGED
        unsafe
#endif
        public ByteBuffer putLong(long val)
        {
            checkOverflow(8);
            _valBytes.longVal = val;
            if(NO._o == _order)
            {
#if !MANAGED
                fixed(byte* p = &_bytes[_position])
                {
                    *((long*)p) = _valBytes.longVal;
                }
#else
                _bytes[_position] = _valBytes.b0;
                _bytes[_position + 1] = _valBytes.b1;
                _bytes[_position + 2] = _valBytes.b2;
                _bytes[_position + 3] = _valBytes.b3;
                _bytes[_position + 4] = _valBytes.b4;
                _bytes[_position + 5] = _valBytes.b5;
                _bytes[_position + 6] = _valBytes.b6;
                _bytes[_position + 7] = _valBytes.b7;
#endif
            }
            else
            {
                _bytes[_position + 7] = _valBytes.b0;
                _bytes[_position + 6] = _valBytes.b1;
                _bytes[_position + 5] = _valBytes.b2;
                _bytes[_position + 4] = _valBytes.b3;
                _bytes[_position + 3] = _valBytes.b4;
                _bytes[_position + 2] = _valBytes.b5;
                _bytes[_position + 1] = _valBytes.b6;
                _bytes[_position] = _valBytes.b7;
            }
            _position += 8;
            return this;
        }

        public ByteBuffer putLongSeq(long[] seq)
        {
            int len = System.Buffer.ByteLength(seq);
            checkOverflow(len);
            if(NO._o == _order)
            {
                System.Buffer.BlockCopy(seq, 0, _bytes, _position, len);
            }
            else
            {
                for(int i = 0; i < seq.Length; ++i)
                {
                    int index = _position + i * 8;
                    _valBytes.longVal = seq[i];
                    _bytes[index + 7] = _valBytes.b0;
                    _bytes[index + 6] = _valBytes.b1;
                    _bytes[index + 5] = _valBytes.b2;
                    _bytes[index + 4] = _valBytes.b3;
                    _bytes[index + 3] = _valBytes.b4;
                    _bytes[index + 2] = _valBytes.b5;
                    _bytes[index + 1] = _valBytes.b6;
                    _bytes[index] = _valBytes.b7;
                }
            }
            _position += len;
            return this;
        }

#if !MANAGED
        unsafe
#endif
        public float getFloat()
        {
            checkUnderflow(4);
            if(NO._o == _order)
            {
#if !MANAGED
                fixed(byte* p = &_bytes[_position])
                {
                    _valBytes.floatVal = *((float*)p);
                }
#else
                _valBytes.b0 = _bytes[_position];
                _valBytes.b1 = _bytes[_position + 1];
                _valBytes.b2 = _bytes[_position + 2];
                _valBytes.b3 = _bytes[_position + 3];
#endif
            }
            else
            {
                _valBytes.b3 = _bytes[_position];
                _valBytes.b2 = _bytes[_position + 1];
                _valBytes.b1 = _bytes[_position + 2];
                _valBytes.b0 = _bytes[_position + 3];
            }
            _position += 4;
            return _valBytes.floatVal;
        }

        public void getFloatSeq(float[] seq)
        {
            int len = System.Buffer.ByteLength(seq);
            checkUnderflow(len);
            if(NO._o == _order)
            {
                System.Buffer.BlockCopy(_bytes, _position, seq, 0, len);
            }
            else
            {
                for(int i = 0; i < seq.Length; ++i)
                {
                    int index = _position + i * 4;
                    _valBytes.b3 = _bytes[index];
                    _valBytes.b2 = _bytes[index + 1];
                    _valBytes.b1 = _bytes[index + 2];
                    _valBytes.b0 = _bytes[index + 3];
                    seq[i] = _valBytes.floatVal;
                }
            }
            _position += len;
        }

#if !MANAGED
        unsafe
#endif
        public ByteBuffer putFloat(float val)
        {
            checkOverflow(4);
            _valBytes.floatVal = val;
            if(NO._o == _order)
            {
#if !MANAGED
                fixed(byte* p = &_bytes[_position])
                {
                    *((float*)p) = _valBytes.floatVal;
                }
#else
                _bytes[_position] = _valBytes.b0;
                _bytes[_position + 1] = _valBytes.b1;
                _bytes[_position + 2] = _valBytes.b2;
                _bytes[_position + 3] = _valBytes.b3;
#endif
            }
            else
            {
                _bytes[_position + 3] = _valBytes.b0;
                _bytes[_position + 2] = _valBytes.b1;
                _bytes[_position + 1] = _valBytes.b2;
                _bytes[_position] = _valBytes.b3;
            }
            _position += 4;
            return this;
        }

        public ByteBuffer putFloatSeq(float[] seq)
        {
            int len = System.Buffer.ByteLength(seq);
            checkOverflow(len);
            if(NO._o == _order)
            {
                System.Buffer.BlockCopy(seq, 0, _bytes, _position, len);
            }
            else
            {
                for(int i = 0; i < seq.Length; ++i)
                {
                    int index = _position + i * 4;
                    _valBytes.floatVal = seq[i];
                    _bytes[index + 3] = _valBytes.b0;
                    _bytes[index + 2] = _valBytes.b1;
                    _bytes[index + 1] = _valBytes.b2;
                    _bytes[index] = _valBytes.b3;
                }
            }
            _position += len;
            return this;
        }

#if !MANAGED
        unsafe
#endif
        public double getDouble()
        {
            checkUnderflow(8);
            if(NO._o == _order)
            {
#if !MANAGED
                fixed(byte* p = &_bytes[_position])
                {
                    _valBytes.doubleVal = *((double*)p);
                }
#else
                _valBytes.b0 = _bytes[_position];
                _valBytes.b1 = _bytes[_position + 1];
                _valBytes.b2 = _bytes[_position + 2];
                _valBytes.b3 = _bytes[_position + 3];
                _valBytes.b4 = _bytes[_position + 4];
                _valBytes.b5 = _bytes[_position + 5];
                _valBytes.b6 = _bytes[_position + 6];
                _valBytes.b7 = _bytes[_position + 7];
#endif
            }
            else
            {
                _valBytes.b7 = _bytes[_position];
                _valBytes.b6 = _bytes[_position + 1];
                _valBytes.b5 = _bytes[_position + 2];
                _valBytes.b4 = _bytes[_position + 3];
                _valBytes.b3 = _bytes[_position + 4];
                _valBytes.b2 = _bytes[_position + 5];
                _valBytes.b1 = _bytes[_position + 6];
                _valBytes.b0 = _bytes[_position + 7];
            }
            _position += 8;
            return _valBytes.doubleVal;
        }

        public void getDoubleSeq(double[] seq)
        {
            int len = System.Buffer.ByteLength(seq);
            checkUnderflow(len);
            if(NO._o == _order)
            {
                System.Buffer.BlockCopy(_bytes, _position, seq, 0, len);
            }
            else
            {
                for(int i = 0; i < seq.Length; ++i)
                {
                    int index = _position + i * 8;
                    _valBytes.b7 = _bytes[index];
                    _valBytes.b6 = _bytes[index + 1];
                    _valBytes.b5 = _bytes[index + 2];
                    _valBytes.b4 = _bytes[index + 3];
                    _valBytes.b3 = _bytes[index + 4];
                    _valBytes.b2 = _bytes[index + 5];
                    _valBytes.b1 = _bytes[index + 6];
                    _valBytes.b0 = _bytes[index + 7];
                    seq[i] = _valBytes.doubleVal;
                }
            }
            _position += len;
        }

#if !MANAGED
        unsafe
#endif
        public ByteBuffer putDouble(double val)
        {
            checkOverflow(8);
            _valBytes.doubleVal = val;
            if(NO._o == _order)
            {
#if !MANAGED
                fixed(byte* p = &_bytes[_position])
                {
                    *((double*)p) = _valBytes.doubleVal;
                }
#else
                _bytes[_position] = _valBytes.b0;
                _bytes[_position + 1] = _valBytes.b1;
                _bytes[_position + 2] = _valBytes.b2;
                _bytes[_position + 3] = _valBytes.b3;
                _bytes[_position + 4] = _valBytes.b4;
                _bytes[_position + 5] = _valBytes.b5;
                _bytes[_position + 6] = _valBytes.b6;
                _bytes[_position + 7] = _valBytes.b7;
#endif
            }
            else
            {
                _bytes[_position + 7] = _valBytes.b0;
                _bytes[_position + 6] = _valBytes.b1;
                _bytes[_position + 5] = _valBytes.b2;
                _bytes[_position + 4] = _valBytes.b3;
                _bytes[_position + 3] = _valBytes.b4;
                _bytes[_position + 2] = _valBytes.b5;
                _bytes[_position + 1] = _valBytes.b6;
                _bytes[_position] = _valBytes.b7;
            }
            _position += 8;
            return this;
        }

        public ByteBuffer putDoubleSeq(double[] seq)
        {
            int len = System.Buffer.ByteLength(seq);
            checkOverflow(len);
            if(NO._o == _order)
            {
                System.Buffer.BlockCopy(seq, 0, _bytes, _position, len);
            }
            else
            {
                for(int i = 0; i < seq.Length; ++i)
                {
                    int index = _position + i * 8;
                    _valBytes.doubleVal = seq[i];
                    _bytes[index + 7] = _valBytes.b0;
                    _bytes[index + 6] = _valBytes.b1;
                    _bytes[index + 5] = _valBytes.b2;
                    _bytes[index + 4] = _valBytes.b3;
                    _bytes[index + 3] = _valBytes.b4;
                    _bytes[index + 2] = _valBytes.b5;
                    _bytes[index + 1] = _valBytes.b6;
                    _bytes[index] = _valBytes.b7;
                }
            }
            _position += len;
            return this;
        }

        public byte[] rawBytes()
        {
            return _bytes;
        }

        public byte[] rawBytes(int offset, int len)
        {
            if(offset + len > _limit)
            {
                throw new InvalidOperationException("buffer underflow");
            }
            byte[] rc = new byte[len];
            Array.Copy(_bytes, offset, rc, 0, len);
            return rc;
        }

        private void checkUnderflow(int size)
        {
            if(_position + size > _limit)
            {
                throw new InvalidOperationException("buffer underflow");
            }
        }

        private void checkUnderflow(int pos, int size)
        {
            if(pos + size > _limit)
            {
                throw new InvalidOperationException("buffer underflow");
            }
        }

        private void checkOverflow(int size)
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
        private ValBytes _valBytes;
        private ByteOrder _order;

        private class NO // Native Order
        {
            static NO()
            {
                _o = BitConverter.IsLittleEndian ? ByteOrder.LITTLE_ENDIAN : ByteOrder.BIG_ENDIAN;
            }
            internal static readonly ByteOrder _o;

            private NO()
            {
            }
        }

        private static void throwOutOfRange(string param, object value, string message)
        {
            throw new ArgumentOutOfRangeException(param, value, message);
        }
    }
}
