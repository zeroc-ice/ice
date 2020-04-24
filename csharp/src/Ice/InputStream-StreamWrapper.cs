//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.IO;

namespace Ice
{
    public sealed partial class InputStream
    {
        // Adapts InputStream to System.IO.Stream.
        private sealed class StreamWrapper : Stream
        {
            public override bool CanRead => true;
            public override bool CanSeek => false;
            public override bool CanWrite => false;
            public override long Length => throw new NotSupportedException();

            public override long Position
            {
                get => throw new NotSupportedException();
                set => throw new NotSupportedException();
            }

            private readonly InputStream _stream;

            public override void Flush()
            {
            }

            public override int Read(byte[] buffer, int offset, int count) => Read(buffer.AsSpan(offset, count));

            public override int Read(Span<byte> buffer)
            {
                try
                {
                    return _stream.ReadSpan(buffer);
                }
                catch (Exception ex)
                {
                    throw new IOException("could not read from stream", ex);
                }
            }

            public override int ReadByte()
            {
                try
                {
                    return _stream.ReadByte();
                }
                catch (Exception ex)
                {
                    throw new IOException("could not read from stream", ex);
                }
            }

            public override long Seek(long offset, SeekOrigin origin) => throw new NotSupportedException();
            public override void SetLength(long value) => throw new NotSupportedException();
            public override void Write(byte[] array, int offset, int count) => throw new NotSupportedException();
            public override void WriteByte(byte value) => throw new NotSupportedException();

            internal StreamWrapper(InputStream istr) => _stream = istr;
        }
    }
}
