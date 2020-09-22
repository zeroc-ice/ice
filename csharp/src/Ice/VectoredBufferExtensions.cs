// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace ZeroC.Ice
{
    /// <summary>Provides public extensions methods to manage byte array segments.</summary>
    public static class VectoredBufferExtensions
    {
        /// <summary>Returns the sum of the count of all the array segments in the source enumerable.</summary>
        /// <param name="src">The list of segments.</param>
        /// <returns>The byte count of the segment list.</returns>
        public static int GetByteCount(this IEnumerable<ArraySegment<byte>> src)
        {
            int count = 0;
            foreach (ArraySegment<byte> segment in src)
            {
                count += segment.Count;
            }
            return count;
        }

        /// <summary>Returns an array segment with the requested bytes, starting at the given
        /// byte offset, if the required bytes are available in a single segment this method returns
        /// an slice of the segment without copying the data, otherwise the data is copied into
        /// a byte array and the segment is created from the copied data.</summary>
        /// <param name="src">The source segment list.</param>
        /// <param name="srcOffset">The zero-based byte offset into the source segment list.</param>
        /// <param name="count">The size in bytes of the returned segment.</param>
        /// <returns>A segment with the requested size.</returns>
        public static ArraySegment<byte> GetSegment(this IList<ArraySegment<byte>> src, int srcOffset, int count)
        {
            Debug.Assert(src.GetByteCount() >= srcOffset + count,
                @$"requested {count} bytes starting at offset {srcOffset
                    } but there is only {src.GetByteCount() - srcOffset} bytes remaining.");

            // Skip offset bytes into the source segment list
            int srcIndex = 0;
            int dstOffset = 0;
            byte[]? data = null;
            for (; srcIndex < src.Count; srcIndex++)
            {
                ArraySegment<byte> segment = src[srcIndex];
                if (segment.Count > srcOffset)
                {
                    if (segment.Count - srcOffset >= count)
                    {
                        // If the requested data is available from a single segment return an Slice
                        // of the segment.
                        return segment.Slice(srcOffset, count);
                    }
                    else
                    {
                        // The requested data spans several segments, allocate an array of the requested
                        // size and copy the data into it. We first copy the remaining of the current segment
                        // here and the rest of the data is copied in the loop bellow.
                        data = new byte[count];
                        int remaining = segment.Count - srcOffset;
                        Debug.Assert(segment.Array != null);
                        Buffer.BlockCopy(segment.Array, srcOffset + segment.Offset, data, 0, remaining);
                        dstOffset = remaining;
                        srcIndex++;
                        break;
                    }
                }
                else
                {
                    srcOffset -= segment.Count;
                }
            }

            Debug.Assert(data != null);
            for (; srcIndex < src.Count && count - dstOffset > 0; srcIndex++)
            {
                ArraySegment<byte> segment = src[srcIndex];
                int remaining = Math.Min(count - dstOffset, segment.Count);
                Debug.Assert(segment.Array != null);
                Buffer.BlockCopy(segment.Array, segment.Offset, data, dstOffset, remaining);
                dstOffset += remaining;
            }
            return data;
        }

        internal static ArraySegment<byte> AsArraySegment(this IList<ArraySegment<byte>> src)
        {
            if (src.Count == 1)
            {
                return src[0];
            }
            else
            {
                byte[] data = new byte[src.GetByteCount()];
                int offset = 0;
                foreach (ArraySegment<byte> segment in src)
                {
                    Debug.Assert(segment.Array != null);
                    Buffer.BlockCopy(segment.Array, segment.Offset, data, offset, segment.Count);
                    offset += segment.Count;
                }
                return data;
            }
        }

        internal static byte GetByte(this IEnumerable<ArraySegment<byte>> src, int index)
        {
            Debug.Assert(index >= 0);
            foreach (ArraySegment<byte> segment in src)
            {
                if (index < segment.Count)
                {
                    return segment[index];
                }
                else
                {
                    index -= segment.Count;
                    // and skip this segment
                }
            }
            throw new IndexOutOfRangeException("index not found in vectored buffer");
        }

        internal static List<ArraySegment<byte>> Slice(
            this IList<ArraySegment<byte>> src,
            OutputStream.Position from,
            OutputStream.Position to)
        {
            var dst = new List<ArraySegment<byte>>();
            if (from.Segment == to.Segment)
            {
                dst.Add(src[from.Segment].Slice(from.Offset, to.Offset - from.Offset));
            }
            else
            {
                ArraySegment<byte> segment = src[from.Segment].Slice(from.Offset);
                if (segment.Count > 0)
                {
                    dst.Add(segment);
                }
                for (int i = from.Segment + 1; i < to.Segment; i++)
                {
                    dst.Add(src[i]);
                }

                segment = src[to.Segment].Slice(0, to.Offset);
                if (segment.Count > 0)
                {
                    dst.Add(segment);
                }
            }
            return dst;
        }

        internal static List<ArraySegment<byte>> Slice(this IEnumerable<ArraySegment<byte>> src, int start)
        {
            if (start < 0)
            {
                throw new ArgumentOutOfRangeException($"{nameof(start)} must greater or equal to 0", nameof(start));
            }

            var result = new List<ArraySegment<byte>>();
            foreach (ArraySegment<byte> segment in src)
            {
                if (start == 0)
                {
                    result.Add(segment);
                }
                else if (segment.Count > start)
                {
                    result.Add(segment.Slice(start));
                    start = 0;
                }
                else
                {
                    start -= segment.Count; // and we skip this segment
                }
            }

            if (start > 0)
            {
                throw new ArgumentOutOfRangeException("start exceeds the buffer's length", nameof(start));
            }
            return result;
        }
    }
}
