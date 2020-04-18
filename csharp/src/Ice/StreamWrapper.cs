//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
using System;
using System.Diagnostics;
using System.IO;

namespace Ice
{
    // Classes to provide a System.IO.Stream interface on top of an Ice stream.
    //
    // We use this to serialize arbitrary .NET serializable classes into a Slice byte sequence.
    //
    // For input streams, this class is a wrapper around the InputStream class that passes all methods through.
    //

    internal class InputStreamWrapper : Stream
    {
        public override long Position
        {
            get => throw new NotSupportedException();

            set => throw new NotSupportedException();
        }

        public override long Length => throw new NotSupportedException();

        private readonly InputStream _stream;

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

        public override void Write(byte[] array, int offset, int count) => throw new NotSupportedException();

        public override void WriteByte(byte value) => throw new NotSupportedException();

        public override bool CanRead => true;

        public override bool CanWrite => false;

        public override bool CanSeek => false;

        public override void Flush()
        {
        }

        public override long Seek(long offset, SeekOrigin origin) => throw new NotSupportedException();

        public override void SetLength(long value) => throw new NotSupportedException();

        internal InputStreamWrapper(InputStream s) => _stream = s;
    }
}
