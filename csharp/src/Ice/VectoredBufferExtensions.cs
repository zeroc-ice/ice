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

        /// <summary>Fill the segment list with at most count bytes from the source segment list starting
        /// at the given offset.</summary>
        /// <param name="src">The source segment list.</param>
        /// <param name="srcOffset">The zero-based byte offset into the source segment list.</param>
        /// <param name="dst">The list of segments to fill with data from the source segments list.</param>
        /// <param name="count">The number of bytes to fill the segment lists with.</param>
        public static void
        FillSegments(this IList<ArraySegment<byte>> src, int srcOffset, IList<ArraySegment<byte>> dst, int count)
        {
            Debug.Assert(count > 0 && count <= src.GetBytesCount() - srcOffset,
                $"count: {count} srcSize: {src.GetBytesCount()} srcOffset: {srcOffset}");
            dst.Clear();

            int sz = 0;
            int i = 0;
            for (; i < src.Count && srcOffset > 0; i++)
            {
                ArraySegment<byte> segment = src[i];
                if (segment.Count > srcOffset)
                {
                    sz = Math.Min(count, segment.Count - srcOffset);
                    dst.Add(segment.Slice(srcOffset, sz));
                    i++;
                    break;
                }
                else
                {
                    srcOffset -= segment.Count;
                }
            }

            for (; i < src.Count && sz < count; i++)
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
            Debug.Assert(src.GetBytesCount() >= offset + count,
                $"requested {count} bytes starting at offset {offset} but there is only " +
                $"{src.GetBytesCount() - offset} bytes remaining.");

            // Skip offset bytes into the source segment list
            int remaining;
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
                        // The requested data spams serveral segments, allocate an array of the requested
                        // size and copy the data into it. We first copy the remainig of the current segment
                        // here and the rest of the data is copied in the loop bellow.
                        data = new byte[count];
                        remaining = segment.Count - offset;
                        Buffer.BlockCopy(segment.Array, offset, data, 0, remaining);
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
                remaining = Math.Min(count - offset, segment.Count);
                Buffer.BlockCopy(segment.Array, 0, data, offset, remaining);
                offset += remaining;
            }
            return data;
        }
    }
}
