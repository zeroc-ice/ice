// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Reflection;
#if !COMPACT && !SILVERLIGHT
    using System.Runtime.InteropServices;
    using System.Runtime.Serialization;
    using System.Runtime.Serialization.Formatters.Binary;
#endif
    using System.Threading;

#if !MANAGED && !COMPACT && !SILVERLIGHT
    internal static class NativeMethods
    {
        [DllImport("bzip2.dll")]
        internal static extern IntPtr BZ2_bzlibVersion();

        [DllImport("bzip2.dll")]
        internal static extern int BZ2_bzBuffToBuffCompress(byte[] dest,
                                                            ref int destLen,
                                                            byte[] source,
                                                            int sourceLen,
                                                            int blockSize100k,
                                                            int verbosity,
                                                            int workFactor);

        [DllImport("bzip2.dll")]
        internal static extern int BZ2_bzBuffToBuffDecompress(byte[] dest,
                                                              ref int destLen,
                                                              byte[] source,
                                                              int sourceLen,
                                                              int small,
                                                              int verbosity);
    }
#endif

    public class BasicStream
    {

        static BasicStream()
        {
#if MANAGED || COMPACT || SILVERLIGHT
            //
            // Protocol compression is not supported when using managed code.
            //
            _bzlibInstalled = false;
#else
            //
            // Simple trick to find out whether bzip2 is
            // installed: Call the BZ2_bzlibVersion() function in the
            // library. If we get an exception, the library is
            // not available.
            //
            _bzlibInstalled = false;

            //
            // We are setting the library name here because, under Mono, we don't know the exact library name.
            // In addition, the FileName member of the BadImageFormatException is the empty string, even though
            // it should provide the name of the library.
            //
            string lib = AssemblyUtil.runtime_ == AssemblyUtil.Runtime.Mono ? "bzip2 library" : "bzip2.dll";
            try
            {
                NativeMethods.BZ2_bzlibVersion();
                _bzlibInstalled = true;
            }
            catch(DllNotFoundException)
            {
                // Expected -- bzip2.dll not installed or not in PATH.
            }
            catch(EntryPointNotFoundException)
            {
                Console.Error.WriteLine("warning: found " + lib + " but entry point BZ2_bzlibVersion is missing.");
            }
            catch(BadImageFormatException ex)
            {
                if(ex.FileName != null && ex.FileName.Length != 0)
                {
                    lib = ex.FileName; // Future-proof: we'll do the right thing if the FileName member is non-empty.
                }
                Console.Error.Write("warning: " + lib + " could not be loaded (likely due to 32/64-bit mismatch).");
                if(IntPtr.Size == 8)
                {
                    Console.Error.Write(" Make sure the directory containing the 64-bit " + lib + " is in your PATH.");
                }
                Console.Error.WriteLine();
            }
#endif
        }

        public BasicStream(Instance instance, Ice.EncodingVersion encoding)
        {
            initialize(instance, encoding, false);
        }

        public BasicStream(Instance instance, Ice.EncodingVersion encoding, bool unlimited)
        {
            initialize(instance, encoding, unlimited);
        }

        private void initialize(Instance instance, Ice.EncodingVersion encoding, bool unlimited)
        {
            instance_ = instance;
            _buf = new Buffer(instance_.messageSizeMax());
            _closure = null;
            _encoding = encoding;

            _readEncapsStack = null;
            _writeEncapsStack = null;
            _readEncapsCache = null;
            _writeEncapsCache = null;

            _sliceObjects = true;

            _messageSizeMax = instance_.messageSizeMax(); // Cached for efficiency.
            _unlimited = unlimited;

            _startSeq = -1;
            _sizePos = -1;
        }

        //
        // This function allows this object to be reused, rather than
        // reallocated.
        //
        public void reset()
        {
            _buf.reset();
            clear();
        }

        public void clear()
        {
            if(_readEncapsStack != null)
            {
                Debug.Assert(_readEncapsStack.next == null);
                _readEncapsStack.next = _readEncapsCache;
                _readEncapsCache = _readEncapsStack;
                _readEncapsStack = null;
                _readEncapsCache.reset();
            }

            if(_writeEncapsStack != null)
            {
                Debug.Assert(_writeEncapsStack.next == null);
                _writeEncapsStack.next = _writeEncapsCache;
                _writeEncapsCache = _writeEncapsStack;
                _writeEncapsStack = null;
                _writeEncapsCache.reset();
            }

            _startSeq = -1;

            _sliceObjects = true;
        }

        public Instance instance()
        {
            return instance_;
        }

        public object closure()
        {
            return _closure;
        }

        public object closure(object p)
        {
            object prev = _closure;
            _closure = p;
            return prev;
        }

        public void swap(BasicStream other)
        {
            Debug.Assert(instance_ == other.instance_);

            Buffer tmpBuf = other._buf;
            other._buf = _buf;
            _buf = tmpBuf;

            object tmpClosure = other._closure;
            other._closure = _closure;
            _closure = tmpClosure;

            //
            // Swap is never called for BasicStreams that have encapsulations being read/write. However,
            // encapsulations might still be set in case marshalling or un-marshalling failed. We just
            // reset the encapsulations if there are still some set.
            //
            resetEncaps();
            other.resetEncaps();

            bool tmpUnlimited = other._unlimited;
            other._unlimited = _unlimited;
            _unlimited = tmpUnlimited;

            int tmpStartSeq = other._startSeq;
            other._startSeq = _startSeq;
            _startSeq = tmpStartSeq;

            int tmpMinSeqSize = other._minSeqSize;
            other._minSeqSize = _minSeqSize;
            _minSeqSize = tmpMinSeqSize;

            int tmpSizePos = other._sizePos;
            other._sizePos = _sizePos;
            _sizePos = tmpSizePos;
        }

        public void resetEncaps()
        {
            _readEncapsStack = null;
            _writeEncapsStack = null;
        }

        public void resize(int sz, bool reading)
        {
            //
            // Check memory limit if stream is not unlimited.
            //
            if(!_unlimited && sz > _messageSizeMax)
            {
                Ex.throwMemoryLimitException(sz, _messageSizeMax);
            }

            _buf.resize(sz, reading);
            _buf.b.position(sz);
        }

        public Buffer prepareWrite()
        {
            _buf.b.limit(_buf.size());
            _buf.b.position(0);
            return _buf;
        }

        public Buffer getBuffer()
        {
            return _buf;
        }

        public void startWriteObject(Ice.SlicedData data)
        {
            Debug.Assert(_writeEncapsStack != null && _writeEncapsStack.encoder != null);
            _writeEncapsStack.encoder.startObject(data);
        }

        public void endWriteObject()
        {
            Debug.Assert(_writeEncapsStack != null && _writeEncapsStack.encoder != null);
            _writeEncapsStack.encoder.endObject();
        }

        public void startReadObject()
        {
            Debug.Assert(_readEncapsStack != null && _readEncapsStack.decoder != null);
            _readEncapsStack.decoder.startObject();
        }

        public Ice.SlicedData endReadObject(bool preserve)
        {
            Debug.Assert(_readEncapsStack != null && _readEncapsStack.decoder != null);
            return _readEncapsStack.decoder.endObject(preserve);
        }

        public void startWriteException(Ice.SlicedData data)
        {
            Debug.Assert(_writeEncapsStack != null && _writeEncapsStack.encoder != null);
            _writeEncapsStack.encoder.startException(data);
        }

        public void endWriteException()
        {
            Debug.Assert(_writeEncapsStack != null && _writeEncapsStack.encoder != null);
            _writeEncapsStack.encoder.endException();
        }

        public void startReadException()
        {
            Debug.Assert(_readEncapsStack != null && _readEncapsStack.decoder != null);
            _readEncapsStack.decoder.startException();
        }

        public Ice.SlicedData endReadException(bool preserve)
        {
            Debug.Assert(_readEncapsStack != null && _readEncapsStack.decoder != null);
            return _readEncapsStack.decoder.endException(preserve);
        }

        public void startWriteEncaps()
        {
            //
            // If no encoding version is specified, use the current write
            // encapsulation encoding version if there's a current write
            // encapsulation, otherwise, use the stream encoding version.
            //

            if(_writeEncapsStack != null)
            {
                startWriteEncaps(_writeEncapsStack.encoding, _writeEncapsStack.format);
            }
            else
            {
                startWriteEncaps(_encoding, Ice.FormatType.DefaultFormat);
            }
        }

        public void startWriteEncaps(Ice.EncodingVersion encoding, Ice.FormatType format)
        {
            Protocol.checkSupportedEncoding(encoding);

            WriteEncaps curr = _writeEncapsCache;
            if(curr != null)
            {
                curr.reset();
                _writeEncapsCache = _writeEncapsCache.next;
            }
            else
            {
                curr = new WriteEncaps();
            }
            curr.next = _writeEncapsStack;
            _writeEncapsStack = curr;

            _writeEncapsStack.format = format;
            _writeEncapsStack.setEncoding(encoding);
            _writeEncapsStack.start = _buf.b.position();

            writeInt(0); // Placeholder for the encapsulation length.
            _writeEncapsStack.encoding.write__(this);
        }

        public void endWriteEncaps()
        {
            Debug.Assert(_writeEncapsStack != null);

            if(_writeEncapsStack.encoder != null)
            {
                _writeEncapsStack.encoder.writePendingObjects();
            }

            // Size includes size and version.
            int start = _writeEncapsStack.start;
            int sz = _buf.size() - start;
            _buf.b.putInt(start, sz);

            WriteEncaps curr = _writeEncapsStack;
            _writeEncapsStack = curr.next;
            curr.next = _writeEncapsCache;
            _writeEncapsCache = curr;
            _writeEncapsCache.reset();
        }

        public void endWriteEncapsChecked()
        {
            if(_writeEncapsStack == null)
            {
                throw new Ice.EncapsulationException("not in an encapsulation");
            }
            endWriteEncaps();
        }

        public void writeEmptyEncaps(Ice.EncodingVersion encoding)
        {
            Protocol.checkSupportedEncoding(encoding);
            writeInt(6); // Size
            encoding.write__(this);
        }

        public void writeEncaps(byte[] v)
        {
            if(v.Length < 6)
            {
                throw new Ice.EncapsulationException();
            }
            expand(v.Length);
            _buf.b.put(v);
        }

        public Ice.EncodingVersion getWriteEncoding()
        {
            return _writeEncapsStack != null ? _writeEncapsStack.encoding : _encoding;
        }

        public Ice.EncodingVersion startReadEncaps()
        {
            ReadEncaps curr = _readEncapsCache;
            if(curr != null)
            {
                curr.reset();
                _readEncapsCache = _readEncapsCache.next;
            }
            else
            {
                curr = new ReadEncaps();
            }
            curr.next = _readEncapsStack;
            _readEncapsStack = curr;

            _readEncapsStack.start = _buf.b.position();

            //
            // I don't use readSize() and writeSize() for encapsulations,
            // because when creating an encapsulation, I must know in advance
            // how many bytes the size information will require in the data
            // stream. If I use an Int, it is always 4 bytes. For
            // readSize()/writeSize(), it could be 1 or 5 bytes.
            //
            int sz = readInt();
            if(sz < 6)
            {
                throw new Ice.UnmarshalOutOfBoundsException();
            }
            if(sz - 4 > _buf.b.remaining())
            {
                throw new Ice.UnmarshalOutOfBoundsException();
            }
            _readEncapsStack.sz = sz;

            Ice.EncodingVersion encoding = new Ice.EncodingVersion();
            encoding.read__(this);
            Protocol.checkSupportedEncoding(encoding); // Make sure the encoding is supported.
            _readEncapsStack.setEncoding(encoding);

            return encoding;
        }

        public void endReadEncaps()
        {
            Debug.Assert(_readEncapsStack != null);

            if(_readEncapsStack.decoder != null)
            {
                _readEncapsStack.decoder.readPendingObjects();
            }
            else if(_buf.b.position() < _readEncapsStack.start + _readEncapsStack.sz && !_readEncapsStack.encoding_1_0)
            {
                //
                // Read remaining encapsulation optionals. This returns
                // true if the optionals end with the end marker. The end
                // marker indicates that there are more to read from the
                // encapsulation: object instances. In this case, don't
                // bother reading the objects, just skip to the end of the
                // encapsulation.
                //
                if(skipOpts())
                {
                    _buf.b.position(_readEncapsStack.start + _readEncapsStack.sz);
                }
            }

            if(_buf.b.position() != _readEncapsStack.start + _readEncapsStack.sz)
            {
                if(_buf.b.position() + 1 != _readEncapsStack.start + _readEncapsStack.sz)
                {
                    throw new Ice.EncapsulationException();
                }

                //
                // Ice version < 3.3 had a bug where user exceptions with
                // class members could be encoded with a trailing byte
                // when dispatched with AMD. So we tolerate an extra byte
                // in the encapsulation.
                //
                try
                {
                    _buf.b.get();
                }
                catch(InvalidOperationException ex)
                {
                    throw new Ice.UnmarshalOutOfBoundsException(ex);
                }
            }

            ReadEncaps curr = _readEncapsStack;
            _readEncapsStack = curr.next;
            curr.next = _readEncapsCache;
            _readEncapsCache = curr;
            _readEncapsCache.reset();
        }

        public Ice.EncodingVersion skipEmptyEncaps()
        {
            int sz = readInt();
            if(sz != 6)
            {
                throw new Ice.EncapsulationException();
            }

            Ice.EncodingVersion encoding = new Ice.EncodingVersion();
            encoding.read__(this);
            return encoding;
        }

        public void endReadEncapsChecked() // Used by public stream API.
        {
            if(_readEncapsStack == null)
            {
                throw new Ice.EncapsulationException("not in an encapsulation");
            }
            endReadEncaps();
        }

        public byte[] readEncaps(out Ice.EncodingVersion encoding)
        {
            int sz = readInt();
            if(sz < 6)
            {
                throw new Ice.UnmarshalOutOfBoundsException();
            }

            if(sz - 4 > _buf.b.remaining())
            {
                throw new Ice.UnmarshalOutOfBoundsException();
            }

            encoding = new Ice.EncodingVersion();
            encoding.read__(this);
            _buf.b.position(_buf.b.position() - 6);

            byte[] v = new byte[sz];
            try
            {
                _buf.b.get(v);
                return v;
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        public Ice.EncodingVersion getReadEncoding()
        {
            return _readEncapsStack != null ? _readEncapsStack.encoding : _encoding;
        }

        public int getReadEncapsSize()
        {
            Debug.Assert(_readEncapsStack != null);
            return _readEncapsStack.sz - 6;
        }

        public Ice.EncodingVersion skipEncaps()
        {
            int sz = readInt();
            if(sz < 6)
            {
                throw new Ice.UnmarshalOutOfBoundsException();
            }
            Ice.EncodingVersion encoding = new Ice.EncodingVersion();
            encoding.read__(this);
            try
            {
                _buf.b.position(_buf.b.position() + sz - 6);
            }
            catch(ArgumentOutOfRangeException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
            return encoding;
        }

        public void startWriteSlice(string typeId, bool last)
        {
            Debug.Assert(_writeEncapsStack != null && _writeEncapsStack.encoder != null);
            _writeEncapsStack.encoder.startSlice(typeId, last);
        }

        public void endWriteSlice()
        {
            Debug.Assert(_writeEncapsStack != null && _writeEncapsStack.encoder != null);
            _writeEncapsStack.encoder.endSlice();
        }

        public string startReadSlice() // Returns type ID of next slice
        {
            Debug.Assert(_readEncapsStack != null && _readEncapsStack.decoder != null);
            return _readEncapsStack.decoder.startSlice();
        }

        public void endReadSlice()
        {
            Debug.Assert(_readEncapsStack != null && _readEncapsStack.decoder != null);
            _readEncapsStack.decoder.endSlice();
        }

        public void skipSlice()
        {
            Debug.Assert(_readEncapsStack != null && _readEncapsStack.decoder != null);
            _readEncapsStack.decoder.skipSlice();
        }

        public void readPendingObjects()
        {
            if(_readEncapsStack != null && _readEncapsStack.decoder != null)
            {
                _readEncapsStack.decoder.readPendingObjects();
                _readEncapsStack.decoder = null;
            }
            else if(_readEncapsStack != null ? _readEncapsStack.encoding_1_0 : _encoding.Equals(Ice.Util.Encoding_1_0))
            {
                //
                // If using the 1.0 encoding and no objects were read, we
                // still read an empty sequence of pending objects if
                // requested (i.e.: if this is called).
                //
                // This is required by the 1.0 encoding, even if no objects
                // are written we do marshal an empty sequence if marshaled
                // data types use classes.
                //
                skipSize();
            }
        }

        public void writePendingObjects()
        {
            if(_writeEncapsStack != null && _writeEncapsStack.encoder != null)
            {
                _writeEncapsStack.encoder.writePendingObjects();
                _writeEncapsStack.encoder = null;
            }
            else if(_writeEncapsStack != null ?
                    _writeEncapsStack.encoding_1_0 : _encoding.Equals(Ice.Util.Encoding_1_0))
            {
                //
                // If using the 1.0 encoding and no objects were written, we
                // still write an empty sequence for pending objects if
                // requested (i.e.: if this is called).
                // 
                // This is required by the 1.0 encoding, even if no objects
                // are written we do marshal an empty sequence if marshaled
                // data types use classes.
                //
                writeSize(0);
            }
        }

        public void writeSize(int v)
        {
            if(v > 254)
            {
                expand(5);
                _buf.b.put((byte)255);
                _buf.b.putInt(v);
            }
            else
            {
                expand(1);
                _buf.b.put((byte)v);
            }
        }

        public void writeSizeSeq(ICollection<int> v)
        {
            writeSize(v.Count);
            foreach(int n in v)
            {
                writeSize(n);
            }
        }

        public int readSize()
        {
            try
            {
                //
                // COMPILERFIX: for some reasons _buf.get() doesn't work here on MacOS X with Mono;
                //
                //byte b = _buf.b.get();
                byte b = readByte();
                if(b == 255)
                {
                    int v = _buf.b.getInt();
                    if(v < 0)
                    {
                        throw new Ice.UnmarshalOutOfBoundsException();
                    }
                    return v;
                }
                else
                {
                    return b; // byte is unsigned
                }
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        public int[] readSizeSeq()
        {
            int sz = readSize();
            int[] v = new int[sz];

            if(sz > 0)
            {
                for(int n = 0; n < sz; ++n)
                {
                    v[n] = readSize();
                }
            }

            return v;
        }

        public int readAndCheckSeqSize(int minSize)
        {
            int sz = readSize();

            if(sz == 0)
            {
                return 0;
            }

            //
            // The _startSeq variable points to the start of the sequence for which
            // we expect to read at least _minSeqSize bytes from the stream.
            //
            // If not initialized or if we already read more data than _minSeqSize,
            // we reset _startSeq and _minSeqSize for this sequence (possibly a
            // top-level sequence or enclosed sequence it doesn't really matter).
            //
            // Otherwise, we are reading an enclosed sequence and we have to bump
            // _minSeqSize by the minimum size that this sequence will  require on
            // the stream.
            //
            // The goal of this check is to ensure that when we start un-marshalling
            // a new sequence, we check the minimal size of this new sequence against
            // the estimated remaining buffer size. This estimatation is based on
            // the minimum size of the enclosing sequences, it's _minSeqSize.
            //
            if(_startSeq == -1 || _buf.b.position() > (_startSeq + _minSeqSize))
            {
                _startSeq = _buf.b.position();
                _minSeqSize = sz * minSize;
            }
            else
            {
                _minSeqSize += sz * minSize;
            }

            //
            // If there isn't enough data to read on the stream for the sequence (and
            // possibly enclosed sequences), something is wrong with the marshalled
            // data: it's claiming having more data that what is possible to read.
            //
            if(_startSeq + _minSeqSize > _buf.size())
            {
                throw new Ice.UnmarshalOutOfBoundsException();
            }

            return sz;
        }

        public void startSize()
        {
            _sizePos = _buf.b.position();
            writeInt(0); // Placeholder for 32-bit size
        }

        public void endSize()
        {
            Debug.Assert(_sizePos >= 0);
            rewriteInt(_buf.b.position() - _sizePos - 4, _sizePos);
            _sizePos = -1;
        }

        public void writeBlob(byte[] v)
        {
            if(v == null)
            {
                return;
            }
            expand(v.Length);
            _buf.b.put(v);
        }

        public void readBlob(byte[] v)
        {
            try
            {
                _buf.b.get(v);
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        public byte[] readBlob(int sz)
        {
            if(_buf.b.remaining() < sz)
            {
                throw new Ice.UnmarshalOutOfBoundsException();
            }
            byte[] v = new byte[sz];
            try
            {
                _buf.b.get(v);
                return v;
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        // Read/write type and tag for optionals
        public bool writeOpt(int tag, Ice.OptionalFormat format)
        {
            Debug.Assert(_writeEncapsStack != null);
            if(_writeEncapsStack.encoder != null)
            {
                return _writeEncapsStack.encoder.writeOpt(tag, format);
            }
            else
            {
                return writeOptImpl(tag, format);
            }
        }

        public bool readOpt(int tag, Ice.OptionalFormat expectedFormat)
        {
            Debug.Assert(_readEncapsStack != null);
            if(_readEncapsStack.decoder != null)
            {
                return _readEncapsStack.decoder.readOpt(tag, expectedFormat);
            }
            else
            {
                return readOptImpl(tag, expectedFormat);
            }
        }

        public void writeByte(byte v)
        {
            expand(1);
            _buf.b.put(v);
        }

        public void writeByte(int tag, Ice.Optional<byte> v)
        {
            if(v.HasValue)
            {
                writeByte(tag, v.Value);
            }
        }

        public void writeByte(int tag, byte v)
        {
            if(writeOpt(tag, Ice.OptionalFormat.F1))
            {
                writeByte(v);
            }
        }

        public void rewriteByte(byte v, int dest)
        {
            _buf.b.put(dest, v);
        }

        public void writeByteSeq(byte[] v)
        {
            if(v == null)
            {
                writeSize(0);
            }
            else
            {
                writeSize(v.Length);
                expand(v.Length);
                _buf.b.put(v);
            }
        }

        public void writeByteSeq(int count, IEnumerable<byte> v)
        {
            if(count == 0)
            {
                writeSize(0);
                return;
            }

            {
                List<byte> value = v as List<byte>;
                if(value != null)
                {
                    writeByteSeq(value.ToArray());
                    return;
                }
            }

            {
                LinkedList<byte> value = v as LinkedList<byte>;
                if(value != null)
                {
                    writeSize(count);
                    expand(count);
                    IEnumerator<byte> i = v.GetEnumerator();
                    while(i.MoveNext())
                    {
                        _buf.b.put(i.Current);
                    }
                    return;
                }
            }

            {
                Queue<byte> value = v as Queue<byte>;
                if(value != null)
                {
                    writeByteSeq(value.ToArray());
                    return;
                }
            }

            {
                Stack<byte> value = v as Stack<byte>;
                if(value != null)
                {
                    writeByteSeq(value.ToArray());
                    return;
                }
            }

            writeSize(count);
            expand(count);
            foreach(byte b in v)
            {
                _buf.b.put(b);
            }
        }

        public void writeByteSeq(int tag, Ice.Optional<byte[]> v)
        {
            if(v.HasValue)
            {
                writeByteSeq(tag, v.Value);
            }
        }

        public void writeByteSeq<T>(int tag, int count, Ice.Optional<T> v)
            where T : IEnumerable<byte>
        {
            if(v.HasValue && writeOpt(tag, Ice.OptionalFormat.VSize))
            {
                writeByteSeq(count, v.Value);
            }
        }

        public void writeByteSeq(int tag, byte[] v)
        {
            if(writeOpt(tag, Ice.OptionalFormat.VSize))
            {
                writeByteSeq(v);
            }
        }

        public void writeByteSeq(int tag, int count, IEnumerable<byte> v)
        {
            if(writeOpt(tag, Ice.OptionalFormat.VSize))
            {
                writeByteSeq(count, v);
            }
        }

        public void writeSerializable(object o)
        {
#if !COMPACT && !SILVERLIGHT
            if(o == null)
            {
                writeSize(0);
                return;
            }
            try
            {
                StreamWrapper w = new StreamWrapper(this);
                IFormatter f = new BinaryFormatter();
                f.Serialize(w, o);
                w.Close();
            }
            catch(System.Exception ex)
            {
                throw new Ice.MarshalException("cannot serialize object:", ex);
            }
#else
            throw new Ice.MarshalException("serialization not supported");
#endif
        }

        public byte readByte()
        {
            try
            {
                return _buf.b.get();
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        public Ice.Optional<byte> readByte(int tag)
        {
            if(readOpt(tag, Ice.OptionalFormat.F1))
            {
                return new Ice.Optional<byte>(readByte());
            }
            else
            {
                return new Ice.Optional<byte>();
            }
        }

        public void readByte(int tag, out bool isset, out byte v)
        {
            if(isset = readOpt(tag, Ice.OptionalFormat.F1))
            {
                v = readByte();
            }
            else
            {
                v = 0;
            }
        }

        public byte[] readByteSeq()
        {
            try
            {
                int sz = readAndCheckSeqSize(1);
                byte[] v = new byte[sz];
                _buf.b.get(v);
                return v;
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        public void readByteSeq(out List<byte> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new List<byte>(readByteSeq());
        }

        public void readByteSeq(out LinkedList<byte> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new LinkedList<byte>(readByteSeq());
        }

        public void readByteSeq(out Queue<byte> l)
        {
            //
            // Reading into an array and copy-constructing the
            // queue is faster than constructing the queue
            // and adding to it one element at a time.
            //
            l = new Queue<byte>(readByteSeq());
        }

        public void readByteSeq(out Stack<byte> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            byte[] array = readByteSeq();
            Array.Reverse(array);
            l = new Stack<byte>(array);
        }

        public Ice.Optional<byte[]> readByteSeq(int tag)
        {
            if(readOpt(tag, Ice.OptionalFormat.VSize))
            {
                return new Ice.Optional<byte[]>(readByteSeq());
            }
            else
            {
                return new Ice.Optional<byte[]>();
            }
        }

        public void readByteSeq(int tag, out bool isset, out byte[] v)
        {
            if(isset = readOpt(tag, Ice.OptionalFormat.VSize))
            {
                v = readByteSeq();
            }
            else
            {
                v = null;
            }
        }

        public object readSerializable()
        {
#if !COMPACT && !SILVERLIGHT
            int sz = readAndCheckSeqSize(1);
            if(sz == 0)
            {
                return null;
            }
            try
            {
                StreamWrapper w = new StreamWrapper(sz, this);
                IFormatter f = new BinaryFormatter();
                return f.Deserialize(w);
            }
            catch(System.Exception ex)
            {
                throw new Ice.MarshalException("cannot deserialize object:", ex);
            }
#else
            throw new Ice.MarshalException("serialization not supported");
#endif
        }

        public void writeBool(bool v)
        {
            expand(1);
            _buf.b.put(v ? (byte)1 : (byte)0);
        }

        public void writeBool(int tag, Ice.Optional<bool> v)
        {
            if(v.HasValue)
            {
                writeBool(tag, v.Value);
            }
        }

        public void writeBool(int tag, bool v)
        {
            if(writeOpt(tag, Ice.OptionalFormat.F1))
            {
                writeBool(v);
            }
        }

        public void rewriteBool(bool v, int dest)
        {
            _buf.b.put(dest, v ? (byte)1 : (byte)0);
        }

        public void writeBoolSeq(bool[] v)
        {
            if(v == null)
            {
                writeSize(0);
            }
            else
            {
                writeSize(v.Length);
                expand(v.Length);
                _buf.b.putBoolSeq(v);
            }
        }

        public void writeBoolSeq(int count, IEnumerable<bool> v)
        {
            if(count == 0)
            {
                writeSize(0);
                return;
            }

            {
                List<bool> value = v as List<bool>;
                if(value != null)
                {
                    writeBoolSeq(value.ToArray());
                    return;
                }
            }

            {
                LinkedList<bool> value = v as LinkedList<bool>;
                if(value != null)
                {
                    writeSize(count);
                    expand(count);
                    IEnumerator<bool> i = v.GetEnumerator();
                    while(i.MoveNext())
                    {
                        _buf.b.putBool(i.Current);
                    }
                    return;
                }
            }

            {
                Queue<bool> value = v as Queue<bool>;
                if(value != null)
                {
                    writeBoolSeq(value.ToArray());
                    return;
                }
            }

            {
                Stack<bool> value = v as Stack<bool>;
                if(value != null)
                {
                    writeBoolSeq(value.ToArray());
                    return;
                }
            }

            writeSize(count);
            expand(count);
            foreach(bool b in v)
            {
                _buf.b.putBool(b);
            }
        }

        public void writeBoolSeq(int tag, Ice.Optional<bool[]> v)
        {
            if(v.HasValue)
            {
                writeBoolSeq(tag, v.Value);
            }
        }

        public void writeBoolSeq<T>(int tag, int count, Ice.Optional<T> v)
            where T : IEnumerable<bool>
        {
            if(v.HasValue && writeOpt(tag, Ice.OptionalFormat.VSize))
            {
                writeBoolSeq(count, v.Value);
            }
        }

        public void writeBoolSeq(int tag, bool[] v)
        {
            if(writeOpt(tag, Ice.OptionalFormat.VSize))
            {
                writeBoolSeq(v);
            }
        }

        public void writeBoolSeq(int tag, int count, IEnumerable<bool> v)
        {
            if(writeOpt(tag, Ice.OptionalFormat.VSize))
            {
                writeBoolSeq(count, v);
            }
        }

        public bool readBool()
        {
            try
            {
                return _buf.b.get() == 1;
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        public Ice.Optional<bool> readBool(int tag)
        {
            if(readOpt(tag, Ice.OptionalFormat.F1))
            {
                return new Ice.Optional<bool>(readBool());
            }
            else
            {
                return new Ice.Optional<bool>();
            }
        }

        public void readBool(int tag, out bool isset, out bool v)
        {
            if(isset = readOpt(tag, Ice.OptionalFormat.F1))
            {
                v = readBool();
            }
            else
            {
                v = false;
            }
        }

        public bool[] readBoolSeq()
        {
            try
            {
                int sz = readAndCheckSeqSize(1);
                bool[] v = new bool[sz];
                _buf.b.getBoolSeq(v);
                return v;
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        public void readBoolSeq(out List<bool> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new List<bool>(readBoolSeq());
        }

        public void readBoolSeq(out LinkedList<bool> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new LinkedList<bool>(readBoolSeq());
        }

        public void readBoolSeq(out Queue<bool> l)
        {
            //
            // Reading into an array and copy-constructing the
            // queue is faster than constructing the queue
            // and adding to it one element at a time.
            //
            l = new Queue<bool>(readBoolSeq());
        }

        public void readBoolSeq(out Stack<bool> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            bool[] array = readBoolSeq();
            Array.Reverse(array);
            l = new Stack<bool>(array);
        }

        public Ice.Optional<bool[]> readBoolSeq(int tag)
        {
            if(readOpt(tag, Ice.OptionalFormat.VSize))
            {
                return new Ice.Optional<bool[]>(readBoolSeq());
            }
            else
            {
                return new Ice.Optional<bool[]>();
            }
        }

        public void readBoolSeq(int tag, out bool isset, out bool[] v)
        {
            if(isset = readOpt(tag, Ice.OptionalFormat.VSize))
            {
                v = readBoolSeq();
            }
            else
            {
                v = null;
            }
        }

        public void writeShort(short v)
        {
            expand(2);
            _buf.b.putShort(v);
        }

        public void writeShort(int tag, Ice.Optional<short> v)
        {
            if(v.HasValue)
            {
                writeShort(tag, v.Value);
            }
        }

        public void writeShort(int tag, short v)
        {
            if(writeOpt(tag, Ice.OptionalFormat.F2))
            {
                writeShort(v);
            }
        }

        public void writeShortSeq(short[] v)
        {
            if(v == null)
            {
                writeSize(0);
            }
            else
            {
                writeSize(v.Length);
                expand(v.Length * 2);
                _buf.b.putShortSeq(v);
            }
        }

        public void writeShortSeq(int count, IEnumerable<short> v)
        {
            if(count == 0)
            {
                writeSize(0);
                return;
            }

            {
                List<short> value = v as List<short>;
                if(value != null)
                {
                    writeShortSeq(value.ToArray());
                    return;
                }
            }

            {
                LinkedList<short> value = v as LinkedList<short>;
                if(value != null)
                {
                    writeSize(count);
                    expand(count * 2);
                    IEnumerator<short> i = v.GetEnumerator();
                    while(i.MoveNext())
                    {
                        _buf.b.putShort(i.Current);
                    }
                    return;
                }
            }

            {
                Queue<short> value = v as Queue<short>;
                if(value != null)
                {
                    writeShortSeq(value.ToArray());
                    return;
                }
            }

            {
                Stack<short> value = v as Stack<short>;
                if(value != null)
                {
                    writeShortSeq(value.ToArray());
                    return;
                }
            }

            writeSize(count);
            expand(count * 2);
            foreach(short s in v)
            {
                _buf.b.putShort(s);
            }
        }

        public void writeShortSeq(int tag, Ice.Optional<short[]> v)
        {
            if(v.HasValue)
            {
                writeShortSeq(tag, v.Value);
            }
        }

        public void writeShortSeq<T>(int tag, int count, Ice.Optional<T> v)
            where T : IEnumerable<short>
        {
            if(v.HasValue && writeOpt(tag, Ice.OptionalFormat.VSize))
            {
                writeSize(count == 0 ? 1 : count * 2 + (count > 254 ? 5 : 1));
                writeShortSeq(count, v.Value);
            }
        }

        public void writeShortSeq(int tag, short[] v)
        {
            if(writeOpt(tag, Ice.OptionalFormat.VSize))
            {
                writeSize(v == null || v.Length == 0 ? 1 : v.Length * 2 + (v.Length > 254 ? 5 : 1));
                writeShortSeq(v);
            }
        }

        public void writeShortSeq(int tag, int count, IEnumerable<short> v)
        {
            if(writeOpt(tag, Ice.OptionalFormat.VSize))
            {
                writeSize(v == null || count == 0 ? 1 : count * 2 + (count > 254 ? 5 : 1));
                writeShortSeq(count, v);
            }
        }

        public short readShort()
        {
            try
            {
                return _buf.b.getShort();
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        public Ice.Optional<short> readShort(int tag)
        {
            if(readOpt(tag, Ice.OptionalFormat.F2))
            {
                return new Ice.Optional<short>(readShort());
            }
            else
            {
                return new Ice.Optional<short>();
            }
        }

        public void readShort(int tag, out bool isset, out short v)
        {
            if(isset = readOpt(tag, Ice.OptionalFormat.F2))
            {
                v = readShort();
            }
            else
            {
                v = 0;
            }
        }

        public short[] readShortSeq()
        {
            try
            {
                int sz = readAndCheckSeqSize(2);
                short[] v = new short[sz];
                _buf.b.getShortSeq(v);
                return v;
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        public void readShortSeq(out List<short> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new List<short>(readShortSeq());
        }

        public void readShortSeq(out LinkedList<short> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new LinkedList<short>(readShortSeq());
        }

        public void readShortSeq(out Queue<short> l)
        {
            //
            // Reading into an array and copy-constructing the
            // queue is faster than constructing the queue
            // and adding to it one element at a time.
            //
            l = new Queue<short>(readShortSeq());
        }

        public void readShortSeq(out Stack<short> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            short[] array = readShortSeq();
            Array.Reverse(array);
            l = new Stack<short>(array);
        }

        public Ice.Optional<short[]> readShortSeq(int tag)
        {
            if(readOpt(tag, Ice.OptionalFormat.VSize))
            {
                skipSize();
                return new Ice.Optional<short[]>(readShortSeq());
            }
            else
            {
                return new Ice.Optional<short[]>();
            }
        }

        public void readShortSeq(int tag, out bool isset, out short[] v)
        {
            if(isset = readOpt(tag, Ice.OptionalFormat.VSize))
            {
                skipSize();
                v = readShortSeq();
            }
            else
            {
                v = null;
            }
        }

        public void writeInt(int v)
        {
            expand(4);
            _buf.b.putInt(v);
        }

        public void writeInt(int tag, Ice.Optional<int> v)
        {
            if(v.HasValue)
            {
                writeInt(tag, v.Value);
            }
        }

        public void writeInt(int tag, int v)
        {
            if(writeOpt(tag, Ice.OptionalFormat.F4))
            {
                writeInt(v);
            }
        }

        public void rewriteInt(int v, int dest)
        {
            _buf.b.putInt(dest, v);
        }

        public void writeIntSeq(int[] v)
        {
            if(v == null)
            {
                writeSize(0);
            }
            else
            {
                writeSize(v.Length);
                expand(v.Length * 4);
                _buf.b.putIntSeq(v);
            }
        }

        public void writeIntSeq(int count, IEnumerable<int> v)
        {
            if(count == 0)
            {
                writeSize(0);
                return;
            }

            {
                List<int> value = v as List<int>;
                if(value != null)
                {
                    writeIntSeq(value.ToArray());
                    return;
                }
            }

            {
                LinkedList<int> value = v as LinkedList<int>;
                if(value != null)
                {
                    writeSize(count);
                    expand(count * 4);
                    IEnumerator<int> i = v.GetEnumerator();
                    while(i.MoveNext())
                    {
                        _buf.b.putInt(i.Current);
                    }
                    return;
                }
            }

            {
                Queue<int> value = v as Queue<int>;
                if(value != null)
                {
                    writeIntSeq(value.ToArray());
                    return;
                }
            }

            {
                Stack<int> value = v as Stack<int>;
                if(value != null)
                {
                    writeIntSeq(value.ToArray());
                    return;
                }
            }

            writeSize(count);
            expand(count * 4);
            foreach(int i in v)
            {
                _buf.b.putInt(i);
            }
        }

        public void writeIntSeq(int tag, Ice.Optional<int[]> v)
        {
            if(v.HasValue)
            {
                writeIntSeq(tag, v.Value);
            }
        }

        public void writeIntSeq<T>(int tag, int count, Ice.Optional<T> v)
            where T : IEnumerable<int>
        {
            if(v.HasValue && writeOpt(tag, Ice.OptionalFormat.VSize))
            {
                writeSize(count == 0 ? 1 : count * 4 + (count > 254 ? 5 : 1));
                writeIntSeq(count, v.Value);
            }
        }

        public void writeIntSeq(int tag, int[] v)
        {
            if(writeOpt(tag, Ice.OptionalFormat.VSize))
            {
                writeSize(v == null || v.Length == 0 ? 1 : v.Length * 4 + (v.Length > 254 ? 5 : 1));
                writeIntSeq(v);
            }
        }

        public void writeIntSeq(int tag, int count, IEnumerable<int> v)
        {
            if(writeOpt(tag, Ice.OptionalFormat.VSize))
            {
                writeSize(v == null || count == 0 ? 1 : count * 4 + (count > 254 ? 5 : 1));
                writeIntSeq(count, v);
            }
        }

        public int readInt()
        {
            try
            {
                return _buf.b.getInt();
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        public Ice.Optional<int> readInt(int tag)
        {
            if(readOpt(tag, Ice.OptionalFormat.F4))
            {
                return new Ice.Optional<int>(readInt());
            }
            else
            {
                return new Ice.Optional<int>();
            }
        }

        public void readInt(int tag, out bool isset, out int v)
        {
            if(isset = readOpt(tag, Ice.OptionalFormat.F4))
            {
                v = readInt();
            }
            else
            {
                v = 0;
            }
        }

        public int[] readIntSeq()
        {
            try
            {
                int sz = readAndCheckSeqSize(4);
                int[] v = new int[sz];
                _buf.b.getIntSeq(v);
                return v;
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        public void readIntSeq(out List<int> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new List<int>(readIntSeq());
        }

        public void readIntSeq(out LinkedList<int> l)
        {
            try
            {
                int sz = readAndCheckSeqSize(4);
                l = new LinkedList<int>();
                for(int i = 0; i < sz; ++i)
                {
                    l.AddLast(_buf.b.getInt());
                }
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        public void readIntSeq(out Queue<int> l)
        {
            //
            // Reading into an array and copy-constructing the
            // queue takes the same time as constructing the queue
            // and adding to it one element at a time, so
            // we avoid the copy.
            //
            try
            {
                int sz = readAndCheckSeqSize(4);
                l = new Queue<int>(sz);
                for(int i = 0; i < sz; ++i)
                {
                    l.Enqueue(_buf.b.getInt());
                }
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        public void readIntSeq(out Stack<int> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            int[] array = readIntSeq();
            Array.Reverse(array);
            l = new Stack<int>(array);
        }

        public Ice.Optional<int[]> readIntSeq(int tag)
        {
            if(readOpt(tag, Ice.OptionalFormat.VSize))
            {
                skipSize();
                return new Ice.Optional<int[]>(readIntSeq());
            }
            else
            {
                return new Ice.Optional<int[]>();
            }
        }

        public void readIntSeq(int tag, out bool isset, out int[] v)
        {
            if(isset = readOpt(tag, Ice.OptionalFormat.VSize))
            {
                skipSize();
                v = readIntSeq();
            }
            else
            {
                v = null;
            }
        }

        public void writeLong(long v)
        {
            expand(8);
            _buf.b.putLong(v);
        }

        public void writeLong(int tag, Ice.Optional<long> v)
        {
            if(v.HasValue)
            {
                writeLong(tag, v.Value);
            }
        }

        public void writeLong(int tag, long v)
        {
            if(writeOpt(tag, Ice.OptionalFormat.F8))
            {
                writeLong(v);
            }
        }

        public void writeLongSeq(long[] v)
        {
            if(v == null)
            {
                writeSize(0);
            }
            else
            {
                writeSize(v.Length);
                expand(v.Length * 8);
                _buf.b.putLongSeq(v);
            }
        }

        public void writeLongSeq(int count, IEnumerable<long> v)
        {
            if(count == 0)
            {
                writeSize(0);
                return;
            }

            {
                List<long> value = v as List<long>;
                if(value != null)
                {
                    writeLongSeq(value.ToArray());
                    return;
                }
            }

            {
                LinkedList<long> value = v as LinkedList<long>;
                if(value != null)
                {
                    writeSize(count);
                    expand(count * 8);
                    IEnumerator<long> i = v.GetEnumerator();
                    while(i.MoveNext())
                    {
                        _buf.b.putLong(i.Current);
                    }
                    return;
                }
            }

            {
                Queue<long> value = v as Queue<long>;
                if(value != null)
                {
                    writeLongSeq(value.ToArray());
                    return;
                }
            }

            {
                Stack<long> value = v as Stack<long>;
                if(value != null)
                {
                    writeLongSeq(value.ToArray());
                    return;
                }
            }

            writeSize(count);
            expand(count * 8);
            foreach(long l in v)
            {
                _buf.b.putLong(l);
            }
        }

        public void writeLongSeq(int tag, Ice.Optional<long[]> v)
        {
            if(v.HasValue)
            {
                writeLongSeq(tag, v.Value);
            }
        }

        public void writeLongSeq<T>(int tag, int count, Ice.Optional<T> v)
            where T : IEnumerable<long>
        {
            if(v.HasValue && writeOpt(tag, Ice.OptionalFormat.VSize))
            {
                writeSize(count == 0 ? 1 : count * 8 + (count > 254 ? 5 : 1));
                writeLongSeq(count, v.Value);
            }
        }

        public void writeLongSeq(int tag, long[] v)
        {
            if(writeOpt(tag, Ice.OptionalFormat.VSize))
            {
                writeSize(v == null || v.Length == 0 ? 1 : v.Length * 8 + (v.Length > 254 ? 5 : 1));
                writeLongSeq(v);
            }
        }

        public void writeLongSeq(int tag, int count, IEnumerable<long> v)
        {
            if(writeOpt(tag, Ice.OptionalFormat.VSize))
            {
                writeSize(v == null || count == 0 ? 1 : count * 8 + (count > 254 ? 5 : 1));
                writeLongSeq(count, v);
            }
        }

        public long readLong()
        {
            try
            {
                return _buf.b.getLong();
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        public Ice.Optional<long> readLong(int tag)
        {
            if(readOpt(tag, Ice.OptionalFormat.F8))
            {
                return new Ice.Optional<long>(readLong());
            }
            else
            {
                return new Ice.Optional<long>();
            }
        }

        public void readLong(int tag, out bool isset, out long v)
        {
            if(isset = readOpt(tag, Ice.OptionalFormat.F8))
            {
                v = readLong();
            }
            else
            {
                v = 0;
            }
        }

        public long[] readLongSeq()
        {
            try
            {
                int sz = readAndCheckSeqSize(8);
                long[] v = new long[sz];
                _buf.b.getLongSeq(v);
                return v;
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        public void readLongSeq(out List<long> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new List<long>(readLongSeq());
        }

        public void readLongSeq(out LinkedList<long> l)
        {
            try
            {
                int sz = readAndCheckSeqSize(4);
                l = new LinkedList<long>();
                for(int i = 0; i < sz; ++i)
                {
                    l.AddLast(_buf.b.getLong());
                }
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        public void readLongSeq(out Queue<long> l)
        {
            //
            // Reading into an array and copy-constructing the
            // queue takes the same time as constructing the queue
            // and adding to it one element at a time, so
            // we avoid the copy.
            //
            try
            {
                int sz = readAndCheckSeqSize(4);
                l = new Queue<long>(sz);
                for(int i = 0; i < sz; ++i)
                {
                    l.Enqueue(_buf.b.getLong());
                }
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        public void readLongSeq(out Stack<long> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            long[] array = readLongSeq();
            Array.Reverse(array);
            l = new Stack<long>(array);
        }

        public Ice.Optional<long[]> readLongSeq(int tag)
        {
            if(readOpt(tag, Ice.OptionalFormat.VSize))
            {
                skipSize();
                return new Ice.Optional<long[]>(readLongSeq());
            }
            else
            {
                return new Ice.Optional<long[]>();
            }
        }

        public void readLongSeq(int tag, out bool isset, out long[] v)
        {
            if(isset = readOpt(tag, Ice.OptionalFormat.VSize))
            {
                skipSize();
                v = readLongSeq();
            }
            else
            {
                v = null;
            }
        }

        public void writeFloat(float v)
        {
            expand(4);
            _buf.b.putFloat(v);
        }

        public void writeFloat(int tag, Ice.Optional<float> v)
        {
            if(v.HasValue)
            {
                writeFloat(tag, v.Value);
            }
        }

        public void writeFloat(int tag, float v)
        {
            if(writeOpt(tag, Ice.OptionalFormat.F4))
            {
                writeFloat(v);
            }
        }

        public void writeFloatSeq(float[] v)
        {
            if(v == null)
            {
                writeSize(0);
            }
            else
            {
                writeSize(v.Length);
                expand(v.Length * 4);
                _buf.b.putFloatSeq(v);
            }
        }

        public void writeFloatSeq(int count, IEnumerable<float> v)
        {
            if(count == 0)
            {
                writeSize(0);
                return;
            }

            {
                List<float> value = v as List<float>;
                if(value != null)
                {
                    writeFloatSeq(value.ToArray());
                    return;
                }
            }

            {
                LinkedList<float> value = v as LinkedList<float>;
                if(value != null)
                {
                    writeSize(count);
                    expand(count * 4);
                    IEnumerator<float> i = v.GetEnumerator();
                    while(i.MoveNext())
                    {
                        _buf.b.putFloat(i.Current);
                    }
                    return;
                }
            }

            {
                Queue<float> value = v as Queue<float>;
                if(value != null)
                {
                    writeFloatSeq(value.ToArray());
                    return;
                }
            }

            {
                Stack<float> value = v as Stack<float>;
                if(value != null)
                {
                    writeFloatSeq(value.ToArray());
                    return;
                }
            }

            writeSize(count);
            expand(count * 4);
            foreach(float f in v)
            {
                _buf.b.putFloat(f);
            }
        }

        public void writeFloatSeq(int tag, Ice.Optional<float[]> v)
        {
            if(v.HasValue)
            {
                writeFloatSeq(tag, v.Value);
            }
        }

        public void writeFloatSeq<T>(int tag, int count, Ice.Optional<T> v)
            where T : IEnumerable<float>
        {
            if(v.HasValue && writeOpt(tag, Ice.OptionalFormat.VSize))
            {
                writeSize(count == 0 ? 1 : count * 4 + (count > 254 ? 5 : 1));
                writeFloatSeq(count, v.Value);
            }
        }

        public void writeFloatSeq(int tag, float[] v)
        {
            if(writeOpt(tag, Ice.OptionalFormat.VSize))
            {
                writeSize(v == null || v.Length == 0 ? 1 : v.Length * 4 + (v.Length > 254 ? 5 : 1));
                writeFloatSeq(v);
            }
        }

        public void writeFloatSeq(int tag, int count, IEnumerable<float> v)
        {
            if(writeOpt(tag, Ice.OptionalFormat.VSize))
            {
                writeSize(v == null || count == 0 ? 1 : count * 4 + (count > 254 ? 5 : 1));
                writeFloatSeq(count, v);
            }
        }

        public float readFloat()
        {
            try
            {
                return _buf.b.getFloat();
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        public Ice.Optional<float> readFloat(int tag)
        {
            if(readOpt(tag, Ice.OptionalFormat.F4))
            {
                return new Ice.Optional<float>(readFloat());
            }
            else
            {
                return new Ice.Optional<float>();
            }
        }

        public void readFloat(int tag, out bool isset, out float v)
        {
            if(isset = readOpt(tag, Ice.OptionalFormat.F4))
            {
                v = readFloat();
            }
            else
            {
                v = 0;
            }
        }

        public float[] readFloatSeq()
        {
            try
            {
                int sz = readAndCheckSeqSize(4);
                float[] v = new float[sz];
                _buf.b.getFloatSeq(v);
                return v;
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        public void readFloatSeq(out List<float> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new List<float>(readFloatSeq());
        }

        public void readFloatSeq(out LinkedList<float> l)
        {
            try
            {
                int sz = readAndCheckSeqSize(4);
                l = new LinkedList<float>();
                for(int i = 0; i < sz; ++i)
                {
                    l.AddLast(_buf.b.getFloat());
                }
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        public void readFloatSeq(out Queue<float> l)
        {
            //
            // Reading into an array and copy-constructing the
            // queue takes the same time as constructing the queue
            // and adding to it one element at a time, so
            // we avoid the copy.
            //
            try
            {
                int sz = readAndCheckSeqSize(4);
                l = new Queue<float>(sz);
                for(int i = 0; i < sz; ++i)
                {
                    l.Enqueue(_buf.b.getFloat());
                }
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        public void readFloatSeq(out Stack<float> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            float[] array = readFloatSeq();
            Array.Reverse(array);
            l = new Stack<float>(array);
        }

        public Ice.Optional<float[]> readFloatSeq(int tag)
        {
            if(readOpt(tag, Ice.OptionalFormat.VSize))
            {
                skipSize();
                return new Ice.Optional<float[]>(readFloatSeq());
            }
            else
            {
                return new Ice.Optional<float[]>();
            }
        }

        public void readFloatSeq(int tag, out bool isset, out float[] v)
        {
            if(isset = readOpt(tag, Ice.OptionalFormat.VSize))
            {
                skipSize();
                v = readFloatSeq();
            }
            else
            {
                v = null;
            }
        }

        public void writeDouble(double v)
        {
            expand(8);
            _buf.b.putDouble(v);
        }

        public void writeDouble(int tag, Ice.Optional<double> v)
        {
            if(v.HasValue)
            {
                writeDouble(tag, v.Value);
            }
        }

        public void writeDouble(int tag, double v)
        {
            if(writeOpt(tag, Ice.OptionalFormat.F8))
            {
                writeDouble(v);
            }
        }

        public void writeDoubleSeq(double[] v)
        {
            if(v == null)
            {
                writeSize(0);
            }
            else
            {
                writeSize(v.Length);
                expand(v.Length * 8);
                _buf.b.putDoubleSeq(v);
            }
        }

        public void writeDoubleSeq(int count, IEnumerable<double> v)
        {
            if(count == 0)
            {
                writeSize(0);
                return;
            }

            {
                List<double> value = v as List<double>;
                if(value != null)
                {
                    writeDoubleSeq(value.ToArray());
                    return;
                }
            }

            {
                LinkedList<double> value = v as LinkedList<double>;
                if(value != null)
                {
                    writeSize(count);
                    expand(count * 8);
                    IEnumerator<double> i = v.GetEnumerator();
                    while(i.MoveNext())
                    {
                        _buf.b.putDouble(i.Current);
                    }
                    return;
                }
            }

            {
                Queue<double> value = v as Queue<double>;
                if(value != null)
                {
                    writeDoubleSeq(value.ToArray());
                    return;
                }
            }

            {
                Stack<double> value = v as Stack<double>;
                if (value != null)
                {
                    writeDoubleSeq(value.ToArray());
                    return;
                }
            }

            writeSize(count);
            expand(count * 8);
            foreach(double d in v)
            {
                _buf.b.putDouble(d);
            }
        }

        public void writeDoubleSeq(int tag, Ice.Optional<double[]> v)
        {
            if(v.HasValue)
            {
                writeDoubleSeq(tag, v.Value);
            }
        }

        public void writeDoubleSeq<T>(int tag, int count, Ice.Optional<T> v)
            where T : IEnumerable<double>
        {
            if(v.HasValue && writeOpt(tag, Ice.OptionalFormat.VSize))
            {
                writeSize(count == 0 ? 1 : count * 8 + (count > 254 ? 5 : 1));
                writeDoubleSeq(count, v.Value);
            }
        }

        public void writeDoubleSeq(int tag, double[] v)
        {
            if(writeOpt(tag, Ice.OptionalFormat.VSize))
            {
                writeSize(v == null || v.Length == 0 ? 1 : v.Length * 8 + (v.Length > 254 ? 5 : 1));
                writeDoubleSeq(v);
            }
        }

        public void writeDoubleSeq(int tag, int count, IEnumerable<double> v)
        {
            if(writeOpt(tag, Ice.OptionalFormat.VSize))
            {
                writeSize(v == null || count == 0 ? 1 : count * 8 + (count > 254 ? 5 : 1));
                writeDoubleSeq(count, v);
            }
        }

        public double readDouble()
        {
            try
            {
                return _buf.b.getDouble();
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        public Ice.Optional<double> readDouble(int tag)
        {
            if(readOpt(tag, Ice.OptionalFormat.F8))
            {
                return new Ice.Optional<double>(readDouble());
            }
            else
            {
                return new Ice.Optional<double>();
            }
        }

        public void readDouble(int tag, out bool isset, out double v)
        {
            if(isset = readOpt(tag, Ice.OptionalFormat.F8))
            {
                v = readDouble();
            }
            else
            {
                v = 0;
            }
        }

        public double[] readDoubleSeq()
        {
            try
            {
                int sz = readAndCheckSeqSize(8);
                double[] v = new double[sz];
                _buf.b.getDoubleSeq(v);
                return v;
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        public void readDoubleSeq(out List<double> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new List<double>(readDoubleSeq());
        }

        public void readDoubleSeq(out LinkedList<double> l)
        {
            try
            {
                int sz = readAndCheckSeqSize(4);
                l = new LinkedList<double>();
                for(int i = 0; i < sz; ++i)
                {
                    l.AddLast(_buf.b.getDouble());
                }
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        public void readDoubleSeq(out Queue<double> l)
        {
            //
            // Reading into an array and copy-constructing the
            // queue takes the same time as constructing the queue
            // and adding to it one element at a time, so
            // we avoid the copy.
            //
            try
            {
                int sz = readAndCheckSeqSize(4);
                l = new Queue<double>(sz);
                for(int i = 0; i < sz; ++i)
                {
                    l.Enqueue(_buf.b.getDouble());
                }
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        public void readDoubleSeq(out Stack<double> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            double[] array = readDoubleSeq();
            Array.Reverse(array);
            l = new Stack<double>(array);
        }

        public Ice.Optional<double[]> readDoubleSeq(int tag)
        {
            if(readOpt(tag, Ice.OptionalFormat.VSize))
            {
                skipSize();
                return new Ice.Optional<double[]>(readDoubleSeq());
            }
            else
            {
                return new Ice.Optional<double[]>();
            }
        }

        public void readDoubleSeq(int tag, out bool isset, out double[] v)
        {
            if(isset = readOpt(tag, Ice.OptionalFormat.VSize))
            {
                skipSize();
                v = readDoubleSeq();
            }
            else
            {
                v = null;
            }
        }

        private static System.Text.UTF8Encoding utf8 = new System.Text.UTF8Encoding(false, true);

        public void writeString(string v)
        {
            if(v == null || v.Length == 0)
            {
                writeSize(0);
                return;
            }
            byte[] arr = utf8.GetBytes(v);
            writeSize(arr.Length);
            expand(arr.Length);
            _buf.b.put(arr);
        }

        public void writeString(int tag, Ice.Optional<string> v)
        {
            if(v.HasValue)
            {
                writeString(tag, v.Value);
            }
        }

        public void writeString(int tag, string v)
        {
            if(writeOpt(tag, Ice.OptionalFormat.VSize))
            {
                writeString(v);
            }
        }

        public void writeStringSeq(string[] v)
        {
            if(v == null)
            {
                writeSize(0);
            }
            else
            {
                writeSize(v.Length);
                for(int i = 0; i < v.Length; i++)
                {
                    writeString(v[i]);
                }
            }
        }

        public void writeStringSeq(int size, IEnumerable<string> e)
        {
            writeSize(size);
            if(size != 0)
            {
                foreach(string s in e)
                {
                    writeString(s);
                }
            }
        }

        public void writeStringSeq(int tag, Ice.Optional<String[]> v)
        {
            if(v.HasValue)
            {
                writeStringSeq(tag, v.Value);
            }
        }

        public void writeStringSeq<T>(int tag, int count, Ice.Optional<T> v)
            where T : IEnumerable<string>
        {
            if(v.HasValue && writeOpt(tag, Ice.OptionalFormat.FSize))
            {
                startSize();
                writeStringSeq(count, v.Value);
                endSize();
            }
        }

        public void writeStringSeq(int tag, string[] v)
        {
            if(writeOpt(tag, Ice.OptionalFormat.FSize))
            {
                startSize();
                writeStringSeq(v);
                endSize();
            }
        }

        public void writeStringSeq(int tag, int count, IEnumerable<string> v)
        {
            if(writeOpt(tag, Ice.OptionalFormat.FSize))
            {
                startSize();
                writeStringSeq(count, v);
                endSize();
            }
        }

        public string readString()
        {
            int len = readSize();

            if(len == 0)
            {
                return "";
            }

            //
            // Check the buffer has enough bytes to read.
            //
            if(_buf.b.remaining() < len)
            {
                throw new Ice.UnmarshalOutOfBoundsException();
            }

            try
            {
                //
                // We reuse the _stringBytes array to avoid creating
                // excessive garbage
                //
                if(_stringBytes == null || len > _stringBytes.Length)
                {
                    _stringBytes = new byte[len];
                }
                _buf.b.get(_stringBytes, 0, len);
                return utf8.GetString(_stringBytes, 0, len);
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
            catch(System.ArgumentException ex)
            {
                throw new Ice.MarshalException("Invalid UTF8 string", ex);
            }
        }

        public Ice.Optional<string> readString(int tag)
        {
            if(readOpt(tag, Ice.OptionalFormat.VSize))
            {
                return new Ice.Optional<string>(readString());
            }
            else
            {
                return new Ice.Optional<string>();
            }
        }

        public void readString(int tag, out bool isset, out string v)
        {
            if(isset = readOpt(tag, Ice.OptionalFormat.VSize))
            {
                v = readString();
            }
            else
            {
                v = null;
            }
        }

        public string[] readStringSeq()
        {
            int sz = readAndCheckSeqSize(1);
            string[] v = new string[sz];
            for(int i = 0; i < sz; i++)
            {
                v[i] = readString();
            }
            return v;
        }

        public void readStringSeq(out List<string> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is slower than constructing the list
            // and adding to it one element at a time.
            //
            int sz = readAndCheckSeqSize(1);
            l = new List<string>(sz);
            for(int i = 0; i < sz; ++i)
            {
                l.Add(readString());
            }
        }

        public void readStringSeq(out LinkedList<string> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is slower than constructing the list
            // and adding to it one element at a time.
            //
            int sz = readAndCheckSeqSize(1);
            l = new LinkedList<string>();
            for(int i = 0; i < sz; ++i)
            {
                l.AddLast(readString());
            }
        }

        public void readStringSeq(out Queue<string> l)
        {
            //
            // Reading into an array and copy-constructing the
            // queue is slower than constructing the queue
            // and adding to it one element at a time.
            //
            int sz = readAndCheckSeqSize(1);
            l = new Queue<string>();
            for(int i = 0; i < sz; ++i)
            {
                l.Enqueue(readString());
            }
        }

        public void readStringSeq(out Stack<string> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            string[] array = readStringSeq();
            Array.Reverse(array);
            l = new Stack<string>(array);
        }

        public Ice.Optional<string[]> readStringSeq(int tag)
        {
            if(readOpt(tag, Ice.OptionalFormat.FSize))
            {
                skip(4);
                return new Ice.Optional<string[]>(readStringSeq());
            }
            else
            {
                return new Ice.Optional<string[]>();
            }
        }

        public void readStringSeq(int tag, out bool isset, out string[] v)
        {
            if(isset = readOpt(tag, Ice.OptionalFormat.FSize))
            {
                skip(4);
                v = readStringSeq();
            }
            else
            {
                v = null;
            }
        }

        public void writeProxy(Ice.ObjectPrx v)
        {
            instance_.proxyFactory().proxyToStream(v, this);
        }

        public void writeProxy(int tag, Ice.Optional<Ice.ObjectPrx> v)
        {
            if(v.HasValue)
            {
                writeProxy(tag, v.Value);
            }
        }

        public void writeProxy(int tag, Ice.ObjectPrx v)
        {
            if(writeOpt(tag, Ice.OptionalFormat.FSize))
            {
                startSize();
                writeProxy(v);
                endSize();
            }
        }

        public Ice.ObjectPrx readProxy()
        {
            return instance_.proxyFactory().streamToProxy(this);
        }

        public Ice.Optional<Ice.ObjectPrx> readProxy(int tag)
        {
            if(readOpt(tag, Ice.OptionalFormat.FSize))
            {
                skip(4);
                return new Ice.Optional<Ice.ObjectPrx>(readProxy());
            }
            else
            {
                return new Ice.Optional<Ice.ObjectPrx>();
            }
        }

        public void readProxy(int tag, out bool isset, out Ice.ObjectPrx v)
        {
            if(isset = readOpt(tag, Ice.OptionalFormat.FSize))
            {
                skip(4);
                v = readProxy();
            }
            else
            {
                v = null;
            }
        }

        public void writeEnum(int v, int maxValue)
        {
            if(isWriteEncoding_1_0())
            {
                if(maxValue < 127)
                {
                    writeByte((byte)v);
                }
                else if(maxValue < 32767)
                {
                    writeShort((short)v);
                }
                else
                {
                    writeInt(v);
                }
            }
            else
            {
                writeSize(v);
            }
        }

        public void writeEnum(int tag, int v, int maxValue)
        {
            if(writeOpt(tag, Ice.OptionalFormat.Size))
            {
                writeEnum(v, maxValue);
            }
        }

        public int readEnum(int maxValue)
        {
            if(getReadEncoding().Equals(Ice.Util.Encoding_1_0))
            {
                if(maxValue < 127)
                {
                    return readByte();
                }
                else if(maxValue < 32767)
                {
                    return readShort();
                }
                else
                {
                    return readInt();
                }
            }
            else
            {
                return readSize();
            }
        }

        public void writeObject(Ice.Object v)
        {
            initWriteEncaps();
            _writeEncapsStack.encoder.writeObject(v);
        }

        public void writeObject<T>(int tag, Ice.Optional<T> v)
            where T : Ice.Object
        {
            if(v.HasValue)
            {
                writeObject(tag, v.Value);
            }
        }

        public void writeObject(int tag, Ice.Object v)
        {
            if(writeOpt(tag, Ice.OptionalFormat.Size))
            {
                writeObject(v);
            }
        }

        public void readObject(IPatcher patcher)
        {
            Debug.Assert(patcher != null);
            initReadEncaps();
            _readEncapsStack.decoder.readObject(patcher);
        }

        public void readObject(int tag, IPatcher patcher)
        {
            if(readOpt(tag, Ice.OptionalFormat.Size))
            {
                readObject(patcher);
            }
        }

        public void writeUserException(Ice.UserException v)
        {
            initWriteEncaps();
            _writeEncapsStack.encoder.writeUserException(v);

            //
            // Reset the encoder, the writing of the exception wrote
            // pending objects if any.
            //
            _writeEncapsStack.encoder = null;
        }

        public void throwException(UserExceptionFactory factory)
        {
            initReadEncaps();
            try
            {
                _readEncapsStack.decoder.throwException(factory);
            }
            catch(Ice.UserException ex)
            {
                _readEncapsStack.decoder = null;
                throw ex;
            }
        }

        public void sliceObjects(bool b)
        {
            _sliceObjects = b;
        }

        public bool readOptImpl(int readTag, Ice.OptionalFormat expectedFormat)
        {
            if(isReadEncoding_1_0())
            {
                return false; // Optional members aren't supported with the 1.0 encoding.
            }

            int tag = 0;
            Ice.OptionalFormat format;
            do
            {
                if(_buf.b.position() >= _readEncapsStack.start + _readEncapsStack.sz)
                {
                    return false; // End of encapsulation also indicates end of optionals.
                }

                int v = readByte();
                format = (Ice.OptionalFormat)(v & 0x07); // First 3 bits.
                tag = v >> 3;
                if(tag == 31)
                {
                    tag = readSize();
                }
            }
            while(format != Ice.OptionalFormat.EndMarker && tag < readTag && skipOpt(format)); // Skip optional data members

            if(format == Ice.OptionalFormat.EndMarker || tag > readTag)
            {
                //
                // Rewind the stream to correctly read the next optional data
                // member tag & format next time.
                //
                int offset = tag < 31 ? 1 : (tag < 255 ? 2 : 6);
                _buf.b.position(_buf.b.position() - offset);
                return false; // No optional data members with the requested tag.
            }

            Debug.Assert(readTag == tag);
            if(format != expectedFormat)
            {
                string msg = "invalid optional data member `" + tag + "': unexpected format";
                throw new Ice.MarshalException(msg);
            }

            //
            // We have an optional data member with the requested tag and
            // format.
            //
            return true;
        }

        public bool writeOptImpl(int tag, Ice.OptionalFormat format)
        {
            if(isWriteEncoding_1_0())
            {
                return false; // Optional members aren't supported with the 1.0 encoding.
            }

            int v = (int)format;
            if(tag < 31)
            {
                v |= tag << 3;
                writeByte((byte)v);
            }
            else
            {
                v |= 0x0F8; // tag = 31
                writeByte((byte)v);
                writeSize(tag);
            }
            return true;
        }

        public bool skipOpt(Ice.OptionalFormat format)
        {
            int sz;
            switch(format)
            {
            case Ice.OptionalFormat.F1:
            {
                sz = 1;
                break;
            }
            case Ice.OptionalFormat.F2:
            {
                sz = 2;
                break;
            }
            case Ice.OptionalFormat.F4:
            {
                sz = 4;
                break;
            }
            case Ice.OptionalFormat.F8:
            {
                sz = 8;
                break;
            }
            case Ice.OptionalFormat.Size:
            {
                skipSize();
                return true;
            }
            case Ice.OptionalFormat.VSize:
            {
                sz = readSize();
                break;
            }
            case Ice.OptionalFormat.FSize:
            {
                sz = readInt();
                break;
            }
            default:
            {
                return false;
            }
            }
            skip(sz);
            return true;
        }

        public bool skipOpts()
        {
            //
            // Skip remaining un-read optional members.
            //
            Ice.OptionalFormat format;
            do
            {
                if(_buf.b.position() >= _readEncapsStack.start + _readEncapsStack.sz)
                {
                    return false; // End of encapsulation also indicates end of optionals.
                }

                int v = readByte();
                format = (Ice.OptionalFormat)(v & 0x07); // Read first 3 bits.
                if((v >> 3) == 31)
                {
                    skipSize();
                }
            }
            while(skipOpt(format));
            Debug.Assert(format == Ice.OptionalFormat.EndMarker);
            return true;
        }

        public void skip(int size)
        {
            if(size > _buf.b.remaining())
            {
                throw new Ice.UnmarshalOutOfBoundsException();
            }
            _buf.b.position(_buf.b.position() + size);
        }

        public void skipSize()
        {
            byte b = readByte();
            if(b == 255)
            {
                skip(4);
            }
        }

        public int pos()
        {
            return _buf.b.position();
        }

        public void pos(int n)
        {
            _buf.b.position(n);
        }

        public int size()
        {
            return _buf.size();
        }

#if !MANAGED && !COMPACT && !SILVERLIGHT
        static string getBZ2Error(int error)
        {
            string rc;

            switch(error)
            {
                case BZ_SEQUENCE_ERROR:
                {
                    rc = "BZ_SEQUENCE_ERROR";
                    break;
                }
                case BZ_PARAM_ERROR:
                {
                    rc = "BZ_PARAM_ERROR";
                    break;
                }
                case BZ_MEM_ERROR:
                {
                    rc = "BZ_MEM_ERROR";
                    break;
                }
                case BZ_DATA_ERROR:
                {
                    rc = "BZ_DATA_ERROR";
                    break;
                }
                case BZ_DATA_ERROR_MAGIC:
                {
                    rc = "BZ_DATA_ERROR_MAGIC";
                    break;
                }
                case BZ_IO_ERROR:
                {
                    rc = "BZ_IO_ERROR";
                    break;
                }
                case BZ_UNEXPECTED_EOF:
                {
                    rc = "BZ_UNEXPECTED_EOF";
                    break;
                }
                case BZ_OUTBUFF_FULL:
                {
                    rc = "BZ_OUTBUFF_FULL";
                    break;
                }
                case BZ_CONFIG_ERROR:
                {
                    rc = "BZ_CONFIG_ERROR";
                    break;
                }
                default:
                {
                    rc = "Unknown bzip2 error: " + error;
                    break;
                }
            }
            return rc;
        }
#endif

        public static bool compressible()
        {
            return _bzlibInstalled;
        }

        public bool compress(ref BasicStream cstream, int headerSize, int compressionLevel)
        {
#if MANAGED || COMPACT || SILVERLIGHT
            cstream = this;
            return false;
#else
            if(!_bzlibInstalled)
            {
                cstream = this;
                return false;
            }

            //
            // Compress the message body, but not the header.
            //
            int uncompressedLen = size() - headerSize;
            byte[] uncompressed = _buf.b.rawBytes(headerSize, uncompressedLen);
            int compressedLen = (int)(uncompressedLen * 1.01 + 600);
            byte[] compressed = new byte[compressedLen];

            int rc = NativeMethods.BZ2_bzBuffToBuffCompress(compressed, ref compressedLen, uncompressed,
                                                            uncompressedLen, compressionLevel, 0, 0);
            if(rc == BZ_OUTBUFF_FULL)
            {
                cstream = null;
                return false;
            }
            else if(rc < 0)
            {
                Ice.CompressionException ex = new Ice.CompressionException("BZ2_bzBuffToBuffCompress failed");
                ex.reason = getBZ2Error(rc);
                throw ex;
            }

            //
            // Don't bother if the compressed data is larger than the
            // uncompressed data.
            //
            if(compressedLen >= uncompressedLen)
            {
                return false;
            }

            cstream = new BasicStream(instance_, _encoding);
            cstream.resize(headerSize + 4 + compressedLen, false);
            cstream.pos(0);

            //
            // Copy the header from the uncompressed stream to the
            // compressed one.
            //
            cstream._buf.b.put(_buf.b.rawBytes(0, headerSize));

            //
            // Add the size of the uncompressed stream before the
            // message body.
            //
            cstream.writeInt(size());

            //
            // Add the compressed message body.
            //
            cstream._buf.b.put(compressed, 0, compressedLen);

            return true;
#endif
        }

        public BasicStream uncompress(int headerSize)
        {
#if MANAGED || COMPACT || SILVERLIGHT
            return this;
#else
            if(!_bzlibInstalled)
            {
                return this;
            }

            pos(headerSize);
            int uncompressedSize = readInt();
            if(uncompressedSize <= headerSize)
            {
                throw new Ice.IllegalMessageSizeException("compressed size <= header size");
            }

            int compressedLen = size() - headerSize - 4;
            byte[] compressed = _buf.b.rawBytes(headerSize + 4, compressedLen);
            int uncompressedLen = uncompressedSize - headerSize;
            byte[] uncompressed = new byte[uncompressedLen];
            int rc = NativeMethods.BZ2_bzBuffToBuffDecompress(uncompressed, ref uncompressedLen, compressed,
                                                              compressedLen, 0, 0);
            if(rc < 0)
            {
                Ice.CompressionException ex = new Ice.CompressionException("BZ2_bzBuffToBuffDecompress failed");
                ex.reason = getBZ2Error(rc);
                throw ex;
            }
            BasicStream ucStream = new BasicStream(instance_, _encoding);
            ucStream.resize(uncompressedSize, false);
            ucStream.pos(0);
            ucStream._buf.b.put(_buf.b.rawBytes(0, headerSize));
            ucStream._buf.b.put(uncompressed, 0, uncompressedLen);
            return ucStream;
#endif
        }

        public bool isEmpty()
        {
            return _buf.empty();
        }

        public void expand(int n)
        {
            if(!_unlimited && _buf.b != null && _buf.b.position() + n > _messageSizeMax)
            {
                Ex.throwMemoryLimitException(_buf.b.position() + n, _messageSizeMax);
            }
            _buf.expand(n);
        }

        private Ice.Object createObject(string id)
        {
            Ice.Object obj = null;

            try
            {
                Type c = AssemblyUtil.findType(instance_, typeToClass(id));
                //
                // Ensure the class is instantiable.
                //
                if(c != null && !c.IsAbstract && !c.IsInterface)
                {
                    obj = (Ice.Object)AssemblyUtil.createInstance(c);
                }
            }
            catch(Exception ex)
            {
                Ice.NoObjectFactoryException e = new Ice.NoObjectFactoryException(ex);
                e.type = id;
                throw e;
            }

            return obj;
        }

        private sealed class DynamicUserExceptionFactory : UserExceptionFactory
        {
            internal DynamicUserExceptionFactory(Type c)
            {
                _class = c;
            }

            public void createAndThrow(string typeId)
            {
                try
                {
                    throw (Ice.UserException)AssemblyUtil.createInstance(_class);
                }
                catch(Ice.UserException)
                {
                    throw;
                }
                catch(Exception ex)
                {
                    throw new Ice.SyscallException(ex);
                }
            }

            public void destroy()
            {
            }

            private Type _class;
        }

        private Ice.UserException createUserException(string id)
        {
            Ice.UserException userEx = null;

            try
            {
                Type c = AssemblyUtil.findType(instance_, typeToClass(id));
                if(c != null)
                {
                    //
                    // Ensure the class is instantiable.
                    //
                    Debug.Assert(!c.IsAbstract && !c.IsInterface);
                    userEx = (Ice.UserException)AssemblyUtil.createInstance(c);
                }
            }
            catch(Exception ex)
            {
                throw new Ice.MarshalException(ex);
            }

            return userEx;
        }

        private static string typeToClass(string id)
        {
            if(!id.StartsWith("::", StringComparison.Ordinal))
            {
                throw new Ice.MarshalException("expected type id but received `" + id + "'");
            }
            return id.Substring(2).Replace("::", ".");
        }

        //
        // Optional data member type.
        //
        internal const int MemberFormatF1           = 0;
        internal const int MemberFormatF2           = 1;
        internal const int MemberFormatF4           = 2;
        internal const int MemberFormatF8           = 3;
        internal const int MemberFormatVSize        = 4;
        internal const int MemberFormatFSize        = 5;
        internal const int MemberFormatReserved     = 6;
        internal const int MemberFormatEndMarker    = 7;

        private Instance instance_;
        private Buffer _buf;
        private object _closure;
        private byte[] _stringBytes; // Reusable array for reading strings.

        private enum SliceType { NoSlice, ObjectSlice, ExceptionSlice }

        private sealed class EncapsDecoder
        {
            internal EncapsDecoder(BasicStream stream, ReadEncaps encaps, bool sliceObjects)
            {
                _stream = stream;
                _encaps = encaps;
                _sliceObjects = sliceObjects;
                _traceSlicing = -1;
                _sliceType = SliceType.NoSlice;
                _typeIdIndex = 0;
                _slices = new List<Ice.SliceInfo>();
                _indirectionTables = new List<int[]>();
                _indirectPatchList = new List<IndirectPatchEntry>();
                _patchMap = new Dictionary<int, LinkedList<IPatcher>>();
                _unmarshaledMap = new Dictionary<int, Ice.Object>();
                _typeIdMap = new Dictionary<int, string>();
            }

            internal void readObject(IPatcher patcher)
            {
                int index = 0;
                if(_encaps.encoding_1_0)
                {
                    //
                    // Object references are encoded as a negative integer in 1.0.
                    //
                    index = _stream.readInt();
                    if(index > 0)
                    {
                        throw new Ice.MarshalException("invalid object id");
                    }
                    index = -index;
                }
                else
                {
                    //
                    // Later versions use a size.
                    //
                    index = _stream.readSize();
                    if(index < 0)
                    {
                        throw new Ice.MarshalException("invalid object id");
                    }
                }

                if(index == 0)
                {
                    patcher.patch(null);
                }
                else if(_sliceType != SliceType.NoSlice && (_sliceFlags & FLAG_HAS_INDIRECTION_TABLE) != 0)
                {
                    //
                    // Maintain a list of indirect references. Note that the indirect index
                    // starts at 1, so we decrement it by one to derive an index into
                    // the indirection table that we'll read at the end of the slice.
                    //
                    IndirectPatchEntry e = new IndirectPatchEntry();
                    e.index = index - 1;
                    e.patcher = patcher;
                    _indirectPatchList.Add(e);
                }
                else
                {
                    addPatchEntry(index, patcher);
                }
            }

            internal void throwException(UserExceptionFactory factory)
            {
                Debug.Assert(_sliceType == SliceType.NoSlice);

                //
                // User exception with the 1.0 encoding start with a boolean flag
                // that indicates whether or not the exception has classes.
                //
                // This allows reading the pending objects even if some part of
                // the exception was sliced. With encoding > 1.0, we don't need
                // this, each slice indirect patch table indicates the presence of
                // objects.
                //
                bool usesClasses;
                if(_encaps.encoding_1_0)
                {
                    usesClasses = _stream.readBool();
                }
                else
                {
                    usesClasses = true; // Always call readPendingObjects.
                }

                _sliceType = SliceType.ExceptionSlice;
                _skipFirstSlice = false;

                //
                // Read the first slice header.
                //
                startSlice();
                string mostDerivedId = _typeId;
                while(true)
                {
                    Ice.UserException userEx = null;

                    //
                    // Use a factory if one was provided.
                    //
                    if(factory != null)
                    {
                        try
                        {
                            factory.createAndThrow(_typeId);
                        }
                        catch(Ice.UserException ex)
                        {
                            userEx = ex;
                        }
                    }

                    if(userEx == null)
                    {
                        userEx = _stream.createUserException(_typeId);
                    }

                    //
                    // We found the exception.
                    //
                    if(userEx != null)
                    {
                        userEx.read__(_stream);
                        if(usesClasses)
                        {
                            readPendingObjects();
                        }
                        throw userEx;

                        // Never reached.
                    }

                    //
                    // Performance sensitive, so we use lazy initialization for
                    // tracing.
                    //
                    if(_traceSlicing == -1)
                    {
                        _traceSlicing = _stream.instance().traceLevels().slicing;
                        _slicingCat = _stream.instance().traceLevels().slicingCat;
                    }
                    if(_traceSlicing > 0)
                    {
                        TraceUtil.traceSlicing("exception", _typeId, _slicingCat,
                                               _stream.instance().initializationData().logger);
                    }

                    //
                    // Slice off what we don't understand.
                    //
                    skipSlice();

                    if((_sliceFlags & FLAG_IS_LAST_SLICE) != 0)
                    {
                        if(mostDerivedId.Length > 2 && mostDerivedId[0] == ':' && mostDerivedId[1] == ':')
                        {
                            throw new Ice.UnknownUserException(mostDerivedId.Substring(2));
                        }
                        else
                        {
                            throw new Ice.UnknownUserException(mostDerivedId);
                        }
                    }

                    try
                    {
                        startSlice();
                    }
                    catch(Ice.UnmarshalOutOfBoundsException ex)
                    {
                        //
                        // An oversight in the 1.0 encoding means there is no marker to indicate
                        // the last slice of an exception. As a result, we just try to read the
                        // next type ID, which raises UnmarshalOutOfBoundsException when the
                        // input buffer underflows.
                        //
                        if(_encaps.encoding_1_0)
                        {
                            // Set the reason member to a more helpful message.
                            ex.reason = "unknown exception type `" + mostDerivedId + "'";
                        }
                        throw ex;
                    }
                }
            }

            internal void startObject()
            {
                Debug.Assert(_sliceType == SliceType.ObjectSlice);
                _skipFirstSlice = true;
            }

            internal Ice.SlicedData endObject(bool preserve)
            {
                if(_encaps.encoding_1_0)
                {
                    //
                    // Read the Ice::Object slice.
                    //
                    startSlice();

                    //
                    // For compatibility with the old AFM.
                    //
                    int sz = _stream.readSize();
                    if(sz != 0)
                    {
                        throw new Ice.MarshalException("invalid Object slice");
                    }

                    endSlice();
                }

                _sliceType = SliceType.NoSlice;
                Ice.SlicedData slicedData = null;
                if(preserve)
                {
                    slicedData = readSlicedData();
                }
                _slices.Clear();
                _indirectionTables.Clear();
                return slicedData;
            }

            internal void startException()
            {
                Debug.Assert(_sliceType == SliceType.ExceptionSlice);
                _skipFirstSlice = true;
            }

            internal Ice.SlicedData endException(bool preserve)
            {
                _sliceType = SliceType.NoSlice;
                Ice.SlicedData slicedData = null;
                if(preserve)
                {
                    slicedData = readSlicedData();
                }
                _slices.Clear();
                _indirectionTables.Clear();
                return slicedData;
            }

            internal string startSlice()
            {
                //
                // If first slice, don't read the header, it was already read in
                // readInstance or throwException to find the factory.
                //
                if(_skipFirstSlice)
                {
                    _skipFirstSlice = false;
                    return _typeId;
                }

                //
                // Read the slice flags. For the 1.0 encoding there's no flag but
                // just a bool for object slices. The bool indicates whether
                // or not the type ID is encoded as a string or as an index.
                //
                if(_encaps.encoding_1_0)
                {
                    _sliceFlags = FLAG_HAS_SLICE_SIZE;
                    if(_sliceType == SliceType.ObjectSlice) // For exceptions, the type ID is always encoded as a string
                    {
                        bool isIndex = _stream.readBool();
                        _sliceFlags |= isIndex ? FLAG_HAS_TYPE_ID_INDEX : FLAG_HAS_TYPE_ID_STRING;
                    }
                }
                else
                {
                    _sliceFlags = _stream.readByte();
                }

                //
                // Read the type ID, for object slices the type ID is encoded as a
                // string or as an index, for exceptions it's always encoded as a
                // string.
                //
                if(_sliceType == SliceType.ObjectSlice)
                {
                    if((_sliceFlags & FLAG_HAS_TYPE_ID_INDEX) != 0)
                    {
                        int index = _stream.readSize();
                        if(!_typeIdMap.TryGetValue(index, out _typeId))
                        {
                            throw new Ice.UnmarshalOutOfBoundsException();
                        }
                    }
                    else if((_sliceFlags & FLAG_HAS_TYPE_ID_STRING) != 0)
                    {
                        _typeId = _stream.readString();
                        _typeIdMap.Add(++_typeIdIndex, _typeId);
                    }
                    else
                    {
                        // Only the most derived slice encodes the type ID for the
                        // compact format.
                        _typeId = "";
                    }
                }
                else
                {
                    _typeId = _stream.readString();
                }

                //
                // Read the slice size if necessary.
                //
                if((_sliceFlags & FLAG_HAS_SLICE_SIZE) != 0)
                {
                    _sliceSize = _stream.readInt();
                    if(_sliceSize < 4)
                    {
                        throw new Ice.UnmarshalOutOfBoundsException();
                    }
                }
                else
                {
                    _sliceSize = 0;
                }

                //
                // Reset the indirect patch list for this new slice.
                //
                _indirectPatchList.Clear();
                return _typeId;
            }

            internal void endSlice()
            {
                if((_sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS) != 0)
                {
                    _stream.skipOpts();
                }

                //
                // Read the indirection table if one is present and transform the
                // indirect patch list into patch entries with direct references.
                //
                if((_sliceFlags & FLAG_HAS_INDIRECTION_TABLE) != 0)
                {
                    //
                    // The table is written as a sequence<size> to conserve space.
                    //
                    int[] indirectionTable = _stream.readSizeSeq();

                    //
                    // Sanity checks. If there are optional members, it's possible
                    // that not all object references were read if they are from
                    // unknown optional data members.
                    //
                    if(indirectionTable.Length == 0 && _indirectPatchList.Count > 0)
                    {
                        throw new Ice.MarshalException("empty indirection table");
                    }
                    else if(indirectionTable.Length > 0 && _indirectPatchList.Count == 0 &&
                            (_sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS) == 0)
                    {
                        throw new Ice.MarshalException("no references to indirection table");
                    }

                    //
                    // Convert indirect references into direct references.
                    //
                    foreach(IndirectPatchEntry e in _indirectPatchList)
                    {
                        Debug.Assert(e.index >= 0);
                        if(e.index >= indirectionTable.Length)
                        {
                            throw new Ice.MarshalException("indirection out of range");
                        }
                        int id = indirectionTable[e.index];
                        if(id <= 0)
                        {
                            //
                            // Entries in the table must be positive, just like a regular object reference.
                            //
                            throw new Ice.MarshalException("invalid id in object indirection table");
                        }
                        addPatchEntry(id, e.patcher);
                    }
                }
            }

            internal void skipSlice()
            {
                int start = _stream.pos();

                if((_sliceFlags & FLAG_HAS_SLICE_SIZE) != 0)
                {
                    Debug.Assert(_sliceSize >= 4);
                    _stream.skip(_sliceSize - 4);
                }
                else
                {
                    if(_sliceType == SliceType.ObjectSlice)
                    {
                        throw new Ice.NoObjectFactoryException(
                            "compact format prevents slicing (the sender should use the sliced format instead)", 
                            _typeId);
                    }
                    else
                    {
                        throw new Ice.UnknownUserException(_typeId.Substring(2));
                    }
                }

                if(!_encaps.encoding_1_0)
                {
                    //
                    // Preserve this slice.
                    //
                    Ice.SliceInfo info = new Ice.SliceInfo();
                    info.typeId = _typeId;
                    info.hasOptionalMembers = (_sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS) != 0;
                    info.isLastSlice = (_sliceFlags & FLAG_IS_LAST_SLICE) != 0;
                    ByteBuffer b = _stream.getBuffer().b;
                    int end = b.position();
                    int dataEnd = end;
                    if(info.hasOptionalMembers)
                    {
                        //
                        // Don't include the optional member end marker. It will be re-written by
                        // endSlice when the sliced data is re-written.
                        //
                        --dataEnd;
                    }
                    info.bytes = new byte[dataEnd - start];
                    b.position(start);
                    b.get(info.bytes);
                    b.position(end);
                    _slices.Add(info);

                    if((_sliceFlags & FLAG_HAS_INDIRECTION_TABLE) != 0)
                    {
                        //
                        // Read the indirection table, which is written as a sequence<size> to conserve space.
                        //
                        _indirectionTables.Add(_stream.readSizeSeq());
                    }
                    else
                    {
                        _indirectionTables.Add(new int[0]);
                    }
                }
            }

            internal bool readOpt(int readTag, Ice.OptionalFormat expectedFormat)
            {
                if(_sliceType == SliceType.NoSlice)
                {
                    return _stream.readOptImpl(readTag, expectedFormat);
                }
                else if((_sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS) != 0)
                {
                    return _stream.readOptImpl(readTag, expectedFormat);
                }
                return false;
            }

            internal void readPendingObjects()
            {
                //
                // With the 1.0 encoding, we read pending objects if the marshaled
                // data uses classes. Otherwise, only read pending objects if some
                // non-nil references were read.
                //
                if(!_encaps.encoding_1_0)
                {
                    if(_patchMap.Count == 0)
                    {
                        return;
                    }
                    else
                    {
                        //
                        // Read unread encapsulation optionals before reading the
                        // pending objects.
                        //
                        _stream.skipOpts();
                    }
                }

                int num;
                List<Ice.Object> objectList = new List<Ice.Object>();
                do
                {
                    num = _stream.readSize();
                    for(int k = num; k > 0; --k)
                    {
                        objectList.Add(readInstance());
                    }
                }
                while(num > 0);

                if(_patchMap.Count > 0)
                {
                    //
                    // If any entries remain in the patch map, the sender has sent an index for an object, but failed
                    // to supply the object.
                    //
                    throw new Ice.MarshalException("index for class received, but no instance");
                }

                //
                // Iterate over the object list and invoke ice_postUnmarshal on
                // each object.  We must do this after all objects have been
                // unmarshaled in order to ensure that any object data members
                // have been properly patched.
                //
                foreach(Ice.Object p in objectList)
                {
                    try
                    {
                        p.ice_postUnmarshal();
                    }
                    catch(System.Exception ex)
                    {
                        _stream.instance().initializationData().logger.warning(
                            "exception raised by ice_postUnmarshal:\n" + ex);
                    }
                }
            }

            private Ice.Object readInstance()
            {
                int index;
                if(_encaps.encoding_1_0)
                {
                    index = _stream.readInt();
                }
                else
                {
                    index = _stream.readSize();
                }

                Ice.Object v = null;
                if(index <= 0)
                {
                    throw new Ice.MarshalException("invalid object id");
                }

                _sliceType = SliceType.ObjectSlice;
                _skipFirstSlice = false;

                //
                // Read the first slice header.
                //
                startSlice();
                string mostDerivedId = _typeId;
                ObjectFactoryManager servantFactoryManager = _stream.instance().servantFactoryManager();
                while(true)
                {
                    //
                    // For the 1.0 encoding, the type ID for the base Object class
                    // marks the last slice.
                    //
                    if(_typeId.Equals(Ice.ObjectImpl.ice_staticId()))
                    {
                        throw new Ice.NoObjectFactoryException("", mostDerivedId);
                    }

                    //
                    // Try to find a factory registered for the specific type.
                    //
                    Ice.ObjectFactory userFactory = servantFactoryManager.find(_typeId);
                    if(userFactory != null)
                    {
                        v = userFactory.create(_typeId);
                    }

                    //
                    // If that fails, invoke the default factory if one has been
                    // registered.
                    //
                    if(v == null)
                    {
                        userFactory = servantFactoryManager.find("");
                        if(userFactory != null)
                        {
                            v = userFactory.create(_typeId);
                        }
                    }

                    //
                    // Last chance: try to instantiate the class dynamically.
                    //
                    if(v == null)
                    {
                        v = _stream.createObject(_typeId);
                    }

                    //
                    // We found a factory, we get out of this loop.
                    //
                    if(v != null)
                    {
                        break;
                    }

                    //
                    // Performance sensitive, so we use lazy initialization for tracing.
                    //
                    if(_traceSlicing == -1)
                    {
                        _traceSlicing = _stream.instance().traceLevels().slicing;
                        _slicingCat = _stream.instance().traceLevels().slicingCat;
                    }
                    if(_traceSlicing > 0)
                    {
                        TraceUtil.traceSlicing("class", _typeId, _slicingCat,
                                               _stream.instance().initializationData().logger);
                    }

                    //
                    // If object slicing is disabled, stop un-marshalling.
                    //
                    if(!_sliceObjects)
                    {
                        throw new Ice.NoObjectFactoryException("object slicing is disabled", _typeId);
                    }

                    //
                    // Slice off what we don't understand.
                    //
                    skipSlice();

                    //
                    // If this is the last slice, keep the object as an opaque
                    // UnknownSlicedData object.
                    //
                    if((_sliceFlags & FLAG_IS_LAST_SLICE) != 0)
                    {
                        v = new Ice.UnknownSlicedObject(mostDerivedId);
                        break;
                    }

                    startSlice(); // Read next Slice header for next iteration.
                }

                //
                // Add the object to the map of un-marshalled objects, this must
                // be done before reading the objects (for circular references).
                //
                _unmarshaledMap.Add(index, v);

                //
                // Read the object.
                //
                v.read__(_stream);

                //
                // Patch all instances now that the object is un-marshalled.
                //
                LinkedList<IPatcher> l;
                if(_patchMap.TryGetValue(index, out l))
                {
                    Debug.Assert(l.Count > 0);

                    //
                    // Patch all pointers that refer to the instance.
                    //
                    foreach(IPatcher p in l)
                    {
                        p.patch(v);
                    }

                    //
                    // Clear out the patch map for that index -- there is nothing left
                    // to patch for that index for the time being.
                    //
                    _patchMap.Remove(index);
                }

                return v;
            }

            private void addPatchEntry(int index, IPatcher patcher)
            {
                Debug.Assert(index > 0);

                //
                // Check if already un-marshalled the object. If that's the case,
                // just patch the object smart pointer and we're done.
                //
                Ice.Object obj;
                if(_unmarshaledMap.TryGetValue(index, out obj))
                {
                    patcher.patch(obj);
                    return;
                }

                //
                // Add patch entry if the object isn't un-marshalled yet, the
                // smart pointer will be patched when the instance is
                // un-marshalled.
                //

                LinkedList<IPatcher> l;
                if(!_patchMap.TryGetValue(index, out l))
                {
                    //
                    // We have no outstanding instances to be patched for this
                    // index, so make a new entry in the patch map.
                    //
                    l = new LinkedList<IPatcher>();
                    _patchMap.Add(index, l);
                }

                //
                // Append a patch entry for this instance.
                //
                l.AddLast(patcher);
            }

            private Ice.SlicedData readSlicedData()
            {
                if(_slices.Count == 0) // No preserved slices.
                {
                    return null;
                }

                //
                // The _indirectionTables member holds the indirection table for each slice
                // in _slices.
                //
                Debug.Assert(_slices.Count == _indirectionTables.Count);

                for(int n = 0; n < _slices.Count; ++n)
                {
                    //
                    // We use the "objects" list in SliceInfo to hold references to the target
                    // objects. Note however that we may not have actually read these objects
                    // yet, so they need to be treated just like we had read the object references
                    // directly (i.e., we add them to the patch list).
                    //
                    // Another important note: the SlicedData object that we return here must
                    // not be destroyed before readPendingObjects is called, otherwise the
                    // patch references will refer to invalid addresses.
                    //
                    int[] table = _indirectionTables[n];
                    Ice.SliceInfo info = _slices[n];
                    info.objects = new Ice.Object[table.Length];
                    for(int j = 0; j < table.Length; ++j)
                    {
                        if(table[j] <= 0)
                        {
                            throw new Ice.MarshalException("invalid id in object indirection table");
                        }
                        IPatcher patcher = new ArrayPatcher<Ice.Object>(Ice.ObjectImpl.ice_staticId(), info.objects, j);
                        addPatchEntry(table[j], patcher);
                    }
                }

                return new Ice.SlicedData(_slices.ToArray());
            }

            private bool skipOpt(int format)
            {
                int sz;
                switch(format)
                {
                case MemberFormatF1:
                {
                    sz = 1;
                    break;
                }
                case MemberFormatF2:
                {
                    sz = 2;
                    break;
                }
                case MemberFormatF4:
                {
                    sz = 4;
                    break;
                }
                case MemberFormatF8:
                {
                    sz = 8;
                    break;
                }
                case MemberFormatVSize:
                {
                    sz = _stream.readSize();
                    break;
                }
                case MemberFormatFSize:
                {
                    sz = _stream.readInt();
                    break;
                }
                default:
                {
                    return false;
                }
                }

                int pos = _stream.pos();
                if(pos + sz > _stream.size())
                {
                    throw new Ice.UnmarshalOutOfBoundsException();
                }
                _stream.pos(pos + sz);
                return true;
            }

            private BasicStream _stream;
            private ReadEncaps _encaps;
            private bool _sliceObjects;

            private int _traceSlicing;
            private String _slicingCat;

            // Object/exception attributes
            private SliceType _sliceType;
            private bool _skipFirstSlice;
            private List<Ice.SliceInfo> _slices;     // Preserved slices.
            private List<int[]> _indirectionTables;

            // Slice attributes
            private byte _sliceFlags;
            private int _sliceSize;
            private string _typeId;

            private sealed class IndirectPatchEntry
            {
                internal int index;
                internal IPatcher patcher;
            }
            private List<IndirectPatchEntry> _indirectPatchList;

            // Encapsulation attributes for object un-marshalling
            private Dictionary<int, LinkedList<IPatcher>> _patchMap;
            private Dictionary<int, Ice.Object> _unmarshaledMap;
            private Dictionary<int, string> _typeIdMap;
            private int _typeIdIndex;
        }

        private sealed class EncapsEncoder
        {
            internal EncapsEncoder(BasicStream stream, WriteEncaps encaps)
            {
                _stream = stream;
                _encaps = encaps;
                _sliceType = SliceType.NoSlice;
                _objectIdIndex = 0;
                _typeIdIndex = 0;
                _indirectionTable = new List<int>();
                _indirectionMap = new Dictionary<int, int>();
                _toBeMarshaledMap = new Dictionary<Ice.Object, int>();
                _marshaledMap = new Dictionary<Ice.Object, int>();
                _typeIdMap = new Dictionary<string, int>();
            }

            internal void writeObject(Ice.Object v)
            {
                if(v != null)
                {
                    //
                    // Register the object.
                    //
                    int index = registerObject(v);

                    if(_encaps.encoding_1_0)
                    {
                        //
                        // Object references are encoded as a negative integer in 1.0.
                        //
                        _stream.writeInt(-index);
                    }
                    else if(_sliceType != SliceType.NoSlice && _encaps.format == Ice.FormatType.SlicedFormat)
                    {
                        //
                        // An object reference that appears inside a slice of an
                        // object or exception encoded as a positive non-zero
                        // index into a per-slice indirection table.
                        //
                        // We use _indirectionMap to keep track of the object
                        // references in the current slice; it maps the object
                        // reference to the position in the indirection list. Note
                        // that the position is offset by one (e.g., the first
                        // position = 1).
                        //
                        int p;
                        if(!_indirectionMap.TryGetValue(index, out p))
                        {
                            _indirectionTable.Add(index);
                            int sz = _indirectionTable.Count; // Position + 1
                            _indirectionMap.Add(index, sz);
                            _stream.writeSize(sz);
                        }
                        else
                        {
                            _stream.writeSize(p);
                        }
                    }
                    else
                    {
                        _stream.writeSize(index);
                    }
                }
                else
                {
                    //
                    // Write nil reference.
                    //
                    if(_encaps.encoding_1_0)
                    {
                        _stream.writeInt(0);
                    }
                    else
                    {
                        _stream.writeSize(0);
                    }
                }
            }

            internal void writeUserException(Ice.UserException v)
            {
                //
                // User exception with the 1.0 encoding start with a boolean
                // flag that indicates whether or not the exception uses
                // classes. 
                //
                // This allows reading the pending objects even if some part of
                // the exception was sliced. With encoding > 1.0, we don't need
                // this, each slice indirect patch table indicates the presence of
                // objects.
                //
                bool usesClasses;
                if(_encaps.encoding_1_0)
                {
                    usesClasses = v.usesClasses__();
                    _stream.writeBool(usesClasses);
                }
                else
                {
                    usesClasses = true; // Always call writePendingObjects
                }
                v.write__(_stream);
                if(usesClasses)
                {
                    writePendingObjects();
                }
            }

            internal void startObject(Ice.SlicedData data)
            {
                _sliceType = SliceType.ObjectSlice;
                _firstSlice = true;
                if(data != null)
                {
                    writeSlicedData(data);
                }
            }

            internal void endObject()
            {
                if(_encaps.encoding_1_0)
                {
                    //
                    // Write the Object slice.
                    //
                    startSlice(Ice.ObjectImpl.ice_staticId(), true);
                    _stream.writeSize(0); // For compatibility with the old AFM.
                    endSlice();
                }
                _sliceType = SliceType.NoSlice;
            }

            internal void startException(Ice.SlicedData data)
            {
                _sliceType = SliceType.ExceptionSlice;
                _firstSlice = true;
                if(data != null)
                {
                    writeSlicedData(data);
                }
            }

            internal void endException()
            {
                _sliceType = SliceType.NoSlice;
            }

            internal void startSlice(string typeId, bool last)
            {
                Debug.Assert(_indirectionTable.Count == 0 && _indirectionMap.Count == 0);
                _sliceFlags = (byte)0;
                _sliceFlagsPos = _stream.pos();

                //
                // Encode the slice size for the old encoding and if using the
                // sliced format.
                //
                if(_encaps.encoding_1_0 || _encaps.format == Ice.FormatType.SlicedFormat)
                {
                    _sliceFlags |= FLAG_HAS_SLICE_SIZE;
                }

                //
                // This is the last slice.
                //
                if(last)
                {
                    _sliceFlags |= FLAG_IS_LAST_SLICE;
                }

                //
                // For object slices, encode the flag and the type ID either as a
                // string or index. For exception slices, don't encode slice flags
                // for the old encoding and always encode the type ID a string.
                //
                if(_sliceType == SliceType.ObjectSlice)
                {
                    _stream.writeByte((byte)0); // Placeholder for the slice flags

                    //
                    // Encode the type ID (only in the first slice for the compact
                    // encoding).
                    //
                    if(_encaps.format == Ice.FormatType.SlicedFormat || _encaps.encoding_1_0 || _firstSlice)
                    {
                        //
                        // If the type ID has already been seen, write the index
                        // of the type ID, otherwise allocate a new type ID and
                        // write the string.
                        //
                        int p;
                        if(_typeIdMap.TryGetValue(typeId, out p))
                        {
                            _sliceFlags |= FLAG_HAS_TYPE_ID_INDEX;
                            _stream.writeSize(p);
                        }
                        else
                        {
                            _sliceFlags |= FLAG_HAS_TYPE_ID_STRING;
                            _typeIdMap.Add(typeId, ++_typeIdIndex);
                            _stream.writeString(typeId);
                        }
                    }
                }
                else
                {
                    if(!_encaps.encoding_1_0)
                    {
                        _stream.writeByte((byte)0); // Placeholder for the slice flags
                    }
                    _stream.writeString(typeId);
                }

                if((_sliceFlags & FLAG_HAS_SLICE_SIZE) != 0)
                {
                    _stream.writeInt(0); // Placeholder for the slice length.
                }

                _writeSlice = _stream.pos();
                _firstSlice = false;
            }

            internal void endSlice()
            {
                //
                // Write the optional member end marker if some optional members
                // were encoded. Note that the optional members are encoded before
                // the indirection table and are included in the slice size.
                //
                if((_sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS) != 0)
                {
                    Debug.Assert(!_encaps.encoding_1_0);
                    _stream.writeByte((byte)Ice.OptionalFormat.EndMarker);
                }

                //
                // Write the slice length if necessary.
                //
                if((_sliceFlags & FLAG_HAS_SLICE_SIZE) != 0)
                {
                    int sz = _stream.pos() - _writeSlice + 4;
                    _stream.rewriteInt(sz, _writeSlice - 4);
                }

                //
                // Only write the indirection table if it contains entries.
                //
                if(_indirectionTable.Count > 0)
                {
                    Debug.Assert(!_encaps.encoding_1_0);
                    Debug.Assert(_encaps.format == Ice.FormatType.SlicedFormat);
                    _sliceFlags |= FLAG_HAS_INDIRECTION_TABLE;

                    //
                    // Write the indirection table as a sequence<size> to conserve space.
                    //
                    _stream.writeSizeSeq(_indirectionTable);

                    _indirectionTable.Clear();
                    _indirectionMap.Clear();
                }

                //
                // Finally, update the slice flags (or the object slice has index
                // type ID bool for the 1.0 encoding)
                //
                if(_encaps.encoding_1_0)
                {
                    if(_sliceType == SliceType.ObjectSlice) // No flags for 1.0 exception slices.
                    {
                        _stream.rewriteBool((_sliceFlags & FLAG_HAS_TYPE_ID_INDEX) != 0, _sliceFlagsPos);
                    }
                }
                else
                {
                    _stream.rewriteByte(_sliceFlags, _sliceFlagsPos);
                }
            }

            internal bool writeOpt(int tag, Ice.OptionalFormat format)
            {
                if(_sliceType == SliceType.NoSlice)
                {
                    return _stream.writeOptImpl(tag, format);
                }
                else
                {
                    if(_stream.writeOptImpl(tag, format))
                    {
                        _sliceFlags |= FLAG_HAS_OPTIONAL_MEMBERS;
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
            }

            internal void writePendingObjects()
            {
                //
                // With the 1.0 encoding, write pending objects if the marshalled
                // data uses classes. Otherwise with encoding > 1.0, only write
                // pending objects if some non-nil references were written.
                //
                if(!_encaps.encoding_1_0)
                {
                    if(_toBeMarshaledMap.Count == 0)
                    {
                        return;
                    }
                    else
                    {
                        //
                        // Write end marker for encapsulation optionals before encoding
                        // the pending objects.
                        //
                        _stream.writeByte((byte)Ice.OptionalFormat.EndMarker);
                    }
                }

                while(_toBeMarshaledMap.Count > 0)
                {
                    //
                    // Consider the to be marshalled objects as marshalled now,
                    // this is necessary to avoid adding again the "to be
                    // marshalled objects" into _toBeMarshaledMap while writing
                    // objects.
                    //
                    foreach(KeyValuePair<Ice.Object, int> e in _toBeMarshaledMap)
                    {
                        _marshaledMap.Add(e.Key, e.Value);
                    }

                    Dictionary<Ice.Object, int> savedMap = _toBeMarshaledMap;
                    _toBeMarshaledMap = new Dictionary<Ice.Object, int>();
                    _stream.writeSize(savedMap.Count);
                    foreach(KeyValuePair<Ice.Object, int> e in savedMap)
                    {
                        //
                        // Ask the instance to marshal itself. Any new class
                        // instances that are triggered by the classes marshaled
                        // are added to toBeMarshaledMap.
                        //
                        if(_encaps.encoding_1_0)
                        {
                            _stream.writeInt(e.Value);
                        }
                        else
                        {
                            _stream.writeSize(e.Value);
                        }

                        try
                        {
                            e.Key.ice_preMarshal();
                        }
                        catch(System.Exception ex)
                        {
                            _stream.instance().initializationData().logger.warning(
                                "exception raised by ice_preMarshal:\n" + ex);
                        }

                        e.Key.write__(_stream);
                    }
                }
                _stream.writeSize(0); // Zero marker indicates end of sequence of sequences of instances.
            }

            private void writeSlicedData(Ice.SlicedData slicedData)
            {
                Debug.Assert(slicedData != null);

                //
                // We only remarshal preserved slices if the target encoding is > 1.0 and we are
                // using the sliced format. Otherwise, we ignore the preserved slices, which
                // essentially "slices" the object into the most-derived type known by the sender.
                //
                if(_encaps.encoding_1_0 || _encaps.format != Ice.FormatType.SlicedFormat)
                {
                    return;
                }

                for(int n = 0; n < slicedData.slices.Length; ++n)
                {
                    Ice.SliceInfo info = slicedData.slices[n];
                    startSlice(info.typeId, info.isLastSlice);

                    //
                    // Write the bytes associated with this slice.
                    //
                    _stream.writeBlob(info.bytes);

                    if(info.hasOptionalMembers)
                    {
                        _sliceFlags |= FLAG_HAS_OPTIONAL_MEMBERS;
                    }

                    //
                    // Assemble and write the indirection table. The table must have the same order
                    // as the list of objects.
                    //
                    for(int j = 0; j < info.objects.Length; ++j)
                    {
                        _indirectionTable.Add(registerObject(info.objects[j]));
                    }

                    endSlice();
                }
            }

            private int registerObject(Ice.Object v)
            {
                //
                // Look for this instance in the to-be-marshaled map.
                //
                int p;
                if(_toBeMarshaledMap.TryGetValue(v, out p))
                {
                    return p;
                }

                //
                // Didn't find it, try the marshaled map next.
                //
                if(_marshaledMap.TryGetValue(v, out p))
                {
                    return p;
                }

                //
                // We haven't seen this instance previously, create a new
                // index, and insert it into the to-be-marshaled map.
                //
                _toBeMarshaledMap.Add(v, ++_objectIdIndex);
                return _objectIdIndex;
            }

            private BasicStream _stream;
            private WriteEncaps _encaps;

            // Object/exception attributes
            private SliceType _sliceType;
            private bool _firstSlice;

            // Slice attributes
            private byte _sliceFlags;
            private int _writeSlice;        // Position of the slice data members
            private int _sliceFlagsPos;     // Position of the slice flags
            private List<int> _indirectionTable;
            private Dictionary<int, int> _indirectionMap;

            // Encapsulation attributes for object marshalling.
            private int _objectIdIndex;
            private Dictionary<Ice.Object, int> _toBeMarshaledMap;
            private Dictionary<Ice.Object, int> _marshaledMap;
            private Dictionary<string, int> _typeIdMap;
            private int _typeIdIndex;
        }

        private sealed class ReadEncaps
        {
            internal void reset()
            {
                decoder = null;
            }

            internal void setEncoding(Ice.EncodingVersion encoding)
            {
                this.encoding = encoding;
                encoding_1_0 = encoding.Equals(Ice.Util.Encoding_1_0);
            }

            internal int start;
            internal int sz;
            internal Ice.EncodingVersion encoding;
            internal bool encoding_1_0;

            internal EncapsDecoder decoder;

            internal ReadEncaps next;
        }

        private sealed class WriteEncaps
        {
            internal void reset()
            {
                encoder = null;
            }

            internal void setEncoding(Ice.EncodingVersion encoding)
            {
                this.encoding = encoding;
                encoding_1_0 = encoding.Equals(Ice.Util.Encoding_1_0);
            }

            internal int start;
            internal Ice.EncodingVersion encoding;
            internal bool encoding_1_0;
            internal Ice.FormatType format = Ice.FormatType.DefaultFormat;

            internal EncapsEncoder encoder;

            internal WriteEncaps next;
        }

        //
        // The encoding version to use when there's no encapsulation to
        // read from or write to. This is for example used to read message
        // headers or when the user is using the streaming API with no
        // encapsulation.
        //
        private Ice.EncodingVersion _encoding;

        private bool isReadEncoding_1_0()
        {
            return _readEncapsStack != null ? _readEncapsStack.encoding_1_0 : _encoding.Equals(Ice.Util.Encoding_1_0);
        }

        private bool isWriteEncoding_1_0()
        {
            return _writeEncapsStack != null ? _writeEncapsStack.encoding_1_0 : _encoding.Equals(Ice.Util.Encoding_1_0);
        }

        private ReadEncaps _readEncapsStack;
        private WriteEncaps _writeEncapsStack;
        private ReadEncaps _readEncapsCache;
        private WriteEncaps _writeEncapsCache;

        private void initReadEncaps()
        {
            if(_readEncapsStack == null) // Lazy initialization
            {
                _readEncapsStack = _readEncapsCache;
                if(_readEncapsStack != null)
                {
                    _readEncapsCache = _readEncapsCache.next;
                }
                else
                {
                    _readEncapsStack = new ReadEncaps();
                }
                _readEncapsStack.setEncoding(_encoding);
                _readEncapsStack.sz = _buf.b.limit();
            }

            if(_readEncapsStack.decoder == null) // Lazy initialization.
            {
                _readEncapsStack.decoder = new EncapsDecoder(this, _readEncapsStack, _sliceObjects);
            }
        }

        private void initWriteEncaps()
        {
            if(_writeEncapsStack == null) // Lazy initialization
            {
                _writeEncapsStack = _writeEncapsCache;
                if(_writeEncapsStack != null)
                {
                    _writeEncapsCache = _writeEncapsCache.next;
                }
                else
                {
                    _writeEncapsStack = new WriteEncaps();
                }
                _writeEncapsStack.setEncoding(_encoding);
            }

            if(_writeEncapsStack.format == Ice.FormatType.DefaultFormat)
            {
                _writeEncapsStack.format = instance_.defaultsAndOverrides().defaultFormat;
            }

            if(_writeEncapsStack.encoder == null) // Lazy initialization.
            {
                _writeEncapsStack.encoder = new EncapsEncoder(this, _writeEncapsStack);
            }
        }

        private bool _sliceObjects;

        private int _messageSizeMax;
        private bool _unlimited;

        private int _startSeq;
        private int _minSeqSize;

        private int _sizePos;

        private const byte FLAG_HAS_TYPE_ID_STRING       = (byte)(1<<0);
        private const byte FLAG_HAS_TYPE_ID_INDEX        = (byte)(1<<1);
        private const byte FLAG_HAS_OPTIONAL_MEMBERS     = (byte)(1<<2);
        private const byte FLAG_HAS_INDIRECTION_TABLE    = (byte)(1<<3);
        private const byte FLAG_HAS_SLICE_SIZE           = (byte)(1<<4);
        private const byte FLAG_IS_LAST_SLICE            = (byte)(1<<5);

        private static bool _bzlibInstalled;

        const int BZ_SEQUENCE_ERROR = -1;
        const int BZ_PARAM_ERROR = -2;
        const int BZ_MEM_ERROR = -3;
        const int BZ_DATA_ERROR = -4;
        const int BZ_DATA_ERROR_MAGIC = -5;
        const int BZ_IO_ERROR = -6;
        const int BZ_UNEXPECTED_EOF = -7;
        const int BZ_OUTBUFF_FULL = -8;
        const int BZ_CONFIG_ERROR = -9;
    }

}
