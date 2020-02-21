//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace Ice
{
    /// <summary>Represents a position if a VectoredBuffer, the position is compose of
    /// the segment index and the relative offset in the segment.</summary>
    public struct BufferPosition
    {
        /// <summary>Creates a new position from the segment and offset values.</summary>
        /// <param name="segment">The zero based index of the segment.</param>
        /// <param name="offset">The offset into the segment.</param>
        public BufferPosition(int segment, int offset)
        {
            Segment = segment;
            Offset = offset;
        }

        /// <summary>The zero based index of the segment.</summary>
        public int Segment;
        /// <summary>The offset into the segment.</summary>
        public int Offset;
    }

    /// <summary>A byte buffer class that holds data in a list of byte arrays,
    /// the buffer expands its capacity by adding new arrays to the list,
    /// this allow to grow without need to reallocate data.</summary>
    public class VectoredBuffer
    {
        /// <summary>Number of bytes that the buffer can hold.</summary>
        public int Capacity { get; private set; }

        public BufferPosition Pos
        {
            get => _position;
            set
            {
                int index = value.Segment;
                _currentSegmentOffset = 0;
                for (int i = 0; i < index; i++)
                {
                    _currentSegmentOffset += Segments[i].Count;
                }
                _currentSegment = Segments[index];

                if (value.Offset > _currentSegment.Count)
                {
                    throw new ArgumentException("The position offset is invalid for the current buffer", nameof(value));
                }
                _position = value;
            }
        }

        /// <summary>Number of bytes from current position to the end.</summary>
        public int Remaining => Size - (_position.Offset + _currentSegmentOffset);

        /// <summary>Determines the current size of the stream, this correspond
        /// to the number of bytes already writen to the stream.</summary>
        /// <value>The current size.</value>
        public int Size { get; private set; }

        /// <summary>List of array segments own by the buffer, the last segment size reflects the size
        /// of the underlying array until Prepare is call after that point it reflects the portion
        /// of the last segment that is in use.</summary>
        public IList<ArraySegment<byte>> Segments { get; }
        // The segment curretnly used by write operations
        private ArraySegment<byte> _currentSegment;
        // Cached offset of the current segment
        private int _currentSegmentOffset;
        private const int MinSegmentSize = 256;
        private BufferPosition _position;

        /// <summary>
        /// Create a new buffer if the initialData param is not null it will be used as the buffer
        /// first segment, otherwise and empty segment will be created as the first segment.</summary>
        /// <param name="initialData">Optional initial data for the buffer.</param>
        public VectoredBuffer(byte[]? initialData = null)
        {
            Segments = new List<ArraySegment<byte>>();
            _position = new BufferPosition(0, 0);
            if (initialData == null)
            {
                Segments.Add(new ArraySegment<byte>(new byte[MinSegmentSize]));
                Size = 0;
                Capacity = MinSegmentSize;
            }
            else
            {
                Segments.Add(new ArraySegment<byte>(initialData));
                Size = initialData.Length;
                Capacity = initialData.Length;
            }
            _currentSegment = Segments[0];
            _currentSegmentOffset = 0;
        }

        /// <summary>
        /// Create a buffer from a list of byte arrays, it array is added as a segment to the buffer,
        /// the Size and Capacity are set to the summ of the arrays lengths.</summary>
        /// <param name="segments"></param>
        public VectoredBuffer(byte[][] segments)
        {
            Segments = new List<ArraySegment<byte>>();
            _position = new BufferPosition(0, 0);
            foreach (byte[] segment in segments)
            {
                Segments.Add(segment);
                Size += segment.Length;
            }
            Capacity = Size;
            _currentSegment = Segments[0];
            _currentSegmentOffset = 0;
        }

        /// <summary>Add additional data segments to the stream, the segments are append to the end
        /// of the segments list, Prepare must be call before calling this to ensure there
        /// is not gaps when adding segments.</summary>
        /// <param name="segments">The list of segments to add to the buffer.</param>
        public void Add(IList<ArraySegment<byte>> segments)
        {
            foreach (ArraySegment<byte> segment in segments)
            {
                Segments.Add(segment);
                Size += segment.Count;
            }
            Capacity = Size;
        }

        public void Add(ArraySegment<byte> segment)
        {
            Segments.Add(segment);
            Size += segment.Count;
            Capacity = Size;
        }

        /// <summary>Advanced the buffer position the given number of bytes.</summary>
        /// <param name="size">The number of bytes to advance the buffer position.</param>
        public void Advance(int size)
        {
            Debug.Assert(size <= Remaining);
            while (size > 0)
            {
                ArraySegment<byte> segment = Segments[_position.Segment];
                int remaining = segment.Count - _position.Offset;
                if (remaining >= size)
                {
                    _position.Offset += size;
                    size -= remaining;
                }
                else
                {
                    size -= remaining;
                    _currentSegmentOffset += segment.Count;
                    _position.Offset = 0;
                    _position.Segment++;
                    _currentSegment = Segments[_position.Segment];
                }
            }
        }

        /// <summary>Release all the data hold by this buffer</summary>
        public void Clear()
        {
            Segments.Clear();
            Size = 0;
            Capacity = 0;
            _currentSegment = Array.Empty<byte>();
            _currentSegmentOffset = 0;
            _position.Segment = 0;
            _position.Offset = 0;
        }

        /// <summary> Fill the segments with almost legth bytes that from the buffer.</summary>
        /// <param name="segments">The list of segments to fill with data from the buffers</param>
        /// <param name="length">The number of bytes to fill the segments with.</param>
        public void FillSegments(IList<ArraySegment<byte>> segments, int length)
        {
            Debug.Assert(length > 0);
            segments.Clear();
            int remainig = Math.Min(length, _currentSegment.Count - _position.Offset);
            if (remainig > 0)
            {
                if (_position.Offset == 0 && remainig <= length)
                {
                    segments.Add(_currentSegment);
                }
                else
                {
                    segments.Add(_currentSegment.Slice(_position.Offset, remainig));
                }
                length -= remainig;
            }

            for (int i = _position.Segment + 1; length > 0 && i < Segments.Count; ++i)
            {
                ArraySegment<byte> segment = Segments[i];
                if (segment.Count > length)
                {
                    segments.Add(segment.Slice(0, length));
                }
                else
                {
                    segments.Add(segment);
                    length -= segment.Count;
                }
            }
        }

        public byte[] GetBytes(int offset, int size)
        {
            Debug.Assert(size <= offset + Size);
            byte[] data = new byte[size];
            int i;
            for (i = 0; i < Segments.Count; ++i)
            {
                ArraySegment<byte> segment = Segments[i];
                if (segment.Count < offset)
                {
                    offset -= segment.Count;
                }
                else
                {
                    int remaining = Math.Min(size, segment.Count - offset);
                    Buffer.BlockCopy(segment.Array, offset, data, 0, remaining);
                    size -= remaining;
                    break;
                }
            }

            for (i += 1; i < Segments.Count && size > 0; ++i)
            {
                ArraySegment<byte> segment = Segments[i];
                int remaining = Math.Min(size, segment.Count);
                Buffer.BlockCopy(segment.Array, 0, data, data.Length - size, remaining);
                size -= remaining;
            }
            return data;
        }

        /// <summary>Returns and array segment with the required bytes starting at
        /// the current position, if the required bytes are available in the current
        /// segment this method returns an slice of the current segment without copying
        /// the data, otherwise the data is copied into a byte array and the segment is
        /// created from the copied data.</summary>
        /// <param name="size">The size in bytes of the returned segment.</param>
        /// <returns>A segment with the required size.</returns>
        public ArraySegment<byte> GetSegment(int size)
        {
            if (_currentSegment.Count - _position.Offset >= size)
            {
                return _currentSegment.Slice(_position.Offset, size);
            }
            else
            {
                byte[] data = new byte[size];
                int remaining = _currentSegment.Count - _position.Offset;
                if (remaining > 0)
                {
                    Buffer.BlockCopy(_currentSegment.Array, _position.Offset, data, 0, remaining);
                }

                int offset = remaining;
                for (int i = _position.Segment + 1; i < Segments.Count && offset < size; ++i)
                {
                    ArraySegment<byte> segment = Segments[i];
                    Buffer.BlockCopy(segment.Array, 0, data, offset, Math.Min(segment.Count, size - offset));
                    offset += segment.Count;
                }
                return data;
            }
        }

        /// <summary>Prepare the buffer for writing its contents to a stream,
        /// slice the last segment to the written size and move the position
        /// to the begining.</summary>
        public void Prepare()
        {
            int offset = 0;
            for (int i = 1; i < Segments.Count; ++i)
            {
                offset += Segments[i - 1].Count;
            }
            ArraySegment<byte> last = Segments[Segments.Count - 1];
            if (last.Count + offset > Size)
            {
                Segments[Segments.Count - 1] = last.Slice(0, Size - offset);
            }
            Pos = new BufferPosition(0, 0);
        }

        /// <summary>Reset the buffer to the initial state, after reset the stream it holds a single
        /// segment of size MinSegmentSize.</summary>
        public void Reset()
        {
            Segments.Clear();
            Segments.Add(new ArraySegment<byte>(new byte[MinSegmentSize]));
            Size = 0;
            Capacity = MinSegmentSize;
            Pos = new BufferPosition(0, 0);
        }

        /// <summary>Write a byte at a given position of the stream.</summary>
        /// <param name="v">The byte to write,</param>
        /// <param name="pos">The position to write to.</param>
        public void RewriteByte(byte v, BufferPosition pos)
        {
            ArraySegment<byte> segment = Segments[pos.Segment];
            if (pos.Offset < segment.Count)
            {
                segment[pos.Offset] = v;
            }
            else
            {
                segment = Segments[pos.Segment + 1];
                segment[0] = v;
            }
        }

        // <summary>Write a byte at a given position of the stream.</summary>
        /// <param name="v">The int to write.</param>
        /// <param name="pos">The position to write to.</param>
        public void RewriteInt(int v, BufferPosition pos)
        {
            Span<byte> data = stackalloc byte[4];
            MemoryMarshal.Write(data, ref v);

            int offset = pos.Offset;
            ArraySegment<byte> segment = Segments[pos.Segment];
            int remaining = Math.Min(4, segment.Count - offset);
            if (remaining > 0)
            {
                data.Slice(0, remaining).CopyTo(segment.AsSpan(offset, remaining));
            }

            if (remaining < 4)
            {
                segment = Segments[pos.Segment + 1];
                data.Slice(remaining, 4 - remaining).CopyTo(segment.AsSpan(0, 4 - remaining));
            }
        }

        /// <summary>Return all the data as a byte array.</summary>
        /// <returns>A byte array with the contest of the buffer.</returns>
        public byte[] ToArray()
        {
            byte[] data = new byte[Size];
            int offset = 0;
            foreach (ArraySegment<byte> segment in Segments)
            {
                Buffer.BlockCopy(segment.Array, 0, data, offset, Math.Min(segment.Count, Size - offset));
                offset += segment.Count;
            }
            return data;
        }

        /// <summary>Write a byte to the current buffer position, the buffer is
        /// expanded if required, the position and Size are increase.</summary>
        /// <param name="v">The byte to write.</param>
        public void WriteByte(byte v)
        {
            Expand(1);
            int offset = _position.Offset;
            int remaining = _currentSegment.Count - offset;
            if (remaining > 0)
            {
                _currentSegment[offset] = v;
                _position.Offset++;
            }
            else
            {
                _currentSegmentOffset += _currentSegment.Count;
                _currentSegment = Segments[++_position.Segment];
                _currentSegment[0] = v;
                _position.Offset = 1;
            }
        }

        /// <summary>Write a boolean value to buffer, the value is write as a single
        /// byte 1 (true) 0 (false).</summary>
        /// <param name="v">The boolean value to write.</param>
        public void WriteBool(bool v) => WriteByte(v ? (byte)1 : (byte)0);

        /// <summary>Write a short value to the buffer, the value is write as two bytes.</summary>
        /// <param name="v">The short value to write.</param>
        public void WriteShort(short v) => WriteNumeric(v, 2);

        /// <summary>Write a int value to the buffer, the value is write as four bytes.</summary>
        /// <param name="v">The int value to write.</param>
        public void WriteInt(int v) => WriteNumeric(v, 4);

        /// <summary>Write a long value to the buffer, the value is write as eight bytes.</summary>
        /// <param name="v">The long value to write.</param>
        public void WriteLong(long v) => WriteNumeric(v, 8);

        /// <summary>Write a float value to the buffer, the value is write as four bytes.</summary>
        /// <param name="v">The float value to write.</param>
        public void WriteFloat(float v) => WriteNumeric(v, 4);

        /// <summary>Write a double value to the buffer, the value is write as eight bytes.</summary>
        /// <param name="v">The double value to write.</param>
        public void WriteDouble(double v) => WriteNumeric(v, 8);

        /// <summary>Write an array of bytes to the buffer.</summary>
        /// <param name="v">The array to write.</param>
        public void WriteByteSeq(byte[] v) => WriteSpan(v.AsSpan());

        /// <summary>Write an array of bool values to the buffer.</summary>
        /// <param name="v">The array to write.</param>
        public void WriteBoolSeq(bool[] v) => WriteNumericSeq(v);

        /// <summary>Write an array of short values to the buffer.</summary>
        /// <param name="v">The array to write.</param>
        public void WriteShortSeq(short[] v) => WriteNumericSeq(v);

        /// <summary>Write an array of int values to the buffer.</summary>
        /// <param name="v">The array to write.</param>
        public void WriteIntSeq(int[] v) => WriteNumericSeq(v);

        /// <summary>Write an array of long values to the buffer.</summary>
        /// <param name="v">The array to write.</param>
        public void WriteLongSeq(long[] v) => WriteNumericSeq(v);

        /// <summary>Write an array of float values to the buffer.</summary>
        /// <param name="v">The array to write.</param>
        public void WriteFloatSeq(float[] v) => WriteNumericSeq(v);

        /// <summary>Write an array of double values to the buffer.</summary>
        /// <param name="v">The array to write.</param>
        public void WriteDoubleSeq(double[] v) => WriteNumericSeq(v);

        /// <summary>Returns the distance in bytes from start position to the current position.</summary>
        /// <param name="start"></param>
        /// <returns>The distance in bytes from the current position to the start position.</returns>
        internal int Distance(BufferPosition start)
        {
            Debug.Assert(_position.Segment > start.Segment ||
                         (_position.Segment == start.Segment && _position.Offset > start.Offset));
            // Compute the size from a given buffer position to the
            // current buffer position
            if (start.Segment == _position.Segment)
            {
                // if both the start and end position are in the same array segment
                // just compute the offsets distance.
                return _position.Offset - start.Offset;
            }

            // If start and end position are in different array segments we need
            // to acumulate the size from start offset to the end of the start segment,
            // the size of the intermediary segments, and the current offset into
            // the last segment.
            ArraySegment<byte> segment = Segments[start.Segment];
            int size = segment.Count - start.Offset;
            for (int i = start.Segment + 1; i < _position.Segment; ++i)
            {
                size += Segments[i].Count;
            }
            return size + _position.Offset;
        }

        /// <summary>Write an span of bytes to the buffer, the buffer is expand of required, the size
        /// and position are increase according to the spam length.</summary>
        /// <param name="span">The data to write as a span of bytes.</param>
        internal void WriteSpan(Span<byte> span)
        {
            int length = span.Length;
            Expand(length);
            int offset = _position.Offset;
            int remaining = _currentSegment.Count - offset;
            if (remaining > 0)
            {
                int sz = Math.Min(length, remaining);
                if (length > remaining)
                {
                    span.Slice(0, remaining).CopyTo(_currentSegment.AsSpan(_position.Offset, sz));
                }
                else
                {
                    span.CopyTo(_currentSegment.AsSpan(_position.Offset, sz));
                }
                _position.Offset += sz;
                length -= sz;
            }

            if (length > 0)
            {
                _currentSegmentOffset += _currentSegment.Count;
                _currentSegment = Segments[++_position.Segment];
                span.Slice(remaining, length).CopyTo(_currentSegment.AsSpan(0, length));
                _position.Offset = length;
            }
        }

        /// <summary>Expand the stream to accept more data.</summary>
        /// <param name="n">The number of bytes to accommodate in the stream.</param>
        internal void Expand(int n)
        {
            int sz = _currentSegmentOffset + _position.Offset + n;
            if (sz > Size)
            {
                Size += n;
                if (sz > Capacity)
                {
                    byte[] buffer = new byte[Math.Max(n, _currentSegment.Count * 2)];
                    Segments.Add(buffer);
                    Capacity += buffer.Length;
                }
            }
        }

        /// <summary>Helper method used to write an array of numeric types to the stream.</summary>
        /// <param name="arr">The numeric array to write to the stream.</param>
        private void WriteNumericSeq(Array arr)
        {
            int length = Buffer.ByteLength(arr);
            Expand(length);
            int offset = _position.Offset;
            int remaining = Math.Min(_currentSegment.Count - offset, length);
            if (remaining > 0)
            {
                Buffer.BlockCopy(arr, 0, _currentSegment.Array, offset, remaining);
                _position.Offset += remaining;
            }
            length -= remaining;
            if (length > 0)
            {
                _currentSegmentOffset += _currentSegment.Count;
                _currentSegment = Segments[++_position.Segment];
                Buffer.BlockCopy(arr, remaining, _currentSegment.Array, 0, length);
                _position.Offset = length;
            }
        }

        /// <summary>Helper method used to write numeric types to the stream.</summary>
        /// <param name="v">The numeric value to write to the stream.</param>
        /// <param name="elementSize">The size in bytes of the numeric type.</param>
        private void WriteNumeric<T>(T v, int elementSize) where T : struct
        {
            Span<byte> data = stackalloc byte[elementSize];
            MemoryMarshal.Write(data, ref v);
            WriteSpan(data);
        }
    }
}
