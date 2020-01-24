//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Runtime.InteropServices;

namespace IceInternal
{
    public class ByteBuffer
    {

        public ByteBuffer() => _order = ByteOrder.BIG_ENDIAN;

        public enum ByteOrder { BIG_ENDIAN, LITTLE_ENDIAN };

        public static ByteOrder NativeOrder() => NO.O;

        public ByteOrder Order() => _order;

        public ByteBuffer Order(ByteOrder bo)
        {
            _order = bo;
            return this;
        }

        public static ByteBuffer Allocate(int capacity)
        {
            if (capacity < 0)
            {
                ThrowOutOfRange("capacity", capacity, "capacity must be non-negative");
            }
            var ret = new ByteBuffer();
            ret._position = 0;
            ret._limit = capacity;
            ret._capacity = capacity;
            ret._bytes = new byte[capacity];
            ret._valBytes = new ValBytes();
            return ret;
        }

        public static ByteBuffer Wrap(byte[] bytes)
        {
            var ret = new ByteBuffer();
            ret._position = 0;
            ret._limit = bytes.Length;
            ret._capacity = bytes.Length;
            ret._bytes = bytes;
            ret._valBytes = new ValBytes();
            return ret;
        }

        public int Position() => _position;

        public ByteBuffer Position(int pos)
        {
            if (pos < 0)
            {
                ThrowOutOfRange("pos", pos, "position must be non-negative");
            }
            if (pos > _limit)
            {
                ThrowOutOfRange("pos", pos, "position must be less than limit");
            }
            _position = pos;
            return this;
        }

        public int Limit() => _limit;

        public ByteBuffer Limit(int newLimit)
        {
            if (newLimit < 0)
            {
                ThrowOutOfRange("newLimit", newLimit, "limit must be non-negative");
            }
            if (newLimit > _capacity)
            {
                ThrowOutOfRange("newLimit", newLimit, "limit must be less than capacity");
            }
            _limit = newLimit;
            return this;
        }

        public void Clear()
        {
            _position = 0;
            _limit = _capacity;
        }

        public void Flip()
        {
            _limit = _position;
            _position = 0;
        }

