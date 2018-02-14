// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
     * Writes a ByteBuffer to the stream.
     *
     * @param v The ByteBuffer to write.
     * Passing <code>null</code> causes an empty sequence to be written to the stream.
     **/
    void writeByteBuffer(java.nio.ByteBuffer v);

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
     * Writes a ShortBuffer to the stream.
     *
     * @param v The ShortBuffer to write.
     * Passing <code>null</code> causes an empty sequence to be written to the stream.
     **/
    void writeShortBuffer(java.nio.ShortBuffer v);

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
     * Writes a IntBuffer to the stream.
     *
     * @param v The IntBuffer to write.
     * Passing <code>null</code> causes an empty sequence to be written to the stream.
     **/
    void writeIntBuffer(java.nio.IntBuffer v);

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
     * Writes a LongBuffer to the stream.
     *
     * @param v The LongBuffer to write.
     * Passing <code>null</code> causes an empty sequence to be written to the stream.
     **/
    void writeLongBuffer(java.nio.LongBuffer v);

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
     * Writes a FloatBuffer to the stream.
     *
     * @param v The FloatBuffer to write.
     * Passing <code>null</code> causes an empty sequence to be written to the stream.
     **/
    void writeFloatBuffer(java.nio.FloatBuffer v);

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
     * Writes a DoubleBuffer to the stream.
     *
     * @param v The DoubleBuffer to write.
     * Passing <code>null</code> causes an empty sequence to be written to the stream.
     **/
    void writeDoubleBuffer(java.nio.DoubleBuffer v);

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
     * Write an enumerated value.
     *
     * @param v The enumerator.
     * @param maxValue The maximum enumerator value in the definition.
     **/
    void writeEnum(int v, int maxValue);

    /**
     * Writes a user exception to the stream.
     *
     * @param ex The user exception to write.
     **/
    void writeException(UserException ex);

    /**
     * Marks the start of an Ice object.
     *
     * @param slicedData Preserved slices for this object, or null.
     **/
    void startObject(SlicedData slicedData);

    /**
     * Marks the end of an Ice object.
     **/
    void endObject();

    /**
     * Marks the start of a user exception.
     *
     * @param slicedData Preserved slices for this exception, or null.
     **/
    void startException(SlicedData slicedData);

    /**
     * Marks the end of a user exception.
     **/
    void endException();

    /**
     * Marks the start of a new slice for an Ice object or user exception.
     *
     * @param typeId The Slice type ID corresponding to this slice.

     * @param compactId The Slice compact type ID corresponding to
     *        this slice or -1 if no compact ID is defined for the
     *        type ID.

     * @param last True if this is the last slice, false otherwise.
     **/
    void startSlice(String typeId, int compactId, boolean last);

    /**
     * Marks the end of a slice for an Ice object or user exception.
     **/
    void endSlice();

    /**
     * Writes the start of an encapsulation to the stream.
     *
     * @param encoding The encoding version of the encapsulation.
     *
     * @param format Specify the compact or sliced format.
     *
     **/
    void startEncapsulation(Ice.EncodingVersion encoding, Ice.FormatType format);

    /**
     * Writes the start of an encapsulation to the stream.
     **/
    void startEncapsulation();

    /**
     * Ends the previous encapsulation.
     **/
    void endEncapsulation();

    /**
     * Determines the current encoding version.
     *
     * @return The encoding version.
     **/
    EncodingVersion getEncoding();

    /**
     * Writes the state of Slice classes whose index was previously
     * written with {@link #writeObject} to the stream.
     **/
    void writePendingObjects();

    /**
     * Write the header information for an optional value.
     *
     * @param tag The numeric tag associated with the value.
     * @param format The optional format of the value.
     **/
    boolean writeOptional(int tag, Ice.OptionalFormat format);

    /**
     * Determines the current position in the stream.
     *
     * @return The current position.
     **/
    int pos();

    /**
     * Inserts a fixed 32-bit size value into the stream at the given position.
     *
     * @param sz The 32-bit size value.
     * @param pos The position at which to write the value.
     **/
    void rewrite(int sz, int pos);

    /**
     * Returns the current position and allocates four bytes for a fixed-length (32-bit)
     * size value.
     *
     * @return The current position.
     **/
    int startSize();

    /**
     * Computes the amount of data written since the previous call to startSize and
     * writes that value at the saved position.
     *
     * @param pos The saved position.
     **/
    void endSize(int pos);

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
