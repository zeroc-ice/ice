// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    public class OutputStreamI : OutputStream
    {
        public OutputStreamI(Communicator communicator)
        {
            _communicator = communicator;
            _os = new IceInternal.BasicOutputStream(Util.getInstance(communicator), this);
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
            _os.endWriteEncaps();
        }

        public void writePendingObjects()
        {
            _os.writePendingObjects();
        }

        public byte[] finished()
        {
            IceInternal.ByteBuffer buf = _os.prepareWrite();
            byte[] result = new byte[buf.limit()];
            buf.get(result);

            return result;
        }

        public void destroy()
        {
            if(_os != null)
            {
                _os = null;
            }
        }

        private Communicator _communicator;
        private IceInternal.BasicOutputStream _os;
    }
}
