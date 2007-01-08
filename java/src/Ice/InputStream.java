// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public interface InputStream
{
    Communicator communicator();

    void sliceObjects(boolean slice);

    boolean readBool();
    boolean[] readBoolSeq();

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

    String readString();
    String[] readStringSeq();

    int readSize();

    ObjectPrx readProxy();

    void readObject(ReadObjectCallback cb);

    String readTypeId();

    void throwException() throws UserException;

    void startSlice();
    void endSlice();
    void skipSlice();

    void startEncapsulation();
    void endEncapsulation();

    void readPendingObjects();

    void destroy();
}
