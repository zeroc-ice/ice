
// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System;
    using System.Diagnostics;
    using System.IO;
    using System.Text;

    //
    // Class to provide a System.IO.Stream interface on top of a BasicStream.
    // We use this to serialize arbitrary .NET serializable classes into
    // a Slice byte sequence.
    //
    // For input streams, this class is a wrapper around the BasicStream
    // class that passes all methods through.
    //
    // For output streams, we use a different stragegy:
    // Slice sequences are encoded on the wire as a count of elements, followed
    // by the sequence contents. For arbitrary .NET classes, we do not know how
    // big the sequence that is eventually written will be. To avoid excessive
    // data copying, this class mantains a private bytes_ array of 254 bytes and,
    // initially, writes data into that array. If more than 254 bytes end up being
    // written, we write a dummy sequence size of 255 (which occupies five bytes
    // on the wire) into the BasicStream and, once this class is disposed, patch
    // that size to match the actual size. Otherwise, if the bytes_ buffer contains
    // fewer than 255 bytes when this class is disposed, we write the sequence size
    // as a single byte, followed by the contents of the bytes_ buffer.
    //

    enum StreamType { Read, Write };

    public class StreamWrapper : System.IO.Stream, System.IDisposable
    {
        //
        // Writeable stream constructor
        //
        public StreamWrapper(BasicStream s)
        {
            type_ = StreamType.Write;
            s_ = s;
            spos_ = s.pos();
            bytes_ = new byte[254];
            pos_ = 0;
            length_ = 0;
        }

        //
        // Readable stream constructor
        //
        public StreamWrapper(int size, BasicStream s)
        {
            type_ = StreamType.Read;
            s_ = s;
            spos_ = 0;
            bytes_ = null;
            pos_ = 0;
            length_ = size;
        }

        public override int Read(byte[] buffer, int offset, int count)
        {
            Debug.Assert(buffer != null && offset >= 0 && count >= 0 && offset + count <= buffer.Length);
            try
            {
                s_.getBuffer().b.get(buffer, offset, count);
            }
            catch(System.Exception ex)
            {
                throw new IOException("could not read from stream", ex);
            }
            return count;
        }

        public override int ReadByte()
        {
            try
            {
                return s_.getBuffer().b.get();
            }
            catch(System.Exception ex)
            {
                throw new IOException("could not read from stream", ex);
            }
        }

        public override void Write(byte[] array, int offset, int count)
        {
            Debug.Assert(type_ == StreamType.Write);
            Debug.Assert(array != null && offset >= 0 && count >= 0 && offset + count <= array.Length);
            try
            {
                if(bytes_ != null)
                {
                    //
                    // If we can fit the data into the first 254 bytes, write it to bytes_.
                    //
                    if(count <= bytes_.Length - pos_)
                    {
                        System.Buffer.BlockCopy(array, offset, bytes_, pos_, count);
                        pos_ += count;
                        return;
                    }

                    s_.writeSize(255); // Dummy size, until we know how big the stream
                                       // really is and can patch the size.
                    if(pos_ > 0)
                    {
                        //
                        // Write the current contents of bytes_.
                        //
                        s_.expand(pos_);
                        s_.getBuffer().b.put(bytes_, 0, pos_);
                    }

                    bytes_ = null;
                }

                //
                // Write data passed by caller.
                //
                s_.expand(count);
                s_.getBuffer().b.put(array, offset, count);
                pos_ += count;
            }
            catch(System.Exception ex)
            {
                throw new IOException("could not write to stream", ex);
            }
        }

        public override void WriteByte(byte value)
        {
            Debug.Assert(type_ == StreamType.Write);
            try
            {
                if(bytes_ != null)
                {
                    //
                    // If we can fit the data into the first 254 bytes, write it to bytes_.
                    //
                    if(pos_ < bytes_.Length)
                    {
                        bytes_[pos_++] = value;
                        return;
                    }

                    s_.writeSize(255); // Dummy size, until we know how big the stream
                                       // really is and can patch the size.
                    if(pos_ > 0)
                    {
                        //
                        // Write the current contents of bytes_.
                        //
                        s_.expand(pos_);
                        s_.getBuffer().b.put(bytes_, 0, pos_);
                    }

                    bytes_ = null;
                }

                //
                // Write data passed by caller.
                //
                s_.expand(1);
                s_.getBuffer().b.put(value);
                pos_ += 1;
            }
            catch(System.Exception ex)
            {
                throw new IOException("could not write to stream", ex);
            }
        }

        public override bool CanRead
        {
            get
            {
                return true;
            }
        }

        public override bool CanWrite
        {
            get
            {
                return type_ == StreamType.Write;
            }
        }

        public override bool CanSeek
        {
            get
            {
                if(AssemblyUtil.runtime_ == AssemblyUtil.Runtime.Mono)
                {
                    //
                    // The Mono deserialization implementation has a bug that causes a call to Seek() such
                    // that the reading position is set to -1.
                    //
                    return false;
                }
                else
                {
                    return type_ == StreamType.Read;
                }
            }
        }

        public override void Flush()
        {
            try
            {
                if(bytes_ != null)
                {
                    Debug.Assert(pos_ <= bytes_.Length);
                    s_.pos(spos_);
                    s_.writeSize(pos_);
                    s_.expand(pos_);
                    s_.getBuffer().b.put(bytes_, 0, pos_);
                }
                else
                {
                    int currentPos = s_.pos();
                    s_.pos(spos_);
                    s_.writeSize(pos_); // Patch previously-written dummy value.
                    s_.pos(currentPos);
                }
            }
            catch(System.Exception ex)
            {
                throw new IOException("could not flush stream", ex);
            }
        }

        public override long Length
        {
            get
            {
                return length_;
            }
        }

        public override long Position
        {
            get
            {
                return pos_;
            }

            set
            {
                Seek(value, SeekOrigin.Begin);
            }
        }

        public override long Seek(long offset, SeekOrigin origin)
        {
            Debug.Assert(type_ != StreamType.Write);

            // Deliberately no size check here--positioning beyond the limit of the stream is legal.
            switch(origin)
            {
                case SeekOrigin.Begin:
                {
                    pos_ = (int)offset;
                    break;
                }
                case SeekOrigin.Current:
                {
                    pos_ += (int)offset;
                    break;
                }
                case SeekOrigin.End:
                {
                    pos_ = (int)length_ + (int)offset;
                    break;
                }
                default:
                {
                    Debug.Assert(false);
                    break;
                }
            }
            s_.pos(pos_);
            return pos_;
        }

        public override void SetLength(long value)
        {
            Debug.Assert(type_ == StreamType.Write && value >= 0);
            length_ = value;
        }

        private StreamType type_;
        private BasicStream s_;
        private int spos_;
        private byte[] bytes_;
        private int pos_;
        private long length_;
    }
}
