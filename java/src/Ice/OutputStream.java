// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Interface for output streams used to write Slice types to a sequence
 * of bytes.
 *
 * @see InputStream
 **/
public interface OutputStream
{
    /**
     * Returns the communicator for this output stream.
     *
     * @return The communicator.
     **/
    Communicator communicator();

    /**
     * Writes a boolean to the stream.
     *
     * @param v The boolean to write to the stream.
     **/
    void writeBool(boolean v);

    /**
     * Writes a sequence of booleans to the stream.
     *
     * @param v The sequence of booleans to write.
     * Passing <code>null</code> causes an empty sequence to be written to the stream.
     **/
    void writeBoolSeq(boolean[] v);

    /**
     * Writes a byte to the stream.
     *
     * @param v The byte to write to the stream.
     **/
    void writeByte(byte v);

    /**
     * Writes a sequence of bytes to the stream.
     *
     * @param v The sequence of bytes to write.
     * Passing <code>null</code> causes an empty sequence to be written to the stream.
     **/
    void writeByteSeq(byte[] v);

    /**
     * Writes a serializable Java object to the stream.
     *
     * @param o The serializable object to write.
     **/
    void writeSerializable(java.io.Serializable o);

    /**
     * Writes a short to the stream.
     *
     * @param v The short to write to the stream.
     **/
    void writeShort(short v);

    /**
     * Writes a sequence of shorts to the stream.
     *
     * @param v The sequence of shorts to write.
     * Passing <code>null</code> causes an empty sequence to be written to the stream.
     **/
    void writeShortSeq(short[] v);

    /**
     * Writes an integer to the stream.
     *
     * @param v The integer to write to the stream.
     **/
    void writeInt(int v);

    /**
     * Writes a sequence of integers to the stream.
     *
     * @param v The sequence of integers to write.
     * Passing <code>null</code> causes an empty sequence to be written to the stream.
     **/
    void writeIntSeq(int[] v);

    /**
     * Writes a long to the stream.
     *
     * @param v The long to write to the stream.
     **/
    void writeLong(long v);

    /**
     * Writes a sequence of longs to the stream.
     *
     * @param v The sequence of longs to write.
     * Passing <code>null</code> causes an empty sequence to be written to the stream.
     **/
    void writeLongSeq(long[] v);

    /**
     * Writes a float to the stream.
     *
     * @param v The float to write to the stream.
     **/
    void writeFloat(float v);

    /**
     * Writes a sequence of floats to the stream.
     *
     * @param v The sequence of floats to write.
     * Passing <code>null</code> causes an empty sequence to be written to the stream.
     **/
    void writeFloatSeq(float[] v);

    /**
     * Writes a double to the stream.
     *
     * @param v The double to write to the stream.
     **/
    void writeDouble(double v);

    /**
     * Writes a sequence of doubles to the stream.
     *
     * @param v The sequence of doubles to write.
     * Passing <code>null</code> causes an empty sequence to be written to the stream.
     **/
    void writeDoubleSeq(double[] v);

    /**
     * Writes a string to the stream.
     *
     * @param v The string to write to the stream. Passing <code>null</code> causes
     * an empty string to be written to the stream.
     **/
    void writeString(String v);

    /**
     * Writes a sequence of strings to the stream.
     *
     * @param v The sequence of strings to write.
     * Passing <code>null</code> causes an empty sequence to be written to the stream.
     **/
    void writeStringSeq(String[] v);

    /**
     * Writes a size to the stream.
     *
     * @param sz The size to write.
     **/
    void writeSize(int sz);

    /**
     * Writes a proxy to the stream.
     *
     * @param v The proxy to write.
     **/
    void writeProxy(ObjectPrx v);

    /**
     * Writes a Slice class to the stream.
     *
     * @param v The class to write. This method writes the index of a Slice class; the state of the class is
     * written once {@link #writePendingObjects} is called.
     **/
    void writeObject(Ice.Object v);

    /**
     * Writes a Slice type ID to the stream.
     *
     * @param id The Slice type ID to write.
     **/
    void writeTypeId(String id);

    /**
     * Writes a user exception to the stream.
     *
     * @param id The user exception to write.
     **/
    void writeException(UserException ex);

    /**
     * Writes the start of a slice to the stream.
     **/
    void startSlice();

    /**
     * Ends the previous slice.
     **/
    void endSlice();

    /**
     * Writes the start of an encapsulation to the stream.
     *
     * @param encoding The encoding version of the encapsulation.
     **/
    void startEncapsulation(Ice.EncodingVersion encoding);

    /**
     * Writes the start of an encapsulation to the stream.
     **/
    void startEncapsulation();

    /**
     * Ends the previous encapsulation.
     **/
    void endEncapsulation();

    /**
     * Writes the state of Slice classes whose index was previously
     * written with {@link #writeObject} to the stream.
     **/
    void writePendingObjects();

    /**
     * Indicates that marshaling of a request or reply is finished.
     *
     * @return The byte sequence containing the encoded request or reply.
     **/
    byte[] finished();

    /**
     * Resets this output stream. This method allows the stream to be reused, to avoid creating
     * unnecessary garbage.
     *
     * @param clearBuffer If <code>true</code>, the stream's internal buffer becomes eligible for
     * garbage collection; if <code>false</code>, the stream's internal buffer is retained, to avoid
     * creating unnecessary garbage. If retained, the internal buffer may be resized to a smaller
     * capacity. Either way, <code>reset</code> resets the stream's writing position to zero.
     **/
    void reset(boolean clearBuffer);

    /**
     * Destroys the stream and its associated resources. The application must call <code>destroy</code> prior
     * to releasing the last reference to a stream; failure to do so may result in resource leaks.
     **/
    void destroy();
}
