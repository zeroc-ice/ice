// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Diagnostics;

namespace Ice
{
    public interface ReadObjectCallback
    {
        void invoke(Ice.Object obj);
    }

    public interface InputStream
    {
        Communicator communicator();

        void sliceObjects(bool slice);

        bool readBool();
        bool[] readBoolSeq();

        byte readByte();
        byte[] readByteSeq();

        short readShort();
        short[] readShortSeq();

        int readInt();
        int[] readIntSeq();

        long readLong();
        long[] readLongSeq();

        float readFloat();
        float[] readFloatSeq();

        double readDouble();
        double[] readDoubleSeq();

        string readString();
        string[] readStringSeq();

        int readSize();

        ObjectPrx readProxy();

        void readObject(ReadObjectCallback cb);

        string readTypeId();

        void throwException();

        void startSlice();
        void endSlice();
        void skipSlice();

        void startEncapsulation();
        void endEncapsulation();
        void skipEncapsulation();

        int getEncapsulationSize();
        byte[] readBlob(int size);

        void readPendingObjects();

        void destroy();
    }

    public interface OutputStream
    {
        Communicator communicator();

        void writeBool(bool v);
        void writeBoolSeq(bool[] v);

        void writeByte(byte v);
        void writeByteSeq(byte[] v);

        void writeShort(short v);
        void writeShortSeq(short[] v);

        void writeInt(int v);
        void writeIntSeq(int[] v);

        void writeLong(long v);
        void writeLongSeq(long[] v);

        void writeFloat(float v);
        void writeFloatSeq(float[] v);

        void writeDouble(double v);
        void writeDoubleSeq(double[] v);

        void writeString(string v);
        void writeStringSeq(string[] v);

        void writeSize(int sz);

        void writeProxy(ObjectPrx v);

        void writeObject(Ice.Object v);

        void writeTypeId(string id);

        void writeException(UserException ex);

        void startSlice();
        void endSlice();

        void startEncapsulation();
        void endEncapsulation();

        void writeBlob(byte[] data);

        void writePendingObjects();

        byte[] finished();
        void destroy();
    }

    public abstract class ObjectReader : ObjectImpl
    {
        public abstract void read(InputStream inStream, bool rid);

        public override void write__(IceInternal.BasicStream os)
        {
            Debug.Assert(false);
        }

        public override void read__(IceInternal.BasicStream istr, bool rid)
        {
            InputStream stream = (InputStream)istr.closure();
            read(stream, rid);
        }
    }

    public abstract class ObjectWriter : ObjectImpl
    {
        public abstract void write(OutputStream outStream);

        public override void write__(IceInternal.BasicStream os)
        {
            OutputStream stream = (OutputStream)os.closure();
            write(stream);
        }

        public override void read__(IceInternal.BasicStream istr, bool rid)
        {
            Debug.Assert(false);
        }
    }

    public abstract class UserExceptionWriter : UserException
    {
        public UserExceptionWriter(Communicator communicator)
        {
            communicator_ = communicator;
        }

        public abstract void write(OutputStream os);
        public abstract bool usesClasses();

        public override void write__(IceInternal.BasicStream ostr)
        {
            OutputStream stream = new OutputStreamI(communicator_, ostr);
            write(stream);
        }

        public override void read__(IceInternal.BasicStream istr, bool rid)
        {
            Debug.Assert(false);
        }

        public override void write__(Ice.OutputStream ostr)
        {
            write(ostr);
        }

        public override void read__(Ice.InputStream istr, bool rid)
        {
            Debug.Assert(false);
        }

        public override bool usesClasses__()
        {
            return usesClasses();
        }

        protected Communicator communicator_;
    }
}
