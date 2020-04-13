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
    // For output streams, we use a different strategy:
    //
    // Slice sequences are encoded on the wire as a count of elements, followed by the sequence contents. For arbitrary
    // .NET classes, we do not know how big the sequence will be. To avoid excessive data copying, this class maintains
    // a private _data array of 254 bytes and, initially, writes data into that array. If more than 254 bytes end up
    // being written, we write a dummy sequence size of 255 (which occupies five bytes on the wire) into the stream and,
    // once this class is disposed, patch that size to match the actual size. Otherwise, if the _data buffer contains
    // fewer than 255 bytes when this class is disposed, we write the sequence size as a single byte, followed by the
    // contents of the _data buffer.
    internal class OutputStreamWrapper : Stream
    {
        public override bool CanRead => false;

        public override bool CanWrite => true;

        public override bool CanSeek => false;

        public override long Length => throw new NotSupportedException();

        public override long Position
        {
            get => throw new NotSupportedException();

            set => throw new NotSupportedException();
        }

        private readonly OutputStream _stream;
        private OutputStream.Position _startPos;
        private byte[]? _data;
        private int _pos;

        public override void Flush()
        {
            try
            {
                if (_data != null)
                {
                    Debug.Assert(_pos <= _data.Length);
                    _stream.WriteSize(_pos);
                    _stream.WriteSpan(_data.AsSpan(0, _pos));
                }
                else
                {
                    _stream.RewriteSize(_pos, _startPos); // Patch previously-written dummy value.
                }
            }
            catch (Exception ex)
            {
                throw new IOException("could not flush stream", ex);
            }
        }

        public override int Read(byte[] buffer, int offset, int count) => throw new NotSupportedException();

        public override int ReadByte() => throw new NotSupportedException();

        public override long Seek(long offset, SeekOrigin origin) => throw new NotSupportedException();

        public override void SetLength(long value) => throw new NotSupportedException();

        public override void Write(byte[] array, int offset, int count) => Write(array.AsSpan(offset, count));

        public override void Write(ReadOnlySpan<byte> buffer)
        {
            try
            {
                if (_data != null)
                {
                    // If we can fit the data into the first 254 bytes, write it to _data.
                    if (buffer.Length <= _data.Length - _pos)
                    {
                        buffer.CopyTo(_data.AsSpan(_pos, buffer.Length));
                        _pos += buffer.Length;
                        return;
                    }

                    // Dummy size, until we know how big the stream really is and can patch the size.
                    _stream.WriteSize(255);
                    if (_pos > 0)
                    {
                        // Write the current contents of _data.
                        _stream.WriteSpan(_data.AsSpan(0, _pos));
                    }

                    _data = null;
                }

                // Write data passed by caller.
                _stream.WriteSpan(buffer);
                _pos += buffer.Length;
            }
            catch (Exception ex)
            {
                throw new IOException("could not write to stream", ex);
            }
        }

        public override void WriteByte(byte value)
        {
            try
            {
                if (_data != null)
                {
                    // If we can fit the data into the first 254 bytes, write it to _data.
                    if (_pos < _data.Length)
                    {
                        _data[_pos++] = value;
                        return;
                    }

                    // Dummy size, until we know how big the stream really is and can patch the size.
                    _stream.WriteSize(255);
                    if (_pos > 0)
                    {
                        // Write the current contents of _data.
                        _stream.WriteSpan(_data.AsSpan(0, _pos));
                    }

                    _data = null;
                }

                // Write data passed by caller.
                _stream.WriteByte(value);
                _pos += 1;
            }
            catch (Exception ex)
            {
                throw new IOException("could not write to stream", ex);
            }
        }

        internal OutputStreamWrapper(OutputStream stream)
        {
            _stream = stream;
            _data = new byte[254];
            _pos = 0;
            _startPos = stream.Tail;
        }
    }

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
