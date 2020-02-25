//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace Ice
{
    public static class VectoredBufferExtensions
    {
        public static int GetBytesCount(this IList<ArraySegment<byte>> segments)
        {
            int count = 0;
            foreach (ArraySegment<byte> segment in segments)
            {
                count += segment.Count;
            }
            return count;
        }

        /// <summary> Fill the segments list with almost legth bytes from the source segment list starting
        /// at the given offset.</summary>
        /// <param name="src">The source segment list.</param>
        /// <param name="srcOffset">The zero-based byte offset into the source segments.</param>
        /// <param name="srcSize">The zie in bytes of the source list.</param>
        /// <param name="dst">The list of segments to fill with data from the source segments.</param>
        /// <param name="count">The number of bytes to fill the segment lists with.</param>
        public static void GetSegments(this IList<ArraySegment<byte>> src, int srcOffset, int srcSize,
            IList<ArraySegment<byte>> dst, int count)
        {
            Debug.Assert(count > 0 && count <= srcSize - srcOffset,
                $"count: {count} srcSize: {srcSize} srcOffset: {srcOffset}");
            dst.Clear();

            int sz = 0;
            int i;
            for (i = 0; i < src.Count && sz < count; i++)
            {
                ArraySegment<byte> segment = src[i];
                if (segment.Count > srcOffset)
                {
                    int remaining = segment.Count - srcOffset;
                    dst.Add(segment.Slice(srcOffset, remaining));
                    sz += remaining;
                    break;
                }
                else
                {
                    srcOffset -= segment.Count;
                }
            }

            for (i += 1; i < src.Count && sz < count; i++)
            {
                ArraySegment<byte> segment = src[i];
                if (segment.Count > count - sz)
                {
                    dst.Add(segment.Slice(0, count - sz));
                    break;
                }
                else
                {
                    dst.Add(segment);
                    sz += segment.Count;
                }
            }
        }

        /// <summary>Returns and array segment with the required bytes, if the required bytes
        /// are available in the first segment this method returns an slice of the first segment
        /// without  the data, otherwise the data is copied into a byte array and the segment is
        /// created from the copied data.</summary>
        /// <param name="src">The source segment list.</param>
        /// <param name="size">The size in bytes of the returned segment.</param>
        /// <returns>A segment with the required size.</returns>
        public static ArraySegment<byte> GetSegment(this IList<ArraySegment<byte>> src, int size)
        {
            Debug.Assert(src.Count > 0);
            ArraySegment<byte> segment = src[0];
            if (segment.Count >= size)
            {
                return segment.Slice(0, size);
            }
            else
            {
                byte[] data = new byte[size];

                int offset = 0;
                for (int i = 0; i < src.Count && offset < size; i++)
                {
                    segment = src[i];
                    Buffer.BlockCopy(segment.Array, 0, data, offset, Math.Min(segment.Count, size - offset));
                    offset += segment.Count;
                }
                return data;
            }
        }
    }
}
