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
        /// <summary>Returns the sum of the count of all the array segments in
        /// source segment list.</summary>
        /// <param name="src">The source segment list.</param>
        /// <returns>The byte count of the segment list.</returns>
        public static int GetByteCount(this IList<ArraySegment<byte>> src)
        {
            int count = 0;
            foreach (ArraySegment<byte> segment in src)
            {
                count += segment.Count;
            }
            return count;
        }

        /// <summary>Fill the destination segment list with at most count bytes from the source segment list
        /// starting at the given offset. The destination list is fill with segments from the source list, or
        /// with slices of the segments in the source list if the offset is beyong the start of a segment or
        /// the count is reach before the end of a segment. The underlying arrays are own by the source list.</summary>
        /// <param name="src">The source list.</param>
        /// <param name="srcOffset">The zero-based byte offset into the source list.</param>
        /// <param name="dst">The destination list to fill with array segments from the source list.</param>
        /// <param name="count">The number of bytes to fill the destination lists with.</param>
        public static void FillSegments(this IList<ArraySegment<byte>> src, int srcOffset,
            IList<ArraySegment<byte>> dst, int count)
        {
            Debug.Assert(count > 0 && count <= src.GetByteCount() - srcOffset,
                $"count: {count} srcSize: {src.GetByteCount()} srcOffset: {srcOffset}");
            dst.Clear();

            int dstCount = 0;
            int srcIndex = 0;
            for (; srcIndex < src.Count && srcOffset > 0; srcIndex++)
            {
                ArraySegment<byte> segment = src[srcIndex];
                if (segment.Count > srcOffset)
                {
                    dstCount = Math.Min(count, segment.Count - srcOffset);
                    dst.Add(segment.Slice(srcOffset, dstCount));
                    srcIndex++;
                    break;
                }
                else
                {
                    srcOffset -= segment.Count;
                }
            }

            for (; srcIndex < src.Count && dstCount < count; srcIndex++)
            {
                ArraySegment<byte> segment = src[srcIndex];
                if (segment.Count > count - dstCount)
                {
                    dst.Add(segment.Slice(0, count - dstCount));
                    break;
                }
                else
                {
                    dst.Add(segment);
                    dstCount += segment.Count;
                }
            }
        }

        /// <summary>Returns and array segment with the requested bytes, starting at the given
        /// byte offset, if the required bytes are available in a single segment this method returns
        /// an slice of the segment without copying the data, otherwise the data is copied into
        /// a byte array and the segment is created from the copied data.</summary>
        /// <param name="src">The source segment list.</param>
        /// <param name="offset">The zero-based byte offset into the source segment list.</param>
        /// <param name="count">The size in bytes of the returned segment.</param>
        /// <returns>A segment with the requested size.</returns>
        public static ArraySegment<byte> GetSegment(this IList<ArraySegment<byte>> src, int offset, int count)
        {
            Debug.Assert(src.GetByteCount() >= offset + count,
                $"requested {count} bytes starting at offset {offset} but there is only " +
                $"{src.GetByteCount() - offset} bytes remaining.");

            // Skip offset bytes into the source segment list
            int i = 0;
            byte[]? data = null;
            for (; i < src.Count; i++)
            {
                ArraySegment<byte> segment = src[i];
                if (segment.Count > offset)
                {
                    if (segment.Count - offset >= count)
                    {
                        // If the requested data is available from a single segment return an Slice
                        // of the segment.
                        return segment.Slice(offset, count);
                    }
                    else
                    {
                        // The requested data spans serveral segments, allocate an array of the requested
                        // size and copy the data into it. We first copy the remainig of the current segment
                        // here and the rest of the data is copied in the loop bellow.
                        data = new byte[count];
                        int remaining = segment.Count - offset;
                        Buffer.BlockCopy(segment.Array, offset + segment.Offset, data, 0, remaining);
                        offset = remaining;
                        i++;
                        break;
                    }
                }
                else
                {
                    offset -= segment.Count;
                }
            }

            Debug.Assert(data != null);
            for (; i < src.Count && count - offset > 0; i++)
            {
                ArraySegment<byte> segment = src[i];
                int remaining = Math.Min(count - offset, segment.Count);
                Buffer.BlockCopy(segment.Array, segment.Offset, data, offset, remaining);
                offset += remaining;
            }
            return data;
        }
    }
}
