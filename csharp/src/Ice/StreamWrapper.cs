//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
using System;
using System.Diagnostics;
using System.IO;

namespace IceInternal
{
    //
    // Classes to provide a System.IO.Stream interface on top of an Ice stream.
    // We use this to serialize arbitrary .NET serializable classes into
    // a Slice byte sequence.
    //
    // For input streams, this class is a wrapper around the InputStream
    // class that passes all methods through.
    //
    // For output streams, we use a different strategy:
    // Slice sequences are encoded on the wire as a count of elements, followed
    // by the sequence contents. For arbitrary .NET classes, we do not know how
    // big the sequence that is eventually written will be. To avoid excessive
    // data copying, this class maintains a private _bytes array of 254 bytes and,
    // initially, writes data into that array. If more than 254 bytes end up being
    // written, we write a dummy sequence size of 255 (which occupies five bytes
    // on the wire) into the stream and, once this class is disposed, patch
    // that size to match the actual size. Otherwise, if the _bytes buffer contains
    // fewer than 255 bytes when this class is disposed, we write the sequence size
    // as a single byte, followed by the contents of the _bytes buffer.
    //

    public class OutputStreamWrapper : Stream
    {
        public OutputStreamWrapper(Ice.OutputStream s)
        {
            _s = s;
            _bytes = new byte[254];
            _pos = 0;
            _length = 0;
        }

        public override int Read(byte[] buffer, int offset, int count)
        {
            Debug.Assert(false);
            return 0;
        }

        public override int ReadByte()
        {
            Debug.Assert(false);
            return 0;
        }

        public override void Write(byte[] array, int offset, int count)
        {
            Debug.Assert(array != null && offset >= 0 && count >= 0 && offset + count <= array.Length);
            try
            {
                if (_bytes != null)
                {
                    //
                    // If we can fit the data into the first 254 bytes, write it to _bytes.
                    //
                    if (count <= _bytes.Length - _pos)
                    {
                        System.Buffer.BlockCopy(array, offset, _bytes, _pos, count);
                        _pos += count;
                        return;
                    }

                    _s.WriteSize(255); // Dummy size, until we know how big the stream
                                       // really is and can patch the size.
                    if (_pos > 0)
                    {
                        //
                        // Write the current contents of _bytes.
                        //
                        _s.WriteSpan(_bytes.AsSpan(0, _pos));
                    }

                    _bytes = null;
                }

                //
                // Write data passed by caller.
                //
                _s.WriteSpan(array.AsSpan());
                _pos += count;
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
                if (_bytes != null)
                {
                    //
                    // If we can fit the data into the first 254 bytes, write it to _bytes.
                    //
                    if (_pos < _bytes.Length)
                    {
                        _bytes[_pos++] = value;
                        return;
                    }

                    _spos = _s.StartSize(); // Dummy size, until we know how big the stream
                                            // really is and can patch the size.
                    if (_pos > 0)
                    {
                        //
                        // Write the current contents of _bytes.
                        //
                        _s.WriteSpan(_bytes.AsSpan(0, _pos));
                    }

                    _bytes = null;
                }

                //
                // Write data passed by caller.
                //
                _s.WriteByte(value);
                _pos += 1;
            }
            catch (Exception ex)
            {
                throw new IOException("could not write to stream", ex);
            }
        }

        public override bool CanRead => false;

        public override bool CanWrite => true;

        public override bool CanSeek => false;

        public override void Flush()
        {
            try
            {
                if (_bytes != null)
                {
                    Debug.Assert(_pos <= _bytes.Length);
                    _s.WriteSize(_pos);
                    _s.WriteSpan(_bytes.AsSpan(0, _pos));
                }
                else
                {
                    _s.RewriteInt(_pos, _spos); // Patch previously-written dummy value.
                }
            }
            catch (Exception ex)
            {
                throw new IOException("could not flush stream", ex);
            }
        }

        public override long Length => _length;

        public override long Position
        {
            get => _pos;

            set => _ = Seek(value, SeekOrigin.Begin);
        }

        public override long Seek(long offset, SeekOrigin origin)
        {
            Debug.Assert(false);
            return 0;
        }

        public override void SetLength(long value)
        {
            Debug.Assert(value >= 0);
            _length = value;
        }

        private readonly Ice.OutputStream _s;
        private Ice.OutputStream.Position _spos;
        private byte[]? _bytes;
        private int _pos;
        private long _length;
    }

    public class InputStreamWrapper : Stream
    {
        public InputStreamWrapper(int size, Ice.InputStream s)
        {
            _s = s;
            _pos = 0;
            _length = size;
        }

        public override int Read(byte[] buffer, int offset, int count)
        {
            Debug.Assert(buffer != null && offset >= 0 && count >= 0 && offset + count <= buffer.Length);
            try
            {
                Span<byte> span = buffer.AsSpan(offset, count);
                _s.ReadSpan(span);
            }
            catch (System.Exception ex)
            {
                throw new IOException("could not read from stream", ex);
            }
            return count;
        }

        public override int ReadByte()
        {
            try
            {
                return _s.ReadByte();
            }
            catch (System.Exception ex)
            {
                throw new IOException("could not read from stream", ex);
            }
        }

        public override void Write(byte[] array, int offset, int count) => Debug.Assert(false);

        public override void WriteByte(byte value) => Debug.Assert(false);

        public override bool CanRead => true;

        public override bool CanWrite => false;

        public override bool CanSeek => true;

        public override void Flush()
        {
        }

        public override long Length => _length;

        public override long Position
        {
            get => _pos;

            set => _ = Seek(value, SeekOrigin.Begin);
        }

        public override long Seek(long offset, SeekOrigin origin)
        {
            // Deliberately no size check here--positioning beyond the limit of the stream is legal.
            switch (origin)
            {
                case SeekOrigin.Begin:
                    {
                        _pos = (int)offset;
                        break;
                    }
                case SeekOrigin.Current:
                    {
                        _pos += (int)offset;
                        break;
                    }
                case SeekOrigin.End:
                    {
                        _pos = (int)_length + (int)offset;
                        break;
                    }
                default:
                    {
                        Debug.Assert(false);
                        break;
                    }
            }
            _s.Pos = _pos;
            return _pos;
        }

        public override void SetLength(long value) => Debug.Assert(false);

        private readonly Ice.InputStream _s;
        private int _pos;
        private readonly long _length;
    }
}
