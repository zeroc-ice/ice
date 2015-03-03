// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    public class InputStreamI : InputStream
    {
        public InputStreamI(Communicator communicator, byte[] data)
        {
            _communicator = communicator;

            _is = new IceInternal.BasicStream(IceInternal.Util.getInstance(communicator));
            _is.closure(this);
            _is.resize(data.Length, true);
            IceInternal.Buffer buf = _is.getBuffer();
            buf.b.position(0);
            buf.b.put(data);
            buf.b.position(0);
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

        private class Patcher<T> : IceInternal.Patcher<T>
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

        public string readTypeId()
        {
            return _is.readTypeId();
        }

        public void throwException()
        {
            _is.throwException();
        }

        public void startSlice()
        {
            _is.startReadSlice();
        }

        public void endSlice()
        {
            _is.endReadSlice();
        }

        public void skipSlice()
        {
            _is.skipSlice();
        }

        public void startEncapsulation()
        {
            _is.startReadEncaps();
        }

        public void endEncapsulation()
        {
            _is.endReadEncapsChecked();
        }

        public void skipEncapsulation()
        {
            _is.skipEncaps();
        }

        public int getEncapsulationSize()
        {
            return _is.getReadEncapsSize();
        }

        public byte[] readBlob(int sz)
        {
            return _is.readBlob(sz);
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
        public OutputStreamI(Communicator communicator) :
            this(communicator, new IceInternal.BasicStream(IceInternal.Util.getInstance(communicator)))
        {
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

        public void writeTypeId(string id)
        {
            _os.writeTypeId(id);
        }

        public void writeException(UserException v)
        {
            _os.writeUserException(v);
        }

        public void startSlice()
        {
            _os.startWriteSlice();
        }

        public void endSlice()
        {
            _os.endWriteSlice();
        }

        public void startEncapsulation()
        {
            _os.startWriteEncaps();
        }

        public void endEncapsulation()
        {
            _os.endWriteEncapsChecked();
        }

        public void writeBlob(byte[] data)
        {
            _os.writeBlob(data);
        }

        public void writePendingObjects()
        {
            _os.writePendingObjects();
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
