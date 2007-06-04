// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System;
    using System.Collections;
    using System.Diagnostics;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Threading;

    public class BasicStream
    {
        [DllImport("libbz2")]
        static extern IntPtr BZ2_bzlibVersion();

        static BasicStream()
        {
            //
            // Simple trick to find out whether libbz2.dll is
            // installed: Call the BZ2_bzlibVersion() function in the
            // library. If we get a DllNotFoundException, the library is
            // not available.
            //
            _bzlibInstalled = true;
            try
            {
                BZ2_bzlibVersion();
            }
            catch(System.DllNotFoundException)
            {
                _bzlibInstalled = false;
            }
            catch(System.EntryPointNotFoundException)
            {
                _bzlibInstalled = false;
            }
        }

        public BasicStream(IceInternal.Instance instance)
        {
            initialize(instance, false);
        }

        public BasicStream(IceInternal.Instance instance, bool unlimited)
        {
            initialize(instance, unlimited);
        }

        private void initialize(IceInternal.Instance instance, bool unlimited)
        {
            instance_ = instance;
            _unlimited = unlimited;
            allocate(1500);
            _capacity = _buf.capacity();
            _limit = 0;
            Debug.Assert(_buf.limit() == _capacity);
            
            _readEncapsStack = null;
            _writeEncapsStack = null;
            _readEncapsCache = null;
            _writeEncapsCache = null;
            
            _traceSlicing = -1;
            
            _sliceObjects = true;
            
            _messageSizeMax = instance_.messageSizeMax(); // Cached for efficiency.

            _seqDataStack = null;
            _objectList = null;
        }

        //
        // This function allows this object to be reused, rather than
        // reallocated.
        //
        public virtual void reset()
        {
            _limit = 0;
            _buf.limit(_capacity);
            _buf.position(0);
            
            if(_readEncapsStack != null)
            {
                Debug.Assert(_readEncapsStack.next == null);
                _readEncapsStack.next = _readEncapsCache;
                _readEncapsCache = _readEncapsStack;
                _readEncapsStack = null;
                _readEncapsCache.reset();
            }

            if(_objectList != null)
            {
                _objectList.Clear();
            }
        }

        public virtual IceInternal.Instance instance()
        {
            return instance_;
        }
        
        public virtual void swap(BasicStream other)
        {
            Debug.Assert(instance_ == other.instance_);
            
            ByteBuffer tmpBuf = other._buf;
            other._buf = _buf;
            _buf = tmpBuf;
            
            int tmpCapacity = other._capacity;
            other._capacity = _capacity;
            _capacity = tmpCapacity;
            
            int tmpLimit = other._limit;
            other._limit = _limit;
            _limit = tmpLimit;
            
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

            SeqData tmpSeqDataStack = other._seqDataStack;
            other._seqDataStack = _seqDataStack;
            _seqDataStack = tmpSeqDataStack;

            ArrayList tmpObjectList = other._objectList;
            other._objectList = _objectList;
            _objectList = tmpObjectList;

            bool tmpUnlimited = other._unlimited;
            other._unlimited = _unlimited;
            _unlimited = tmpUnlimited;
        }
        
        public virtual void resize(int total, bool reading)
        {
            if(!_unlimited && total > _messageSizeMax)
            {
                throw new Ice.MemoryLimitException("Message size > Ice.MessageSizeMax");
            }
            if(total > _capacity)
            {
                int cap2 = _capacity << 1;
                int newCapacity = cap2 > total ? cap2 : total;
                _buf.limit(_limit);
                reallocate(newCapacity);
                _capacity = _buf.capacity();
            }
            //
            // If this stream is used for reading, then we want to set
            // the buffer's limit to the new total size. If this
            // buffer is used for writing, then we must set the
            // buffer's limit to the buffer's capacity.
            //
            if(reading)
            {
                _buf.limit(total);
            }
            else
            {
                _buf.limit(_capacity);
            }
            _buf.position(total);
            _limit = total;
        }
        
        public virtual ByteBuffer prepareRead()
        {
            return _buf;
        }
        
        public virtual ByteBuffer prepareWrite()
        {
            _buf.limit(_limit);
            _buf.position(0);
            return _buf;
        }
        
        //
        // startSeq() and endSeq() sanity-check sequence sizes during
        // unmarshaling and prevent malicious messages with incorrect
        // sequence sizes from causing the receiver to use up all
        // available memory by allocating sequences with an impossibly
        // large number of elements.
        //
        // The code generator inserts calls to startSeq() and endSeq()
        // around the code to unmarshal a sequence. startSeq() is
        // called immediately after reading the sequence size, and
        // endSeq() is called after reading the final element of a
        // sequence.
        //
        // For sequences that contain constructed types that, in turn,
        // contain sequences, the code generator also inserts a call
        // to endElement() after unmarshaling each element.
        //
        // startSeq() is passed the unmarshaled element count, plus
        // the minimum size (in bytes) occupied by the sequence's
        // element type. numElements * minSize is the smallest
        // possible number of bytes that the sequence will occupy on
        // the wire.
        //
        // Every time startSeq() is called, it pushes the element
        // count and the minimum size on a stack. Every time endSeq()
        // is called, it pops the stack.
        //
        // For an ordinary sequence (one that does not (recursively)
        // contain nested sequences), numElements * minSize must be
        // less than the number of bytes remaining in the stream.
        //
        // For a sequence that is nested within some other sequence,
        // there must be enough bytes remaining in the stream for this
        // sequence (numElements + minSize), plus the sum of the bytes
        // required by the remaining elements of all the enclosing
        // sequences.
        //
        // For the enclosing sequences, numElements - 1 is the number
        // of elements for which unmarshaling has not started
        // yet. (The call to endElement() in the generated code
        // decrements that number whenever a sequence element is
        // unmarshaled.)
        //
        // For sequence that variable-length elements, checkSeq() is
        // called whenever an element is unmarshaled. checkSeq() also
        // checks whether the stream has a sufficient number of bytes
        // remaining.  This means that, for messages with bogus
        // sequence sizes, unmarshaling is aborted at the earliest
        // possible point.
        //

        public void startSeq(int numElements, int minSize)
        {
            if(numElements == 0) // Optimization to avoid pushing a useless stack frame.
            {
                return;
            }

            //
            // Push the current sequence details on the stack.
            //
            SeqData sd = new SeqData(numElements, minSize);
            sd.previous = _seqDataStack;
            _seqDataStack = sd;

            int bytesLeft = _buf.remaining();
            if(_seqDataStack.previous == null) // Outermost sequence
            {
                //
                // The sequence must fit within the message.
                //
                if(numElements * minSize > bytesLeft) 
                {
                    throw new Ice.UnmarshalOutOfBoundsException();
                }
            }
            else // Nested sequence
            {
                checkSeq(bytesLeft);
            }
        }

        //
        // Check, given the number of elements requested for this
        // sequence, that this sequence, plus the sum of the sizes of
        // the remaining number of elements of all enclosing
        // sequences, would still fit within the message.
        //
        public void checkSeq()
        {
            checkSeq(_buf.remaining());
        }

        public void checkSeq(int bytesLeft)
        {
            int size = 0;
            SeqData sd = _seqDataStack;
            do
            {
                size += (sd.numElements - 1) * sd.minSize;
                sd = sd.previous;
            }
            while(sd != null);

            if(size > bytesLeft)
            {
                throw new Ice.UnmarshalOutOfBoundsException();
            }
        }

        public void checkFixedSeq(int numElements, int elemSize)
        {
            int bytesLeft = _buf.remaining();
            if(_seqDataStack == null) // Outermost sequence
            {
                //
                // The sequence must fit within the message.
                //
                if(numElements * elemSize > bytesLeft) 
                {
                    throw new Ice.UnmarshalOutOfBoundsException();
                }
            }
            else // Nested sequence
            {
                checkSeq(bytesLeft - numElements * elemSize);
            }
        }

        public void endElement()
        {
            Debug.Assert(_seqDataStack != null);
            --_seqDataStack.numElements;
        }

        public void endSeq(int sz)
        {
            if(sz == 0) // Pop only if something was pushed previously.
            {
                return;
            }

            //
            // Pop the sequence stack.
            //
            SeqData oldSeqData = _seqDataStack;
            Debug.Assert(oldSeqData != null);
            _seqDataStack = oldSeqData.previous;
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
            
            _writeEncapsStack.start = _buf.position();
            writeInt(0); // Placeholder for the encapsulation length.
            writeByte(Protocol.encodingMajor);
            writeByte(Protocol.encodingMinor);
        }
        
        public virtual void endWriteEncaps()
        {
            Debug.Assert(_writeEncapsStack != null);
            int start = _writeEncapsStack.start;
            int sz = _buf.position() - start; // Size includes size and version.
            _buf.putInt(start, sz);
            
            WriteEncaps curr = _writeEncapsStack;
            _writeEncapsStack = curr.next;
            curr.next = _writeEncapsCache;
            _writeEncapsCache = curr;
            _writeEncapsCache.reset();
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
            
            _readEncapsStack.start = _buf.position();
            
            //
            // I don't use readSize() and writeSize() for
            // encapsulations, because when creating an encapsulation,
            // I must know in advance how many bytes the size
            // information will require in the data stream. If I use
            // an Int, it is always 4 bytes. For
            // readSize()/writeSize(), it could be 1 or 5 bytes.
            //
            int sz = readInt();
            if(sz < 0)
            {
                throw new Ice.NegativeSizeException();
            }

            if(sz - 4 > _buf.limit())
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
            _readEncapsStack.encodingMajor = eMajor;
            _readEncapsStack.encodingMinor = eMinor;
        }
        
        public virtual void endReadEncaps()
        {
            Debug.Assert(_readEncapsStack != null);
            int start = _readEncapsStack.start;
            int sz = _readEncapsStack.sz;
            try
            {
                _buf.position(start + sz);
            }
            catch(ArgumentOutOfRangeException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
            
            ReadEncaps curr = _readEncapsStack;
            _readEncapsStack = curr.next;
            curr.next = _readEncapsCache;
            _readEncapsCache = curr;
            _readEncapsCache.reset();
        }
        
        public virtual void checkReadEncaps()
        {
            Debug.Assert(_readEncapsStack != null);
            int start = _readEncapsStack.start;
            int sz = _readEncapsStack.sz;
            if(_buf.position() != start + sz)
            {
                throw new Ice.EncapsulationException();
            }
        }
        
        public virtual int getReadEncapsSize()
        {
            Debug.Assert(_readEncapsStack != null);
            return _readEncapsStack.sz - 6;
        }
        
        public virtual void skipEncaps()
        {
            int sz = readInt();
            if(sz < 0)
            {
                throw new Ice.NegativeSizeException();
            }
            try
            {
                _buf.position(_buf.position() + sz - 4);
            }
            catch(ArgumentOutOfRangeException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }
        
        public virtual void startWriteSlice()
        {
            writeInt(0); // Placeholder for the slice length.
            _writeSlice = _buf.position();
        }
        
        public virtual void endWriteSlice()
        {
            int sz = _buf.position() - _writeSlice + 4;
            _buf.putInt(_writeSlice - 4, sz);
        }
        
        public virtual void startReadSlice()
        {
            int sz = readInt();
            if(sz < 0)
            {
                throw new Ice.NegativeSizeException();
            }
            _readSlice = _buf.position();
        }
        
        public virtual void endReadSlice()
        {
        }
        
        public virtual void skipSlice()
        {
            int sz = readInt();
            if(sz < 0)
            {
                throw new Ice.NegativeSizeException();
            }
            try
            {
                _buf.position(_buf.position() + sz - 4);
            }
            catch(ArgumentOutOfRangeException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }
        
        public virtual void writeSize(int v)
        {
            if(v > 254)
            {
                expand(5);
                _buf.put((byte)255);
                _buf.putInt(v);
            }
            else
            {
                expand(1);
                _buf.put((byte)v);
            }
        }
        
        public virtual int readSize()
        {
            try
            {
                //
                // COMPILERFIX: for some reasons _buf.get() doesn't work here on MacOS X with Mono;
                //
                //byte b = _buf.get();
                byte b = readByte();
                if(b == 255)
                {
                    int v = _buf.getInt();
                    if(v < 0)
                    {
                        throw new Ice.NegativeSizeException();
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
            _buf.put(v);
        }
        
        public virtual void readBlob(byte[] v)
        {
            try
            {
                _buf.get(v);
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }

        public virtual byte[] readBlob(int sz)
        {
            byte[] v = new byte[sz];
            try
            {
                _buf.get(v);
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
            _buf.put(v);
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
                _buf.put(v);
            }
        }
        
        public virtual byte readByte()
        {
            try
            {
                return _buf.get();
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }
        
        public virtual byte[] readByteSeq()
        {
            try
            {
                int sz = readSize();
                checkFixedSeq(sz, 1);
                byte[] v = new byte[sz];
                _buf.get(v);
                return v;
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }
        
        public virtual void writeBool(bool v)
        {
            expand(1);
            _buf.put(v ? (byte)1 : (byte)0);
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
                _buf.putBoolSeq(v);
            }
        }
        
        public virtual bool readBool()
        {
            try
            {
                return _buf.get() == 1;
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
                int sz = readSize();
                checkFixedSeq(sz, 1);
                bool[] v = new bool[sz];
                _buf.getBoolSeq(v);
                return v;
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }
        
        public virtual void writeShort(short v)
        {
            expand(2);
            _buf.putShort(v);
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
                _buf.putShortSeq(v);
            }
        }
        
        public virtual short readShort()
        {
            try
            {
                return _buf.getShort();
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }
        
        public virtual short[] readShortSeq()
        {
            try
            {
                int sz = readSize();
                checkFixedSeq(sz, 2);
                short[] v = new short[sz];
                _buf.getShortSeq(v);
                return v;
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }
        
        public virtual void writeInt(int v)
        {
            expand(4);
            _buf.putInt(v);
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
                _buf.putIntSeq(v);
            }
        }
        
        public virtual int readInt()
        {
            try
            {
                return _buf.getInt();
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }
        
        public virtual int[] readIntSeq()
        {
            try
            {
                int sz = readSize();
                checkFixedSeq(sz, 4);
                int[] v = new int[sz];
                _buf.getIntSeq(v);
                return v;
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }
        
        public virtual void writeLong(long v)
        {
            expand(8);
            _buf.putLong(v);
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
                _buf.putLongSeq(v);
            }
        }
        
        public virtual long readLong()
        {
            try
            {
                return _buf.getLong();
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
                int sz = readSize();
                checkFixedSeq(sz, 8);
                long[] v = new long[sz];
                _buf.getLongSeq(v);
                return v;
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }
        
        public virtual void writeFloat(float v)
        {
            expand(4);
            _buf.putFloat(v);
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
                _buf.putFloatSeq(v);
            }
        }
        
        public virtual float readFloat()
        {
            try
            {
                return _buf.getFloat();
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
                int sz = readSize();
                checkFixedSeq(sz, 4);
                float[] v = new float[sz];
                _buf.getFloatSeq(v);
                return v;
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
            }
        }
        
        public virtual void writeDouble(double v)
        {
            expand(8);
            _buf.putDouble(v);
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
                _buf.putDoubleSeq(v);
            }
        }
        
        public virtual double readDouble()
        {
            try
            {
                return _buf.getDouble();
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
                int sz = readSize();
                checkFixedSeq(sz, 8);
                double[] v = new double[sz];
                _buf.getDoubleSeq(v);
                return v;
            }
            catch(InvalidOperationException ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException(ex);
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
            _buf.put(arr);
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

        public virtual string readString()
        {
            int len = readSize();
            
            if(len == 0)
            {
                return "";
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
                _buf.get(_stringBytes, 0, len);
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
            catch(Exception)
            {
                Debug.Assert(false);
                return "";
            }
        }
        
        public virtual string[] readStringSeq()
        {
            int sz = readSize();
            startSeq(sz, 1);
            string[] v = new string[sz];
            for(int i = 0; i < sz; i++)
            {
                v[i] = readString();
                checkSeq();
                endElement();
            }
            endSeq(sz);
            return v;
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
        
        public virtual void readObject(IceInternal.Patcher patcher)
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
            
            if(index == 0)
            {
                patcher.patch(null);
                return;
            }
            
            if(index < 0 && patcher != null)
            {
                int i = -index;
                IceUtil.LinkedList patchlist = (IceUtil.LinkedList)_readEncapsStack.patchMap[i];
                if(patchlist == null)
                {
                    //
                    // We have no outstanding instances to be patched
                    // for this index, so make a new entry in the
                    // patch map.
                    //
                    patchlist = new IceUtil.LinkedList();
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
            
            string mostDerivedId = readTypeId();
            string id = string.Copy(mostDerivedId);

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
                        throw ex;
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
                    id = readString(); // Read type id for next slice.
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
                        instance_.initializationData().logger.warning("exception raised by ice_postUnmarshal::\n" + ex);
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
            
            IceUtil.LinkedList patchlist;
            Ice.Object v;
            if((object)instanceIndex != null)
            {
                //
                // We have just unmarshaled an instance -- check if
                // something needs patching for that instance.
                //
                patchlist = (IceUtil.LinkedList)_readEncapsStack.patchMap[instanceIndex];
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
                patchlist = (IceUtil.LinkedList)_readEncapsStack.patchMap[patchIndex];
            }
            Debug.Assert(patchlist != null && patchlist.Count > 0);
            Debug.Assert(v != null);
            
            //
            // Patch all references that refer to the instance.
            //
            foreach(IceInternal.Patcher patcher in patchlist)
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

        public static bool compressible()
        {
            return _bzlibInstalled;
        }

        [DllImport("libbz2.dll")]
        extern static int BZ2_bzBuffToBuffCompress(byte[] dest,
                                                   ref int destLen,
                                                   byte[] source,
                                                   int sourceLen,
                                                   int blockSize100k,
                                                   int verbosity,
                                                   int workFactor);

        public bool compress(ref BasicStream cstream, int headerSize, int compressionLevel)
        {
            if(!_bzlibInstalled)
            {
                cstream = this;
                return false;
            }

            //
            // Compress the message body, but not the header.
            //
            int uncompressedLen = size() - headerSize;
            byte[] uncompressed = _buf.rawBytes(headerSize, uncompressedLen);
            int compressedLen = (int)(uncompressedLen * 1.01 + 600);
            byte[] compressed = new byte[compressedLen];

            int rc = BZ2_bzBuffToBuffCompress(compressed, ref compressedLen, uncompressed, uncompressedLen,
                                              compressionLevel, 0, 0);
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
            cstream._buf.put(_buf.rawBytes(0, headerSize));

            //
            // Add the size of the uncompressed stream before the
            // message body.
            //
            cstream.writeInt(size());

            //
            // Add the compressed message body.
            //
            cstream._buf.put(compressed, 0, compressedLen);

            return true;
        }

        [DllImport("libbz2.dll")]
        extern static int BZ2_bzBuffToBuffDecompress(byte[] dest,
                                                     ref int destLen,
                                                     byte[] source,
                                                     int sourceLen,
                                                     int small,
                                                     int verbosity);

        public BasicStream uncompress(int headerSize)
        {
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
            byte[] compressed = _buf.rawBytes(headerSize + 4, compressedLen);
            int uncompressedLen = uncompressedSize - headerSize;
            byte[] uncompressed = new byte[uncompressedLen];
            int rc = BZ2_bzBuffToBuffDecompress(uncompressed, ref uncompressedLen, compressed, compressedLen, 0, 0);
            if(rc < 0)
            {
                Ice.CompressionException ex = new Ice.CompressionException("BZ2_bzBuffToBuffDecompress failed");
                ex.reason = getBZ2Error(rc);
                throw ex;
            }
            BasicStream ucStream = new BasicStream(instance_);
            ucStream.resize(uncompressedSize, false);
            ucStream.pos(0);
            ucStream._buf.put(_buf.rawBytes(0, headerSize));
            ucStream._buf.put(uncompressed, 0, uncompressedLen);
            return ucStream;
        }
        
        internal virtual int pos()
        {
            return _buf.position();
        }
        
        internal virtual void pos(int n)
        {
            _buf.position(n);
        }
        
        public virtual int size()
        {
            return _limit;
        }

        virtual internal bool isEmpty()
        {
            return _limit == 0;
        }
        
        private void expand(int size)
        {
            if(_buf.position() == _limit)
            {
                int oldLimit = _limit;
                _limit += size;
                if(!_unlimited && _limit > _messageSizeMax)
                {
                    throw new Ice.MemoryLimitException("Message larger than Ice.MessageSizeMax");
                }
                if(_limit > _capacity)
                {
                    int cap2 = _capacity << 1;
                    int newCapacity = cap2 > _limit ? cap2 : _limit;
                    _buf.limit(oldLimit);
                    int pos = _buf.position();
                    reallocate(newCapacity);
                    _capacity = _buf.capacity();
                    _buf.limit(_capacity);
                    _buf.position(pos);
                }
            }
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
                Type c = AssemblyUtil.findType(typeToClass(id));
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
                        Type c = AssemblyUtil.findType(typeToClass(id));
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
                        throw new Ice.UnknownUserException(ex);
                    }
                }
            }
            return factory;
        }
        
        private static string typeToClass(string id)
        {
            if(!id.StartsWith("::"))
            {
                throw new Ice.MarshalException("type ID does not start with `::'");
            }
            return id.Substring(2).Replace("::", ".");
        }

        private void allocate(int size)
        {
            ByteBuffer buf = null;
            try
            {
                buf = ByteBuffer.allocate(size);
            }
            catch(System.OutOfMemoryException ex)
            {
                Ice.MarshalException e = new Ice.MarshalException(ex);
                e.reason = "OutOfMemoryException occurred while allocating a ByteBuffer";
                throw e;
            }
            buf.order(ByteBuffer.ByteOrder.LITTLE_ENDIAN);
            _buf = buf;
        }

        private void reallocate(int size)
        {
            //
            // Limit the buffer size to MessageSizeMax
            //
            if(!_unlimited)
            {
                size = size > _messageSizeMax ? _messageSizeMax : size;
            }

            ByteBuffer old = _buf;
            Debug.Assert(old != null);

            allocate(size);
            Debug.Assert(_buf != null);

            old.position(0);
            _buf.put(old);
        }

        private IceInternal.Instance instance_;
        private ByteBuffer _buf;
        private int _capacity; // Cache capacity to avoid excessive method calls.
        private int _limit; // Cache limit to avoid excessive method calls.
        private byte[] _stringBytes; // Reusable array for reading strings.
        
        private sealed class ReadEncaps
        {
            internal int start;
            internal int sz;
            
            internal byte encodingMajor;
            internal byte encodingMinor;
            
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

        private sealed class SeqData
        {
            public SeqData(int numElements, int minSize)
            {
                this.numElements = numElements;
                this.minSize = minSize;
            }

            public int numElements;
            public int minSize;
            public SeqData previous;
        }
        SeqData _seqDataStack;

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
