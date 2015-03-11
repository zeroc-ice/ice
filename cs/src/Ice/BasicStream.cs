// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
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
#if !COMPACT
    using System.Runtime.InteropServices;
    using System.Runtime.Serialization;
    using System.Runtime.Serialization.Formatters.Binary;
#endif
    using System.Threading;

#if !MANAGED && !COMPACT
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
#if MANAGED || COMPACT
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

        public BasicStream(Instance instance)
        {
            initialize(instance, false);
        }

        public BasicStream(Instance instance, bool unlimited)
        {
            initialize(instance, unlimited);
        }

        private void initialize(Instance instance, bool unlimited)
        {
            instance_ = instance;
            _buf = new Buffer(instance_.messageSizeMax());
            _closure = null;
            _unlimited = unlimited;

            _readEncapsStack = null;
            _writeEncapsStack = null;
            _readEncapsCache = null;
            _writeEncapsCache = null;

            _traceSlicing = -1;

            _sliceObjects = true;

            _messageSizeMax = instance_.messageSizeMax(); // Cached for efficiency.

            _startSeq = -1;
            _objectList = null;
        }

        //
        // This function allows this object to be reused, rather than
        // reallocated.
        //
        public virtual void reset()
        {
            _buf.reset();
            clear();
        }

        public virtual void clear()
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

            if(_objectList != null)
            {
                _objectList.Clear();
            }

            _sliceObjects = true;
        }

        public virtual Instance instance()
        {
            return instance_;
        }

        public virtual object closure()
        {
            return _closure;
        }

        public virtual object closure(object p)
        {
            object prev = _closure;
            _closure = p;
            return prev;
        }

        public virtual void swap(BasicStream other)
        {
            Debug.Assert(instance_ == other.instance_);
            
            object tmpClosure = other._closure;
            other._closure = _closure;
            _closure = tmpClosure;

            Buffer tmpBuf = other._buf;
            other._buf = _buf;
            _buf = tmpBuf;

            ReadEncaps tmpRead = other._readEncapsStack;
            other._readEncapsStack = _readEncapsStack;
            _readEncapsStack = tmpRead;

            tmpRead = other._readEncapsCache;
            other._readEncapsCache = _readEncapsCache;
            _readEncapsCache = tmpRead;

            WriteEncaps tmpWrite = other._writeEncapsStack;
            other._writeEncapsStack = _writeEncapsStack;
            _writeEncapsStack = tmpWrite;

            tmpWrite = other._writeEncapsCache;
            other._writeEncapsCache = _writeEncapsCache;
            _writeEncapsCache = tmpWrite;

            int tmpReadSlice = other._readSlice;
            other._readSlice = _readSlice;
            _readSlice = tmpReadSlice;

            int tmpWriteSlice = other._writeSlice;
            other._writeSlice = _writeSlice;
            _writeSlice = tmpWriteSlice;

            int tmpStartSeq = other._startSeq;
            other._startSeq = _startSeq;
            _startSeq = tmpStartSeq;

            int tmpMinSeqSize = other._minSeqSize;
            other._minSeqSize = _minSeqSize;
            _minSeqSize = tmpMinSeqSize;

            ArrayList tmpObjectList = other._objectList;
            other._objectList = _objectList;
            _objectList = tmpObjectList;

            bool tmpUnlimited = other._unlimited;
            other._unlimited = _unlimited;
            _unlimited = tmpUnlimited;
        }

        public virtual void resize(int sz, bool reading)
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

        public virtual Buffer prepareWrite()
        {
            _buf.b.limit(_buf.size());
            _buf.b.position(0);
            return _buf;
        }

        public virtual Buffer getBuffer()
        {
            return _buf;
        }

        public virtual void startWriteEncaps()
        {
            {
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
            }

            _writeEncapsStack.start = _buf.b.position();
            writeInt(0); // Placeholder for the encapsulation length.
            writeByte(Protocol.encodingMajor);
            writeByte(Protocol.encodingMinor);
        }

        public virtual void endWriteEncaps()
        {
            Debug.Assert(_writeEncapsStack != null);
            int start = _writeEncapsStack.start;
            int sz = _buf.size() - start; // Size includes size and version.
            _buf.b.putInt(start, sz);

            WriteEncaps curr = _writeEncapsStack;
            _writeEncapsStack = curr.next;
            curr.next = _writeEncapsCache;
            _writeEncapsCache = curr;
            _writeEncapsCache.reset();
        }

        public virtual void endWriteEncapsChecked()
        {
            if(_writeEncapsStack == null)
            {
                throw new Ice.EncapsulationException("not in an encapsulation");
            }

            endWriteEncaps();
        }

        public virtual void startReadEncaps()
        {
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
            }

            _readEncapsStack.start = _buf.b.position();

            //
            // I don't use readSize() and writeSize() for
            // encapsulations, because when creating an encapsulation,
            // I must know in advance how many bytes the size
            // information will require in the data stream. If I use
            // an Int, it is always 4 bytes. For
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

            byte eMajor = readByte();
            byte eMinor = readByte();
            if(eMajor != Protocol.encodingMajor || eMinor > Protocol.encodingMinor)
            {
                Ice.UnsupportedEncodingException e = new Ice.UnsupportedEncodingException();
                e.badMajor = eMajor < 0 ? eMajor + 256 : eMajor;
                e.badMinor = eMinor < 0 ? eMinor + 256 : eMinor;
                e.major = Protocol.encodingMajor;
                e.minor = Protocol.encodingMinor;
                throw e;
            }
            // _readEncapsStack.encodingMajor = eMajor; // Currently unused
            // _readEncapsStack.encodingMinor = eMinor; // Currently unused
        }

        public virtual void endReadEncaps()
        {
            Debug.Assert(_readEncapsStack != null);
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

        public virtual void skipEmptyEncaps()
        {
            int sz = readInt();
            if(sz != 6)
            {
                throw new Ice.EncapsulationException();
            }

            try
            {
                _buf.b.position(_buf.b.position() + 2);
            }
            catch(ArgumentOutOfRangeException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        public virtual void endReadEncapsChecked()
        {
            if(_readEncapsStack == null)
            {
                throw new Ice.EncapsulationException("not in an encapsulation");
            }

            endReadEncaps();
        }

        public virtual int getReadEncapsSize()
        {
            Debug.Assert(_readEncapsStack != null);
            return _readEncapsStack.sz - 6;
        }

        public virtual void skipEncaps()
        {
            int sz = readInt();
            if(sz < 6)
            {
                throw new Ice.UnmarshalOutOfBoundsException();
            }
            try
            {
                _buf.b.position(_buf.b.position() + sz - 4);
            }
            catch(ArgumentOutOfRangeException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        public virtual void startWriteSlice()
        {
            writeInt(0); // Placeholder for the slice length.
            _writeSlice = _buf.size();
        }

        public virtual void endWriteSlice()
        {
            int sz = _buf.size() - _writeSlice + 4;
            _buf.b.putInt(_writeSlice - 4, sz);
        }

        public virtual void startReadSlice()
        {
            int sz = readInt();
            if(sz < 4)
            {
                throw new Ice.UnmarshalOutOfBoundsException();
            }
            _readSlice = _buf.b.position();
        }

        public virtual void endReadSlice()
        {
        }

        public virtual void skipSlice()
        {
            int sz = readInt();
            if(sz < 4)
            {
                throw new Ice.UnmarshalOutOfBoundsException();
            }
            try
            {
                _buf.b.position(_buf.b.position() + sz - 4);
            }
            catch(ArgumentOutOfRangeException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
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

        public virtual void writeSize(int v)
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

        public virtual int readSize()
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
                    return (int) (b < 0 ? b + 256 : b);
                }
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        public virtual void writeTypeId(string id)
        {
            if(_writeEncapsStack == null || _writeEncapsStack.typeIdMap == null)
            {
                throw new Ice.MarshalException("type ids require an encapsulation");
            }

            object o = _writeEncapsStack.typeIdMap[id];
            if(o != null)
            {
                writeBool(true);
                writeSize((int)o);
            }
            else
            {
                int index = ++_writeEncapsStack.typeIdIndex;
                _writeEncapsStack.typeIdMap[id] = index;
                writeBool(false);
                writeString(id);
            }
        }

        public virtual string readTypeId()
        {
            if(_readEncapsStack == null || _readEncapsStack.typeIdMap == null)
            {
                throw new Ice.MarshalException("type ids require an encapsulation");
            }

            string id;
            int index;
            bool isIndex = readBool();
            if(isIndex)
            {
                index = readSize();
                id = (string)_readEncapsStack.typeIdMap[index];
                if(id == null)
                {
                    throw new Ice.UnmarshalOutOfBoundsException("Missing type ID");
                }
            }
            else
            {
                id = readString();
                index = ++_readEncapsStack.typeIdIndex;
                _readEncapsStack.typeIdMap[index] = id;
            }
            return id;
        }

        public virtual void writeBlob(byte[] v)
        {
            if(v == null)
            {
                return;
            }
            expand(v.Length);
            _buf.b.put(v);
        }

        public virtual void readBlob(byte[] v)
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

        public virtual byte[] readBlob(int sz)
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

        public virtual void writeByte(byte v)
        {
            expand(1);
            _buf.b.put(v);
        }

        public virtual void writeByte(byte v, int end)
        {
            if(v < 0 || v >= end)
            {
                throw new Ice.MarshalException("enumerator out of range");
            }
            writeByte(v);
        }

        public virtual void writeByteSeq(byte[] v)
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

        public virtual void writeByteSeq(int count, IEnumerable<byte> v)
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

        public virtual void writeSerializable(object o)
        {
#if !COMPACT
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

        public virtual byte readByte()
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

        public virtual byte readByte(int end)
        {
            byte v = readByte();
            if(v < 0 || v >= end)
            {
                throw new Ice.MarshalException("enumerator out of range");
            }
            return v;
        }
        
        public virtual byte[] readByteSeq()
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

        public virtual void readByteSeq(out List<byte> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new List<byte>(readByteSeq());
        }

        public virtual void readByteSeq(out LinkedList<byte> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new LinkedList<byte>(readByteSeq());
        }

        public virtual void readByteSeq(out Queue<byte> l)
        {
            //
            // Reading into an array and copy-constructing the
            // queue is faster than constructing the queue
            // and adding to it one element at a time.
            //
            l = new Queue<byte>(readByteSeq());
        }

        public virtual void readByteSeq(out Stack<byte> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            byte[] array = readByteSeq();
            l = new Stack<byte>(array.Length);
            for(int i = array.Length - 1; i >= 0; --i)
            {
                l.Push(array[i]);
            }
        }

        public virtual object readSerializable()
        {
#if !COMPACT
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

        public virtual void writeBool(bool v)
        {
            expand(1);
            _buf.b.put(v ? (byte)1 : (byte)0);
        }

        public virtual void writeBoolSeq(bool[] v)
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

        public virtual void writeBoolSeq(int count, IEnumerable<bool> v)
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

        public virtual bool readBool()
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

        public virtual bool[] readBoolSeq()
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

        public virtual void readBoolSeq(out List<bool> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new List<bool>(readBoolSeq());
        }

        public virtual void readBoolSeq(out LinkedList<bool> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new LinkedList<bool>(readBoolSeq());
        }

        public virtual void readBoolSeq(out Queue<bool> l)
        {
            //
            // Reading into an array and copy-constructing the
            // queue is faster than constructing the queue
            // and adding to it one element at a time.
            //
            l = new Queue<bool>(readBoolSeq());
        }

        public virtual void readBoolSeq(out Stack<bool> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            bool[] array = readBoolSeq();
            l = new Stack<bool>(array.Length);
            for(int i = array.Length - 1; i >= 0; --i)
            {
                l.Push(array[i]);
            }
        }

        public virtual void writeShort(short v)
        {
            expand(2);
            _buf.b.putShort(v);
        }

        public virtual void writeShort(short v, int end)
        {
            if(v < 0 || v >= end)
            {
                throw new Ice.MarshalException("enumerator out of range");
            }
            writeShort(v);
        }

        public virtual void writeShortSeq(short[] v)
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

        public virtual void writeShortSeq(int count, IEnumerable<short> v)
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

        public virtual short readShort()
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

        public virtual short readShort(int end)
        {
            short v = readShort();
            if(v < 0 || v >= end)
            {
                throw new Ice.MarshalException("enumerator out of range");
            }
            return v;
        }

        public virtual short[] readShortSeq()
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

        public virtual void readShortSeq(out List<short> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new List<short>(readShortSeq());
        }

        public virtual void readShortSeq(out LinkedList<short> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new LinkedList<short>(readShortSeq());
        }

        public virtual void readShortSeq(out Queue<short> l)
        {
            //
            // Reading into an array and copy-constructing the
            // queue is faster than constructing the queue
            // and adding to it one element at a time.
            //
            l = new Queue<short>(readShortSeq());
        }

        public virtual void readShortSeq(out Stack<short> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            short[] array = readShortSeq();
            l = new Stack<short>(array.Length);
            for(int i = array.Length - 1; i >= 0; --i)
            {
                l.Push(array[i]);
            }
        }

        public virtual void writeInt(int v)
        {
            expand(4);
            _buf.b.putInt(v);
        }

        public virtual void writeInt(int v, int end)
        {
            if(v < 0 || v >= end)
            {
                throw new Ice.MarshalException("enumerator out of range");
            }
            writeInt(v);
        }

        public virtual void writeIntSeq(int[] v)
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

        public virtual void writeIntSeq(int count, IEnumerable<int> v)
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

        public virtual int readInt()
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

        public virtual int readInt(int end)
        {
            int v = readInt();
            if(v < 0 || v >= end)
            {
                throw new Ice.MarshalException("enumerator out of range");
            }
            return v;
        }

        public virtual int[] readIntSeq()
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

        public virtual void readIntSeq(out List<int> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new List<int>(readIntSeq());
        }

        public virtual void readIntSeq(out LinkedList<int> l)
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

        public virtual void readIntSeq(out Queue<int> l)
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

        public virtual void readIntSeq(out Stack<int> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            int[] array = readIntSeq();
            l = new Stack<int>(array.Length);
            for(int i = array.Length - 1; i >= 0; --i)
            {
                l.Push(array[i]);
            }
        }

        public virtual void writeLong(long v)
        {
            expand(8);
            _buf.b.putLong(v);
        }

        public virtual void writeLongSeq(long[] v)
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

        public virtual void writeLongSeq(int count, IEnumerable<long> v)
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

        public virtual long readLong()
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

        public virtual long[] readLongSeq()
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

        public virtual void readLongSeq(out List<long> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new List<long>(readLongSeq());
        }

        public virtual void readLongSeq(out LinkedList<long> l)
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

        public virtual void readLongSeq(out Queue<long> l)
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

        public virtual void readLongSeq(out Stack<long> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            long[] array = readLongSeq();
            l = new Stack<long>(array.Length);
            for(int i = array.Length - 1; i >= 0; --i)
            {
                l.Push(array[i]);
            }
        }

        public virtual void writeFloat(float v)
        {
            expand(4);
            _buf.b.putFloat(v);
        }

        public virtual void writeFloatSeq(float[] v)
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

        public virtual void writeFloatSeq(int count, IEnumerable<float> v)
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

        public virtual float readFloat()
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

        public virtual float[] readFloatSeq()
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

        public virtual void readFloatSeq(out List<float> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new List<float>(readFloatSeq());
        }

        public virtual void readFloatSeq(out LinkedList<float> l)
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

        public virtual void readFloatSeq(out Queue<float> l)
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

        public virtual void readFloatSeq(out Stack<float> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            float[] array = readFloatSeq();
            l = new Stack<float>(array.Length);
            for(int i = array.Length - 1; i >= 0; --i)
            {
                l.Push(array[i]);
            }
        }

        public virtual void writeDouble(double v)
        {
            expand(8);
            _buf.b.putDouble(v);
        }

        public virtual void writeDoubleSeq(double[] v)
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

        public virtual void writeDoubleSeq(int count, IEnumerable<double> v)
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

        public virtual double readDouble()
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

        public virtual double[] readDoubleSeq()
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

        public virtual void readDoubleSeq(out List<double> l)
        {
            //
            // Reading into an array and copy-constructing the
            // list is faster than constructing the list
            // and adding to it one element at a time.
            //
            l = new List<double>(readDoubleSeq());
        }

        public virtual void readDoubleSeq(out LinkedList<double> l)
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

        public virtual void readDoubleSeq(out Queue<double> l)
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

        public virtual void readDoubleSeq(out Stack<double> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            double[] array = readDoubleSeq();
            l = new Stack<double>(array.Length);
            for(int i = array.Length - 1; i >= 0; --i)
            {
                l.Push(array[i]);
            }
        }

        private static System.Text.UTF8Encoding utf8 = new System.Text.UTF8Encoding(false, true);

        public virtual void writeString(string v)
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

        public virtual void writeStringSeq(string[] v)
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

        public virtual void writeStringSeq(int size, IEnumerable<string> e)
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

        public virtual string readString()
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

        public virtual string[] readStringSeq()
        {
            int sz = readAndCheckSeqSize(1);
            string[] v = new string[sz];
            for(int i = 0; i < sz; i++)
            {
                v[i] = readString();
            }
            return v;
        }

        public virtual void readStringSeq(out List<string> l)
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

        public virtual void readStringSeq(out LinkedList<string> l)
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

        public virtual void readStringSeq(out Queue<string> l)
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

        public virtual void readStringSeq(out Stack<string> l)
        {
            //
            // Reverse the contents by copying into an array first
            // because the stack is marshaled in top-to-bottom order.
            //
            string[] array = readStringSeq();
            l = new Stack<string>(array.Length);
            for(int i = array.Length - 1; i >= 0; --i)
            {
                l.Push(array[i]);
            }
        }

        public virtual void writeProxy(Ice.ObjectPrx v)
        {
            instance_.proxyFactory().proxyToStream(v, this);
        }

        public virtual Ice.ObjectPrx readProxy()
        {
            return instance_.proxyFactory().streamToProxy(this);
        }

        public virtual void writeObject(Ice.Object v)
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
            }

            if(_writeEncapsStack.toBeMarshaledMap == null) // Lazy initialization
            {
                _writeEncapsStack.toBeMarshaledMap = new Hashtable();
                _writeEncapsStack.marshaledMap = new Hashtable();
                _writeEncapsStack.typeIdMap = new Hashtable();
            }
            if(v != null)
            {
                //
                // Look for this instance in the to-be-marshaled map.
                //
                object p = _writeEncapsStack.toBeMarshaledMap[v];
                if(p == null)
                {
                    //
                    // Didn't find it, try the marshaled map next.
                    //
                    object q = _writeEncapsStack.marshaledMap[v];
                    if(q == null)
                    {
                        //
                        // We haven't seen this instance previously,
                        // create a new index, and insert it into the
                        // to-be-marshaled map.
                        //
                        q = ++_writeEncapsStack.writeIndex;
                        _writeEncapsStack.toBeMarshaledMap[v] = q;
                    }
                    p = q;
                }
                writeInt(-((int)p));
            }
            else
            {
                writeInt(0); // Write null reference
            }
        }

        public virtual void readObject(IPatcher patcher)
        {
            Ice.Object v = null;

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
            }

            if(_readEncapsStack.patchMap == null) // Lazy initialization
            {
                _readEncapsStack.patchMap = new Hashtable();
                _readEncapsStack.unmarshaledMap = new Hashtable();
                _readEncapsStack.typeIdMap = new Hashtable();
            }

            int index = readInt();

            if(patcher != null)
            {
                if(index == 0)
                {
                    patcher.patch(null);
                    return;
                }

                if(index < 0)
                {
                    int i = -index;
                    IceUtilInternal.LinkedList patchlist = (IceUtilInternal.LinkedList)_readEncapsStack.patchMap[i];
                    if(patchlist == null)
                    {
                        //
                        // We have no outstanding instances to be patched
                        // for this index, so make a new entry in the
                        // patch map.
                        //
                        patchlist = new IceUtilInternal.LinkedList();
                        _readEncapsStack.patchMap[i] = patchlist;
                    }
                    //
                    // Append a patcher for this instance and see if we
                    // can patch the instance. (The instance may have been
                    // unmarshaled previously.)
                    //
                    patchlist.Add(patcher);
                    patchReferences(null, i);
                    return;
                }
            }
            if(index < 0)
            {
                throw new Ice.MarshalException("Invalid class instance index");
            }

            string mostDerivedId = readTypeId();
            string id = mostDerivedId;

            while(true)
            {
                //
                // If we slice all the way down to Ice::Object, we throw
                // because Ice::Object is abstract.
                //
                if(id == Ice.ObjectImpl.ice_staticId())
                {
                    Ice.NoObjectFactoryException ex
                        = new Ice.NoObjectFactoryException();
                    ex.type = mostDerivedId;
                    throw ex;
                }

                //
                // Try to find a factory registered for the specific
                // type.
                //
                Ice.ObjectFactory userFactory = instance_.servantFactoryManager().find(id);
                if(userFactory != null)
                {
                    v = userFactory.create(id);
                }

                //
                // If that fails, invoke the default factory if one
                // has been registered.
                //
                if(v == null)
                {
                    userFactory = instance_.servantFactoryManager().find("");
                    if(userFactory != null)
                    {
                        v = userFactory.create(id);
                    }
                }

                //
                // Last chance: check whether the class is
                // non-abstract and dynamically instantiate it using
                // reflection.
                //
                if(v == null)
                {
                    userFactory = loadObjectFactory(id);
                    if(userFactory != null)
                    {
                        v = userFactory.create(id);
                    }
                }

                if(v == null)
                {
                    if(_sliceObjects)
                    {
                        //
                        // Performance sensitive, so we use lazy
                        // initialization for tracing.
                        //
                        if(_traceSlicing == -1)
                        {
                            _traceSlicing = instance_.traceLevels().slicing;
                            _slicingCat = instance_.traceLevels().slicingCat;
                        }
                        if(_traceSlicing > 0)
                        {
                            TraceUtil.traceSlicing("class", id, _slicingCat, instance_.initializationData().logger);
                        }
                        skipSlice(); // Slice off this derived part -- we don't understand it.
                        id = readTypeId(); // Read next id for next iteration.
                        continue;
                    }
                    else
                    {
                        Ice.NoObjectFactoryException ex = new Ice.NoObjectFactoryException();
                        ex.type = id;
                        throw ex;
                    }
                }

                int i = index;
                _readEncapsStack.unmarshaledMap[i] = v;

                //
                // Record each object instance so that
                // readPendingObjects can invoke ice_postUnmarshal
                // after all objects have been unmarshaled.
                //
                if(_objectList == null)
                {
                    _objectList = new ArrayList();
                }
                _objectList.Add(v);

                v.read__(this, false);
                patchReferences(i, null);
                return;
            }
        }

        public virtual void writeUserException(Ice.UserException v)
        {
            writeBool(v.usesClasses__());
            v.write__(this);
            if(v.usesClasses__())
            {
                writePendingObjects();
            }
        }

        public virtual void throwException()
        {
            bool usesClasses = readBool();

            string id = readString();
            string origId = id;

            for(;;)
            {
                //
                // Look for a factory for this ID.
                //
                UserExceptionFactory factory = getUserExceptionFactory(id);

                if(factory != null)
                {
                    //
                    // Got factory -- get the factory to instantiate
                    // the exception, initialize the exception
                    // members, and throw the exception.
                    //
                    try
                    {
                        factory.createAndThrow();
                    }
                    catch(Ice.UserException ex)
                    {
                        ex.read__(this, false);
                        if(usesClasses)
                        {
                            readPendingObjects();
                        }
                        throw;
                    }
                }
                else
                {
                    //
                    // Performance sensitive, so we use lazy
                    // initialization for tracing.
                    //
                    if(_traceSlicing == -1)
                    {
                        _traceSlicing = instance_.traceLevels().slicing;
                        _slicingCat = instance_.traceLevels().slicingCat;
                    }
                    if(_traceSlicing > 0)
                    {
                        TraceUtil.traceSlicing("exception", id, _slicingCat, instance_.initializationData().logger);
                    }

                    skipSlice(); // Slice off what we don't understand.

                    try
                    {
                        id = readString(); // Read type id for next slice.
                    }
                    catch(Ice.UnmarshalOutOfBoundsException ex)
                    {
                        //
                        // When readString raises this exception it means we've seen the last slice,
                        // so we set the reason member to a more helpful message.
                        //
                        ex.reason = "unknown exception type `" + origId + "'";
                        throw;
                    }
                }
            }

            //
            // The only way out of the loop above is to find an
            // exception for which the receiver has a factory. If this
            // does not happen, sender and receiver disagree about the
            // Slice definitions they use. In that case, the receiver
            // will eventually fail to read another type ID and throw
            // a MarshalException.
            //
        }

        public virtual void writePendingObjects()
        {
            if(_writeEncapsStack != null && _writeEncapsStack.toBeMarshaledMap != null)
            {
                while(_writeEncapsStack.toBeMarshaledMap.Count > 0)
                {
                    Hashtable savedMap = new Hashtable(_writeEncapsStack.toBeMarshaledMap);
                    writeSize(savedMap.Count);
                    foreach(DictionaryEntry e in savedMap)
                    {
                        //
                        // Add an instance from the old
                        // to-be-marshaled map to the marshaled map
                        // and then ask the instance to marshal
                        // itself. Any new class instances that are
                        // triggered by the classes marshaled are
                        // added to toBeMarshaledMap.
                        //
                        _writeEncapsStack.marshaledMap[e.Key] = e.Value;
                        writeInstance((Ice.Object)e.Key, (int)e.Value);
                    }

                    //
                    // We have marshaled all the instances for this
                    // pass, substract what we have marshaled from the
                    // toBeMarshaledMap.
                    //
                    foreach(DictionaryEntry e in savedMap)
                    {
                        _writeEncapsStack.toBeMarshaledMap.Remove(e.Key);
                    }
                }
            }
            writeSize(0); // Zero marker indicates end of sequence of sequences of instances.
        }

        public virtual void readPendingObjects()
        {
            int num;
            do 
            {
                num = readSize();
                for(int k = num; k > 0; --k)
                {
                    readObject(null);
                }
            }
            while(num > 0);

            if(_readEncapsStack != null && _readEncapsStack.patchMap != null && _readEncapsStack.patchMap.Count != 0)
            {
                //
                // If any entries remain in the patch map, the sender has sent an index for an object, but failed
                // to supply the object.
                //
                throw new Ice.MarshalException("Index for class received, but no instance");
            }

            //
            // Iterate over unmarshaledMap and invoke
            // ice_postUnmarshal on each object.  We must do this
            // after all objects in this encapsulation have been
            // unmarshaled in order to ensure that any object data
            // members have been properly patched.
            //
            if(_objectList != null)
            {
                foreach(Ice.Object obj in _objectList)
                {
                    try
                    {
                        obj.ice_postUnmarshal();
                    }
                    catch(System.Exception ex)
                    {
                        instance_.initializationData().logger.warning("exception raised by ice_postUnmarshal::\n" +
                                                                      ex);
                    }
                }
            }
        }

        public void
        sliceObjects(bool b)
        {
            _sliceObjects = b;
        }

        internal virtual void writeInstance(Ice.Object v, int index)
        {
            writeInt(index);
            try
            {
                v.ice_preMarshal();
            }
            catch(System.Exception ex)
            {
                instance_.initializationData().logger.warning("exception raised by ice_preMarshal::\n" + ex);
            }
            v.write__(this);
        }

        internal virtual void patchReferences(object instanceIndex, object patchIndex)
        {
            //
            // Called whenever we have unmarshaled a new instance or
            // an index.  The instanceIndex is the index of the
            // instance just unmarshaled and patchIndex is the index
            // just unmarshaled. (Exactly one of the two parameters
            // must be null.) Patch any pointers in the patch map with
            // the new address.
            //
            Debug.Assert(   ((object)instanceIndex != null && (object)patchIndex == null)
                         || ((object)instanceIndex == null && (object)patchIndex != null));

            IceUtilInternal.LinkedList patchlist;
            Ice.Object v;
            if((object)instanceIndex != null)
            {
                //
                // We have just unmarshaled an instance -- check if
                // something needs patching for that instance.
                //
                patchlist = (IceUtilInternal.LinkedList)_readEncapsStack.patchMap[instanceIndex];
                if(patchlist == null)
                {
                    return; // We don't have anything to patch for the instance just unmarshaled.
                }
                v = (Ice.Object)_readEncapsStack.unmarshaledMap[instanceIndex];
                patchIndex = instanceIndex;
            }
            else
            {
                //
                // We have just unmarshaled an index -- check if we
                // have unmarshaled the instance for that index yet.
                //
                v = (Ice.Object)_readEncapsStack.unmarshaledMap[patchIndex];
                if(v == null)
                {
                    return; // We haven't unmarshaled the instance for this index yet.
                }
                patchlist = (IceUtilInternal.LinkedList)_readEncapsStack.patchMap[patchIndex];
            }
            Debug.Assert(patchlist != null && patchlist.Count > 0);
            Debug.Assert(v != null);

            //
            // Patch all references that refer to the instance.
            //
            foreach(IPatcher patcher in patchlist)
            {
                try
                {
                    patcher.patch(v);
                }
                catch(InvalidCastException ex)
                {
                    //
                    // TODO: Fix this (also for C++ and Java):
                    // NoObjectFactoryException is misleading because
                    // the type sent by the sender is incompatible
                    // with what is expected. This really should be a
                    // MarshalException.
                    //
                    Ice.NoObjectFactoryException nof = new Ice.NoObjectFactoryException(ex);
                    nof.type = patcher.type();
                    throw nof;
                }
            }

            //
            // Clear out the patch map for that index -- there is
            // nothing left to patch for that index for the time
            // being.
            //
            _readEncapsStack.patchMap.Remove(patchIndex);
        }

#if !MANAGED && !COMPACT
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
#if MANAGED || COMPACT
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

            cstream = new BasicStream(instance_);
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
#if MANAGED || COMPACT
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
            BasicStream ucStream = new BasicStream(instance_);
            ucStream.resize(uncompressedSize, false);
            ucStream.pos(0);
            ucStream._buf.b.put(_buf.b.rawBytes(0, headerSize));
            ucStream._buf.b.put(uncompressed, 0, uncompressedLen);
            return ucStream;
#endif
        }

        internal virtual int pos()
        {
            return _buf.b.position();
        }

        internal virtual void pos(int n)
        {
            _buf.b.position(n);
        }

        public virtual int size()
        {
            return _buf.size();
        }

        public virtual bool isEmpty()
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

        private sealed class DynamicObjectFactory : Ice.ObjectFactory
        {
            internal DynamicObjectFactory(Type c)
            {
                _class = c;
            }

            public Ice.Object create(string type)
            {
                try
                {
                    return (Ice.Object)AssemblyUtil.createInstance(_class);
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

        private Ice.ObjectFactory loadObjectFactory(string id)
        {
            Ice.ObjectFactory factory = null;

            try
            {
                Type c = AssemblyUtil.findType(instance_, typeToClass(id));
                if(c == null)
                {
                    return null;
                }
                //
                // Ensure the class is instantiable.
                //
                if(!c.IsAbstract && !c.IsInterface)
                {
                    Ice.ObjectFactory dynamicFactory = new DynamicObjectFactory(c);
                    //
                    // We will try to install the dynamic factory, but
                    // another thread may install a factory first.
                    //
                    while(factory == null)
                    {
                        try
                        {
                            instance_.servantFactoryManager().add(dynamicFactory, id);
                            factory = dynamicFactory;
                        }
                        catch(Ice.AlreadyRegisteredException)
                        {
                            //
                            // Another thread already installed the
                            // factory, so try to obtain it. It's
                            // possible (but unlikely) that the
                            // factory will have already been removed,
                            // in which case the return value will be
                            // null and the while loop will attempt to
                            // install the dynamic factory again.
                            //
                            factory = instance_.servantFactoryManager().find(id);
                        }
                    }
                }
            }
            catch(Exception ex)
            {
                Ice.NoObjectFactoryException e = new Ice.NoObjectFactoryException(ex);
                e.type = id;
                throw e;
            }

            return factory;
        }

        private sealed class DynamicUserExceptionFactory : UserExceptionFactory
        {
            internal DynamicUserExceptionFactory(Type c)
            {
                _class = c;
            }

            public void createAndThrow()
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

        private UserExceptionFactory getUserExceptionFactory(string id)
        {
            UserExceptionFactory factory = null;

            lock(_exceptionFactories)
            {
                factory = (UserExceptionFactory)_exceptionFactories[id];
                if(factory == null)
                {
                    try
                    {
                        Type c = AssemblyUtil.findType(instance_, typeToClass(id));
                        if(c == null)
                        {
                            return null;
                        }
                        //
                        // Ensure the class is instantiable.
                        //
                        Debug.Assert(!c.IsAbstract && !c.IsInterface);
                        factory = new DynamicUserExceptionFactory(c);
                        _exceptionFactories[id] = factory;
                    }
                    catch(Exception ex)
                    {
                        throw new Ice.UnknownUserException(id.Substring(2), ex);
                    }
                }
            }
            return factory;
        }

        private static string typeToClass(string id)
        {
            if(!id.StartsWith("::", StringComparison.Ordinal))
            {
                throw new Ice.MarshalException("expected type id but received `" + id + "'");
            }
            return id.Substring(2).Replace("::", ".");
        }

        private Instance instance_;
        private Buffer _buf;
        private object _closure;
        private byte[] _stringBytes; // Reusable array for reading strings.

        private sealed class ReadEncaps
        {
            internal int start;
            internal int sz;

            // internal byte encodingMajor; // Currently unused
            // internal byte encodingMinor; // Currently unused

            internal Hashtable patchMap;
            internal Hashtable unmarshaledMap;
            internal int typeIdIndex;
            internal Hashtable typeIdMap;
            internal ReadEncaps next;

            internal void reset()
            {
                if(patchMap != null)
                {
                    patchMap.Clear();
                    unmarshaledMap.Clear();
                    typeIdIndex = 0;
                    typeIdMap.Clear();
                }
            }
        }

        private sealed class WriteEncaps
        {
            internal int start;

            internal int writeIndex;
            internal Hashtable toBeMarshaledMap;
            internal Hashtable marshaledMap;
            internal int typeIdIndex;
            internal Hashtable typeIdMap;
            internal WriteEncaps next;

            internal void reset()
            {
                if(toBeMarshaledMap != null)
                {
                    writeIndex = 0;
                    toBeMarshaledMap.Clear();
                    marshaledMap.Clear();
                    typeIdIndex = 0;
                    typeIdMap.Clear();
                }
            }
        }

        private ReadEncaps _readEncapsStack;
        private WriteEncaps _writeEncapsStack;
        private ReadEncaps _readEncapsCache;
        private WriteEncaps _writeEncapsCache;

        private int _readSlice;
        private int _writeSlice;

        private int _traceSlicing;
        private string _slicingCat;

        private bool _sliceObjects;

        private int _messageSizeMax;
        private bool _unlimited;

        int _startSeq;
        int _minSeqSize;

        private ArrayList _objectList;

        private static Hashtable _exceptionFactories = new Hashtable(); // <type name, factory> pairs.

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
