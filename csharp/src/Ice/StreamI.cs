// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    public class InputStreamI : InputStream
    {
        public InputStreamI(Communicator communicator, byte[] data, bool copyData)
        {
            _communicator = communicator;
            IceInternal.Instance instance = IceInternal.Util.getInstance(communicator);
            initialize(instance, data, instance.defaultsAndOverrides().defaultEncoding, copyData);
        }

        public InputStreamI(Communicator communicator, byte[] data, EncodingVersion v, bool copyData)
        {
            _communicator = communicator;
            initialize(IceInternal.Util.getInstance(communicator), data, v, copyData);
        }

        private void initialize(IceInternal.Instance instance, byte[] data, EncodingVersion v, bool copyData)
        {
            if(copyData)
            {
                _is = new IceInternal.BasicStream(instance, v);
                _is.resize(data.Length, true);
                IceInternal.Buffer buf = _is.getBuffer();
                buf.b.position(0);
                buf.b.put(data);
                buf.b.position(0);
            }
            else
            {
                _is = new IceInternal.BasicStream(instance, v, data);
            }
            _is.closure(this);
        }

        public Communicator communicator()
        {
            return _communicator;
        }

        public void sliceObjects(bool slice)
        {
            _is.sliceObjects(slice);
        }

        public bool readBool()
        {
            return _is.readBool();
        }

        public bool[] readBoolSeq()
        {
            return _is.readBoolSeq();
        }

        public byte readByte()
        {
            return _is.readByte();
        }

        public byte[] readByteSeq()
        {
            return _is.readByteSeq();
        }

        public object readSerializable()
        {
            return _is.readSerializable();
        }

        public short readShort()
        {
            return _is.readShort();
        }

        public short[] readShortSeq()
        {
            return _is.readShortSeq();
        }

        public int readInt()
        {
            return _is.readInt();
        }

        public int[] readIntSeq()
        {
            return _is.readIntSeq();
        }

        public long readLong()
        {
            return _is.readLong();
        }

        public long[] readLongSeq()
        {
            return _is.readLongSeq();
        }

        public float readFloat()
        {
            return _is.readFloat();
        }

        public float[] readFloatSeq()
        {
            return _is.readFloatSeq();
        }

        public double readDouble()
        {
            return _is.readDouble();
        }

        public double[] readDoubleSeq()
        {
            return _is.readDoubleSeq();
        }

        public string readString()
        {
            return _is.readString();
        }

        public string[] readStringSeq()
        {
            return _is.readStringSeq();
        }

        public int readSize()
        {
            return _is.readSize();
        }

        public int readAndCheckSeqSize(int minSize)
        {
            return _is.readAndCheckSeqSize(minSize);
        }

        public ObjectPrx readProxy()
        {
            return _is.readProxy();
        }

        private class Patcher<T> : IceInternal.Patcher
        {
            public Patcher(ReadObjectCallback cb) : base("unknown")
            {
                _cb = cb;
            }

            public override void patch(Ice.Object v)
            {
                _cb.invoke(v);
            }

            ReadObjectCallback _cb;
        }

        public void readObject(ReadObjectCallback cb)
        {
            _is.readObject(new Patcher<Ice.Object>(cb));
        }

        public int readEnum(int maxValue)
        {
            return _is.readEnum(maxValue);
        }

        public void throwException()
        {
            _is.throwException(null);
        }

        internal class UserExceptionFactoryI : IceInternal.UserExceptionFactory
        {
            internal UserExceptionFactoryI(UserExceptionReaderFactory factory)
            {
                _factory = factory;
            }

            public void createAndThrow(string id)
            {
                _factory.createAndThrow(id);
            }

            public void destroy()
            {
            }

            private UserExceptionReaderFactory _factory;
        }

        public void throwException(UserExceptionReaderFactory factory)
        {
            _is.throwException(new UserExceptionFactoryI(factory));
        }

        public void startObject()
        {
            _is.startReadObject();
        }

        public SlicedData endObject(bool preserve)
        {
            return _is.endReadObject(preserve);
        }

        public void startException()
        {
            _is.startReadException();
        }

        public SlicedData endException(bool preserve)
        {
            return _is.endReadException(preserve);
        }

        public string startSlice()
        {
            return _is.startReadSlice();
        }

        public void endSlice()
        {
            _is.endReadSlice();
        }

        public void skipSlice()
        {
            _is.skipSlice();
        }

        public EncodingVersion startEncapsulation()
        {
            return _is.startReadEncaps();
        }

        public void endEncapsulation()
        {
            _is.endReadEncapsChecked();
        }

        public EncodingVersion skipEncapsulation()
        {
            return _is.skipEncaps();
        }

        public EncodingVersion getEncoding()
        {
            return _is.getReadEncoding();
        }

        public void readPendingObjects()
        {
            _is.readPendingObjects();
        }

        public void rewind()
        {
            _is.clear();
            _is.getBuffer().b.position(0);
        }

        public void skip(int sz)
        {
            _is.skip(sz);
        }

        public void skipSize()
        {
            _is.skipSize();
        }

        public bool readOptional(int tag, OptionalFormat format)
        {
            return _is.readOpt(tag, format);
        }

        public int pos()
        {
            return _is.pos();
        }

        public void destroy()
        {
            if(_is != null)
            {
                _is = null;
            }
        }

        private Communicator _communicator;
        private IceInternal.BasicStream _is;
    }

    public class OutputStreamI : OutputStream
    {
        public OutputStreamI(Communicator communicator)
        {
            _communicator = communicator;

            IceInternal.Instance instance = IceInternal.Util.getInstance(communicator);
            _os = new IceInternal.BasicStream(instance, instance.defaultsAndOverrides().defaultEncoding);
            _os.closure(this);
        }

        public OutputStreamI(Communicator communicator, EncodingVersion v)
        {
            _communicator = communicator;

            IceInternal.Instance instance = IceInternal.Util.getInstance(communicator);
            _os = new IceInternal.BasicStream(instance, v);
            _os.closure(this);
        }

        public OutputStreamI(Communicator communicator, IceInternal.BasicStream os)
        {
            _communicator = communicator;
            _os = os;
            _os.closure(this);
        }

        public Communicator communicator()
        {
            return _communicator;
        }

        public void writeBool(bool v)
        {
            _os.writeBool(v);
        }

        public void writeBoolSeq(bool[] v)
        {
            _os.writeBoolSeq(v);
        }

        public void writeByte(byte v)
        {
            _os.writeByte(v);
        }

        public void writeByteSeq(byte[] v)
        {
            _os.writeByteSeq(v);
        }

        public void writeSerializable(object v)
        {
            _os.writeSerializable(v);
        }

        public void writeShort(short v)
        {
            _os.writeShort(v);
        }

        public void writeShortSeq(short[] v)
        {
            _os.writeShortSeq(v);
        }

        public void writeInt(int v)
        {
            _os.writeInt(v);
        }

        public void writeIntSeq(int[] v)
        {
            _os.writeIntSeq(v);
        }

        public void writeLong(long v)
        {
            _os.writeLong(v);
        }

        public void writeLongSeq(long[] v)
        {
            _os.writeLongSeq(v);
        }

        public void writeFloat(float v)
        {
            _os.writeFloat(v);
        }

        public void writeFloatSeq(float[] v)
        {
            _os.writeFloatSeq(v);
        }

        public void writeDouble(double v)
        {
            _os.writeDouble(v);
        }

        public void writeDoubleSeq(double[] v)
        {
            _os.writeDoubleSeq(v);
        }

        public void writeString(string v)
        {
            _os.writeString(v);
        }

        public void writeStringSeq(string[] v)
        {
            _os.writeStringSeq(v);
        }

        public void writeSize(int sz)
        {
            if(sz < 0)
            {
                throw new MarshalException();
            }

            _os.writeSize(sz);
        }

        public void writeProxy(ObjectPrx v)
        {
            _os.writeProxy(v);
        }

        public void writeObject(Ice.Object v)
        {
            _os.writeObject(v);
        }

        public void writeEnum(int v, int maxValue)
        {
            _os.writeEnum(v, maxValue);
        }

        public void writeException(UserException v)
        {
            _os.writeUserException(v);
        }

        public void startObject(SlicedData slicedData)
        {
            _os.startWriteObject(slicedData);
        }

        public void endObject()
        {
            _os.endWriteObject();
        }

        public void startException(SlicedData slicedData)
        {
            _os.startWriteException(slicedData);
        }

        public void endException()
        {
            _os.endWriteException();
        }

        public void startSlice(string typeId, int compactId, bool last)
        {
            _os.startWriteSlice(typeId, compactId, last);
        }

        public void endSlice()
        {
            _os.endWriteSlice();
        }

        public void startEncapsulation(EncodingVersion encoding, FormatType format)
        {
            _os.startWriteEncaps(encoding, format);
        }

        public void startEncapsulation()
        {
            _os.startWriteEncaps();
        }

        public void endEncapsulation()
        {
            _os.endWriteEncapsChecked();
        }

        public EncodingVersion getEncoding()
        {
            return _os.getWriteEncoding();
        }

        public void writePendingObjects()
        {
            _os.writePendingObjects();
        }

        public bool writeOptional(int tag, OptionalFormat format)
        {
            return _os.writeOpt(tag, format);
        }

        public int pos()
        {
            return _os.pos();
        }

        public void rewrite(int sz, int pos)
        {
            _os.rewriteInt(sz, pos);
        }

        public int startSize()
        {
            return _os.startSize();
        }

        public void endSize(int pos)
        {
            _os.endSize(pos);
        }

        public byte[] finished()
        {
            IceInternal.Buffer buf = _os.prepareWrite();
            byte[] result = new byte[buf.b.limit()];
            buf.b.get(result);

            return result;
        }

        public void reset(bool clearBuffer)
        {
            _os.clear();

            IceInternal.Buffer buf = _os.getBuffer();
            if(clearBuffer)
            {
                buf.clear();
            }
            else
            {
                buf.reset();
            }
            buf.b.position(0);
        }

        public void destroy()
        {
            if(_os != null)
            {
                _os = null;
            }
        }

        private Communicator _communicator;
        private IceInternal.BasicStream _os;
    }
}
