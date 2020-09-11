//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace ZeroC.Ice
{
    public sealed partial class InputStream
    {
        // Adapts InputStream to System.IO.Stream.
        private sealed class StreamWrapper : System.IO.Stream
        {
            public override bool CanRead => true;
            public override bool CanSeek => false;
            public override bool CanWrite => false;
            public override long Length => throw new NotSupportedException();

            public override long Position
            {
                get => _pos;
                set => throw new NotSupportedException();
            }

            private readonly ReadOnlyMemory<byte> _buffer;
            private int _pos;

            public override void Flush()
            {
            }

            public override int Read(byte[] buffer, int offset, int count) => Read(buffer.AsSpan(offset, count));

            public override int Read(Span<byte> span)
            {
                try
                {
                    int length = Math.Min(span.Length, _buffer.Length - _pos);
                    _buffer.Span.Slice(_pos, length).CopyTo(span);
                    _pos += length;
                    return length;
                }
                catch (Exception ex)
                {
                    throw new System.IO.IOException("could not read from stream", ex);
                }
            }

            public override int ReadByte()
            {
                try
                {
                    return _buffer.Span[_pos++];
                }
                catch (Exception ex)
                {
                    throw new System.IO.IOException("could not read from stream", ex);
                }
            }

            public override long Seek(long offset, System.IO.SeekOrigin origin) => throw new NotSupportedException();
            public override void SetLength(long value) => throw new NotSupportedException();
            public override void Write(byte[] array, int offset, int count) => throw new NotSupportedException();
            public override void WriteByte(byte value) => throw new NotSupportedException();

            internal StreamWrapper(ReadOnlyMemory<byte> buffer) => _buffer = buffer;
        }
    }
}
