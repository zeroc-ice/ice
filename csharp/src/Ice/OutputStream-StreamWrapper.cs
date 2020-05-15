//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
using System;
using System.Diagnostics;
using System.IO;

namespace ZeroC.Ice
{
    public sealed partial class OutputStream
    {
        // Adapts OutputStream to System.IO.Stream.
        // We use this class to serialize arbitrary .NET serializable classes into a Slice byte sequence.
        //
        // Slice sequences are encoded on the wire as a count of elements, followed by the sequence contents.
        //
        // 1.1 encoding: For arbitrary .NET classes, we do not know how big the sequence will be. To avoid excessive
        // data copying, this class maintains a private _data array of 254 bytes and, initially, writes data into that
        // array. If more than  254 bytes end up being written, we write a dummy sequence size of 255 (which occupies
        // five bytes on the wire) into the stream and, once this class is disposed, patch that size to match the actual
        // size. Otherwise, if the _data buffer contains fewer than 255 bytes when this class is disposed, we write the
        // sequence size as a single byte, followed by the contents of the _data buffer.
        //
        // 2.0 encoding: we write the size on DefaultSizeLength bytes and the data directly into the OutputStream
        // buffer.
        private sealed class StreamWrapper : Stream
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
            private readonly Position _startPos;
            private byte[]? _data; // always null with 2.0 encoding
            private int _pos;

            public override void Flush()
            {
                // _pos represents the size

                try
                {
                    if (_data != null)
                    {
                        Debug.Assert(_stream.OldEncoding);
                        Debug.Assert(_pos <= _data.Length);
                        _stream.WriteSize(_pos); // 1 byte size length
                        _stream.WriteByteSpan(_data.AsSpan(0, _pos));
                    }
                    else
                    {
                        // Patch previously-written dummy value.

                        if (_stream.OldEncoding)
                        {
                            Debug.Assert(_pos >= 255);
                            Span<byte> data = stackalloc byte[5];
                            data[0] = 255;
                            OutputStream.WriteInt(_pos, data.Slice(1, 4));
                            _stream.RewriteByteSpan(data, _startPos);
                        }
                        else
                        {
                            Span<byte> data = stackalloc byte[OutputStream.DefaultSizeLength];
                            OutputStream.WriteFixedLength20Size(_pos, data);
                            _stream.RewriteByteSpan(data, _startPos);
                        }
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
                            _stream.WriteByteSpan(_data.AsSpan(0, _pos));
                        }

                        _data = null;
                    }

                    // Write data passed by caller.
                    _stream.WriteByteSpan(buffer);
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
                            _stream.WriteByteSpan(_data.AsSpan(0, _pos));
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

            internal StreamWrapper(OutputStream stream)
            {
                _stream = stream;
                _pos = 0;
                _startPos = stream.Tail;
                if (_stream.OldEncoding)
                {
                    _data = new byte[254];
                }
                else
                {
                    // placeholder for future size
                    _stream.WriteByteSpan(stackalloc byte[OutputStream.DefaultSizeLength]);
                }
            }
        }
    }
}