        public void Compact()
        {
            if (_position < _limit)
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

        public int Remaining() => _limit - _position;

        public bool HasRemaining() => _position < _limit;

        public int Capacity() => _capacity;

        public byte[] ToArray()
        {
            int len = Remaining();
            byte[] rc = new byte[len];
            System.Buffer.BlockCopy(_bytes, 0, rc, 0, len);
            return rc;
        }

        public byte[] ToArray(int startIndex, int length)
        {
            if (startIndex < 0)
            {
                ThrowOutOfRange("startIndex", startIndex, "startIndex must be non-negative");
            }
            if (startIndex >= _position)
            {
                ThrowOutOfRange("startIndex", startIndex, "startIndex must be less than position");
            }
            if (length < 0)
            {
                ThrowOutOfRange("length", length, "length must be non-negative");
            }
            if (startIndex + length > _position)
            {
                throw new ArgumentException("startIndex + length must not exceed end mark of buffer");
            }
            byte[] rc = new byte[length];
            System.Buffer.BlockCopy(_bytes, startIndex, rc, 0, length);
            return rc;
        }

        public ByteBuffer Put(ByteBuffer buf)
        {
            int len = buf.Remaining();
            CheckOverflow(len);
            System.Buffer.BlockCopy(buf._bytes, buf._position, _bytes, _position, len);
            _position += len;
            return this;
        }

        public byte Get()
        {
            CheckUnderflow(1);
            return System.Buffer.GetByte(_bytes, _position++);
        }

        public byte Get(int pos) => System.Buffer.GetByte(_bytes, pos);

        public ByteBuffer Get(byte[] b) => Get(b, 0, b.Length);

        public ByteBuffer Get(byte[] b, int offset, int length)
        {
            if (offset < 0)
            {
                ThrowOutOfRange("offset", offset, "offset must be non-negative");
            }
            if (offset + length > b.Length)
            {
                ThrowOutOfRange("length", length, "insufficient room beyond given offset in destination array");
            }
            CheckUnderflow(length);
            System.Buffer.BlockCopy(_bytes, _position, b, offset, length);
            _position += length;
            return this;
        }

        public ByteBuffer Put(byte b)
        {
            CheckOverflow(1);
            _bytes[_position++] = b;
            return this;
        }

        public ByteBuffer Put(int pos, byte b)
        {
            _bytes[pos] = b;
            return this;
        }

        public ByteBuffer Put(byte[] b) => Put(b, 0, b.Length);

        public ByteBuffer Put(byte[] b, int offset, int length)
        {
            if (offset < 0)
            {
                ThrowOutOfRange("offset", offset, "offset must be non-negative");
            }
            if (offset + length > b.Length)
            {
                ThrowOutOfRange("length", length, "insufficient data beyond given offset in source array");
            }
            if (length > 0)
            {
                CheckOverflow(length);
                System.Buffer.BlockCopy(b, offset, _bytes, _position, length);
                _position += length;
            }
            return this;
        }

        public bool GetBool() => Get() == 1;

        public void GetBoolSeq(bool[] seq)
        {
            int len = System.Buffer.ByteLength(seq);
            CheckUnderflow(len);
            System.Buffer.BlockCopy(_bytes, _position, seq, 0, len);
            _position += len;
        }

        public ByteBuffer PutBool(bool b) => Put(b ? (byte)1 : (byte)0);

        public ByteBuffer PutBoolSeq(bool[] seq)
        {
            int len = System.Buffer.ByteLength(seq);
            CheckOverflow(len);
            System.Buffer.BlockCopy(seq, 0, _bytes, _position, len);
            _position += len;
            return this;
        }

        [StructLayout(LayoutKind.Explicit)]
        public struct ValBytes
        {
            [FieldOffset(0)]
            public short ShortVal;

            [FieldOffset(0)]
            public int IntVal;

            [FieldOffset(0)]
            public long LongVal;

            [FieldOffset(0)]
            public float FloatVal;

            [FieldOffset(0)]
            public double DoubleVal;

            [FieldOffset(0)]
            public byte B0;
            [FieldOffset(1)]
            public byte B1;
            [FieldOffset(2)]
            public byte B2;
            [FieldOffset(3)]
            public byte B3;
            [FieldOffset(4)]
            public byte B4;
            [FieldOffset(5)]
            public byte B5;
            [FieldOffset(6)]
            public byte B6;
            [FieldOffset(7)]
            public byte B7;
        }

        public short GetShort()
        {
            short v = GetShort(_position);
            _position += 2;
            return v;
        }

        public short GetShort(int pos)
        {
            CheckUnderflow(pos, 2);
            if (NO.O == _order)
            {
                _valBytes.B0 = _bytes[pos];
                _valBytes.B1 = _bytes[pos + 1];
            }
            else
            {
                _valBytes.B1 = _bytes[pos];
                _valBytes.B0 = _bytes[pos + 1];
            }
            return _valBytes.ShortVal;
        }

        public void GetShortSeq(short[] seq)
        {
            int len = System.Buffer.ByteLength(seq);
            CheckUnderflow(len);
            if (NO.O == _order)
            {
                System.Buffer.BlockCopy(_bytes, _position, seq, 0, len);
            }
            else
            {
                for (int i = 0; i < seq.Length; ++i)
                {
                    int index = _position + (i * 2);
                    _valBytes.B1 = _bytes[index];
                    _valBytes.B0 = _bytes[index + 1];
                    seq[i] = _valBytes.ShortVal;
                }
            }
            _position += len;
        }

        public ByteBuffer PutShort(short val)
        {
            CheckOverflow(2);
            _valBytes.ShortVal = val;
            if (NO.O == _order)
            {
                _bytes[_position] = _valBytes.B0;
                _bytes[_position + 1] = _valBytes.B1;
            }
            else
            {
                _bytes[_position + 1] = _valBytes.B0;
                _bytes[_position] = _valBytes.B1;
            }
            _position += 2;
            return this;
        }

        public ByteBuffer PutShortSeq(short[] seq)
        {
            int len = System.Buffer.ByteLength(seq);
            CheckOverflow(len);
            if (NO.O == _order)
            {
                System.Buffer.BlockCopy(seq, 0, _bytes, _position, len);
            }
            else
            {
                for (int i = 0; i < seq.Length; ++i)
                {
                    int index = _position + (i * 2);
                    _valBytes.ShortVal = seq[i];
                    _bytes[index + 1] = _valBytes.B0;
                    _bytes[index] = _valBytes.B1;
                }
            }
            _position += len;
            return this;
        }

        public int GetInt()
        {
            CheckUnderflow(4);
            if (NO.O == _order)
            {
                _valBytes.B0 = _bytes[_position];
                _valBytes.B1 = _bytes[_position + 1];
                _valBytes.B2 = _bytes[_position + 2];
                _valBytes.B3 = _bytes[_position + 3];
            }
            else
            {
                _valBytes.B3 = _bytes[_position];
                _valBytes.B2 = _bytes[_position + 1];
                _valBytes.B1 = _bytes[_position + 2];
                _valBytes.B0 = _bytes[_position + 3];
            }
            _position += 4;
            return _valBytes.IntVal;
        }

        public void GetIntSeq(int[] seq)
        {
            int len = System.Buffer.ByteLength(seq);
            CheckUnderflow(len);
            if (NO.O == _order)
            {
                System.Buffer.BlockCopy(_bytes, _position, seq, 0, len);
            }
            else
            {
                for (int i = 0; i < seq.Length; ++i)
                {
                    int index = _position + (i * 4);
                    _valBytes.B3 = _bytes[index];
                    _valBytes.B2 = _bytes[index + 1];
                    _valBytes.B1 = _bytes[index + 2];
                    _valBytes.B0 = _bytes[index + 3];
                    seq[i] = _valBytes.IntVal;
                }
            }
            _position += len;
        }

        public ByteBuffer PutInt(int val)
        {
            PutInt(_position, val);
            _position += 4;
            return this;
        }

        public ByteBuffer PutInt(int pos, int val)
        {
            if (pos < 0)
            {
                ThrowOutOfRange("pos", pos, "position must be non-negative");
            }
            if (pos + 4 > _limit)
            {
                ThrowOutOfRange("pos", pos, "position must be less than limit - 4");
            }
            _valBytes.IntVal = val;
            if (NO.O == _order)
            {
                _bytes[pos] = _valBytes.B0;
                _bytes[pos + 1] = _valBytes.B1;
                _bytes[pos + 2] = _valBytes.B2;
                _bytes[pos + 3] = _valBytes.B3;
            }
            else
            {
                _bytes[pos + 3] = _valBytes.B0;
                _bytes[pos + 2] = _valBytes.B1;
                _bytes[pos + 1] = _valBytes.B2;
                _bytes[pos] = _valBytes.B3;
            }
            return this;
        }

        public ByteBuffer PutIntSeq(int[] seq)
        {
            int len = System.Buffer.ByteLength(seq);
            CheckOverflow(len);
            if (NO.O == _order)
            {
                System.Buffer.BlockCopy(seq, 0, _bytes, _position, len);
            }
            else
            {
                for (int i = 0; i < seq.Length; ++i)
                {
                    int index = _position + (i * 4);
                    _valBytes.IntVal = seq[i];
                    _bytes[index + 3] = _valBytes.B0;
                    _bytes[index + 2] = _valBytes.B1;
                    _bytes[index + 1] = _valBytes.B2;
                    _bytes[index] = _valBytes.B3;
                }
            }
            _position += len;
            return this;
        }

        public long GetLong()
        {
            long v = GetLong(_position);
            _position += 8;
            return v;
        }

        public long GetLong(int pos)
        {
            CheckUnderflow(pos, 8);
            if (NO.O == _order)
            {
                _valBytes.B0 = _bytes[pos];
                _valBytes.B1 = _bytes[pos + 1];
                _valBytes.B2 = _bytes[pos + 2];
                _valBytes.B3 = _bytes[pos + 3];
                _valBytes.B4 = _bytes[pos + 4];
                _valBytes.B5 = _bytes[pos + 5];
                _valBytes.B6 = _bytes[pos + 6];
                _valBytes.B7 = _bytes[pos + 7];
            }
            else
            {
                _valBytes.B7 = _bytes[pos];
                _valBytes.B6 = _bytes[pos + 1];
                _valBytes.B5 = _bytes[pos + 2];
                _valBytes.B4 = _bytes[pos + 3];
                _valBytes.B3 = _bytes[pos + 4];
                _valBytes.B2 = _bytes[pos + 5];
                _valBytes.B1 = _bytes[pos + 6];
                _valBytes.B0 = _bytes[pos + 7];
            }
            return _valBytes.LongVal;
        }

        public void GetLongSeq(long[] seq)
        {
            int len = System.Buffer.ByteLength(seq);
            CheckUnderflow(len);
            if (NO.O == _order)
            {
                System.Buffer.BlockCopy(_bytes, _position, seq, 0, len);
            }
            else
            {
                for (int i = 0; i < seq.Length; ++i)
                {
                    int index = _position + (i * 8);
                    _valBytes.B7 = _bytes[index];
                    _valBytes.B6 = _bytes[index + 1];
                    _valBytes.B5 = _bytes[index + 2];
                    _valBytes.B4 = _bytes[index + 3];
                    _valBytes.B3 = _bytes[index + 4];
                    _valBytes.B2 = _bytes[index + 5];
                    _valBytes.B1 = _bytes[index + 6];
                    _valBytes.B0 = _bytes[index + 7];
                    seq[i] = _valBytes.LongVal;
                }
            }
            _position += len;
        }

        public ByteBuffer PutLong(long val)
        {
            CheckOverflow(8);
            _valBytes.LongVal = val;
            if (NO.O == _order)
            {
                _bytes[_position] = _valBytes.B0;
                _bytes[_position + 1] = _valBytes.B1;
                _bytes[_position + 2] = _valBytes.B2;
                _bytes[_position + 3] = _valBytes.B3;
                _bytes[_position + 4] = _valBytes.B4;
                _bytes[_position + 5] = _valBytes.B5;
                _bytes[_position + 6] = _valBytes.B6;
                _bytes[_position + 7] = _valBytes.B7;
            }
            else
            {
                _bytes[_position + 7] = _valBytes.B0;
                _bytes[_position + 6] = _valBytes.B1;
                _bytes[_position + 5] = _valBytes.B2;
                _bytes[_position + 4] = _valBytes.B3;
                _bytes[_position + 3] = _valBytes.B4;
                _bytes[_position + 2] = _valBytes.B5;
                _bytes[_position + 1] = _valBytes.B6;
                _bytes[_position] = _valBytes.B7;
            }
            _position += 8;
            return this;
        }

        public ByteBuffer PutLongSeq(long[] seq)
        {
            int len = System.Buffer.ByteLength(seq);
            CheckOverflow(len);
            if (NO.O == _order)
            {
                System.Buffer.BlockCopy(seq, 0, _bytes, _position, len);
            }
            else
            {
                for (int i = 0; i < seq.Length; ++i)
                {
                    int index = _position + (i * 8);
                    _valBytes.LongVal = seq[i];
                    _bytes[index + 7] = _valBytes.B0;
                    _bytes[index + 6] = _valBytes.B1;
                    _bytes[index + 5] = _valBytes.B2;
                    _bytes[index + 4] = _valBytes.B3;
                    _bytes[index + 3] = _valBytes.B4;
                    _bytes[index + 2] = _valBytes.B5;
                    _bytes[index + 1] = _valBytes.B6;
                    _bytes[index] = _valBytes.B7;
                }
            }
            _position += len;
            return this;
        }

        public float GetFloat()
        {
            CheckUnderflow(4);
            if (NO.O == _order)
            {
                _valBytes.B0 = _bytes[_position];
                _valBytes.B1 = _bytes[_position + 1];
                _valBytes.B2 = _bytes[_position + 2];
                _valBytes.B3 = _bytes[_position + 3];
            }
            else
            {
                _valBytes.B3 = _bytes[_position];
                _valBytes.B2 = _bytes[_position + 1];
                _valBytes.B1 = _bytes[_position + 2];
                _valBytes.B0 = _bytes[_position + 3];
            }
            _position += 4;
            return _valBytes.FloatVal;
        }

        public void GetFloatSeq(float[] seq)
        {
            int len = System.Buffer.ByteLength(seq);
            CheckUnderflow(len);
            if (NO.O == _order)
            {
                System.Buffer.BlockCopy(_bytes, _position, seq, 0, len);
            }
            else
            {
                for (int i = 0; i < seq.Length; ++i)
                {
                    int index = _position + (i * 4);
                    _valBytes.B3 = _bytes[index];
                    _valBytes.B2 = _bytes[index + 1];
                    _valBytes.B1 = _bytes[index + 2];
                    _valBytes.B0 = _bytes[index + 3];
                    seq[i] = _valBytes.FloatVal;
                }
            }
            _position += len;
        }

        public ByteBuffer PutFloat(float val)
        {
            CheckOverflow(4);
            _valBytes.FloatVal = val;
            if (NO.O == _order)
            {
                _bytes[_position] = _valBytes.B0;
                _bytes[_position + 1] = _valBytes.B1;
                _bytes[_position + 2] = _valBytes.B2;
                _bytes[_position + 3] = _valBytes.B3;
            }
            else
            {
                _bytes[_position + 3] = _valBytes.B0;
                _bytes[_position + 2] = _valBytes.B1;
                _bytes[_position + 1] = _valBytes.B2;
                _bytes[_position] = _valBytes.B3;
            }
            _position += 4;
            return this;
        }

        public ByteBuffer PutFloatSeq(float[] seq)
        {
            int len = System.Buffer.ByteLength(seq);
            CheckOverflow(len);
            if (NO.O == _order)
            {
                System.Buffer.BlockCopy(seq, 0, _bytes, _position, len);
            }
            else
            {
                for (int i = 0; i < seq.Length; ++i)
                {
                    int index = _position + (i * 4);
                    _valBytes.FloatVal = seq[i];
                    _bytes[index + 3] = _valBytes.B0;
                    _bytes[index + 2] = _valBytes.B1;
                    _bytes[index + 1] = _valBytes.B2;
                    _bytes[index] = _valBytes.B3;
                }
            }
            _position += len;
            return this;
        }

        public double GetDouble()
        {
            CheckUnderflow(8);
            if (NO.O == _order)
            {
                _valBytes.B0 = _bytes[_position];
                _valBytes.B1 = _bytes[_position + 1];
                _valBytes.B2 = _bytes[_position + 2];
                _valBytes.B3 = _bytes[_position + 3];
                _valBytes.B4 = _bytes[_position + 4];
                _valBytes.B5 = _bytes[_position + 5];
                _valBytes.B6 = _bytes[_position + 6];
                _valBytes.B7 = _bytes[_position + 7];
            }
            else
            {
                _valBytes.B7 = _bytes[_position];
                _valBytes.B6 = _bytes[_position + 1];
                _valBytes.B5 = _bytes[_position + 2];
                _valBytes.B4 = _bytes[_position + 3];
                _valBytes.B3 = _bytes[_position + 4];
                _valBytes.B2 = _bytes[_position + 5];
                _valBytes.B1 = _bytes[_position + 6];
                _valBytes.B0 = _bytes[_position + 7];
            }
            _position += 8;
            return _valBytes.DoubleVal;
        }

        public void GetDoubleSeq(double[] seq)
        {
            int len = System.Buffer.ByteLength(seq);
            CheckUnderflow(len);
            if (NO.O == _order)
            {
                System.Buffer.BlockCopy(_bytes, _position, seq, 0, len);
            }
            else
            {
                for (int i = 0; i < seq.Length; ++i)
                {
                    int index = _position + (i * 8);
                    _valBytes.B7 = _bytes[index];
                    _valBytes.B6 = _bytes[index + 1];
                    _valBytes.B5 = _bytes[index + 2];
                    _valBytes.B4 = _bytes[index + 3];
                    _valBytes.B3 = _bytes[index + 4];
                    _valBytes.B2 = _bytes[index + 5];
                    _valBytes.B1 = _bytes[index + 6];
                    _valBytes.B0 = _bytes[index + 7];
                    seq[i] = _valBytes.DoubleVal;
                }
            }
            _position += len;
        }

        public ByteBuffer PutDouble(double val)
        {
            CheckOverflow(8);
            _valBytes.DoubleVal = val;
            if (NO.O == _order)
            {
                _bytes[_position] = _valBytes.B0;
                _bytes[_position + 1] = _valBytes.B1;
                _bytes[_position + 2] = _valBytes.B2;
                _bytes[_position + 3] = _valBytes.B3;
                _bytes[_position + 4] = _valBytes.B4;
                _bytes[_position + 5] = _valBytes.B5;
                _bytes[_position + 6] = _valBytes.B6;
                _bytes[_position + 7] = _valBytes.B7;
            }
            else
            {
                _bytes[_position + 7] = _valBytes.B0;
                _bytes[_position + 6] = _valBytes.B1;
                _bytes[_position + 5] = _valBytes.B2;
                _bytes[_position + 4] = _valBytes.B3;
                _bytes[_position + 3] = _valBytes.B4;
                _bytes[_position + 2] = _valBytes.B5;
                _bytes[_position + 1] = _valBytes.B6;
                _bytes[_position] = _valBytes.B7;
            }
            _position += 8;
            return this;
        }

        public ByteBuffer PutDoubleSeq(double[] seq)
        {
            int len = System.Buffer.ByteLength(seq);
            CheckOverflow(len);
            if (NO.O == _order)
            {
                System.Buffer.BlockCopy(seq, 0, _bytes, _position, len);
            }
            else
            {
                for (int i = 0; i < seq.Length; ++i)
                {
                    int index = _position + (i * 8);
                    _valBytes.DoubleVal = seq[i];
                    _bytes[index + 7] = _valBytes.B0;
                    _bytes[index + 6] = _valBytes.B1;
                    _bytes[index + 5] = _valBytes.B2;
                    _bytes[index + 4] = _valBytes.B3;
                    _bytes[index + 3] = _valBytes.B4;
                    _bytes[index + 2] = _valBytes.B5;
                    _bytes[index + 1] = _valBytes.B6;
                    _bytes[index] = _valBytes.B7;
                }
            }
            _position += len;
            return this;
        }

        public byte[] RawBytes() => _bytes;

        public byte[] RawBytes(int offset, int len)
        {
            if (offset + len > _limit)
            {
                throw new InvalidOperationException("buffer underflow");
            }
            byte[] rc = new byte[len];
            Array.Copy(_bytes, offset, rc, 0, len);
            return rc;
        }

        private void CheckUnderflow(int size)
        {
            if (_position + size > _limit)
            {
                throw new InvalidOperationException("buffer underflow");
            }
        }

        private void CheckUnderflow(int pos, int size)
        {
            if (pos + size > _limit)
            {
                throw new InvalidOperationException("buffer underflow");
            }
        }

        private void CheckOverflow(int size)
        {
            if (_position + size > _limit)
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
            static NO() => O = BitConverter.IsLittleEndian ? ByteOrder.LITTLE_ENDIAN : ByteOrder.BIG_ENDIAN;
            internal static readonly ByteOrder O;

            private NO()
            {
            }
        }

        private static void ThrowOutOfRange(string param, object value, string message) =>
            throw new ArgumentOutOfRangeException(param, value, message);
    }
}
