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

        public static byte[] ToArray(this IList<ArraySegment<byte>> src)
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
}
