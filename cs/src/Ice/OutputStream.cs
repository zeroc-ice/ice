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

        void writePendingObjects();

        byte[] finished();
        void destroy();
    }
}
